/**
 * BetterSave - Save Manager Popup
 * Created by: sidastuff
 */

#include "SaveManagerPopup.hpp"
#include "LogsViewerPopup.hpp"
#include "ProgressPopup.hpp"
#include "SettingsPopup.hpp"
#include "AccountManagerPopup.hpp"
#include "AdminPanel.hpp"
#include "BetterSaveLogger.hpp"
#include "SettingsManager.hpp"
#include "SaveIntegrityChecker.hpp"
#include "RateLimiter.hpp"
#include <Geode/utils/web.hpp>
#include <Geode/loader/Dirs.hpp>
#include <Geode/loader/Loader.hpp>
#include <fstream>
#include <sstream>
#include <ctime>
#include <thread>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#ifdef _WIN32
    #include <io.h>
#else
    #include <unistd.h>
#endif

SaveManagerPopup* SaveManagerPopup::create() {
    auto ret = new SaveManagerPopup();
    if (ret && ret->initAnchored(380.f, 320.f)) {
        ret->autorelease();
        return ret;
    }
    CC_SAFE_DELETE(ret);
    return nullptr;
}

bool SaveManagerPopup::setup() {
    this->setTitle("BetterSave Manager");
    
    // Test log to ensure logging works
    BetterSaveLogger::get()->info("Manager", "BetterSave Manager opened");
    BetterSaveLogger::get()->forceSave(); // Ensure log is written immediately
    
    auto winSize = this->m_mainLayer->getContentSize();
    
    // User info label
    std::string userEmail = FirebaseAuth::get()->getEmail();
    auto userLabel = CCLabelBMFont::create(
        fmt::format("Logged in as: {}", userEmail).c_str(),
        "goldFont.fnt"
    );
    userLabel->setPosition(winSize.width / 2, winSize.height / 2 + 95);
    userLabel->setScale(0.4f);
    this->m_mainLayer->addChild(userLabel);
    
    // Info text
    auto infoText = CCLabelBMFont::create(
        "Upload your save data to the cloud\nor download your backed-up data",
        "bigFont.fnt"
    );
    infoText->setPosition(winSize.width / 2, winSize.height / 2 + 60);
    infoText->setScale(0.35f);
    infoText->setAlignment(CCTextAlignment::kCCTextAlignmentCenter);
    this->m_mainLayer->addChild(infoText);
    
    // Status label
    m_statusLabel = CCLabelBMFont::create("", "goldFont.fnt");
    m_statusLabel->setPosition(winSize.width / 2, winSize.height / 2 - 90);
    m_statusLabel->setScale(0.4f);
    this->m_mainLayer->addChild(m_statusLabel);
    
    // Button menu
    m_buttonMenu = CCMenu::create();
    m_buttonMenu->setPosition(0, 0);
    this->m_mainLayer->addChild(m_buttonMenu);
    
    // Upload button
    auto uploadBtn = ButtonSprite::create("Upload Save", "goldFont.fnt", "GJ_button_01.png", 0.8f);
    auto uploadBtnItem = CCMenuItemSpriteExtra::create(
        uploadBtn, this, menu_selector(SaveManagerPopup::onUpload)
    );
    uploadBtnItem->setPosition(winSize.width / 2, winSize.height / 2 + 20);
    m_buttonMenu->addChild(uploadBtnItem);
    
    // Download button
    auto downloadBtn = ButtonSprite::create("Download Save", "goldFont.fnt", "GJ_button_04.png", 0.8f);
    auto downloadBtnItem = CCMenuItemSpriteExtra::create(
        downloadBtn, this, menu_selector(SaveManagerPopup::onDownload)
    );
    downloadBtnItem->setPosition(winSize.width / 2 - 60, winSize.height / 2 - 15);
    m_buttonMenu->addChild(downloadBtnItem);
    
    // Download to Custom Location button
    auto customBtn = ButtonSprite::create("Custom", "goldFont.fnt", "GJ_button_04.png", 0.6f);
    auto customBtnItem = CCMenuItemSpriteExtra::create(
        customBtn, this, menu_selector(SaveManagerPopup::onDownloadCustom)
    );
    customBtnItem->setPosition(winSize.width / 2 + 60, winSize.height / 2 - 15);
    m_buttonMenu->addChild(customBtnItem);
    
    // Check Integrity button
    auto integrityBtn = ButtonSprite::create("Check Integrity", "goldFont.fnt", "GJ_button_03.png", 0.7f);
    auto integrityBtnItem = CCMenuItemSpriteExtra::create(
        integrityBtn, this, menu_selector(SaveManagerPopup::onCheckIntegrity)
    );
    integrityBtnItem->setPosition(winSize.width / 2, winSize.height / 2 - 50);
    m_buttonMenu->addChild(integrityBtnItem);
    
    // Account Manager button
    auto accountBtn = CCMenuItemSpriteExtra::create(
        CCSprite::createWithSpriteFrameName("GJ_profileButton_001.png"),
        this, menu_selector(SaveManagerPopup::onAccountManager)
    );
    accountBtn->setPosition(winSize.width / 2 - 70, winSize.height / 2 - 105);
    m_buttonMenu->addChild(accountBtn);
    
    // Settings button
    auto settingsBtn = CCMenuItemSpriteExtra::create(
        CCSprite::createWithSpriteFrameName("GJ_optionsBtn_001.png"),
        this, menu_selector(SaveManagerPopup::onSettings)
    );
    settingsBtn->setPosition(winSize.width / 2 - 20, winSize.height / 2 - 105);
    m_buttonMenu->addChild(settingsBtn);
    
    // Logout button
    auto logoutBtn = CCMenuItemSpriteExtra::create(
        CCSprite::createWithSpriteFrameName("GJ_deleteIcon_001.png"),
        this, menu_selector(SaveManagerPopup::onLogout)
    );
    logoutBtn->setPosition(winSize.width / 2 + 30, winSize.height / 2 - 105);
    m_buttonMenu->addChild(logoutBtn);
    
    // Admin Panel button (only for admin)
    if (AdminPanel::isAdmin(FirebaseAuth::get()->getUserId())) {
        auto adminBtn = CCMenuItemSpriteExtra::create(
            CCSprite::createWithSpriteFrameName("GJ_starsIcon_001.png"),
            this, menu_selector(SaveManagerPopup::onAdminPanel)
        );
        adminBtn->setPosition(winSize.width / 2 + 80, winSize.height / 2 - 105);
        m_buttonMenu->addChild(adminBtn);
        
        auto adminLabel = CCLabelBMFont::create("Admin", "goldFont.fnt");
        adminLabel->setPosition(winSize.width / 2 + 80, winSize.height / 2 - 125);
        adminLabel->setScale(0.3f);
        adminLabel->setColor({255, 215, 0});
        this->m_mainLayer->addChild(adminLabel);
    }
    
    // Labels for icon buttons
    auto accountLabelTxt = CCLabelBMFont::create("Account", "bigFont.fnt");
    accountLabelTxt->setPosition(winSize.width / 2 - 70, winSize.height / 2 - 125);
    accountLabelTxt->setScale(0.3f);
    this->m_mainLayer->addChild(accountLabelTxt);
    
    auto settingsLabelTxt = CCLabelBMFont::create("Settings", "bigFont.fnt");
    settingsLabelTxt->setPosition(winSize.width / 2 - 20, winSize.height / 2 - 125);
    settingsLabelTxt->setScale(0.3f);
    this->m_mainLayer->addChild(settingsLabelTxt);
    
    auto logoutLabelTxt = CCLabelBMFont::create("Logout", "bigFont.fnt");
    logoutLabelTxt->setPosition(winSize.width / 2 + 30, winSize.height / 2 - 125);
    logoutLabelTxt->setScale(0.3f);
    this->m_mainLayer->addChild(logoutLabelTxt);
    
    // Credits label (centered at bottom)
    auto creditsLabel = CCLabelBMFont::create("by sidastuff", "goldFont.fnt");
    creditsLabel->setPosition(winSize.width / 2, 15);
    creditsLabel->setScale(0.35f);
    creditsLabel->setOpacity(180);
    this->m_mainLayer->addChild(creditsLabel);
    
    return true;
}

