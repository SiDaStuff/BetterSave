/**
 * BetterSave - Progress Popup for Upload/Download Operations
 * Created by: sidastuff
 */

#include "ProgressPopup.hpp"

ProgressPopup* ProgressPopup::create(const std::string& title, bool fullscreen) {
    auto ret = new ProgressPopup();
    if (ret && ret->initAnchored(320.f, 200.f)) {
        ret->autorelease();
        ret->m_title = title;
        ret->m_fullscreenMode = fullscreen;
        ret->setTitle(title);
        return ret;
    }
    CC_SAFE_DELETE(ret);
    return nullptr;
}

void ProgressPopup::setTitle(const std::string& title) {
    m_title = title;
    Popup::setTitle(title);
}

void ProgressPopup::hideAllLayers() {
    auto scene = CCDirector::sharedDirector()->getRunningScene();
    if (!scene) return;
    
    // Hide all children of the scene except this popup
    CCObject* child;
    CCARRAY_FOREACH(scene->getChildren(), child) {
        auto node = dynamic_cast<CCNode*>(child);
        if (node && node != this && node->isVisible()) {
            m_hiddenLayers.push_back(node);
            node->setVisible(false);
        }
    }
}

void ProgressPopup::restoreAllLayers() {
    // Restore all previously hidden layers
    for (auto layer : m_hiddenLayers) {
        if (layer) {
            layer->setVisible(true);
        }
    }
    m_hiddenLayers.clear();
}

bool ProgressPopup::setup() {
    Popup::setTitle("BetterSave - by sidastuff");
    
    auto winSize = this->m_mainLayer->getContentSize();
    
    // If fullscreen mode, hide all background layers and create black background
    if (m_fullscreenMode) {
        // Hide all scene layers
        hideAllLayers();
        
        // Create a black background that covers everything
        auto screenSize = CCDirector::sharedDirector()->getWinSize();
        m_blackBackground = CCLayerColor::create({0, 0, 0, 255}, screenSize.width, screenSize.height);
        m_blackBackground->setZOrder(-1);
        this->addChild(m_blackBackground);
        
        // Hide the popup background and border
        if (this->m_bgSprite) {
            this->m_bgSprite->setVisible(false);
        }
    }
    
    // Warning label at top (red)
    m_warningLabel = CCLabelBMFont::create(
        "DO NOT CLOSE THIS WINDOW\nOperation in progress...",
        "bigFont.fnt"
    );
    m_warningLabel->setPosition(winSize.width / 2, winSize.height / 2 + 50);
    m_warningLabel->setScale(0.4f);
    m_warningLabel->setColor({255, 100, 100});
    m_warningLabel->setAlignment(CCTextAlignment::kCCTextAlignmentCenter);
    this->m_mainLayer->addChild(m_warningLabel);
    
    // Status label
    m_statusLabel = CCLabelBMFont::create("Starting...", "bigFont.fnt");
    m_statusLabel->setPosition(winSize.width / 2, winSize.height / 2 + 10);
    m_statusLabel->setScale(0.45f);
    m_statusLabel->setAlignment(CCTextAlignment::kCCTextAlignmentCenter);
    this->m_mainLayer->addChild(m_statusLabel);
    
    // Progress label
    m_progressLabel = CCLabelBMFont::create("0 / 0", "goldFont.fnt");
    m_progressLabel->setPosition(winSize.width / 2, winSize.height / 2 - 30);
    m_progressLabel->setScale(0.5f);
    this->m_mainLayer->addChild(m_progressLabel);
    
    // Info label at bottom
    auto infoLabel = CCLabelBMFont::create(
        "Please wait, this may take a moment...",
        "bigFont.fnt"
    );
    infoLabel->setPosition(winSize.width / 2, winSize.height / 2 - 65);
    infoLabel->setScale(0.35f);
    infoLabel->setColor({200, 200, 200});
    this->m_mainLayer->addChild(infoLabel);
    
    // Hide the default close button
    if (this->m_closeBtn) {
        this->m_closeBtn->setVisible(false);
    }
    
    return true;
}

void ProgressPopup::setStatus(const std::string& message, ccColor3B color) {
    m_statusLabel->setString(message.c_str());
    m_statusLabel->setColor(color);
}

void ProgressPopup::setProgress(int current, int total) {
    m_progressLabel->setString(fmt::format("{} / {}", current, total).c_str());
}

void ProgressPopup::allowClose(bool allow) {
    m_canClose = allow;
}

void ProgressPopup::enableCloseButton() {
    m_canClose = true;
    if (this->m_closeBtn) {
        this->m_closeBtn->setVisible(true);
    }
    m_warningLabel->setVisible(false);
}

void ProgressPopup::setFullscreenMode(bool fullscreen) {
    m_fullscreenMode = fullscreen;
}

void ProgressPopup::closePopup() {
    // Restore layers before closing if in fullscreen mode
    if (m_fullscreenMode) {
        restoreAllLayers();
    }
    
    // Force close
    m_canClose = true;
    this->onClose(nullptr);
}

void ProgressPopup::onClose(CCObject* sender) {
    if (!m_canClose) {
        FLAlertLayer::create(
            "Cannot Close",
            "<cr>WARNING:</c> An operation is in progress!\n\n"
            "Closing this window now <cy>may corrupt your save data</c>.\n\n"
            "Please wait for the operation to complete.",
            "OK"
        )->show();
        return;
    }
    
    // Restore layers before closing if in fullscreen mode
    if (m_fullscreenMode) {
        restoreAllLayers();
    }
    
    Popup::onClose(sender);
}

