/**
 * BetterSave - Settings Manager
 * Created by: sidastuff
 */

#include "SettingsManager.hpp"
#include "BetterSaveLogger.hpp"
#include <Geode/loader/Dirs.hpp>
#include <matjson.hpp>
#include <fstream>
#include <sstream>
#include <chrono>
#include <iomanip>

SettingsManager* SettingsManager::s_instance = nullptr;

SettingsManager::SettingsManager() {
    m_settingsFilePath = geode::dirs::getSaveDir() / "bettersave_settings.json";
    loadFromFile();
}

void SettingsManager::saveToFile() {
    try {
        matjson::Value json;
        json["autoBackupEnabled"] = m_settings.autoBackupEnabled;
        json["autoBackupIntervalMinutes"] = m_settings.autoBackupIntervalMinutes;
        json["showNotifications"] = m_settings.showNotifications;
        json["confirmBeforeDownload"] = m_settings.confirmBeforeDownload;
        json["confirmBeforeUpload"] = m_settings.confirmBeforeUpload;
        json["autoCheckIntegrity"] = m_settings.autoCheckIntegrity;
        
        std::ofstream file(m_settingsFilePath, std::ios::out | std::ios::trunc);
        if (file.is_open()) {
            file << json.dump();
            file.close();
            BetterSaveLogger::get()->info("Settings", "Settings saved successfully");
        }
    } catch (const std::exception& e) {
        BetterSaveLogger::get()->error("Settings", fmt::format("Failed to save settings: {}", e.what()));
    }
}

void SettingsManager::loadFromFile() {
    try {
        if (!std::filesystem::exists(m_settingsFilePath)) {
            saveToFile(); // Create default settings file
            return;
        }
        
        std::ifstream file(m_settingsFilePath);
        std::stringstream buffer;
        buffer << file.rdbuf();
        file.close();
        
        auto jsonResult = matjson::parse(buffer.str());
        if (!jsonResult.isOk()) {
            BetterSaveLogger::get()->error("Settings", "Failed to parse settings JSON");
            return;
        }
        
        auto json = jsonResult.unwrap();
        
        if (json.contains("autoBackupEnabled") && json["autoBackupEnabled"].isBool()) {
            m_settings.autoBackupEnabled = json["autoBackupEnabled"].as<bool>().unwrapOr(false);
        }
        if (json.contains("autoBackupIntervalMinutes") && json["autoBackupIntervalMinutes"].isNumber()) {
            m_settings.autoBackupIntervalMinutes = json["autoBackupIntervalMinutes"].as<int>().unwrapOr(30);
        }
        if (json.contains("showNotifications") && json["showNotifications"].isBool()) {
            m_settings.showNotifications = json["showNotifications"].as<bool>().unwrapOr(true);
        }
        if (json.contains("confirmBeforeDownload") && json["confirmBeforeDownload"].isBool()) {
            m_settings.confirmBeforeDownload = json["confirmBeforeDownload"].as<bool>().unwrapOr(true);
        }
        if (json.contains("confirmBeforeUpload") && json["confirmBeforeUpload"].isBool()) {
            m_settings.confirmBeforeUpload = json["confirmBeforeUpload"].as<bool>().unwrapOr(false);
        }
        if (json.contains("autoCheckIntegrity") && json["autoCheckIntegrity"].isBool()) {
            m_settings.autoCheckIntegrity = json["autoCheckIntegrity"].as<bool>().unwrapOr(true);
        }
        
        BetterSaveLogger::get()->info("Settings", "Settings loaded successfully");
        
    } catch (const std::exception& e) {
        BetterSaveLogger::get()->error("Settings", fmt::format("Failed to load settings: {}", e.what()));
    }
}

void SettingsManager::updateSettings(const BetterSaveSettings& newSettings) {
    m_settings = newSettings;
    saveToFile();
}

void SettingsManager::setAutoBackupEnabled(bool enabled) {
    m_settings.autoBackupEnabled = enabled;
    saveToFile();
}

void SettingsManager::setAutoBackupInterval(int minutes) {
    m_settings.autoBackupIntervalMinutes = minutes;
    saveToFile();
}

