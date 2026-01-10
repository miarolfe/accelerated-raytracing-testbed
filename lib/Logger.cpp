// Copyright Mia Rolfe. All rights reserved.
#include <Logger.h>

#include <chrono>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <mutex>
#include <string>
#include <thread>

namespace ART
{

Logger& Logger::Get()
{
    static Logger instance;
    return instance;
}

void Logger::Log(const std::string& log_msg, LogSeverity log_severity)
{
    std::string err_level_str;

    // Don't log messages less severe than the filter level
    if (log_severity > m_log_severity_level_filter)
    {
        return;
    }

    switch (log_severity)
    {
        case LogSeverity::LOG_FATAL:
            err_level_str = "[FATAL]";
            break;
        case LogSeverity::LOG_ERROR:
            err_level_str = "[ERROR]";
            break;
        case LogSeverity::LOG_WARN:
            err_level_str = "[WARN]";
            break;
        case LogSeverity::LOG_INFO:
            err_level_str = "[INFO]";
            break;
        case LogSeverity::LOG_DEBUG:
            err_level_str = "[DEBUG]";
            break;
    }

    const std::string timestamped_log_msg = GetCurrentTimestamp() + " " + err_level_str + " " + log_msg;

    {
        std::scoped_lock lock(m_log_queue_mutex);
        m_log_queue.push(timestamped_log_msg);
#ifdef DEBUG
        std::cerr << timestamped_log_msg << std::endl;
#endif
    }
}

void Logger::LogFatal(const std::string& log_msg)
{
    Log(log_msg, LogSeverity::LOG_FATAL);
}

void Logger::LogError(const std::string& log_msg)
{
    Log(log_msg, LogSeverity::LOG_ERROR);
}

void Logger::LogWarn(const std::string& log_msg)
{
    Log(log_msg, LogSeverity::LOG_WARN);
}

void Logger::LogInfo(const std::string& log_msg)
{
    Log(log_msg, LogSeverity::LOG_INFO);
}

void Logger::LogDebug(const std::string& log_msg)
{
    Log(log_msg, LogSeverity::LOG_DEBUG);
}

void Logger::Flush()
{
    std::scoped_lock lock(m_log_queue_mutex, m_log_filestream_mutex);
    while (!m_log_queue.empty())
    {
        m_log_filestream << m_log_queue.front() << std::endl;
        m_log_queue.pop();
    }
}

Logger::~Logger()
{
    m_exit_flag = true;

    if (m_log_thread.joinable())
    {
        m_log_thread.join();

        // Ensure all remaining log msgs flushed
        Flush();
    }
}

Logger::Logger() : m_log_filestream("log.txt", std::ios_base::trunc)
{
    m_log_thread = std::thread(&Logger::ProcessLogs, this);
}

void Logger::ProcessLogs()
{
    while (true)
    {
        Flush();

        if (m_exit_flag)
        {
            break;
        }

        static constexpr int logger_thread_sleep_time_ms = 1000;
        std::this_thread::sleep_for(std::chrono::milliseconds(logger_thread_sleep_time_ms));
    }
}

const std::string Logger::GetCurrentTimestamp()
{
    const auto now = std::chrono::system_clock::now();
    const std::time_t now_time_t = std::chrono::system_clock::to_time_t(now);

    std::stringstream string_stream;
    string_stream << std::put_time(std::localtime(&now_time_t), "%Y-%m-%d %X");
    return string_stream.str();
}

} // namespace ART
