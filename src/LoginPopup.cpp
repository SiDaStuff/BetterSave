#include "LoginPopup.hpp"

LoginPopup* LoginPopup::create() {
    auto ret = new LoginPopup();
    if (ret && ret->initAnchored(360.f, 240.f)) {
        ret->autorelease();
        return ret;
    }
    CC_SAFE_DELETE(ret);
    return nullptr;
}

bool LoginPopup::setup() {
    m_isSignUpMode = false;
    
    this->setTitle("BetterSave Login - by sidastuff");
    
    auto winSize = this->m_mainLayer->getContentSize();
    
    // Email input
    auto emailBG = CCScale9Sprite::create("square02b_001.png", { 0, 0, 80, 80 });
    emailBG->setContentSize({ 280, 30 });
    emailBG->setColor({ 0, 0, 0 });
    emailBG->setOpacity(75);
    emailBG->setPosition(winSize.width / 2, winSize.height / 2 + 35);
    this->m_mainLayer->addChild(emailBG);
    
    m_emailInput = TextInput::create(270.f, "Email", "bigFont.fnt");
    m_emailInput->setPosition(winSize.width / 2, winSize.height / 2 + 35);
    m_emailInput->setMaxCharCount(100);
    // Allow all characters needed for emails: letters, numbers, @, ., _, -, +
    m_emailInput->setFilter("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789@._-+");
    this->m_mainLayer->addChild(m_emailInput);
    
    // Email label
    auto emailLabel = CCLabelBMFont::create("Email:", "bigFont.fnt");
    emailLabel->setPosition(winSize.width / 2 - 140, winSize.height / 2 + 60);
    emailLabel->setScale(0.4f);
    this->m_mainLayer->addChild(emailLabel);
    
    // Password input
    auto passwordBG = CCScale9Sprite::create("square02b_001.png", { 0, 0, 80, 80 });
    passwordBG->setContentSize({ 280, 30 });
    passwordBG->setColor({ 0, 0, 0 });
    passwordBG->setOpacity(75);
    passwordBG->setPosition(winSize.width / 2, winSize.height / 2 - 15);
    this->m_mainLayer->addChild(passwordBG);
    
    m_passwordInput = TextInput::create(270.f, "Password", "bigFont.fnt");
    m_passwordInput->setPosition(winSize.width / 2, winSize.height / 2 - 15);
    m_passwordInput->setMaxCharCount(100);
    m_passwordInput->setPasswordMode(true);
    // Allow all printable ASCII characters for passwords (includes special chars)
    m_passwordInput->setCommonFilter(CommonFilter::Any);
    this->m_mainLayer->addChild(m_passwordInput);
    
    // Password label
    auto passwordLabel = CCLabelBMFont::create("Password:", "bigFont.fnt");
    passwordLabel->setPosition(winSize.width / 2 - 140, winSize.height / 2 + 10);
    passwordLabel->setScale(0.4f);
    this->m_mainLayer->addChild(passwordLabel);
    
    // Status label
    m_statusLabel = CCLabelBMFont::create("", "goldFont.fnt");
    m_statusLabel->setPosition(winSize.width / 2, winSize.height / 2 - 55);
    m_statusLabel->setScale(0.4f);
    this->m_mainLayer->addChild(m_statusLabel);
    
    // Button menu
    m_buttonMenu = CCMenu::create();
    m_buttonMenu->setPosition(0, 0);
    this->m_mainLayer->addChild(m_buttonMenu);
    
    // Login button
    auto loginBtn = ButtonSprite::create("Login", "goldFont.fnt", "GJ_button_01.png", 0.8f);
    auto loginBtnItem = CCMenuItemSpriteExtra::create(
        loginBtn,
        this,
        menu_selector(LoginPopup::onLogin)
    );
    loginBtnItem->setPosition(winSize.width / 2 - 60, winSize.height / 2 - 80);
    m_buttonMenu->addChild(loginBtnItem);
    
    // Sign up button
    auto signUpBtn = ButtonSprite::create("Sign Up", "goldFont.fnt", "GJ_button_01.png", 0.8f);
    auto signUpBtnItem = CCMenuItemSpriteExtra::create(
        signUpBtn,
        this,
        menu_selector(LoginPopup::onSignUp)
    );
    signUpBtnItem->setPosition(winSize.width / 2 + 60, winSize.height / 2 - 80);
    m_buttonMenu->addChild(signUpBtnItem);
    
    // Toggle mode text
    auto toggleLabel = CCLabelBMFont::create("New user? Click Sign Up", "bigFont.fnt");
    toggleLabel->setPosition(winSize.width / 2, winSize.height / 2 - 105);
    toggleLabel->setScale(0.35f);
    toggleLabel->setOpacity(180);
    this->m_mainLayer->addChild(toggleLabel);
    
    return true;
}

void LoginPopup::onLogin(CCObject*) {
    std::string email = m_emailInput->getString();
    std::string password = m_passwordInput->getString();
    
    if (email.empty() || password.empty()) {
        showStatus("Please enter email and password", { 255, 100, 100 });
        return;
    }
    
    showStatus("Logging in...", { 255, 255, 255 });
    
    FirebaseAuth::get()->signIn(email, password, [this](bool success, const std::string& message) {
        if (success) {
            showStatus("Login successful!", { 100, 255, 100 });
            // Close popup after 1 second
            this->runAction(CCSequence::create(
                CCDelayTime::create(1.0f),
                CCCallFuncO::create(this, callfuncO_selector(Popup::onClose), nullptr),
                nullptr
            ));
        } else {
            showStatus(message, { 255, 100, 100 });
        }
    });
}

void LoginPopup::onSignUp(CCObject*) {
    std::string email = m_emailInput->getString();
    std::string password = m_passwordInput->getString();
    
    if (email.empty() || password.empty()) {
        showStatus("Please enter email and password", { 255, 100, 100 });
        return;
    }
    
    if (password.length() < 6) {
        showStatus("Password must be at least 6 characters", { 255, 100, 100 });
        return;
    }
    
    showStatus("Creating account...", { 255, 255, 255 });
    
    FirebaseAuth::get()->signUp(email, password, [this](bool success, const std::string& message) {
        if (success) {
            showStatus("Account created!", { 100, 255, 100 });
            // Close popup after 1 second
            this->runAction(CCSequence::create(
                CCDelayTime::create(1.0f),
                CCCallFuncO::create(this, callfuncO_selector(Popup::onClose), nullptr),
                nullptr
            ));
        } else {
            showStatus(message, { 255, 100, 100 });
        }
    });
}

void LoginPopup::onToggleMode(CCObject*) {
    m_isSignUpMode = !m_isSignUpMode;
    updateUI();
}

void LoginPopup::updateUI() {
    if (m_isSignUpMode) {
        this->setTitle("BetterSave Sign Up");
    } else {
        this->setTitle("BetterSave Login");
    }
}

void LoginPopup::showStatus(const std::string& message, ccColor3B color) {
    m_statusLabel->setString(message.c_str());
    m_statusLabel->setColor(color);
}

