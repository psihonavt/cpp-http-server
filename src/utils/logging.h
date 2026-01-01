#pragma once

#include "cpptrace/formatting.hpp"
#include "cpptrace/from_current.hpp"
#include <chrono>
#include <filesystem>
#include <format>
#include <fstream>
#include <iostream>
#include <memory>
#include <optional>
#include <ostream>
#include <source_location>
#include <stdexcept>
#include <vector>

enum class LogLevel { DEBUG,
    INFO,
    WARN,
    ERROR,
    CRITICAL };

class Logger {
    LogLevel min_level_ { LogLevel::INFO };
    std::vector<std::ostream*> sinks_ {};
    std::unique_ptr<std::ofstream> file_;

public:
    Logger() = default;

    void add_console(std::ostream& cout = std::cout)
    {
        sinks_.push_back(&cout);
    }

    void add_file(std::string const& filename)
    {
        file_ = std::make_unique<std::ofstream>(filename, std::ios::app);
        if (file_->is_open()) {
            sinks_.push_back(file_.get());
        } else {
            throw std::runtime_error(std::format("Can't add {} as a log file", filename));
        }
    }

    void set_log_level(LogLevel level)
    {
        min_level_ = level;
    }

    static Logger& instance()
    {
        static Logger logger;
        return logger;
    }

    template<typename... Args>
    void log(LogLevel level, std::source_location loc, bool on_exception, std::format_string<Args...> fmt, Args&&... args)
    {
        if (level < min_level_) {
            return;
        }

        auto now = std::chrono::system_clock::now();
        std::string tstamp = std::format("{:%Y-%m-%d %H:%M:%S}", now);
        std::string msg = std::format(fmt, std::forward<Args>(args)...);

        std::string loc_filename { loc.file_name() };
        std::string log_msg = std::format("[{}|{}|{}:{}] {}", Logger::log_level_as_string(level), tstamp,
            std::filesystem::path(loc_filename).filename().string(), loc.line(), msg);
        for (auto const& sink : sinks_) {
            *sink << log_msg << "\n";
            if (on_exception) {
                cpptrace::from_current_exception().print(*sink);
            }
        }
    }

    void log(LogLevel level, std::source_location loc, bool on_exception, std::string const& msg)
    {
        if (level < min_level_) {
            return;
        }

        auto now = std::chrono::system_clock::now();
        std::string tstamp = std::format("{:%Y-%m-%d %H:%M:%S}", now);
        std::string loc_filename { loc.file_name() };
        std::string log_msg = std::format("[{}|{}|{}:{}] {}", Logger::log_level_as_string(level), tstamp,
            std::filesystem::path(loc_filename).filename().string(), loc.line(), msg);
        for (auto const& sink : sinks_) {
            *sink << log_msg << "\n";
            if (on_exception) {
                cpptrace::from_current_exception().print(*sink);
            }
        }
    }

private:
    static std::string log_level_as_string(LogLevel level)
    {
        switch (level) {
        case LogLevel::INFO:
            return "INFO";
        case LogLevel::WARN:
            return "WARNING";
        case LogLevel::DEBUG:
            return "DEBUG";
        case LogLevel::ERROR:
            return "ERROR";
        case LogLevel::CRITICAL:
            return "CRITICAL";
        default:
            return "???";
        }
    }
};

inline void setup_logging(LogLevel log_level = LogLevel::INFO, bool use_console = true, std::optional<std::string> filename = std::nullopt)
{
    auto formatter = cpptrace::formatter {}
                         .header("Stack trace:")
                         .addresses(cpptrace::formatter::address_mode::object)
                         .snippets(true);

    if (use_console) {
        Logger::instance().add_console();
    }
    if (filename) {
        Logger::instance().add_file(*filename);
    }
    Logger::instance().set_log_level(log_level);
}

#define LOG_DEBUG(...) Logger::instance().log(LogLevel::DEBUG, std::source_location::current(), false, __VA_ARGS__)
#define LOG_INFO(...) Logger::instance().log(LogLevel::INFO, std::source_location::current(), false, __VA_ARGS__)
#define LOG_ERROR(...) Logger::instance().log(LogLevel::ERROR, std::source_location::current(), false, __VA_ARGS__)
#define LOG_WARN(...) Logger::instance().log(LogLevel::WARN, std::source_location::current(), false, __VA_ARGS__)
#define LOG_EXCEPTION(...) Logger::instance().log(LogLevel::ERROR, std::source_location::current(), true, __VA_ARGS__)
