/**
 * BetterSave - Backup History Popup
 * Created by: sidastuff
 */

#include "BackupHistoryPopup.hpp"
#include "BetterSaveLogger.hpp"

BackupHistoryPopup* BackupHistoryPopup::create() {
    auto ret = new BackupHistoryPopup();
    if (ret && ret->initAnchored(400.f, 300.f)) {
        ret->autorelease();
        return ret;
    }
    CC_SAFE_DELETE(ret);
    return nullptr;
}

bool BackupHistoryPopup::setup() {
    this->setTitle("Backup History");
    
    auto winSize = this->m_mainLayer->getContentSize();
    
    // Info label
    auto infoLabel = CCLabelBMFont::create("Select a backup to restore from:", "bigFont.fnt");
    infoLabel->setPosition(winSize.width / 2, winSize.height / 2 + 115);
    infoLabel->setScale(0.4f);
    this->m_mainLayer->addChild(infoLabel);
    
    // Scroll layer background
    auto scrollBG = CCScale9Sprite::create("square02b_001.png", { 0, 0, 80, 80 });
    scrollBG->setContentSize({ 360, 180 });
    scrollBG->setColor({ 0, 0, 0 });
    scrollBG->setOpacity(100);
    scrollBG->setPosition(winSize.width / 2, winSize.height / 2 + 10);
    this->m_mainLayer->addChild(scrollBG);
    
    // Create scroll layer
    m_contentMenu = CCMenu::create();
    m_contentMenu->setLayout(
        ColumnLayout::create()
            ->setAxisReverse(true)
            ->setAxisAlignment(AxisAlignment::End)
            ->setAutoGrowAxis(180.f)
            ->setGap(5.f)
    );
    
    m_scrollLayer = ScrollLayer::create({ 350, 170 });
    m_scrollLayer->setPosition({winSize.width / 2 - 175, winSize.height / 2 - 75});
    m_scrollLayer->m_contentLayer->addChild(m_contentMenu);
    this->m_mainLayer->addChild(m_scrollLayer);
    
    // Button menu
    auto buttonMenu = CCMenu::create();
    buttonMenu->setPosition(0, 0);
    this->m_mainLayer->addChild(buttonMenu);
    
    // Refresh button
    auto refreshBtn = CCMenuItemSpriteExtra::create(
        CCSprite::createWithSpriteFrameName("GJ_updateBtn_001.png"),
        this, menu_selector(BackupHistoryPopup::onRefresh)
    );
    refreshBtn->setPosition(winSize.width / 2 - 80, winSize.height / 2 - 115);
    buttonMenu->addChild(refreshBtn);
    
    // Clear button
    auto clearBtn = ButtonSprite::create("Clear History", "goldFont.fnt", "GJ_button_06.png", 0.7f);
    auto clearBtnItem = CCMenuItemSpriteExtra::create(
        clearBtn, this, menu_selector(BackupHistoryPopup::onClearHistory)
    );
    clearBtnItem->setPosition(winSize.width / 2 + 80, winSize.height / 2 - 115);
    buttonMenu->addChild(clearBtnItem);
    
    loadHistory();
    
    return true;
}