void SaveManagerPopup::onSettings(CCObject*) {
    SettingsPopup::create()->show();
}

void SaveManagerPopup::onAccountManager(CCObject*) {
    AccountManagerPopup::create()->show();
}

void SaveManagerPopup::onAdminPanel(CCObject*) {
    if (AdminPanel::isAdmin(FirebaseAuth::get()->getUserId())) {
        AdminPanel::create()->show();
    }
}

void SaveManagerPopup::onCheckIntegrity(CCObject*) {
    BetterSaveLogger::get()->info("Integrity", "Starting save integrity check");
    showStatus("Checking save integrity...", {255, 255, 0});
    
    Loader::get()->queueInMainThread([this]() {
        auto results = SaveIntegrityChecker::checkAllSaves();
        auto& gmResult = results.first;
        auto& llResult = results.second;
        
        std::string message = fmt::format(
            "<cg>CCGameManager.dat:</c> {}\n<cl>Size:</c> {} bytes | <cl>Checksum:</c> {}\n\n"
            "<cg>CCLocalLevels.dat:</c> {}\n<cl>Size:</c> {} bytes | <cl>Checksum:</c> {}",
            gmResult.isValid ? "<cg>VALID</c>" : "<cr>INVALID</c>",
            gmResult.fileSize, gmResult.checksum,
            llResult.isValid ? "<cg>VALID</c>" : "<cr>INVALID</c>",
            llResult.fileSize, llResult.checksum
        );
        
        if (!gmResult.isValid) {
            message += fmt::format("\n\n<cr>GM Error:</c> {}", gmResult.message);
        }
        if (!llResult.isValid) {
            message += fmt::format("\n<cr>LL Error:</c> {}", llResult.message);
        }
        
        showInfoDialog("Save Integrity Check", message);
        
        if (gmResult.isValid && llResult.isValid) {
            showStatus("Integrity check passed!", {0, 255, 0});
            BetterSaveLogger::get()->success("Integrity", "All save files passed integrity check");
        } else {
            showStatus("Integrity check failed!", {255, 0, 0});
            BetterSaveLogger::get()->error("Integrity", "Save integrity check failed");
        }
    });
}

