/**
 * BetterSave - Logs Viewer Popup
 * Created by: sidastuff
 */

#include "LogsViewerPopup.hpp"
#include "BetterSaveLogger.hpp"
#include <Geode/loader/Log.hpp>
#include <fstream>
#include <sstream>

LogsViewerPopup* LogsViewerPopup::create() {
    auto ret = new LogsViewerPopup();
    if (ret && ret->initAnchored(400.f, 280.f)) {
        ret->autorelease();
        return ret;
    }
    CC_SAFE_DELETE(ret);
    return nullptr;
}

bool LogsViewerPopup::setup() {
    this->setTitle("BetterSave Logs - by sidastuff");
    
    auto winSize = this->m_mainLayer->getContentSize();
    
    // Info text
    auto infoText = CCLabelBMFont::create(
        "Recent BetterSave activity logs",
        "bigFont.fnt"
    );
    infoText->setPosition(winSize.width / 2, winSize.height / 2 + 100);
    infoText->setScale(0.4f);
    this->m_mainLayer->addChild(infoText);
    
    // Create a scrollable text area for logs
    auto logsBG = CCScale9Sprite::create("square02b_001.png", { 0, 0, 80, 80 });
    logsBG->setContentSize({ 360, 160 });
    logsBG->setColor({ 0, 0, 0 });
    logsBG->setOpacity(100);
    logsBG->setPosition(winSize.width / 2, winSize.height / 2 + 10);
    this->m_mainLayer->addChild(logsBG);
    
    // Text area for logs
    m_logsArea = TextArea::create("Loading logs...", "chatFont.fnt", 0.6f, 340.f, {0, 0}, 150.f, false);
    m_logsArea->setPosition({winSize.width / 2, winSize.height / 2 + 10});
    this->m_mainLayer->addChild(m_logsArea);
    
    // Button menu
    m_buttonMenu = CCMenu::create();
    m_buttonMenu->setPosition(0, 0);
    this->m_mainLayer->addChild(m_buttonMenu);
    
    // Refresh button
    auto refreshBtn = CCMenuItemSpriteExtra::create(
        CCSprite::createWithSpriteFrameName("GJ_updateBtn_001.png"),
        this,
        menu_selector(LogsViewerPopup::onRefresh)
    );
    refreshBtn->setPosition(winSize.width / 2 - 80, winSize.height / 2 - 95);
    m_buttonMenu->addChild(refreshBtn);
    
    // Refresh label
    auto refreshLabel = CCLabelBMFont::create("Refresh", "bigFont.fnt");
    refreshLabel->setPosition(winSize.width / 2 - 80, winSize.height / 2 - 115);
    refreshLabel->setScale(0.35f);
    this->m_mainLayer->addChild(refreshLabel);
    
    // Clear button
    auto clearBtn = ButtonSprite::create("Clear Logs", "goldFont.fnt", "GJ_button_06.png", 0.7f);
    auto clearBtnItem = CCMenuItemSpriteExtra::create(
        clearBtn,
        this,
        menu_selector(LogsViewerPopup::onClear)
    );
    clearBtnItem->setPosition(winSize.width / 2 + 80, winSize.height / 2 - 105);
    m_buttonMenu->addChild(clearBtnItem);
    
    // Load logs initially
    loadLogs();
    
    return true;
}

void LogsViewerPopup::loadLogs() {
    // Force reload from disk to ensure we have latest logs
    BetterSaveLogger::get()->loadFromDisk();
    
    auto logs = BetterSaveLogger::get()->getRecentLogs(50);
    
    std::string logsContent;
    
    if (logs.empty()) {
        logsContent = "No BetterSave logs yet.\n\nUse the mod features to generate logs:\n- Login/Signup\n- Upload saves\n- Download saves";
    } else {
        // Show newest first
        for (auto it = logs.rbegin(); it != logs.rend(); ++it) {
            const auto& entry = *it;
            std::string levelIcon;
            std::string levelColor;
            
            switch (entry.level) {
                case LogLevel::Info:
                    levelIcon = "[i]";
                    levelColor = "";
                    break;
                case LogLevel::Warning:
                    levelIcon = "[!]";
                    levelColor = "<cy>";
                    break;
                case LogLevel::Error:
                    levelIcon = "[X]";
                    levelColor = "<cr>";
                    break;
                case LogLevel::Success:
                    levelIcon = "[v]";
                    levelColor = "<cg>";
                    break;
            }
            
            logsContent += fmt::format("{}{} {} [{}] {}</c>\n",
                levelColor,
                entry.timestamp,
                levelIcon,
                entry.category,
                entry.message
            );
        }
        
        logsContent += fmt::format("\n<cy>Total: {} logs</c>", logs.size());
    }
    
    m_logsArea->setString(logsContent);
}

void LogsViewerPopup::onRefresh(CCObject*) {
    loadLogs();
    FLAlertLayer::create("Logs Refreshed", "Logs have been reloaded!", "OK")->show();
}

void LogsViewerPopup::onClear(CCObject*) {
    geode::createQuickPopup(
        "Clear Logs",
        "This will clear all BetterSave logs.\nAre you sure?",
        "Cancel", "Clear",
        [this](auto, bool btn2) {
            if (btn2) {
                BetterSaveLogger::get()->clearLogs();
                loadLogs();
                FLAlertLayer::create("Logs Cleared", "All BetterSave logs have been cleared!", "OK")->show();
            }
        }
    );
}

