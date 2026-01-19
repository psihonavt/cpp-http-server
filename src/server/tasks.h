#pragma once

#include "server/config.h"
#include "server/context.h"
#include "server/signals.h"
#include "utils/helpers.h"
#include "utils/logging.h"
#include <algorithm>
#include <cassert>
#include <cerrno>
#include <chrono>
#include <csignal>
#include <cstdio>
#include <cstring>
#include <format>
#include <functional>
#include <optional>
#include <string>
#include <sys/fcntl.h>
#include <sys/signal.h>
#include <sys/wait.h>
#include <unistd.h>
#include <unordered_map>
#include <utility>
#include <vector>

namespace Server::Tasks {

enum class TaskState {
    RUNNING,
    ERROR,
    FINISHED,
    TIMEOUT,
};

struct TaskResult {
    pid_t pid;
    bool is_successful;
    std::string stdout_content;
    std::string stderr_content;
};

using on_task_finished_cb_t = std::function<void(TaskResult result)>;
using task_ttl_t = std::chrono::milliseconds;
constexpr std::chrono::seconds TASK_DEFAULT_TTL = std::chrono::seconds(60);

class Task {
private:
    pid_t m_pid;
    int m_stdout;
    int m_stderr;
    TaskState m_state;
    std::string m_command;
    std::optional<time_point_t> m_spawned_at;
    std::optional<time_point_t> m_finished_at;
    std::optional<on_task_finished_cb_t> m_on_finished_cb;
    std::optional<task_ttl_t> m_ttl;
    int m_exit_code;
    int m_exit_by_signal;
    std::string m_captured_stdout;
    std::string m_captured_stderr;

    std::string read_fd(int fd);

public:
    Task(std::vector<std::string> const& args, std::optional<on_task_finished_cb_t> cb = std::nullopt, std::optional<task_ttl_t> ttl = TASK_DEFAULT_TTL)
        : m_pid { -1 }
        , m_stdout { -1 }
        , m_stderr { -1 }
        , m_state { TaskState::ERROR }
        , m_command { str_vector_join(args, " ") }
        , m_spawned_at { std::nullopt }
        , m_finished_at { std::nullopt }
        , m_on_finished_cb { cb }
        , m_ttl { ttl }
        , m_exit_code { -1 }
        , m_exit_by_signal { -1 }
        , m_captured_stdout { "" }
        , m_captured_stderr { "" }

    {
        if (args.empty()) {
            LOG_ERROR("args can't be empty");
            return;
        }

        if (args.size() > MAX_TASK_CMD_LEN - 1) {
            LOG_ERROR("Task command is too long; the limit: {}", MAX_TASK_CMD_LEN - 1);
            return;
        }

        int stdout_pipe[2];
        int stderr_pipe[2];

        if (pipe(stdout_pipe) != 0) {
            LOG_ERROR("pipe: {}", strerror(errno));
            return;
        }

        if (pipe(stderr_pipe) != 0) {
            close(stdout_pipe[0]);
            close(stdout_pipe[1]);
            LOG_ERROR("pipe: {}", strerror(errno));
            return;
        }

        // temporary block SIGTERM singals so we have time to reset the SIGTERM handlers in the child process
        sigset_t set, oldset;
        sigemptyset(&set);
        sigaddset(&set, SIGTERM);
        sigprocmask(SIG_BLOCK, &set, &oldset);

        m_pid = fork();

        if (m_pid == -1) {
            close(stdout_pipe[0]);
            close(stdout_pipe[1]);
            close(stderr_pipe[0]);
            close(stderr_pipe[1]);
            LOG_ERROR("fork: {}", strerror(errno));
            return;
        }

        // child
        if (m_pid == 0) {
            reset_all_signals_to_default();
            sigprocmask(SIG_SETMASK, &oldset, nullptr);

            if (dup2(stdout_pipe[1], STDOUT_FILENO) == -1) {
                write(STDERR_FILENO, "dup2 stdout failed\n", 19);
                _exit(1);
            }

            if (dup2(stderr_pipe[1], STDERR_FILENO) == -1) {
                write(2, "dup2 stdout failed\n", 19);
                _exit(1);
            }

            // sweep-close any inherited file descriptors that we don't need in the child process
            for (int fd { 0 }; fd < MAX_OPENED_FILES; fd++) {
                if (fd != STDOUT_FILENO && fd != STDERR_FILENO && fd != STDIN_FILENO) {
                    close(fd);
                }
            }

            char* argv[MAX_TASK_CMD_LEN];
            for (size_t i { 0 }; i < args.size(); i++) {
                argv[i] = const_cast<char*>(args[i].c_str());
            }
            argv[args.size()] = nullptr;

            execvp(argv[0], argv);
            perror("execvp");
            _exit(127);
        } else {
            // parent process
            sigprocmask(SIG_SETMASK, &oldset, nullptr);

            setpgid(m_pid, m_pid);
            m_spawned_at = std::chrono::system_clock::now();

            // close the write end of a pipe, child will use it
            close(stdout_pipe[1]);
            close(stderr_pipe[1]);

            m_stdout = stdout_pipe[0];
            m_stderr = stderr_pipe[0];

            for (int fd : std::vector { m_stdout, m_stderr }) {
                int flags = fcntl(fd, F_GETFL, 0);
                if (flags == -1) {
                    LOG_ERROR("fcntl F_GETFL: {}", strerror(errno));
                    close(m_stdout);
                    close(m_stderr);
                    return;
                }
                if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) != 0) {
                    LOG_ERROR("fcntl F_SETFL: {}", strerror(errno));
                    close(m_stdout);
                    close(m_stderr);
                    return;
                }
            }