std::filesystem::path SaveManagerPopup::getGDSavePath() {
    #ifdef GEODE_IS_WINDOWS
        // Windows: C:/Users/Name/AppData/Local/GeometryDash
        std::string appData = getenv("LOCALAPPDATA");
        return std::filesystem::path(appData) / "GeometryDash";
    #elif defined(GEODE_IS_MACOS)
        // Mac: ~/Library/Application Support/GeometryDash
        std::string home = getenv("HOME");
        return std::filesystem::path(home) / "Library" / "Application Support" / "GeometryDash";
    #else
        // Default fallback
        return std::filesystem::path("");
    #endif
}

// Hex encoding - simpler and more reliable than base64
static std::string hexEncode(const std::string& data) {
    static const char hex[] = "0123456789abcdef";
    std::string result;
    result.reserve(data.size() * 2);
    for (unsigned char c : data) {
        result += hex[c >> 4];
        result += hex[c & 0x0F];
    }
    return result;
}

static std::string hexDecode(const std::string& hex) {
    std::string result;
    result.reserve(hex.size() / 2);
    for (size_t i = 0; i < hex.size(); i += 2) {
        int high = (hex[i] >= 'a') ? (hex[i] - 'a' + 10) : (hex[i] - '0');
        int low = (hex[i+1] >= 'a') ? (hex[i+1] - 'a' + 10) : (hex[i+1] - '0');
        result += static_cast<char>((high << 4) | low);
    }
    return result;
}

// Split into chunks - larger chunks = fewer requests = faster upload
static std::vector<std::string> splitChunks(const std::string& data, size_t chunkSize = 200000) {
    std::vector<std::string> chunks;
    for (size_t i = 0; i < data.size(); i += chunkSize) {
        chunks.push_back(data.substr(i, std::min(chunkSize, data.size() - i)));
    }
    return chunks;
}

void SaveManagerPopup::onUpload(CCObject*) {
    geode::createQuickPopup(
        "Upload Save Data",
        "This will upload your save files to the cloud.\n"
        "Your current cloud save will be <cr>overwritten</c>.\n\n"
        "Continue?",
        "Cancel", "Upload",
        [this](auto, bool btn2) {
            if (btn2) {
                this->uploadSaveData();
            }
        }
    );
}

