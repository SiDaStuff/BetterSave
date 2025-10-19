/**
 * BetterSave - Settings Manager
 * Created by: sidastuff
 */

#pragma once
#include <Geode/Geode.hpp>
#include <string>

using namespace geode::prelude;

struct BetterSaveSettings {
    bool autoBackupEnabled = false;
    int autoBackupIntervalMinutes = 30;  // Default: 30 minutes
    bool showNotifications = true;
    bool confirmBeforeDownload = true;
    bool confirmBeforeUpload = false;
    bool autoCheckIntegrity = true;
};

class SettingsManager {
private:
    static SettingsManager* s_instance;
    BetterSaveSettings m_settings;
    std::filesystem::path m_settingsFilePath;
    
    void saveToFile();
    void loadFromFile();

public:
    static SettingsManager* get() {
        if (!s_instance) {
            s_instance = new SettingsManager();
        }
        return s_instance;
    }

    SettingsManager();
    
    BetterSaveSettings& getSettings() { return m_settings; }
    void updateSettings(const BetterSaveSettings& newSettings);
    
    // Convenience methods
    void setAutoBackupEnabled(bool enabled);
    void setAutoBackupInterval(int minutes);
};

