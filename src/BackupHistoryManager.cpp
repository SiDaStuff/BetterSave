/**
 * BetterSave - Backup History Manager
 * Created by: sidastuff
 */

#include "BackupHistoryManager.hpp"
#include "BetterSaveLogger.hpp"
#include <Geode/loader/Dirs.hpp>
#include <matjson.hpp>
#include <fstream>
#include <sstream>
#include <chrono>
#include <iomanip>

BackupHistoryManager* BackupHistoryManager::s_instance = nullptr;

BackupHistoryManager::BackupHistoryManager() {
    m_historyFilePath = geode::dirs::getSaveDir() / "bettersave_history.json";
    loadFromFile();
}

void BackupHistoryManager::addBackupEntry(const std::string& slotName, const std::string& deviceName,
                                         size_t gmSize, size_t llSize, bool isAuto) {
    BackupEntry entry;
    entry.slotName = slotName;
    entry.deviceName = deviceName;
    entry.gmSize = gmSize;
    entry.llSize = llSize;
    entry.isAutoBackup = isAuto;
    
    // Generate timestamp
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    std::stringstream ss;
    ss << std::put_time(std::localtime(&time), "%Y-%m-%d %H:%M:%S");
    entry.timestamp = ss.str();
    
    m_history.push_back(entry);
    
    // Keep only last 50 backups in history
    if (m_history.size() > 50) {
        m_history.erase(m_history.begin());
    }
    
    saveToFile();
    BetterSaveLogger::get()->info("History", fmt::format("Backup entry added: {} ({})", slotName, isAuto ? "auto" : "manual"));
}

std::vector<BackupEntry> BackupHistoryManager::getHistory() {
    return m_history;
}

std::vector<std::string> BackupHistoryManager::getAvailableSlots() {
    std::vector<std::string> slots;
    std::set<std::string> uniqueSlots;
    
    for (const auto& entry : m_history) {
        uniqueSlots.insert(entry.slotName);
    }
    
    for (const auto& slot : uniqueSlots) {
        slots.push_back(slot);
    }
    
    return slots;
}

void BackupHistoryManager::clearHistory() {
    m_history.clear();
    saveToFile();
    BetterSaveLogger::get()->info("History", "Backup history cleared");
}

void BackupHistoryManager::removeSlot(const std::string& slotName) {
    m_history.erase(
        std::remove_if(m_history.begin(), m_history.end(),
            [&slotName](const BackupEntry& entry) { return entry.slotName == slotName; }),
        m_history.end()
    );
    saveToFile();
    BetterSaveLogger::get()->info("History", fmt::format("Removed slot: {}", slotName));
}

void BackupHistoryManager::saveToFile() {
    try {
        std::vector<matjson::Value> historyArray;
        
        for (const auto& entry : m_history) {
            matjson::Value historyEntry;
            historyEntry["slotName"] = entry.slotName;
            historyEntry["timestamp"] = entry.timestamp;
            historyEntry["deviceName"] = entry.deviceName;
            historyEntry["gmSize"] = static_cast<int>(entry.gmSize);
            historyEntry["llSize"] = static_cast<int>(entry.llSize);
            historyEntry["isAutoBackup"] = entry.isAutoBackup;
            historyArray.push_back(historyEntry);
        }
        
        matjson::Value historyJson = historyArray;
        
        std::ofstream file(m_historyFilePath, std::ios::out | std::ios::trunc);
        if (file.is_open()) {
            file << historyJson.dump();
            file.close();
        }
    } catch (const std::exception& e) {
        BetterSaveLogger::get()->error("History", fmt::format("Failed to save history: {}", e.what()));
    }
}

void BackupHistoryManager::loadFromFile() {
    try {
        if (!std::filesystem::exists(m_historyFilePath)) {
            return;
        }
        
        std::ifstream file(m_historyFilePath);
        std::stringstream buffer;
        buffer << file.rdbuf();
        file.close();
        
        auto jsonResult = matjson::parse(buffer.str());
        if (!jsonResult.isOk() || !jsonResult.unwrap().isArray()) {
            return;
        }
        
        auto historyJson = jsonResult.unwrap();
        auto arrayResult = historyJson.as<std::vector<matjson::Value>>();
        if (!arrayResult.isOk()) {
            return;
        }
        
        m_history.clear();
        auto historyArray = arrayResult.unwrap();
        
        for (const auto& entryJson : historyArray) {
            if (!entryJson.isObject()) continue;
            
            BackupEntry entry;
            
            if (entryJson.contains("slotName")) {
                auto result = entryJson["slotName"].asString();
                if (result.isOk()) entry.slotName = result.unwrap();
            }
            if (entryJson.contains("timestamp")) {
                auto result = entryJson["timestamp"].asString();
                if (result.isOk()) entry.timestamp = result.unwrap();
            }
            if (entryJson.contains("deviceName")) {
                auto result = entryJson["deviceName"].asString();
                if (result.isOk()) entry.deviceName = result.unwrap();
            }
            if (entryJson.contains("gmSize") && entryJson["gmSize"].isNumber()) {
                entry.gmSize = entryJson["gmSize"].as<int>().unwrapOr(0);
            }
            if (entryJson.contains("llSize") && entryJson["llSize"].isNumber()) {
                entry.llSize = entryJson["llSize"].as<int>().unwrapOr(0);
            }
            if (entryJson.contains("isAutoBackup") && entryJson["isAutoBackup"].isBool()) {
                entry.isAutoBackup = entryJson["isAutoBackup"].as<bool>().unwrapOr(false);
            }
            
            m_history.push_back(entry);
        }
        
        BetterSaveLogger::get()->info("History", fmt::format("Loaded {} backup entries", m_history.size()));
        
    } catch (const std::exception& e) {
        BetterSaveLogger::get()->error("History", fmt::format("Failed to load history: {}", e.what()));
    }
}

