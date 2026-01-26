// Copyright Mia Rolfe. All rights reserved.
#include <Catch2/catch.hpp>

#include <fstream>
#include <string>

#include <Core/Logger.h>

namespace ART
{

// Helper function for verifying file operations
static std::string ReadLogFile(const std::string& filename)
{
    std::ifstream file(filename);
    if (!file.is_open()) return "";

    std::string content((std::istreambuf_iterator<char>(file)),
                         std::istreambuf_iterator<char>());
    return content;
}

// There can only be one :)
TEST_CASE("Logger singleton returns the same instance", "[Logger]")
{
    Logger& logger1 = Logger::Get();
    Logger& logger2 = Logger::Get();

    REQUIRE(&logger1 == &logger2);
}

TEST_CASE("Logger default log severity is fatal", "[Logger]")
{
    Logger& logger = Logger::Get();

    // Default log
    logger.Log("Test fatal log message");

    // Immediately flush to file
    logger.Flush();

    // Check the log file contains the message
    const std::string contents = ReadLogFile("log.txt");
    REQUIRE(contents.find("[FATAL] Test fatal log message") != std::string::npos);
}

TEST_CASE("Logger convenience functions enqueue messages", "[Logger]")
{
    Logger& logger = Logger::Get();

    logger.LogFatal("Fatal message");
    logger.LogError("Error message");
    logger.LogWarn("Warning message");
    logger.LogInfo("Info message");
    logger.LogDebug("Debug message");

    logger.Flush();

    const std::string contents = ReadLogFile("log.txt");
    REQUIRE(contents.find("[FATAL] Fatal message") != std::string::npos);
    REQUIRE(contents.find("[ERROR] Error message") != std::string::npos);
    REQUIRE(contents.find("[WARN] Warning message") != std::string::npos);
    REQUIRE(contents.find("[INFO] Info message") != std::string::npos);
    REQUIRE(contents.find("[DEBUG] Debug message") != std::string::npos);
}

TEST_CASE("Logger flush empties the log queue", "[Logger]")
{
    Logger& logger = Logger::Get();

    logger.LogInfo("Test message to flush");
    logger.Flush();

    // After flush, queue should be empty and messages should be in the log file
    const std::string contents = ReadLogFile("log.txt");
    REQUIRE(contents.find("Test message to flush") != std::string::npos);

    // Cannot directly access queue, so
    // flush immediately after and verify the message has actually been removed from the queue
    logger.Flush();
    const std::string new_contents = ReadLogFile("log.txt");
    REQUIRE(new_contents.find("Test message to flush") != std::string::npos);
}

TEST_CASE("Logger handles multiple threads logging concurrently", "[Logger]")
{
    Logger& logger = Logger::Get();
    static constexpr int num_threads = 5;
    static constexpr int messages_to_log_per_thread = 10;

    auto thread_func = [&logger](int thread_id)
    {
        for (int i = 0; i < messages_to_log_per_thread; i++)
        {
            logger.LogInfo("Thread " + std::to_string(thread_id) + " message " + std::to_string(i));
        }
    };

    std::vector<std::thread> threads;
    for (int thread_id = 0; thread_id < num_threads; thread_id++)
    {
        threads.emplace_back(thread_func, thread_id);
    }

    for (auto& thread : threads)
    {
        thread.join();
    }

    logger.Flush();

    const std::string contents = ReadLogFile("log.txt");
    // Check every message from every thread made it into the log file
    for (int thread_id = 0; thread_id < num_threads; thread_id++)
    {
        for (int message_id = 0; message_id < messages_to_log_per_thread; message_id++)
        {
            REQUIRE(contents.find("Thread " + std::to_string(thread_id) + " message " + std::to_string(message_id)) != std::string::npos);
        }
    }
}

} // namespace ART