void SaveManagerPopup::uploadSaveData(bool autoRestart) {
    m_autoRestartAfterUpload = autoRestart;
    
    // Close this popup and open progress popup
    this->onClose(nullptr);
    
    auto progressPopup = ProgressPopup::create("Uploading Save Data");
    progressPopup->show();
    progressPopup->setStatus("Reading save files...", {255, 255, 100});
    
    BetterSaveLogger::get()->info("Upload", "Starting upload process");
    BetterSaveLogger::get()->forceSave();
    
    // Use Geode's save directory (same location as download)
    auto savePath = geode::dirs::getSaveDir();
    auto gmPath = savePath / "CCGameManager.dat";
    auto llPath = savePath / "CCLocalLevels.dat";
    
    if (!std::filesystem::exists(gmPath) || !std::filesystem::exists(llPath)) {
        progressPopup->setStatus("Error: Save files not found!", {255, 100, 100});
        progressPopup->enableCloseButton();
        BetterSaveLogger::get()->error("Upload", "Save files not found");
        BetterSaveLogger::get()->forceSave();
        
        FLAlertLayer::create("Upload Failed", "Save files not found!", "OK")->show();
        return;
    }
    
    try {
        // Read files
        std::ifstream gmFile(gmPath, std::ios::binary);
        std::ifstream llFile(llPath, std::ios::binary);
        
        std::string gmData((std::istreambuf_iterator<char>(gmFile)), std::istreambuf_iterator<char>());
        std::string llData((std::istreambuf_iterator<char>(llFile)), std::istreambuf_iterator<char>());
        
        BetterSaveLogger::get()->info("Upload", fmt::format("Read GM: {} bytes, LL: {} bytes", gmData.size(), llData.size()));
        
        // Hex encode
        progressPopup->setStatus("Encoding data...", {255, 255, 100});
        std::string gmHex = hexEncode(gmData);
        std::string llHex = hexEncode(llData);
        
        BetterSaveLogger::get()->info("Upload", fmt::format("Encoded GM: {} hex, LL: {} hex", gmHex.size(), llHex.size()));
        
        // Split into larger chunks for faster upload
        auto gmChunks = splitChunks(gmHex, 200000);
        auto llChunks = splitChunks(llHex, 200000);
        
        BetterSaveLogger::get()->info("Upload", fmt::format("Split into {} GM chunks, {} LL chunks", 
            gmChunks.size(), llChunks.size()));
        BetterSaveLogger::get()->forceSave();
        
        // Upload metadata first
        std::string userId = FirebaseAuth::get()->getUserId();
        std::string metaUrl = fmt::format(
            "https://gdbettersave-default-rtdb.firebaseio.com/users/{}/saveData.json?auth={}",
            userId, FirebaseAuth::get()->getIdToken()
        );
        
        matjson::Value meta;
        meta["gmChunks"] = (int)gmChunks.size();
        meta["llChunks"] = (int)llChunks.size();
        meta["timestamp"] = (int64_t)std::time(nullptr);
        
        web::WebRequest metaReq = web::WebRequest();
        metaReq.userAgent("");
        metaReq.bodyJSON(meta);
        
        progressPopup->setStatus("Uploading metadata...", {255, 255, 100});
        metaReq.put(metaUrl).listen([progressPopup, gmChunks, llChunks, userId](web::WebResponse* resp) {
            if (!resp->ok()) {
                auto err = resp->string().unwrapOr("Unknown error");
                BetterSaveLogger::get()->error("Upload", fmt::format("Metadata failed: {}", err));
                BetterSaveLogger::get()->forceSave();
                
                progressPopup->setStatus("Upload failed!", {255, 100, 100});
                progressPopup->enableCloseButton();
                FLAlertLayer::create("Upload Failed", 
                    fmt::format("Metadata upload failed\n{}", err), "OK")->show();
                return;
            }
            
            BetterSaveLogger::get()->info("Upload", "Metadata uploaded, starting parallel chunk upload");
            SaveManagerPopup::uploadChunksParallel(gmChunks, userId, "gm", [progressPopup, llChunks, userId]() {
                BetterSaveLogger::get()->info("Upload", "GM done, uploading LL");
                SaveManagerPopup::uploadChunksParallel(llChunks, userId, "ll", [progressPopup]() {
                    progressPopup->setStatus("Upload complete!", {100, 255, 100});
                    progressPopup->enableCloseButton();
                    BetterSaveLogger::get()->success("Upload", "All data uploaded successfully");
                    BetterSaveLogger::get()->forceSave();
                    FLAlertLayer::create("Upload Successful",
                        "Your save data has been uploaded to the cloud!",
                        "OK")->show();
                }, progressPopup);
            }, progressPopup);
        });
        
    } catch (const std::exception& e) {
        progressPopup->setStatus(fmt::format("Error: {}", e.what()), {255, 100, 100});
        progressPopup->enableCloseButton();
        BetterSaveLogger::get()->error("Upload", fmt::format("Exception: {}", e.what()));
        BetterSaveLogger::get()->forceSave();
    }
}

// Upload all chunks in parallel for maximum speed
void SaveManagerPopup::uploadChunksParallel(const std::vector<std::string>& chunks, 
                                             const std::string& userId, const std::string& prefix,
                                             std::function<void()> onComplete, ProgressPopup* progressPopup) {
    if (chunks.empty()) {
        onComplete();
        return;
    }
    
    // Counter to track completed chunks
    auto completedCount = std::make_shared<std::atomic<int>>(0);
    auto totalChunks = chunks.size();
    auto hasError = std::make_shared<std::atomic<bool>>(false);
    
    BetterSaveLogger::get()->info("Upload", fmt::format("Starting parallel upload of {} {} chunks", totalChunks, prefix));
    
    // Upload all chunks at once
    for (size_t i = 0; i < chunks.size(); i++) {
        std::string url = fmt::format(
            "https://gdbettersave-default-rtdb.firebaseio.com/users/{}/chunks/{}{}.json?auth={}",
            userId, prefix, i, FirebaseAuth::get()->getIdToken()
        );
        
        matjson::Value chunkData;
        chunkData["d"] = chunks[i];
        
        web::WebRequest req = web::WebRequest();
        req.userAgent("");
        req.bodyJSON(chunkData);
        
        req.put(url).listen([progressPopup, completedCount, totalChunks, onComplete, hasError, i, prefix](web::WebResponse* resp) {
            if (!resp->ok() && !hasError->load()) {
                hasError->store(true);
                auto err = resp->string().unwrapOr("Unknown");
                BetterSaveLogger::get()->error("Upload", fmt::format("Chunk {} failed: {}", i, err));
                BetterSaveLogger::get()->forceSave();
                
                Loader::get()->queueInMainThread([progressPopup]() {
                    progressPopup->setStatus("Upload failed!", {255, 100, 100});
                    progressPopup->enableCloseButton();
                });
                
                FLAlertLayer::create("Upload Failed",
                    fmt::format("Failed at chunk {}\n{}", i, err),
                    "OK")->show();
                return;
            }
            
            int completed = ++(*completedCount);
            
            // Update progress
            Loader::get()->queueInMainThread([progressPopup, completed, totalChunks, prefix]() {
                progressPopup->setStatus(fmt::format("Uploading {} chunks...", prefix), {100, 200, 255});
                progressPopup->setProgress(completed, totalChunks);
            });
            
            // Check if all chunks are done
            if (completed == totalChunks && !hasError->load()) {
                Loader::get()->queueInMainThread([onComplete]() {
                    onComplete();
                });
            }
        });
    }
}

