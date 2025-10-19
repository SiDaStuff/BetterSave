/**
 * BetterSave - Auto Backup Scheduler
 * Created by: sidastuff
 */

#pragma once
#include <Geode/Geode.hpp>
#include <chrono>

using namespace geode::prelude;

class AutoBackupScheduler {
private:
    static AutoBackupScheduler* s_instance;
    std::chrono::system_clock::time_point m_lastBackupTime;
    bool m_isRunning = false;
    
public:
    static AutoBackupScheduler* get() {
        if (!s_instance) {
            s_instance = new AutoBackupScheduler();
        }
        return s_instance;
    }

    AutoBackupScheduler();
    
    void checkAndBackup();
    void performAutoBackup();
    bool shouldBackup();
    void resetTimer();
    void start();
    void stop();
};

