#pragma once
#include <Geode/Geode.hpp>
#include <Geode/ui/Popup.hpp>
#include "FirebaseAuth.hpp"

using namespace geode::prelude;

class LoginPopup : public Popup<> {
protected:
    TextInput* m_emailInput;
    TextInput* m_passwordInput;
    CCLabelBMFont* m_statusLabel;
    CCMenu* m_buttonMenu;
    bool m_isSignUpMode;

    bool setup() override;
    void onLogin(CCObject*);
    void onSignUp(CCObject*);
    void onToggleMode(CCObject*);
    void updateUI();
    void showStatus(const std::string& message, ccColor3B color);

public:
    static LoginPopup* create();
};

