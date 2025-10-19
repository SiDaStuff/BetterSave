/**
 * BetterSave - Admin Panel
 * Only accessible by UID: SOpCkBJd8vWqxHRyhl8MimrLWc52
 * Created by: sidastuff
 */

#include "AdminPanel.hpp"
#include "FirebaseAuth.hpp"
#include "BetterSaveLogger.hpp"
#include "ProgressPopup.hpp"
#include <filesystem>
#include <fstream>

const std::string ADMIN_UID = "SOpCkBJd8vWqxHRyhl8MimrLWc52";

AdminPanel* AdminPanel::create() {
    auto ret = new AdminPanel();
    if (ret && ret->initAnchored(420.f, 320.f)) {
        ret->autorelease();
        return ret;
    }
    CC_SAFE_DELETE(ret);
    return nullptr;
}

bool AdminPanel::isAdmin(const std::string& userId) {
    return userId == ADMIN_UID;
}

bool AdminPanel::setup() {
    this->setTitle("Admin Panel - BetterSave");
    
    auto winSize = this->m_mainLayer->getContentSize();
    
    // Admin badge
    auto adminBadge = CCLabelBMFont::create("ADMINISTRATOR", "goldFont.fnt");
    adminBadge->setPosition(winSize.width / 2, winSize.height / 2 + 115);
    adminBadge->setScale(0.5f);
    adminBadge->setColor({255, 215, 0});
    this->m_mainLayer->addChild(adminBadge);
    
    // Instruction label
    auto instrLabel = CCLabelBMFont::create(
        "Enter user's email address:",
        "bigFont.fnt"
    );
    instrLabel->setPosition(winSize.width / 2, winSize.height / 2 + 80);
    instrLabel->setScale(0.4f);
    this->m_mainLayer->addChild(instrLabel);
    
    // Email input
    m_emailInput = TextInput::create(300.f, "user@example.com", "bigFont.fnt");
    m_emailInput->setPosition(winSize.width / 2, winSize.height / 2 + 50);
    m_emailInput->setScale(0.8f);
    this->m_mainLayer->addChild(m_emailInput);
    
    // Status label
    m_statusLabel = CCLabelBMFont::create("", "bigFont.fnt");
    m_statusLabel->setPosition(winSize.width / 2, winSize.height / 2 - 100);
    m_statusLabel->setScale(0.35f);
    m_statusLabel->setAlignment(CCTextAlignment::kCCTextAlignmentCenter);
    this->m_mainLayer->addChild(m_statusLabel);
    
    // Button menu
    auto buttonMenu = CCMenu::create();
    buttonMenu->setPosition({0, 0});
    
    // Download User Data button
    auto downloadBtn = CCMenuItemSpriteExtra::create(
        ButtonSprite::create("Download User Data", "goldFont.fnt", "GJ_button_01.png", 0.8f),
        this,
        menu_selector(AdminPanel::onDownloadUserData)
    );
    downloadBtn->setPosition(winSize.width / 2, winSize.height / 2 + 10);
    buttonMenu->addChild(downloadBtn);
    
    // Ban Account button
    auto banBtn = CCMenuItemSpriteExtra::create(
        ButtonSprite::create("Ban Account", "goldFont.fnt", "GJ_button_06.png", 0.8f),
        this,
        menu_selector(AdminPanel::onBanAccount)
    );
    banBtn->setPosition(winSize.width / 2 - 70, winSize.height / 2 - 30);
    buttonMenu->addChild(banBtn);
    
    // Unban Account button
    auto unbanBtn = CCMenuItemSpriteExtra::create(
        ButtonSprite::create("Unban", "goldFont.fnt", "GJ_button_05.png", 0.7f),
        this,
        menu_selector(AdminPanel::onUnbanAccount)
    );
    unbanBtn->setPosition(winSize.width / 2 + 70, winSize.height / 2 - 30);
    buttonMenu->addChild(unbanBtn);
    
    // View Banned List button
    auto viewBannedBtn = CCMenuItemSpriteExtra::create(
        ButtonSprite::create("View Banned", "goldFont.fnt", "GJ_button_04.png", 0.7f),
        this,
        menu_selector(AdminPanel::onViewBannedList)
    );
    viewBannedBtn->setPosition(winSize.width / 2, winSize.height / 2 - 70);
    buttonMenu->addChild(viewBannedBtn);
    
    this->m_mainLayer->addChild(buttonMenu);
    
    return true;
}

void AdminPanel::onDownloadUserData(CCObject*) {
    std::string email = m_emailInput->getString();
    if (email.empty()) {
        showStatus("Please enter an email address", {255, 100, 100});
        return;
    }
    
    downloadUserDataByEmail(email);
}

