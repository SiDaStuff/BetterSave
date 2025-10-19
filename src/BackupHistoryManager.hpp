/**
 * BetterSave - Backup History Manager
 * Created by: sidastuff
 */

#pragma once
#include <Geode/Geode.hpp>
#include <string>
#include <vector>

using namespace geode::prelude;

struct BackupEntry {
    std::string slotName;
    std::string timestamp;
    std::string deviceName;
    size_t gmSize;
    size_t llSize;
    bool isAutoBackup;
};

class BackupHistoryManager {
private:
    static BackupHistoryManager* s_instance;
    std::vector<BackupEntry> m_history;
    std::filesystem::path m_historyFilePath;
    
    void saveToFile();

public:
    static BackupHistoryManager* get() {
        if (!s_instance) {
            s_instance = new BackupHistoryManager();
        }
        return s_instance;
    }

    BackupHistoryManager();
    
    void addBackupEntry(const std::string& slotName, const std::string& deviceName, 
                       size_t gmSize, size_t llSize, bool isAuto);
    std::vector<BackupEntry> getHistory();
    std::vector<std::string> getAvailableSlots();
    void clearHistory();
    void removeSlot(const std::string& slotName);
    void loadFromFile();
};

