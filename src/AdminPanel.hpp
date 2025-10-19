/**
 * BetterSave - Admin Panel
 * Only accessible by UID: SOpCkBJd8vWqxHRyhl8MimrLWc52
 * Created by: sidastuff
 */

#pragma once
#include <Geode/Geode.hpp>
#include <Geode/ui/Popup.hpp>
#include <Geode/ui/TextInput.hpp>

using namespace geode::prelude;

class AdminPanel : public Popup<> {
protected:
    TextInput* m_emailInput;
    CCLabelBMFont* m_statusLabel;
    
    bool setup() override;
    void onDownloadUserData(CCObject*);
    void onBanAccount(CCObject*);
    void onUnbanAccount(CCObject*);
    void onViewBannedList(CCObject*);
    void showStatus(const std::string& message, ccColor3B color);
    
    void downloadUserDataByEmail(const std::string& email);
    void banAccountByEmail(const std::string& email);
    void unbanAccountByEmail(const std::string& email);
    
    static std::string getUserIdByEmail(const std::string& email, const std::string& callback);
    
public:
    static AdminPanel* create();
    static bool isAdmin(const std::string& userId);
};