            m_state = TaskState::RUNNING;
        }
    }

    ~Task()
    {
        if (m_stderr != -1) {
            close(m_stderr);
        }

        if (m_stdout != -1) {
            close(m_stdout);
        }
    }

    Task(Task const&) = delete;
    Task& operator=(Task const&) = delete;

    Task(Task&& t) noexcept
        : m_pid { std::exchange(t.m_pid, -1) }
        , m_stdout { std::exchange(t.m_stdout, -1) }
        , m_stderr { std::exchange(t.m_stderr, -1) }
        , m_state { t.m_state }
        , m_command { t.m_command }
        , m_spawned_at { t.m_spawned_at }
        , m_finished_at { t.m_finished_at }
        , m_on_finished_cb { t.m_on_finished_cb }
        , m_ttl { t.m_ttl }
        , m_exit_code { t.m_exit_code }
        , m_exit_by_signal { t.m_exit_by_signal }
        , m_captured_stdout { t.m_captured_stdout }
        , m_captured_stderr { t.m_captured_stderr }
    {
    }

    TaskState state() const { return m_state; }

    bool was_spawned() const { return m_pid != -1; }

    pid_t pid() const { return m_pid; }

    void mark_as_completed(int exit_code, int exit_by_signal);
    void mark_as_expired();

    bool is_successful() const
    {
        return m_state == TaskState::FINISHED && m_exit_code == 0;
    }

    std::optional<time_point_t> finished_at() const { return m_finished_at; }

    std::string const& command() const { return m_command; }

    std::optional<time_point_t> spawned_at() const { return m_spawned_at; }

    std::optional<task_ttl_t> ttl() const { return m_ttl; }

    friend void safe_task_cb_call(Task const& t, TaskResult const& result);
    std::optional<time_point_t> expires_at() const;
};

class Queue {
private:
    size_t m_cap;
    std::unordered_map<pid_t, Task> m_tasks;
    arm_timer_callback_t m_arm_timer_cb;

    bool has_running_tasks()
    {
        return std::count_if(m_tasks.begin(), m_tasks.end(), [](auto const& item) {
            return item.second.state() == TaskState::RUNNING;
        }) != 0;
    }

public:
    Queue(size_t cap, arm_timer_callback_t arm_timer_cb)
        : m_cap { cap }
        , m_tasks {}
        , m_arm_timer_cb { arm_timer_cb }
    {
    }

    ~Queue();

    Queue(Queue const&) = default;
    Queue& operator=(Queue const&) = default;
    Queue(Queue&&) = default;
    Queue& operator=(Queue&&) = default;

    std::optional<pid_t> schedule_task(
        std::vector<std::string> const&,
        std::optional<on_task_finished_cb_t> = std::nullopt,
        std::optional<task_ttl_t> = std::nullopt);

    void terminate_task(pid_t task_pid);
    void remove_task(pid_t task_pid);
    int handle_finished_tasks();
    int handle_expired_tasks();
    void arm_timer_if_needed();

    void set_arm_timer_cb(arm_timer_callback_t cb)
    {
        m_arm_timer_cb = cb;
    }

    void process_tasks();

    std::optional<std::reference_wrapper<Task const>> get_task(pid_t pid)
    {
        if (m_tasks.contains(pid)) {
            return m_tasks.at(pid);
        }
        return std::nullopt;
    }
};
}
