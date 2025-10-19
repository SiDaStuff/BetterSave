#pragma once
#include <Geode/Geode.hpp>
#include <Geode/ui/Popup.hpp>
#include "FirebaseAuth.hpp"
#include "ProgressPopup.hpp"

using namespace geode::prelude;

class SaveManagerPopup : public Popup<> {
protected:
    CCLabelBMFont* m_statusLabel;
    CCMenu* m_buttonMenu;
    ProgressPopup* m_progressPopup = nullptr;
    bool m_autoRestartAfterUpload = false;

    bool setup() override;
    void onUpload(CCObject*);
    void onDownload(CCObject*);
    void onDownloadCustom(CCObject*);
    void onSettings(CCObject*);
    void onCheckIntegrity(CCObject*);
    void onLogout(CCObject*);
    void onAccountManager(CCObject*);
    void onAdminPanel(CCObject*);
    void showStatus(const std::string& message, ccColor3B color);
    void showInfoDialog(const std::string& title, const std::string& message);
    
    std::filesystem::path getGDSavePath();
    void downloadSaveData();
    void downloadToCustomLocation();
    void restartGame();
    void deleteOldDataBeforeUpload(std::function<void()> callback);
    
    static void downloadChunksParallel(const std::string& userId, const std::string& prefix,
                                        int totalChunks, std::function<void(std::string)> onComplete,
                                        ProgressPopup* progressPopup);

public:
    static SaveManagerPopup* create();
    
    // Public methods for external access
    void uploadSaveData(bool autoRestart = false);
    static void uploadChunksParallel(const std::vector<std::string>& chunks, const std::string& userId,
                                      const std::string& prefix, std::function<void()> onComplete,
                                      ProgressPopup* progressPopup);
};

