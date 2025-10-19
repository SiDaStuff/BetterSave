/**
 * BetterSave - Settings Popup
 * Created by: sidastuff
 */

#include "SettingsPopup.hpp"
#include "SettingsManager.hpp"
#include "BetterSaveLogger.hpp"

SettingsPopup* SettingsPopup::create() {
    auto ret = new SettingsPopup();
    if (ret && ret->initAnchored(420.f, 320.f)) {
        ret->autorelease();
        return ret;
    }
    CC_SAFE_DELETE(ret);
    return nullptr;
}

bool SettingsPopup::setup() {
    this->setTitle("BetterSave Settings - by sidastuff");
    
    auto winSize = this->m_mainLayer->getContentSize();
    auto& settings = SettingsManager::get()->getSettings();
    
    // Create menu
    auto menu = CCMenu::create();
    menu->setPosition(0, 0);
    this->m_mainLayer->addChild(menu);
    
    float yPos = winSize.height / 2 + 100;
    float leftX = 80.f;
    float rightX = winSize.width - 100.f;
    
    // Auto-backup toggle
    auto autoBackupLabel = CCLabelBMFont::create("Auto-Backup:", "bigFont.fnt");
    autoBackupLabel->setPosition(leftX, yPos);
    autoBackupLabel->setScale(0.4f);
    autoBackupLabel->setAnchorPoint({0, 0.5f});
    this->m_mainLayer->addChild(autoBackupLabel);
    
    auto autoBackupOffSpr = CCSprite::createWithSpriteFrameName("GJ_checkOff_001.png");
    auto autoBackupOnSpr = CCSprite::createWithSpriteFrameName("GJ_checkOn_001.png");
    m_autoBackupToggle = CCMenuItemToggler::create(
        autoBackupOffSpr, autoBackupOnSpr, this, nullptr
    );
    m_autoBackupToggle->toggle(settings.autoBackupEnabled);
    m_autoBackupToggle->setPosition(rightX, yPos);
    menu->addChild(m_autoBackupToggle);
    
    yPos -= 35;
    
    // Auto-backup interval slider
    auto intervalTitleLabel = CCLabelBMFont::create("Backup Interval:", "bigFont.fnt");
    intervalTitleLabel->setPosition(leftX, yPos);
    intervalTitleLabel->setScale(0.4f);
    intervalTitleLabel->setAnchorPoint({0, 0.5f});
    this->m_mainLayer->addChild(intervalTitleLabel);
    
    m_intervalLabel = CCLabelBMFont::create(fmt::format("{}min", settings.autoBackupIntervalMinutes).c_str(), "bigFont.fnt");
    m_intervalLabel->setPosition(rightX + 30, yPos);
    m_intervalLabel->setScale(0.35f);
    this->m_mainLayer->addChild(m_intervalLabel);
    
    yPos -= 20;
    
    m_intervalSlider = Slider::create(this, menu_selector(SettingsPopup::onIntervalChanged), 0.8f);
    m_intervalSlider->setPosition({winSize.width / 2, yPos});
    m_intervalSlider->setValue((settings.autoBackupIntervalMinutes - 10.f) / 110.f); // 10-120 minutes
    this->m_mainLayer->addChild(m_intervalSlider);
    
    yPos -= 30;
    
    // Show notifications toggle
    auto notifLabel = CCLabelBMFont::create("Show Notifications:", "bigFont.fnt");
    notifLabel->setPosition(leftX, yPos);
    notifLabel->setScale(0.4f);
    notifLabel->setAnchorPoint({0, 0.5f});
    this->m_mainLayer->addChild(notifLabel);
    
    auto notifOffSpr = CCSprite::createWithSpriteFrameName("GJ_checkOff_001.png");
    auto notifOnSpr = CCSprite::createWithSpriteFrameName("GJ_checkOn_001.png");
    m_notificationsToggle = CCMenuItemToggler::create(
        notifOffSpr, notifOnSpr, this, nullptr
    );
    m_notificationsToggle->toggle(settings.showNotifications);
    m_notificationsToggle->setPosition(rightX, yPos);
    menu->addChild(m_notificationsToggle);
    
    yPos -= 35;
    
    // Confirm before download toggle
    auto confirmDownloadLabel = CCLabelBMFont::create("Confirm Downloads:", "bigFont.fnt");
    confirmDownloadLabel->setPosition(leftX, yPos);
    confirmDownloadLabel->setScale(0.4f);
    confirmDownloadLabel->setAnchorPoint({0, 0.5f});
    this->m_mainLayer->addChild(confirmDownloadLabel);
    
    auto confirmDownloadOffSpr = CCSprite::createWithSpriteFrameName("GJ_checkOff_001.png");
    auto confirmDownloadOnSpr = CCSprite::createWithSpriteFrameName("GJ_checkOn_001.png");
    m_confirmDownloadToggle = CCMenuItemToggler::create(
        confirmDownloadOffSpr, confirmDownloadOnSpr, this, nullptr
    );
    m_confirmDownloadToggle->toggle(settings.confirmBeforeDownload);
    m_confirmDownloadToggle->setPosition(rightX, yPos);
    menu->addChild(m_confirmDownloadToggle);
    
    yPos -= 35;
    
    // Confirm before upload toggle
    auto confirmUploadLabel = CCLabelBMFont::create("Confirm Uploads:", "bigFont.fnt");
    confirmUploadLabel->setPosition(leftX, yPos);
    confirmUploadLabel->setScale(0.4f);
    confirmUploadLabel->setAnchorPoint({0, 0.5f});
    this->m_mainLayer->addChild(confirmUploadLabel);
    
    auto confirmUploadOffSpr = CCSprite::createWithSpriteFrameName("GJ_checkOff_001.png");
    auto confirmUploadOnSpr = CCSprite::createWithSpriteFrameName("GJ_checkOn_001.png");
    m_confirmUploadToggle = CCMenuItemToggler::create(
        confirmUploadOffSpr, confirmUploadOnSpr, this, nullptr
    );
    m_confirmUploadToggle->toggle(settings.confirmBeforeUpload);
    m_confirmUploadToggle->setPosition(rightX, yPos);
    menu->addChild(m_confirmUploadToggle);
    
    yPos -= 35;
    
    // Auto integrity check toggle
    auto integrityLabel = CCLabelBMFont::create("Auto Integrity Check:", "bigFont.fnt");
    integrityLabel->setPosition(leftX, yPos);
    integrityLabel->setScale(0.4f);
    integrityLabel->setAnchorPoint({0, 0.5f});
    this->m_mainLayer->addChild(integrityLabel);
    
    auto integrityOffSpr = CCSprite::createWithSpriteFrameName("GJ_checkOff_001.png");
    auto integrityOnSpr = CCSprite::createWithSpriteFrameName("GJ_checkOn_001.png");
    m_integrityCheckToggle = CCMenuItemToggler::create(
        integrityOffSpr, integrityOnSpr, this, nullptr
    );
    m_integrityCheckToggle->toggle(settings.autoCheckIntegrity);
    m_integrityCheckToggle->setPosition(rightX, yPos);
    menu->addChild(m_integrityCheckToggle);
    
    // Save button
    auto saveBtn = ButtonSprite::create("Save Settings", "goldFont.fnt", "GJ_button_01.png", 0.8f);
    auto saveBtnItem = CCMenuItemSpriteExtra::create(
        saveBtn, this, menu_selector(SettingsPopup::onSave)
    );
    saveBtnItem->setPosition(winSize.width / 2, winSize.height / 2 - 130);
    menu->addChild(saveBtnItem);
    
    return true;
}

