/**
 * BetterSave - Account Manager Popup
 * Created by: sidastuff
 */

#pragma once
#include <Geode/Geode.hpp>
#include <Geode/ui/Popup.hpp>

using namespace geode::prelude;

class AccountManagerPopup : public Popup<> {
protected:
    CCLabelBMFont* m_statusLabel;
    
    bool setup() override;
    void onDeleteData(CCObject*);
    void onDeleteAccount(CCObject*);
    void showStatus(const std::string& message, ccColor3B color);
    
    void deleteAllUserData();
    void deleteFirebaseAccount();
    
public:
    static AccountManagerPopup* create();
};

