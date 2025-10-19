/**
 * BetterSave - Settings Popup
 * Created by: sidastuff
 */

#pragma once
#include <Geode/Geode.hpp>
#include <Geode/ui/Popup.hpp>
#include <Geode/ui/TextInput.hpp>

using namespace geode::prelude;

class SettingsPopup : public Popup<> {
protected:
    CCLabelBMFont* m_intervalLabel = nullptr;
    CCMenuItemToggler* m_autoBackupToggle = nullptr;
    CCMenuItemToggler* m_notificationsToggle = nullptr;
    CCMenuItemToggler* m_confirmDownloadToggle = nullptr;
    CCMenuItemToggler* m_confirmUploadToggle = nullptr;
    CCMenuItemToggler* m_integrityCheckToggle = nullptr;
    Slider* m_intervalSlider = nullptr;
    
    bool setup() override;
    void onClose(cocos2d::CCObject* sender) override;
    void onSave(CCObject*);
    void onIntervalChanged(CCObject*);
    
public:
    static SettingsPopup* create();
};