void SettingsPopup::onIntervalChanged(CCObject*) {
    int interval = static_cast<int>(m_intervalSlider->getValue() * 110.f) + 10; // 10-120 minutes
    m_intervalLabel->setString(fmt::format("{}min", interval).c_str());
}

void SettingsPopup::onSave(CCObject*) {
    auto& settings = SettingsManager::get()->getSettings();
    
    settings.autoBackupEnabled = m_autoBackupToggle->isToggled();
    settings.autoBackupIntervalMinutes = static_cast<int>(m_intervalSlider->getValue() * 110.f) + 10;
    settings.showNotifications = m_notificationsToggle->isToggled();
    settings.confirmBeforeDownload = m_confirmDownloadToggle->isToggled();
    settings.confirmBeforeUpload = m_confirmUploadToggle->isToggled();
    settings.autoCheckIntegrity = m_integrityCheckToggle->isToggled();
    
    SettingsManager::get()->updateSettings(settings);
    
    BetterSaveLogger::get()->success("Settings", "Settings saved successfully!");
    FLAlertLayer::create("Settings Saved", "Your BetterSave settings have been saved!", "OK")->show();
    
    this->onClose(nullptr);
}

void SettingsPopup::onClose(cocos2d::CCObject* sender) {
    Popup::onClose(sender);
}

