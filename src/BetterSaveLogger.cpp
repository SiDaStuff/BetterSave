#include "BetterSaveLogger.hpp"
#include <Geode/loader/Dirs.hpp>
#include <matjson.hpp>
#include <fstream>
#include <sstream>
#include <chrono>
#include <iomanip>

BetterSaveLogger* BetterSaveLogger::s_instance = nullptr;

BetterSaveLogger::BetterSaveLogger() {
    m_logFilePath = geode::dirs::getSaveDir() / "bettersave_logs.json";
    loadFromDisk();
}

std::string BetterSaveLogger::getLevelString(LogLevel level) {
    switch (level) {
        case LogLevel::Info: return "INFO";
        case LogLevel::Warning: return "WARNING";
        case LogLevel::Error: return "ERROR";
        case LogLevel::Success: return "SUCCESS";
        default: return "UNKNOWN";
    }
}

std::string BetterSaveLogger::getCurrentTimestamp() {
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    std::stringstream ss;
    ss << std::put_time(std::localtime(&time), "%Y-%m-%d %H:%M:%S");
    return ss.str();
}

void BetterSaveLogger::log(LogLevel level, const std::string& category, const std::string& message) {
    LogEntry entry;
    entry.timestamp = getCurrentTimestamp();
    entry.level = level;
    entry.category = category;
    entry.message = message;
    
    m_logs.push_back(entry);
    
    // Also log to Geode's console
    std::string fullMessage = fmt::format("[BetterSave:{}] {}", category, message);
    switch (level) {
        case LogLevel::Info:
            geode::log::info("{}", fullMessage);
            break;
        case LogLevel::Warning:
            geode::log::warn("{}", fullMessage);
            break;
        case LogLevel::Error:
            geode::log::error("{}", fullMessage);
            break;
        case LogLevel::Success:
            geode::log::info("✓ {}", fullMessage);
            break;
    }
    
    // Keep only last 500 logs in memory
    if (m_logs.size() > 500) {
        m_logs.erase(m_logs.begin(), m_logs.begin() + 100);
    }
    
    // Save to disk immediately for critical logs
    if (level == LogLevel::Error || level == LogLevel::Success) {
        saveToDisk();
    }
}

void BetterSaveLogger::info(const std::string& category, const std::string& message) {
    log(LogLevel::Info, category, message);
}

void BetterSaveLogger::warning(const std::string& category, const std::string& message) {
    log(LogLevel::Warning, category, message);
}

void BetterSaveLogger::error(const std::string& category, const std::string& message) {
    log(LogLevel::Error, category, message);
}

void BetterSaveLogger::success(const std::string& category, const std::string& message) {
    log(LogLevel::Success, category, message);
}

std::vector<LogEntry> BetterSaveLogger::getAllLogs() {
    return m_logs;
}

std::vector<LogEntry> BetterSaveLogger::getRecentLogs(size_t count) {
    if (m_logs.size() <= count) {
        return m_logs;
    }
    return std::vector<LogEntry>(m_logs.end() - count, m_logs.end());
}

void BetterSaveLogger::clearLogs() {
    m_logs.clear();
    saveToDisk();
}

void BetterSaveLogger::forceSave() {
    saveToDisk();
}

void BetterSaveLogger::saveToDisk() {
    try {
        std::vector<matjson::Value> logsArray;
        
        for (const auto& entry : m_logs) {
            matjson::Value logEntry;
            logEntry["timestamp"] = entry.timestamp;
            logEntry["level"] = getLevelString(entry.level);
            logEntry["category"] = entry.category;
            logEntry["message"] = entry.message;
            logsArray.push_back(logEntry);
        }
        
        matjson::Value logsJson = logsArray;
        
        std::ofstream file(m_logFilePath, std::ios::out | std::ios::trunc);
        if (file.is_open()) {
            file << logsJson.dump();
            file.flush();  // Force write to disk
            file.close();
        }
        
    } catch (const std::exception& e) {
        geode::log::error("Failed to save BetterSave logs: {}", e.what());
    }
}

void BetterSaveLogger::loadFromDisk() {
    try {
        if (!std::filesystem::exists(m_logFilePath)) {
            return;
        }
        
        std::ifstream file(m_logFilePath);
        std::stringstream buffer;
        buffer << file.rdbuf();
        file.close();
        
        auto jsonResult = matjson::parse(buffer.str());
        if (!jsonResult.isOk()) {
            geode::log::error("Failed to parse BetterSave logs JSON");
            return;
        }
        
        auto logsJson = jsonResult.unwrap();
        if (!logsJson.isArray()) {
            return;
        }
        
        m_logs.clear();
        
        // Properly iterate through JSON array
        auto arrayResult = logsJson.as<std::vector<matjson::Value>>();
        if (!arrayResult.isOk()) {
            geode::log::error("Failed to convert logs JSON to array");
            return;
        }
        
        auto logsArray = arrayResult.unwrap();
        for (const auto& logJson : logsArray) {
            try {
                if (!logJson.isObject()) continue;
                
                LogEntry entry;
                
                if (logJson.contains("timestamp")) {
                    auto result = logJson["timestamp"].asString();
                    if (result.isOk()) entry.timestamp = result.unwrap();
                }
                
                if (logJson.contains("level")) {
                    auto result = logJson["level"].asString();
                    if (result.isOk()) {
                        std::string levelStr = result.unwrap();
                        if (levelStr == "INFO") entry.level = LogLevel::Info;
                        else if (levelStr == "WARNING") entry.level = LogLevel::Warning;
                        else if (levelStr == "ERROR") entry.level = LogLevel::Error;
                        else if (levelStr == "SUCCESS") entry.level = LogLevel::Success;
                    }
                }
                
                if (logJson.contains("category")) {
                    auto result = logJson["category"].asString();
                    if (result.isOk()) entry.category = result.unwrap();
                }
                
                if (logJson.contains("message")) {
                    auto result = logJson["message"].asString();
                    if (result.isOk()) entry.message = result.unwrap();
                }
                
                m_logs.push_back(entry);
            } catch (const std::exception& e) {
                geode::log::error("Error parsing log entry: {}", e.what());
            }
        }
        
        geode::log::info("Loaded {} BetterSave logs from disk", m_logs.size());
        
    } catch (const std::exception& e) {
        geode::log::error("Failed to load BetterSave logs: {}", e.what());
    }
}