void SaveManagerPopup::onDownload(CCObject*) {
    geode::createQuickPopup(
        "Download Save Data",
        "<cy>IMPORTANT:</c> If you had a Geometry Dash\naccount, <cg>login to it BEFORE downloading</c>\nto prevent bugs!\n\n"
        "This will download your cloud save and\n"
        "<cr>overwrite your local save files</c>.\n\n"
        "After download, you will have the option to restart.\n\n"
        "Continue?",
        "Cancel", "Download",
        [this](auto, bool btn2) {
            if (btn2) {
                this->downloadSaveData();
            }
        }
    );
}

void SaveManagerPopup::downloadSaveData() {
    // Close this popup and open progress popup in fullscreen mode
    this->onClose(nullptr);
    
    auto progressPopup = ProgressPopup::create("Downloading Save Data", true);
    progressPopup->show();
    progressPopup->setStatus("Downloading metadata...", {255, 255, 100});
    
    std::string userId = FirebaseAuth::get()->getUserId();
    std::string metaUrl = fmt::format(
        "https://gdbettersave-default-rtdb.firebaseio.com/users/{}/saveData.json?auth={}",
        userId, FirebaseAuth::get()->getIdToken()
    );
    
    web::WebRequest req = web::WebRequest();
    req.userAgent("");
    
    req.get(metaUrl).listen([this, progressPopup, userId](web::WebResponse* resp) {
        if (!resp->ok()) {
            progressPopup->setStatus("Download failed!", {255, 100, 100});
            progressPopup->enableCloseButton();
            BetterSaveLogger::get()->error("Download", "Metadata download failed");
            FLAlertLayer::create("Download Failed", "No cloud save found", "OK")->show();
            return;
        }
        
        auto json = resp->json();
        if (!json.isOk()) {
            progressPopup->setStatus("Invalid data!", {255, 100, 100});
            progressPopup->enableCloseButton();
            FLAlertLayer::create("Download Failed", "Invalid cloud save", "OK")->show();
            return;
        }
        
        auto meta = json.unwrap();
        int gmChunks = meta["gmChunks"].as<int>().unwrapOr(0);
        int llChunks = meta["llChunks"].as<int>().unwrapOr(0);
        
        BetterSaveLogger::get()->info("Download", fmt::format("Downloading {} GM + {} LL chunks in parallel", gmChunks, llChunks));
        
        downloadChunksParallel(userId, "gm", gmChunks, [this, progressPopup, userId, llChunks](std::string gmHex) {
            downloadChunksParallel(userId, "ll", llChunks, [this, progressPopup, gmHex](std::string llHex) {
                progressPopup->setStatus("Decoding and saving...", {255, 255, 100});
                
                std::string gmData = hexDecode(gmHex);
                std::string llData = hexDecode(llHex);
                
                BetterSaveLogger::get()->info("Download", fmt::format("Decoded {} + {} bytes", 
                    gmData.size(), llData.size()));
                
                // Get save directory (same location as uploaded from)
                auto savePath = geode::dirs::getSaveDir();
                auto gmPath = savePath / "CCGameManager.dat";
                auto llPath = savePath / "CCLocalLevels.dat";
                auto gmPath2 = savePath / "CCGameManager2.dat";
                auto llPath2 = savePath / "CCLocalLevels2.dat";
                
                // Delete existing files (including backups) if they exist
                try {
                    if (std::filesystem::exists(gmPath)) {
                        std::filesystem::remove(gmPath);
                        BetterSaveLogger::get()->info("Download", "Deleted existing CCGameManager.dat");
                    }
                    if (std::filesystem::exists(llPath)) {
                        std::filesystem::remove(llPath);
                        BetterSaveLogger::get()->info("Download", "Deleted existing CCLocalLevels.dat");
                    }
                    if (std::filesystem::exists(gmPath2)) {
                        std::filesystem::remove(gmPath2);
                        BetterSaveLogger::get()->info("Download", "Deleted backup CCGameManager2.dat");
                    }
                    if (std::filesystem::exists(llPath2)) {
                        std::filesystem::remove(llPath2);
                        BetterSaveLogger::get()->info("Download", "Deleted backup CCLocalLevels2.dat");
                    }
                } catch (const std::exception& e) {
                    BetterSaveLogger::get()->warning("Download", fmt::format("Could not delete old files: {}", e.what()));
                }
                
                // Write new files with FORCED syncing
                try {
                    // Close any open file handles by forcing GameManager to save
                    GameManager::sharedState()->save();
                    
                    // Additional delay to ensure GameManager finished writing
                    std::this_thread::sleep_for(std::chrono::milliseconds(200));
                    
                    // Write CCGameManager.dat
                    {
                        std::ofstream gmFile(gmPath, std::ios::binary | std::ios::trunc);
                        if (!gmFile.is_open()) {
                            throw std::runtime_error("Could not open CCGameManager.dat for writing");
                        }
                        gmFile.write(gmData.data(), gmData.size());
                        
                        // Force flush to OS
                        gmFile.flush();
                        
                        // Sync to disk (platform specific)
                        #ifdef _WIN32
                            gmFile.close();
                            // Force Windows to flush cache
                            FILE* f = fopen(gmPath.string().c_str(), "rb+");
                            if (f) {
                                fflush(f);
                                _commit(_fileno(f));
                                fclose(f);
                            }
                        #else
                            gmFile.close();
                            sync();
                        #endif
                        
                        BetterSaveLogger::get()->info("Download", fmt::format("Wrote CCGameManager.dat ({} bytes)", gmData.size()));
                    }
                    
                    // Write CCLocalLevels.dat
                    {
                        std::ofstream llFile(llPath, std::ios::binary | std::ios::trunc);
                        if (!llFile.is_open()) {
                            throw std::runtime_error("Could not open CCLocalLevels.dat for writing");
                        }
                        llFile.write(llData.data(), llData.size());
                        
                        // Force flush to OS
                        llFile.flush();
                        
                        // Sync to disk (platform specific)
                        #ifdef _WIN32
                            llFile.close();
                            // Force Windows to flush cache
                            FILE* f = fopen(llPath.string().c_str(), "rb+");
                            if (f) {
                                fflush(f);
                                _commit(_fileno(f));
                                fclose(f);
                            }
                        #else
                            llFile.close();
                            sync();
                        #endif
                        
                        BetterSaveLogger::get()->info("Download", fmt::format("Wrote CCLocalLevels.dat ({} bytes)", llData.size()));
                    }
                    
                    // Verify both files exist and have correct size
                    if (!std::filesystem::exists(gmPath)) {
                        throw std::runtime_error("CCGameManager.dat was not created!");
                    }
                    if (!std::filesystem::exists(llPath)) {
                        throw std::runtime_error("CCLocalLevels.dat was not created!");
                    }
                    
                    auto gmSize = std::filesystem::file_size(gmPath);
                    auto llSize = std::filesystem::file_size(llPath);
                    
                    if (gmSize != static_cast<std::uintmax_t>(gmData.size())) {
                        throw std::runtime_error(fmt::format("CCGameManager.dat size wrong: expected {}, got {}", gmData.size(), gmSize));
                    }
                    if (llSize != static_cast<std::uintmax_t>(llData.size())) {
                        throw std::runtime_error(fmt::format("CCLocalLevels.dat size wrong: expected {}, got {}", llData.size(), llSize));
                    }
                    
                    progressPopup->setStatus("Reloading game data...", {255, 255, 100});
                    BetterSaveLogger::get()->success("Download", fmt::format("VERIFIED: GM={} bytes, LL={} bytes", gmSize, llSize));
                    BetterSaveLogger::get()->forceSave();
                    
                    // One final sync delay to ensure files are flushed to disk
                    std::this_thread::sleep_for(std::chrono::milliseconds(500));
                    
                    // CRITICAL: Reload GameManager and LocalLevelManager from disk
                    // This prevents the old in-memory data from overwriting the downloaded files
                    Loader::get()->queueInMainThread([progressPopup]() {
                        BetterSaveLogger::get()->info("Download", "Reloading GameManager from downloaded files");
                        
                        // Force GameManager to reload from the new files
                        // This is critical to prevent overwrite on restart
                        auto gm = GameManager::sharedState();
                        auto llm = LocalLevelManager::sharedState();
                        
                        // Call setup() to reload data from disk
                        // This loads the downloaded files into memory
                        gm->setup();
                        llm->setup();
                        
                        BetterSaveLogger::get()->success("Download", "GameManager reloaded with new data");
                        
                        progressPopup->setStatus("Download Complete!", {100, 255, 100});
                        progressPopup->enableCloseButton();
                        
                        // Close the progress popup and show restart dialog
                        progressPopup->closePopup();
                        
                        // Show restart option dialog
                        geode::createQuickPopup(
                            "Download Complete",
                            "Save data downloaded and loaded successfully!\n\n"
                            "The new save data is now active in memory.\n\n"
                            "Would you like to restart the game?\n"
                            "<cy>(Not Recommended - Already Loaded)</c>\n\n"
                            "You can continue playing with the new save data,\n"
                            "or restart if you experience any issues.",
                            "Continue", "Restart",
                            [](auto, bool btn2) {
                                if (btn2) {
                                    BetterSaveLogger::get()->info("Download", "User chose to restart");
                                    BetterSaveLogger::get()->forceSave();
                                    
                                    // DO NOT call GameManager::save() here!
                                    // The data is already reloaded, just restart
                                    geode::utils::game::restart();
                                }
                            }
                        );
                    });
                    
                } catch (const std::exception& e) {
                    progressPopup->setStatus("File write failed!", {255, 100, 100});
                    progressPopup->enableCloseButton();
                    BetterSaveLogger::get()->error("Download", fmt::format("Failed to write files: {}", e.what()));
                    FLAlertLayer::create("Download Failed", 
                        fmt::format("Could not write save files:\n{}", e.what()), 
                        "OK")->show();
                }
            }, progressPopup);
        }, progressPopup);
    });
}

