#pragma once
#include <Geode/Geode.hpp>
#include <string>
#include <vector>

using namespace geode::prelude;

enum class LogLevel {
    Info,
    Warning,
    Error,
    Success
};

struct LogEntry {
    std::string timestamp;
    LogLevel level;
    std::string message;
    std::string category;
};

class BetterSaveLogger {
private:
    static BetterSaveLogger* s_instance;
    std::vector<LogEntry> m_logs;
    std::filesystem::path m_logFilePath;
    
    std::string getLevelString(LogLevel level);
    std::string getCurrentTimestamp();
    void saveToDisk();

public:
    static BetterSaveLogger* get() {
        if (!s_instance) {
            s_instance = new BetterSaveLogger();
        }
        return s_instance;
    }

    BetterSaveLogger();
    
    void log(LogLevel level, const std::string& category, const std::string& message);
    void info(const std::string& category, const std::string& message);
    void warning(const std::string& category, const std::string& message);
    void error(const std::string& category, const std::string& message);
    void success(const std::string& category, const std::string& message);
    
    std::vector<LogEntry> getAllLogs();
    std::vector<LogEntry> getRecentLogs(size_t count = 50);
    void clearLogs();
    void loadFromDisk();
    void forceSave();  // Force immediate save to disk
};

