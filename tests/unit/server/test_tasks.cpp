#include "catch2/catch_test_macros.hpp"
#include "server/tasks.h"
#include <cassert>
#include <cerrno>
#include <chrono>
#include <optional>
#include <string>
#include <sys/signal.h>
#include <sys/wait.h>
#include <unistd.h>
#include <vector>

std::vector<std::string> const simple_task { "ls", "-l" };
std::vector<std::string> const bad_args_command { "ls", "--screw-you-right-here" };
std::vector<std::string> const failed_execvp_command { "totally-absent-command", "-qwerty" };
std::vector<std::string> const take_long_time_command { "sh", "-c", "echo 123 | cat && sleep 60" };

void wait_for_tasks(Server::Tasks::Queue& q, int expected_tasks)
{
    int finished_tasks { 0 };
    int attempts { 100 };

    while (attempts >= 0) {
        finished_tasks += q.handle_finished_tasks();
        if (finished_tasks >= expected_tasks) {
            return;
        }
        usleep(100000);
        attempts--;
    }

    FAIL("got " << finished_tasks << " finished tasks; wanted " << expected_tasks);
}

auto noop_arm_timer_cb = [](long n) -> size_t { return static_cast<size_t>(n); };

TEST_CASE("Tasks queue isolated", "[tasks_queue]")
{
    auto queue { Server::Tasks::Queue(10, noop_arm_timer_cb) };
    SECTION("simple single task")
    {
        auto t_pid = queue.schedule_task(simple_task);
        REQUIRE(t_pid != std::nullopt);
        INFO("PID " << *t_pid);
        wait_for_tasks(queue, 1);
        auto rtask = queue.get_task(*t_pid);
        REQUIRE(rtask);
        auto& task = rtask->get();
        REQUIRE(task.finished_at() != std::nullopt);
        REQUIRE(task.is_successful());
    }

    SECTION("simple ok task wi callback")
    {

        bool callback_succeeded { false };

        auto cb = [&callback_succeeded](Server::Tasks::TaskResult result) -> void {
            if (result.is_successful) {
                callback_succeeded = true;
            }
        };

        REQUIRE(queue.schedule_task(simple_task, cb));
        wait_for_tasks(queue, 1);
        REQUIRE(callback_succeeded);
    }

    SECTION("simple failing task wi callback")
    {

        bool callback_succeeded { false };

        auto cb = [&callback_succeeded](Server::Tasks::TaskResult result) -> void {
            if (!result.is_successful && !result.stderr_content.empty()) {
                callback_succeeded = true;
            }
        };

        auto t_pid = queue.schedule_task(bad_args_command, cb);
        REQUIRE(t_pid != std::nullopt);
        wait_for_tasks(queue, 1);
        auto rtask = queue.get_task(*t_pid);
        REQUIRE(rtask);
        auto& task = rtask->get();
        REQUIRE(task.finished_at() != std::nullopt);
        REQUIRE(!task.is_successful());
        REQUIRE(callback_succeeded);
    }

    SECTION("queue respects the tasks limit")
    {

        std::vector<pid_t> pids;
        queue = Server::Tasks::Queue(2, noop_arm_timer_cb);
        for (auto const& cmd : std::vector { simple_task, bad_args_command }) {
            auto pid = queue.schedule_task(cmd);
            REQUIRE(pid);
            pids.push_back(*pid);
        }
        auto not_a_pid = queue.schedule_task(simple_task);
        REQUIRE(not_a_pid == std::nullopt);
        wait_for_tasks(queue, 2);
    }

    SECTION("it kills children on queue removing")
    {
        {
            auto queue = Server::Tasks::Queue(10, noop_arm_timer_cb);
            for (auto const& cmd : std::vector { simple_task, bad_args_command, take_long_time_command }) {
                auto pid = queue.schedule_task(cmd);
                REQUIRE(pid);
            }
        }
        int status;
        auto p = waitpid(-1, &status, WNOHANG);
        REQUIRE(p == -1);
        REQUIRE(errno == ECHILD);
    }

    SECTION("it handles bad commands")
    {
        bool callback_succeeded { false };

        auto cb = [&callback_succeeded](Server::Tasks::TaskResult result) -> void {
            if (!result.is_successful && result.stderr_content.contains("execvp")) {
                callback_succeeded = true;
            }
        };

        auto pid = queue.schedule_task(failed_execvp_command, cb);
        REQUIRE(pid);
        wait_for_tasks(queue, 1);
        REQUIRE(callback_succeeded);
    }

    SECTION("it respects task's TTL")
    {
        bool callback_succeeded { false };

        auto cb = [&callback_succeeded](Server::Tasks::TaskResult result) -> void {
            if (!result.is_successful && result.stdout_content.contains("123")) {
                callback_succeeded = true;
            }
        };

        auto ttl = std::chrono::milliseconds(100);
        auto pid = queue.schedule_task(take_long_time_command, cb, ttl);
        REQUIRE(pid);
        usleep(100001);
        REQUIRE(queue.handle_expired_tasks() == 1);
        REQUIRE(queue.get_task(*pid)->get().state() == Server::Tasks::TaskState::TIMEOUT);
        REQUIRE(callback_succeeded);
    }

    SECTION("it arms when there are tasks with TTL")
    {
        bool callback_succeeded { false };
        auto arm_timer_cb = [&callback_succeeded](long timeout_ms) -> size_t {
            if (timeout_ms >= 19 * 1000 && timeout_ms <= 21 * 1000) {
                callback_succeeded = true;
            }
            return 1;
        };
        auto queue = Server::Tasks::Queue(10, arm_timer_cb);
        auto ttl_1 = std::chrono::seconds(30);
        auto ttl_2 = std::chrono::seconds(20);
        auto ttl_3 = std::chrono::seconds(10);

        REQUIRE(queue.schedule_task(take_long_time_command, std::nullopt, ttl_1));
        REQUIRE(queue.schedule_task(take_long_time_command, std::nullopt, ttl_2));
        auto t_pid = queue.schedule_task(simple_task, std::nullopt, ttl_3);
        REQUIRE(t_pid);
        wait_for_tasks(queue, 1);
        queue.process_tasks();
        REQUIRE(callback_succeeded);
    }
}