// Download all chunks in parallel for maximum speed
void SaveManagerPopup::downloadChunksParallel(const std::string& userId, const std::string& prefix,
                                               int totalChunks, std::function<void(std::string)> onComplete,
                                               ProgressPopup* progressPopup) {
    if (totalChunks == 0) {
        onComplete("");
        return;
    }
    
    // Store chunks in a vector (may arrive out of order)
    auto chunkResults = std::make_shared<std::vector<std::string>>(totalChunks);
    auto completedCount = std::make_shared<std::atomic<int>>(0);
    auto hasError = std::make_shared<std::atomic<bool>>(false);
    
    BetterSaveLogger::get()->info("Download", fmt::format("Starting parallel download of {} {} chunks", totalChunks, prefix));
    
    // Download all chunks at once
    for (int i = 0; i < totalChunks; i++) {
        std::string url = fmt::format(
            "https://gdbettersave-default-rtdb.firebaseio.com/users/{}/chunks/{}{}.json?auth={}",
            userId, prefix, i, FirebaseAuth::get()->getIdToken()
        );
        
        web::WebRequest req = web::WebRequest();
        req.userAgent("");
        
        req.get(url).listen([progressPopup, chunkResults, completedCount, totalChunks, onComplete, hasError, i, prefix](web::WebResponse* resp) {
            if (!resp->ok() && !hasError->load()) {
                hasError->store(true);
                BetterSaveLogger::get()->error("Download", fmt::format("Chunk {} failed", i));
                
                Loader::get()->queueInMainThread([progressPopup]() {
                    progressPopup->setStatus("Download failed!", {255, 100, 100});
                    progressPopup->enableCloseButton();
                });
                
                FLAlertLayer::create("Download Failed", fmt::format("Failed at chunk {}", i), "OK")->show();
                return;
            }
            
            auto json = resp->json();
            if (!json.isOk() && !hasError->load()) {
                hasError->store(true);
                
                Loader::get()->queueInMainThread([progressPopup]() {
                    progressPopup->setStatus("Download failed!", {255, 100, 100});
                    progressPopup->enableCloseButton();
                });
                
                return;
            }
            
            // Store chunk in correct position
            (*chunkResults)[i] = json.unwrap()["d"].as<std::string>().unwrapOr("");
            
            int completed = ++(*completedCount);
            
            // Update progress
            Loader::get()->queueInMainThread([progressPopup, completed, totalChunks, prefix]() {
                progressPopup->setStatus(fmt::format("Downloading {} chunks...", prefix), {100, 200, 255});
                progressPopup->setProgress(completed, totalChunks);
            });
            
            // Check if all chunks are done
            if (completed == totalChunks && !hasError->load()) {
                // Concatenate all chunks in correct order
                std::string fullData;
                for (const auto& chunk : *chunkResults) {
                    fullData += chunk;
                }
                
                Loader::get()->queueInMainThread([onComplete, fullData]() {
                    onComplete(fullData);
                });
            }
        });
    }
}