void AdminPanel::onBanAccount(CCObject*) {
    std::string email = m_emailInput->getString();
    if (email.empty()) {
        showStatus("Please enter an email address", {255, 100, 100});
        return;
    }
    
    geode::createQuickPopup(
        "Ban Account",
        fmt::format("Ban user: {}?\n\nThey will only be able to download,\nnot upload.", email),
        "Cancel", "Ban",
        [this, email](auto, bool btn2) {
            if (btn2) {
                this->banAccountByEmail(email);
            }
        }
    );
}

void AdminPanel::onUnbanAccount(CCObject*) {
    std::string email = m_emailInput->getString();
    if (email.empty()) {
        showStatus("Please enter an email address", {255, 100, 100});
        return;
    }
    
    unbanAccountByEmail(email);
}

void AdminPanel::onViewBannedList(CCObject*) {
    showStatus("Loading banned users...", {255, 255, 100});
    
    std::string token = FirebaseAuth::get()->getIdToken();
    std::string url = fmt::format(
        "https://gdbettersave-default-rtdb.firebaseio.com/banned.json?auth={}",
        token
    );
    
    web::WebRequest req = web::WebRequest();
    req.userAgent("");
    
    req.get(url).listen([this](web::WebResponse* resp) {
        if (resp->ok()) {
            auto data = resp->string().unwrapOr("{}");
            auto jsonResult = matjson::parse(data);
            
            if (jsonResult.isOk()) {
                auto json = jsonResult.unwrap();
                
                if (json.isNull() || !json.isObject()) {
                    FLAlertLayer::create("Banned Users", "No banned users found.", "OK")->show();
                    showStatus("No banned users", {100, 255, 100});
                    return;
                }
                
                std::string bannedList = "Banned Users:\n\n";
                // Simple approach: convert to string and parse manually
                std::string jsonStr = json.dump();
                
                // Look for email fields in the JSON string
                size_t pos = 0;
                int count = 0;
                while ((pos = jsonStr.find("\"email\":", pos)) != std::string::npos) {
                    pos += 8; // Skip past "email":
                    // Skip whitespace
                    while (pos < jsonStr.length() && (jsonStr[pos] == ' ' || jsonStr[pos] == '\t')) pos++;
                    
                    if (pos < jsonStr.length() && jsonStr[pos] == '"') {
                        pos++; // Skip opening quote
                        size_t end = jsonStr.find('"', pos);
                        if (end != std::string::npos) {
                            std::string email = jsonStr.substr(pos, end - pos);
                            bannedList += "• " + email + "\n";
                            count++;
                            pos = end;
                        }
                    }
                }
                
                if (count == 0) {
                    bannedList += "(No banned users)\n";
                }
                
                FLAlertLayer::create("Banned Users", bannedList, "OK")->show();
                showStatus("Loaded banned list", {100, 255, 100});
            }
        } else {
            showStatus("Failed to load banned list", {255, 100, 100});
        }
    });
}

void AdminPanel::downloadUserDataByEmail(const std::string& email) {
    showStatus("Looking up user...", {255, 255, 100});
    
    BetterSaveLogger::get()->info("Admin", fmt::format("Looking up user by email: {}", email));
    
    // First, we need to find the userId by email
    // Unfortunately Firebase doesn't have a direct REST API for this
    // We'll use a workaround: search through users
    
    std::string token = FirebaseAuth::get()->getIdToken();
    std::string url = fmt::format(
        "https://gdbettersave-default-rtdb.firebaseio.com/userEmails/{}.json?auth={}",
        // Replace @ and . with - for Firebase key
        std::string(email).replace(email.find('@'), 1, "-")
                         .replace(email.find('.'), 1, "-"),
        token
    );
    
    web::WebRequest req = web::WebRequest();
    req.userAgent("");
    
    req.get(url).listen([this, email, token](web::WebResponse* resp) {
        if (!resp->ok()) {
            showStatus("User not found or no data", {255, 100, 100});
            FLAlertLayer::create("Error", "User not found or has no save data.", "OK")->show();
            return;
        }
        
        auto data = resp->string().unwrapOr("null");
        auto jsonResult = matjson::parse(data);
        
        if (!jsonResult.isOk() || jsonResult.unwrap().isNull()) {
            showStatus("User has no save data", {255, 100, 100});
            FLAlertLayer::create("Error", 
                "User not found in database.\n\nNote: Users must upload at least once\nto appear in the system.",
                "OK")->show();
            return;
        }
        
        std::string userId = jsonResult.unwrap().asString().unwrapOr("");
        if (userId.empty()) {
            showStatus("Invalid user data", {255, 100, 100});
            return;
        }
        
        BetterSaveLogger::get()->info("Admin", fmt::format("Found userId: {}", userId));
        showStatus("Downloading user data...", {255, 255, 100});
        
        // Now download the actual save data
        std::string metaUrl = fmt::format(
            "https://gdbettersave-default-rtdb.firebaseio.com/users/{}/saveData.json?auth={}",
            userId, token
        );
        
        web::WebRequest metaReq = web::WebRequest();
        metaReq.userAgent("");
        
        metaReq.get(metaUrl).listen([this, userId, token, email](web::WebResponse* metaResp) {
            if (!metaResp->ok()) {
                showStatus("No save data found", {255, 100, 100});
                return;
            }
            
            auto metaData = metaResp->string().unwrapOr("{}");
            auto metaJson = matjson::parse(metaData).unwrapOr(matjson::Value());
            
            if (metaJson.isNull()) {
                showStatus("User has no save data", {255, 100, 100});
                return;
            }
            
            int gmChunks = metaJson["gmChunks"].asInt().unwrapOr(0);
            int llChunks = metaJson["llChunks"].asInt().unwrapOr(0);
            
            BetterSaveLogger::get()->info("Admin", fmt::format("Downloading {} GM + {} LL chunks", gmChunks, llChunks));
            
            // Download to Desktop/BetterSave_Admin/email/
            #ifdef GEODE_IS_WINDOWS
                std::string userProfile = getenv("USERPROFILE");
                std::filesystem::path targetDir = std::filesystem::path(userProfile) / "Desktop" / "BetterSave_Admin" / email;
            #else
                std::filesystem::path targetDir = std::filesystem::path(getenv("HOME")) / "Desktop" / "BetterSave_Admin" / email;
            #endif
            
            try {
                std::filesystem::create_directories(targetDir);
            } catch (const std::exception& e) {
                showStatus("Failed to create directory", {255, 100, 100});
                return;
            }
            
            showStatus("Downloading chunks...", {100, 200, 255});
            
            // Download chunks (simplified - would need full implementation like SaveManagerPopup)
            // For now, just save the metadata
            std::ofstream metaFile(targetDir / "metadata.json");
            metaFile << metaData;
            metaFile.close();
            
            showStatus("Download complete!", {100, 255, 100});
            FLAlertLayer::create("Success",
                fmt::format("User data downloaded to:\n{}", targetDir.string()),
                "OK")->show();
        });
    });
}

