/**
 * BetterSave - Auto Backup Scheduler
 * Created by: sidastuff
 */

#include "AutoBackupScheduler.hpp"
#include "SettingsManager.hpp"
#include "BetterSaveLogger.hpp"
#include "FirebaseAuth.hpp"
#include "SaveManagerPopup.hpp"

AutoBackupScheduler* AutoBackupScheduler::s_instance = nullptr;

AutoBackupScheduler::AutoBackupScheduler() {
    m_lastBackupTime = std::chrono::system_clock::now();
}

void AutoBackupScheduler::start() {
    m_isRunning = true;
    m_lastBackupTime = std::chrono::system_clock::now();
    BetterSaveLogger::get()->info("AutoBackup", "Auto-backup scheduler started");
}

void AutoBackupScheduler::stop() {
    m_isRunning = false;
    BetterSaveLogger::get()->info("AutoBackup", "Auto-backup scheduler stopped");
}

bool AutoBackupScheduler::shouldBackup() {
    if (!m_isRunning) return false;
    
    auto& settings = SettingsManager::get()->getSettings();
    if (!settings.autoBackupEnabled) return false;
    
    // Check if user is logged in
    if (!FirebaseAuth::get()->isLoggedIn()) return false;
    
    auto now = std::chrono::system_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::minutes>(now - m_lastBackupTime);
    
    return elapsed.count() >= settings.autoBackupIntervalMinutes;
}

void AutoBackupScheduler::checkAndBackup() {
    if (shouldBackup()) {
        performAutoBackup();
    }
}

void AutoBackupScheduler::performAutoBackup() {
    BetterSaveLogger::get()->info("AutoBackup", "Performing automatic backup...");
    
    // Reset timer first
    resetTimer();
    
    // Create a temporary SaveManagerPopup to use its upload function
    auto popup = SaveManagerPopup::create();
    popup->uploadSaveData();
    
    // Show notification
    auto& settings = SettingsManager::get()->getSettings();
    if (settings.showNotifications) {
        Notification::create("BetterSave: Auto-Backup Started", NotificationIcon::Info, 3.0f)->show();
    }
    
    BetterSaveLogger::get()->info("AutoBackup", "Automatic backup triggered");
}

void AutoBackupScheduler::resetTimer() {
    m_lastBackupTime = std::chrono::system_clock::now();
}

