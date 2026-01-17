#include "tasks.h"
#include "debug_config.h"
#include "utils/logging.h"
#include <algorithm>
#include <cerrno>
#include <chrono>
#include <cpptrace/from_current.hpp>
#include <cpptrace/from_current_macros.hpp>
#include <csignal>
#include <cstring>
#include <exception>
#include <optional>
#include <string>
#include <sys/_types/_pid_t.h>
#include <sys/signal.h>
#include <sys/wait.h>
#include <unistd.h>
#include <utility>
#include <vector>

namespace Server {
namespace Tasks {

void safe_task_cb_call(Task const& t, TaskResult const& result)
{
    if ((t.m_state == TaskState::FINISHED || t.m_state == TaskState::TIMEOUT) && t.m_on_finished_cb) {
        CPPTRACE_TRY
        {
            (*t.m_on_finished_cb)(result);
        }
        CPPTRACE_CATCH(std::exception const& e)
        {
            LOG_EXCEPTION("[t:{}] exception calling a callback: {}", t.pid(), e.what());
            IF_VERBOSE
            {
                cpptrace::rethrow();
            }
        }
    }
}

std::string Task::read_fd(int fd)
{
    std::string result { "" };
    char buff[4096];
    while (true) {
        auto n = read(fd, buff, 4096);
        if (n < 0) {
            if (errno != EWOULDBLOCK && errno != EAGAIN) {
                LOG_WARN("[t:{}] error reading from fd {}:{}", m_pid, fd, strerror(errno));
            }
            break;
        } else if (n == 0) {
            break;
        } else {
            result.append(buff, static_cast<size_t>(n));
            if (n < 4096) {
                break;
            }
        }
    }
    return result;
}

void Task::mark_as_completed(int exit_code, int exit_by_signal)
{
    if (exit_code == -1 && exit_by_signal == -1) {
        LOG_WARN("[t:{}] mark as complete without exit code/signal", m_pid);
        return;
    }
    m_exit_by_signal = exit_by_signal;
    m_exit_code = exit_code;
    m_captured_stdout = read_fd(m_stdout);
    m_captured_stderr = read_fd(m_stderr);
    close(m_stdout);
    close(m_stderr);
    m_stdout = -1;
    m_stderr = -1;

    m_state = TaskState::FINISHED;
    m_finished_at = std::chrono::system_clock::now();

    TaskResult result {
        m_pid,
        is_successful(),
        m_captured_stdout,
        m_captured_stderr,
    };

    safe_task_cb_call(*this, result);
}

void Task::mark_as_expired()
{
    m_captured_stdout = read_fd(m_stdout);
    m_captured_stderr = read_fd(m_stderr);
    close(m_stdout);
    close(m_stderr);
    m_stdout = -1;
    m_stderr = -1;

    m_state = TaskState::TIMEOUT;

    TaskResult result {
        m_pid,
        is_successful(),
        m_captured_stdout,
        m_captured_stderr,
    };

    safe_task_cb_call(*this, result);
}

std::optional<time_point_t> Task::expires_at() const
{
    if (m_spawned_at && m_ttl) {
        return *m_spawned_at + *m_ttl;
    }
    return std::nullopt;
}

bool safe_kill(pid_t pid, int sig)
{
    if (getpid() != pid && pid > 0) {
        if (kill(pid, sig) == -1) {
            LOG_WARN("[t:{}] kill: {}", pid, strerror(errno));
            return false;
        }
        return true;
    }
    LOG_WARN("[t:{}] wtf pid", pid);
    return false;
}

std::optional<pid_t> Queue::schedule_task(std::vector<std::string> const& args, std::optional<on_task_finished_cb_t> cb, std::optional<task_ttl_t> ttl)
{
    if (m_tasks.size() == m_cap) {
        LOG_WARN("the queue is already full ({})", m_tasks.size());
        return std::nullopt;
    }
    auto t = Task(args, cb, ttl);
    if ((t.state() == TaskState::ERROR && t.was_spawned()) || !t.was_spawned()) {
        LOG_WARN("Failed to spwan a task for {}", t.command());
        safe_kill(t.pid(), SIGKILL);
        return {};
    }
    pid_t pid = t.pid();
    m_tasks.emplace(pid, std::move(t));
    LOG_INFO("[t:{}] scheduled a task: {} ...", pid, t.command());
    arm_timer_if_needed();
    return pid;
}

void Queue::terminate_task(pid_t pid)
{
    if (!m_tasks.contains(pid)) {
        LOG_WARN("Request to terminate unregistered task: {}", pid);
    }
    if (kill(pid, SIGTERM) != 0) {
        LOG_ERROR("failed to kill a child {}: {}", pid, strerror(errno));
    };
    m_tasks.erase(pid);
}

int Queue::handle_finished_tasks()
{
    int reaped_children { 0 };
    int status;
    while (true) {
        auto pid = waitpid(-1, &status, WNOHANG);
        if (pid <= 0) {
            if (pid < 0 and errno != ECHILD) {
                LOG_ERROR("error waiting on finished tasks: {}", strerror(errno));
            }
            break;
        }
        if (!m_tasks.contains(pid)) {
            LOG_ERROR("unexpected task PID {}", pid);
            continue;
        }
        Task& task = m_tasks.at(pid);
        int exit_code { -1 };
        int exit_by_signal { -1 };
        std::string signame { "" };
        if (WIFEXITED(status)) {
            exit_code = WEXITSTATUS(status);
        } else if (WIFSIGNALED(status)) {
            exit_by_signal = WTERMSIG(status);
            signame = strsignal(exit_by_signal);
        }
        LOG_DEBUG("[t:{}] completed with ec: {}; sig: {}", task.pid(), exit_code, signame);
        task.mark_as_completed(exit_code, exit_by_signal);
        reaped_children++;
    }
    return reaped_children;
}

bool process_is_running(pid_t pid)
{
    return kill(pid, 0) == 0;
}

Queue::~Queue()
{
    bool has_running_tasks { false };
    for (auto const& [pid, task] : m_tasks) {
        if (task.state() == TaskState::RUNNING && process_is_running(pid)) {
            has_running_tasks = true;
            LOG_INFO("[t:{}] sending SIGTERM on queue shutdown ...", pid);
            safe_kill(pid, SIGTERM);
        }
    }

    if (!has_running_tasks) {
        return;
    }

    usleep(100000);

    for (auto const& [pid, task] : m_tasks) {
        if (task.state() == TaskState::RUNNING && process_is_running(pid)) {
            safe_kill(pid, SIGKILL);
        };
    }

    pid_t pid;
    while ((pid = waitpid(-1, nullptr, WNOHANG)) > 0) {
        LOG_INFO("[t:{}] got SIGCHLD on queue shutdown", pid);
    }
}

int Queue::handle_expired_tasks()
{
    auto now = std::chrono::system_clock::now();
    std::vector<pid_t> expired_tasks {};
    for (auto& [pid, t] : m_tasks) {
        if (t.state() == TaskState::RUNNING && t.expires_at() && *t.expires_at() < now) {
            t.mark_as_expired();
            if (safe_kill(t.pid(), SIGTERM)) {
                expired_tasks.push_back(t.pid());
            } else if (process_is_running(t.pid()) && kill(t.pid(), SIGKILL)) {
                expired_tasks.push_back(t.pid());
            } else {
                LOG_WARN("[t:{}] was marked as expired but couldn't kill!", t.pid());
            }
        }
    }

    usleep(100000);

    for (auto pid : expired_tasks) {
        LOG_DEBUG("[t:{}] reaping after timeout ... ", pid);
        auto wpid = waitpid(pid, nullptr, WNOHANG);
        if (wpid == 0) {
            LOG_WARN("[t:{}] unavailable for reaping after timeout", pid);
        }
        if (wpid < 0) {
            LOG_WARN("[t:{}] error reaping after timeout: {}", pid, strerror(errno));
        }
    }

    return static_cast<int>(expired_tasks.size());
}

void Queue::arm_timer_if_needed()
{
    if (!m_arm_timer_cb) {
        return;
    }
    std::vector<time_point_t> expiry_tstamps;
    auto now = std::chrono::system_clock::now();
    for (auto const& [pid, task] : m_tasks) {
        if (task.state() == TaskState::RUNNING && task.expires_at() && *task.expires_at() > now) {
            expiry_tstamps.push_back(*task.expires_at());
        }
    }
    if (!expiry_tstamps.empty()) {
        auto closest_et = std::min(expiry_tstamps.begin(), expiry_tstamps.end());
        auto timeout = static_cast<int>(std::chrono::duration_cast<std::chrono::milliseconds>(*closest_et - now).count());
        m_arm_timer_cb(timeout);
        return;
    }
}

void Queue::process_tasks()
{
    if (!has_running_tasks()) {
        return;
    }
    LOG_DEBUG("QUEUE: processing tasks... {}", m_tasks.size());
    auto finished = handle_finished_tasks();
    auto expired = handle_expired_tasks();
    arm_timer_if_needed();
    if (finished || expired) {
        LOG_INFO("Queue: finished={};expired={}", finished, expired);
    }
}

}

}
