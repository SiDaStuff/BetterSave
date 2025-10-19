/**
 * BetterSave - Account Manager Popup
 * Created by: sidastuff
 */

#include "AccountManagerPopup.hpp"
#include "FirebaseAuth.hpp"
#include "BetterSaveLogger.hpp"

AccountManagerPopup* AccountManagerPopup::create() {
    auto ret = new AccountManagerPopup();
    if (ret && ret->initAnchored(380.f, 280.f)) {
        ret->autorelease();
        return ret;
    }
    CC_SAFE_DELETE(ret);
    return nullptr;
}

bool AccountManagerPopup::setup() {
    this->setTitle("Account Manager");
    
    auto winSize = this->m_mainLayer->getContentSize();
    
    // User info label
    std::string userEmail = FirebaseAuth::get()->getEmail();
    std::string userId = FirebaseAuth::get()->getUserId();
    
    auto infoLabel = CCLabelBMFont::create(
        fmt::format("Email: {}\nUser ID: {}...", 
            userEmail,
            userId.substr(0, 12)).c_str(),
        "bigFont.fnt"
    );
    infoLabel->setPosition(winSize.width / 2, winSize.height / 2 + 70);
    infoLabel->setScale(0.35f);
    infoLabel->setAlignment(CCTextAlignment::kCCTextAlignmentCenter);
    this->m_mainLayer->addChild(infoLabel);
    
    // Warning label
    auto warningLabel = CCLabelBMFont::create(
        "DANGER ZONE",
        "goldFont.fnt"
    );
    warningLabel->setPosition(winSize.width / 2, winSize.height / 2 + 30);
    warningLabel->setScale(0.5f);
    warningLabel->setColor({255, 100, 100});
    this->m_mainLayer->addChild(warningLabel);
    
    // Status label
    m_statusLabel = CCLabelBMFont::create("", "bigFont.fnt");
    m_statusLabel->setPosition(winSize.width / 2, winSize.height / 2 - 70);
    m_statusLabel->setScale(0.4f);
    m_statusLabel->setAlignment(CCTextAlignment::kCCTextAlignmentCenter);
    this->m_mainLayer->addChild(m_statusLabel);
    
    // Button menu
    auto buttonMenu = CCMenu::create();
    buttonMenu->setPosition({0, 0});
    
    // Delete Data button
    auto deleteDataBtn = CCMenuItemSpriteExtra::create(
        ButtonSprite::create("Delete All Data", "goldFont.fnt", "GJ_button_05.png", 0.8f),
        this,
        menu_selector(AccountManagerPopup::onDeleteData)
    );
    deleteDataBtn->setPosition(winSize.width / 2, winSize.height / 2 - 10);
    buttonMenu->addChild(deleteDataBtn);
    
    // Delete Account button
    auto deleteAccountBtn = CCMenuItemSpriteExtra::create(
        ButtonSprite::create("Delete Account", "goldFont.fnt", "GJ_button_06.png", 0.8f),
        this,
        menu_selector(AccountManagerPopup::onDeleteAccount)
    );
    deleteAccountBtn->setPosition(winSize.width / 2, winSize.height / 2 - 50);
    buttonMenu->addChild(deleteAccountBtn);
    
    this->m_mainLayer->addChild(buttonMenu);
    
    return true;
}

void AccountManagerPopup::onDeleteData(CCObject*) {
    geode::createQuickPopup(
        "Delete All Data",
        "<cr>WARNING:</c> This will permanently delete\n"
        "all your cloud save data!\n\n"
        "This action <cy>CANNOT be undone</c>.\n\n"
        "Continue?",
        "Cancel", "Delete",
        [this](auto, bool btn2) {
            if (btn2) {
                this->deleteAllUserData();
            }
        }
    );
}

void AccountManagerPopup::onDeleteAccount(CCObject*) {
    geode::createQuickPopup(
        "Delete Account",
        "<cr>DANGER:</c> This will permanently:\n"
        "• Delete all your cloud save data\n"
        "• Delete your Firebase account\n"
        "• Log you out\n\n"
        "This action <cy>CANNOT be undone</c>!\n\n"
        "Are you absolutely sure?",
        "Cancel", "DELETE",
        [this](auto, bool btn2) {
            if (btn2) {
                // Final confirmation
                geode::createQuickPopup(
                    "FINAL WARNING",
                    "<cr>Last chance!</c>\n\n"
                    "Delete account and all data forever?",
                    "Cancel", "YES, DELETE",
                    [this](auto, bool finalBtn) {
                        if (finalBtn) {
                            this->deleteAllUserData();
                            this->deleteFirebaseAccount();
                        }
                    }
                );
            }
        }
    );
}