void SaveManagerPopup::onDownloadCustom(CCObject*) {
    geode::createQuickPopup(
        "Download to Custom Location",
        "This will download your cloud save to a\n"
        "<cy>custom folder location</c> that you choose.\n\n"
        "Your local save files will <cg>NOT be affected</c>.\n\n"
        "Continue?",
        "Cancel", "Choose Location",
        [this](auto, bool btn2) {
            if (btn2) {
                this->downloadToCustomLocation();
            }
        }
    );
}

void SaveManagerPopup::downloadToCustomLocation() {
    // For now, download to Desktop as a simple alternative
    // In future, could add a text input popup for custom path
    
    #ifdef GEODE_IS_WINDOWS
        std::string userProfile = getenv("USERPROFILE");
        std::filesystem::path targetDir = std::filesystem::path(userProfile) / "Desktop" / "BetterSave_Download";
    #else
        std::filesystem::path targetDir = std::filesystem::path(getenv("HOME")) / "Desktop" / "BetterSave_Download";
    #endif
    
    // Create directory if it doesn't exist
    try {
        if (!std::filesystem::exists(targetDir)) {
            std::filesystem::create_directories(targetDir);
        }
    } catch (const std::exception& e) {
        FLAlertLayer::create("Error", 
            fmt::format("Could not create download folder:\n{}", e.what()), 
            "OK")->show();
        return;
    }
    
    BetterSaveLogger::get()->info("Download", fmt::format("Custom download to: {}", targetDir.string()));
    
    // Close this popup and open progress popup
    this->onClose(nullptr);
    
    auto progressPopup = ProgressPopup::create("Downloading Save Data");
    progressPopup->show();
    progressPopup->setStatus("Downloading metadata...", {255, 255, 100});
    
    std::string userId = FirebaseAuth::get()->getUserId();
    std::string metaUrl = fmt::format(
        "https://gdbettersave-default-rtdb.firebaseio.com/users/{}/saveData.json?auth={}",
        userId, FirebaseAuth::get()->getIdToken()
    );
    
    web::WebRequest req = web::WebRequest();
    req.userAgent("");
    
    req.get(metaUrl).listen([progressPopup, userId, targetDir](web::WebResponse* resp) {
        if (!resp->ok()) {
            progressPopup->setStatus("Download failed!", {255, 100, 100});
            progressPopup->enableCloseButton();
            BetterSaveLogger::get()->error("Download", "Metadata download failed");
            FLAlertLayer::create("Download Failed", "No cloud save found", "OK")->show();
            return;
        }
        
        auto json = resp->json();
        if (!json.isOk()) {
            progressPopup->setStatus("Invalid data!", {255, 100, 100});
            progressPopup->enableCloseButton();
            FLAlertLayer::create("Download Failed", "Invalid cloud save", "OK")->show();
            return;
        }
        
        auto meta = json.unwrap();
        int gmChunks = meta["gmChunks"].as<int>().unwrapOr(0);
        int llChunks = meta["llChunks"].as<int>().unwrapOr(0);
        
        BetterSaveLogger::get()->info("Download", fmt::format("Downloading {} GM + {} LL chunks", gmChunks, llChunks));
        
        downloadChunksParallel(userId, "gm", gmChunks, [progressPopup, userId, llChunks, targetDir](std::string gmHex) {
            downloadChunksParallel(userId, "ll", llChunks, [progressPopup, gmHex, targetDir](std::string llHex) {
                progressPopup->setStatus("Decoding and saving...", {255, 255, 100});
                
                std::string gmData = hexDecode(gmHex);
                std::string llData = hexDecode(llHex);
                
                BetterSaveLogger::get()->info("Download", fmt::format("Decoded {} + {} bytes", 
                    gmData.size(), llData.size()));
                
                // Save to custom location
                auto gmPath = targetDir / "CCGameManager.dat";
                auto llPath = targetDir / "CCLocalLevels.dat";
                
                // Write files
                std::ofstream gmFile(gmPath, std::ios::binary | std::ios::trunc);
                gmFile.write(gmData.data(), gmData.size());
                gmFile.flush();
                gmFile.close();
                BetterSaveLogger::get()->info("Download", fmt::format("Saved to: {}", gmPath.string()));
                
                std::ofstream llFile(llPath, std::ios::binary | std::ios::trunc);
                llFile.write(llData.data(), llData.size());
                llFile.flush();
                llFile.close();
                BetterSaveLogger::get()->info("Download", fmt::format("Saved to: {}", llPath.string()));
                
                progressPopup->setStatus("Download complete!", {100, 255, 100});
                progressPopup->enableCloseButton();
                BetterSaveLogger::get()->success("Download", "Save downloaded to custom location");
                
                FLAlertLayer::create("Download Successful",
                    fmt::format("Save files downloaded to:\n{}", targetDir.string()),
                    "OK")->show();
            }, progressPopup);
        }, progressPopup);
    });
}

void SaveManagerPopup::restartGame() {
    // Use Geode's built-in game restart
    geode::utils::game::restart();
}

void SaveManagerPopup::onLogout(CCObject*) {
    geode::createQuickPopup(
        "Logout",
        "Are you sure you want to logout?\n"
        "You'll need to login again next time.",
        "Cancel", "Logout",
        [this](auto, bool btn2) {
            if (btn2) {
                FirebaseAuth::get()->signOut();
                FLAlertLayer::create(
                    "Logged Out",
                    "You have been logged out successfully.\n"
                    "Saved credentials have been removed.",
                    "OK"
                )->show();
                this->onClose(nullptr);
            }
        }
    );
}

void SaveManagerPopup::showStatus(const std::string& message, ccColor3B color) {
    m_statusLabel->setString(message.c_str());
    m_statusLabel->setColor(color);
}

void SaveManagerPopup::showInfoDialog(const std::string& title, const std::string& message) {
    FLAlertLayer::create(nullptr, title.c_str(), message.c_str(), "OK", nullptr)->show();
}