void BackupHistoryPopup::loadHistory() {
    m_contentMenu->removeAllChildren();
    
    auto history = BackupHistoryManager::get()->getHistory();
    
    if (history.empty()) {
        auto noHistoryLabel = CCLabelBMFont::create("No backup history yet.\n\nCreate backups to see them here!", "bigFont.fnt");
        noHistoryLabel->setScale(0.4f);
        noHistoryLabel->setAlignment(kCCTextAlignmentCenter);
        m_contentMenu->addChild(noHistoryLabel);
    } else {
        // Show newest first
        for (auto it = history.rbegin(); it != history.rend(); ++it) {
            const auto& entry = *it;
            
            // Create container for entry
            auto entryBG = CCScale9Sprite::create("square02_small.png");
            entryBG->setContentSize({ 330, 45 });
            entryBG->setOpacity(80);
            
            auto entryContainer = CCNode::create();
            entryContainer->setContentSize(entryBG->getContentSize());
            entryContainer->addChild(entryBG);
            entryBG->setPosition(entryContainer->getContentSize() / 2);
            
            // Slot name and timestamp
            std::string slotText = fmt::format("<cg>{}</c>", entry.slotName);
            if (entry.isAutoBackup) {
                slotText += " <cy>[Auto]</c>";
            }
            auto slotLabel = CCLabelBMFont::create(slotText.c_str(), "bigFont.fnt");
            slotLabel->setPosition(10, 30);
            slotLabel->setScale(0.4f);
            slotLabel->setAnchorPoint({0, 0.5f});
            entryContainer->addChild(slotLabel);
            
            auto timestampLabel = CCLabelBMFont::create(entry.timestamp.c_str(), "chatFont.fnt");
            timestampLabel->setPosition(10, 15);
            timestampLabel->setScale(0.6f);
            timestampLabel->setAnchorPoint({0, 0.5f});
            timestampLabel->setOpacity(180);
            entryContainer->addChild(timestampLabel);
            
            // Device and size info
            double totalSizeMB = (entry.gmSize + entry.llSize) / (1024.0 * 1024.0);
            auto infoLabel = CCLabelBMFont::create(
                fmt::format("{} - {:.2f}MB", entry.deviceName, totalSizeMB).c_str(),
                "chatFont.fnt"
            );
            infoLabel->setPosition(200, 22.5f);
            infoLabel->setScale(0.5f);
            infoLabel->setOpacity(180);
            entryContainer->addChild(infoLabel);
            
            // Select button
            auto selectBtn = ButtonSprite::create("Select", "goldFont.fnt", "GJ_button_01.png", 0.6f);
            auto selectItem = CCMenuItemSpriteExtra::create(
                selectBtn, this, menu_selector(BackupHistoryPopup::onSlotSelect)
            );
            selectItem->setUserData(reinterpret_cast<void*>(new std::string(entry.slotName)));
            selectItem->setPosition(280, 22.5f);
            
            auto itemMenu = CCMenu::create();
            itemMenu->setPosition(0, 0);
            itemMenu->addChild(selectItem);
            entryContainer->addChild(itemMenu);
            
            m_contentMenu->addChild(entryContainer);
        }
    }
    
    m_contentMenu->updateLayout();
    m_scrollLayer->moveToTop();
}

void BackupHistoryPopup::onSlotSelect(CCObject* sender) {
    auto btn = static_cast<CCMenuItemSpriteExtra*>(sender);
    auto slotName = *reinterpret_cast<std::string*>(btn->getUserData());
    
    m_selectedSlot = slotName;
    
    geode::createQuickPopup(
        "Restore Backup",
        fmt::format("Restore saves from backup <cy>{}</c>?\n\nThis will download and overwrite your current local saves, then restart the game.", slotName),
        "Cancel", "Restore",
        [this, slotName](auto, bool btn2) {
            if (btn2) {
                // Close this popup and trigger download with selected slot
                BetterSaveLogger::get()->info("History", fmt::format("Restoring from backup: {}", slotName));
                FLAlertLayer::create("Restore Started", fmt::format("Restoring from backup '{}'...", slotName).c_str(), "OK")->show();
                this->onClose(nullptr);
                // TODO: Integrate with download functionality
            }
        }
    );
}

void BackupHistoryPopup::onClearHistory(CCObject*) {
    geode::createQuickPopup(
        "Clear History",
        "This will clear all backup history records.\n<cy>Note: This won't delete your cloud backups.</c>",
        "Cancel", "Clear",
        [this](auto, bool btn2) {
            if (btn2) {
                BackupHistoryManager::get()->clearHistory();
                loadHistory();
                FLAlertLayer::create("History Cleared", "Backup history has been cleared!", "OK")->show();
            }
        }
    );
}

void BackupHistoryPopup::onRefresh(CCObject*) {
    BackupHistoryManager::get()->loadFromFile();
    loadHistory();
    FLAlertLayer::create("Refreshed", "Backup history has been refreshed!", "OK")->show();
}