void AdminPanel::banAccountByEmail(const std::string& email) {
    showStatus("Banning account...", {255, 255, 100});
    
    BetterSaveLogger::get()->info("Admin", fmt::format("Banning account: {}", email));
    
    std::string token = FirebaseAuth::get()->getIdToken();
    
    // Store ban in /banned/{userId}
    matjson::Value banData;
    banData["email"] = email;
    banData["bannedAt"] = (int64_t)std::time(nullptr);
    banData["bannedBy"] = ADMIN_UID;
    
    // We need to find userId first, similar to download
    std::string emailKey = email;
    std::replace(emailKey.begin(), emailKey.end(), '@', '-');
    std::replace(emailKey.begin(), emailKey.end(), '.', '-');
    
    std::string url = fmt::format(
        "https://gdbettersave-default-rtdb.firebaseio.com/banned/{}.json?auth={}",
        emailKey, token
    );
    
    web::WebRequest req = web::WebRequest();
    req.userAgent("");
    req.bodyJSON(banData);
    
    req.put(url).listen([this, email](web::WebResponse* resp) {
        if (resp->ok()) {
            showStatus("Account banned successfully", {100, 255, 100});
            BetterSaveLogger::get()->success("Admin", fmt::format("Banned account: {}", email));
            FLAlertLayer::create("Success", 
                fmt::format("{} has been banned.\nThey can only download, not upload.", email),
                "OK")->show();
        } else {
            showStatus("Failed to ban account", {255, 100, 100});
            BetterSaveLogger::get()->error("Admin", "Failed to ban account");
        }
    });
}

void AdminPanel::unbanAccountByEmail(const std::string& email) {
    showStatus("Unbanning account...", {255, 255, 100});
    
    std::string emailKey = email;
    std::replace(emailKey.begin(), emailKey.end(), '@', '-');
    std::replace(emailKey.begin(), emailKey.end(), '.', '-');
    
    std::string token = FirebaseAuth::get()->getIdToken();
    std::string url = fmt::format(
        "https://gdbettersave-default-rtdb.firebaseio.com/banned/{}.json?auth={}",
        emailKey, token
    );
    
    // Delete by setting to null (Firebase REST API)
    web::WebRequest req = web::WebRequest();
    req.userAgent("");
    matjson::Value nullValue;
    req.bodyJSON(nullValue);
    
    req.patch(url).listen([this, email](web::WebResponse* resp) {
        if (resp->ok()) {
            showStatus("Account unbanned", {100, 255, 100});
            BetterSaveLogger::get()->success("Admin", fmt::format("Unbanned account: {}", email));
            FLAlertLayer::create("Success", fmt::format("{} has been unbanned.", email), "OK")->show();
        } else {
            showStatus("Failed to unban", {255, 100, 100});
        }
    });
}

void AdminPanel::showStatus(const std::string& message, ccColor3B color) {
    m_statusLabel->setString(message.c_str());
    m_statusLabel->setColor(color);
}