void AccountManagerPopup::deleteAllUserData() {
    showStatus("Deleting data...", {255, 255, 100});
    
    std::string userId = FirebaseAuth::get()->getUserId();
    std::string token = FirebaseAuth::get()->getIdToken();
    
    BetterSaveLogger::get()->info("AccountManager", "Deleting all user data");
    
    // Delete saveData
    std::string saveDataUrl = fmt::format(
        "https://gdbettersave-default-rtdb.firebaseio.com/users/{}/saveData.json?auth={}",
        userId, token
    );
    
    // Delete by setting to null (Firebase REST API)
    web::WebRequest req1 = web::WebRequest();
    req1.userAgent("");
    matjson::Value nullValue;
    req1.bodyJSON(nullValue);
    
    req1.patch(saveDataUrl).listen([this, userId, token](web::WebResponse* resp) {
        if (resp->ok()) {
            BetterSaveLogger::get()->info("AccountManager", "Deleted saveData");
            
            // Delete chunks
            std::string chunksUrl = fmt::format(
                "https://gdbettersave-default-rtdb.firebaseio.com/users/{}/chunks.json?auth={}",
                userId, token
            );
            
            // Delete by setting to null (Firebase REST API)
            web::WebRequest req2 = web::WebRequest();
            req2.userAgent("");
            matjson::Value nullValue;
            req2.bodyJSON(nullValue);
            
            req2.patch(chunksUrl).listen([this](web::WebResponse* resp2) {
                if (resp2->ok()) {
                    showStatus("All data deleted successfully!", {100, 255, 100});
                    BetterSaveLogger::get()->success("AccountManager", "All user data deleted");
                    FLAlertLayer::create("Success", "All your cloud data has been deleted.", "OK")->show();
                } else {
                    showStatus("Error deleting chunks", {255, 100, 100});
                    BetterSaveLogger::get()->error("AccountManager", "Failed to delete chunks");
                }
            });
        } else {
            showStatus("Error deleting data", {255, 100, 100});
            BetterSaveLogger::get()->error("AccountManager", "Failed to delete saveData");
        }
    });
}

void AccountManagerPopup::deleteFirebaseAccount() {
    showStatus("Deleting account...", {255, 255, 100});
    
    std::string token = FirebaseAuth::get()->getIdToken();
    std::string apiKey = "AIzaSyDK_EXyqoqnZXVq0fzL5d1_s6z9vxzEuY4"; // Same as in FirebaseAuth
    
    std::string url = fmt::format(
        "https://identitytoolkit.googleapis.com/v1/accounts:delete?key={}",
        apiKey
    );
    
    matjson::Value body;
    body["idToken"] = token;
    
    web::WebRequest req = web::WebRequest();
    req.userAgent("");
    req.bodyJSON(body);
    
    BetterSaveLogger::get()->info("AccountManager", "Deleting Firebase account");
    
    req.post(url).listen([this](web::WebResponse* resp) {
        if (resp->ok()) {
            showStatus("Account deleted!", {100, 255, 100});
            BetterSaveLogger::get()->success("AccountManager", "Firebase account deleted");
            
            // Log out
            FirebaseAuth::get()->signOut();
            
            // Show success and close
            FLAlertLayer::create(
                "Account Deleted",
                "Your account and all data have been\npermanently deleted.\n\nYou have been logged out.",
                "OK"
            )->show();
            
            this->onClose(nullptr);
        } else {
            showStatus("Error deleting account", {255, 100, 100});
            auto error = resp->string().unwrapOr("Unknown error");
            BetterSaveLogger::get()->error("AccountManager", fmt::format("Failed to delete account: {}", error));
            
            FLAlertLayer::create(
                "Error",
                "Failed to delete account.\nPlease try again later.",
                "OK"
            )->show();
        }
    });
}

void AccountManagerPopup::showStatus(const std::string& message, ccColor3B color) {
    m_statusLabel->setString(message.c_str());
    m_statusLabel->setColor(color);
}

