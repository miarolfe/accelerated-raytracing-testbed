// Copyright Mia Rolfe. All rights reserved.
#pragma once

#include <atomic>
#include <fstream>
#include <thread>
#include <mutex>
#include <queue>

namespace ART
{

enum class LogSeverity
{
    LOG_FATAL, // A crash-level error has occured - this requires attention
    LOG_ERROR, // A non-crash-level error has occured - this requires attention
    LOG_WARN,  // Something unexpected has occured - this probably requries attention
    LOG_INFO,  // An informational log about current state
    LOG_DEBUG, // Additional detail for debugging purposes
};

class Logger
{
public:
    // Get a reference to the current Logger instance
    static Logger& Get();

    // Add a log message to the queue
    // Defaulting to fatal severity to ensure no important log messages are missed by accident
    void Log(const std::string& log_msg, LogSeverity log_severity = LogSeverity::LOG_FATAL);

    // Convenience alias for logging fatal error messages
    void LogFatal(const std::string& log_msg);

    // Convenience alias for logging error messages
    void LogError(const std::string& log_msg);

    // Convenience alias for logging warning messages
    void LogWarn(const std::string& log_msg);

    // Convenience alias for logging info messages
    void LogInfo(const std::string& log_msg);

    // Convenience alias for logging debug messages
    void LogDebug(const std::string& log_msg);

    // Immediately write all log messages in the queue to the log file
    void Flush();

    // Custom destructor for cleanup
    ~Logger();

    // No copy constructor
    Logger(const Logger&) = delete;

    // No copy assignment
    Logger& operator=(const Logger&) = delete;

    // No move constructor
    Logger(Logger&&) = delete;

    // No move assignment
    Logger& operator=(Logger&&) = delete;

private:
    // Private constructor to ensure only one instance of Logger exists
    Logger();

    // Thread function to repeatedly flush the log queue to the log file
    void ProcessLogs();

    // Returns the current timestamp in a nicely formatted string
    static const std::string GetCurrentTimestamp();

    std::ofstream m_log_filestream;
    std::thread m_log_thread;
    std::queue<std::string> m_log_queue;
    std::mutex m_log_filestream_mutex;
    std::mutex m_log_queue_mutex;
    std::atomic_bool m_exit_flag = false;
#ifdef NDEBUG
    LogSeverity m_log_severity_level_filter = LogSeverity::LOG_INFO;
#else
    LogSeverity m_log_severity_level_filter = LogSeverity::LOG_DEBUG;
#endif
};

} // namespace ART
