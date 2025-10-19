#include "FirebaseAuth.hpp"
#include "BetterSaveLogger.hpp"
#include <Geode/utils/web.hpp>
#include <Geode/loader/Dirs.hpp>
#include <matjson.hpp>
#include <fstream>

FirebaseAuth* FirebaseAuth::s_instance = nullptr;

void FirebaseAuth::signUp(const std::string& email, const std::string& password,
                          std::function<void(bool, const std::string&)> callback) {
    std::string url = fmt::format(
        "https://identitytoolkit.googleapis.com/v1/accounts:signUp?key={}",
        API_KEY
    );

    matjson::Value body;
    body["email"] = email;
    body["password"] = password;
    body["returnSecureToken"] = true;

    web::WebRequest request = web::WebRequest();
    request.userAgent("");
    request.bodyJSON(body);
    
        request.post(url).listen([this, callback](web::WebResponse* response) {
            if (response->ok()) {
                auto dataString = response->string().unwrapOr("{}");
                this->handleAuthResponse(dataString, callback);
            } else {
                auto error = response->string().unwrapOr("Unknown error");
                BetterSaveLogger::get()->error("Auth", fmt::format("Signup failed: {}", error));
            
            // Parse error message
            std::string errorMsg = "Sign up failed";
            auto jsonResult = matjson::parse(error);
            if (jsonResult.isOk()) {
                auto json = jsonResult.unwrap();
                if (json.contains("error") && json["error"].contains("message")) {
                    auto msgResult = json["error"]["message"].asString();
                    if (msgResult.isOk()) {
                        errorMsg = msgResult.unwrap();
                    }
                }
            }
            
            callback(false, errorMsg);
        }
    });
}

void FirebaseAuth::signIn(const std::string& email, const std::string& password,
                          std::function<void(bool, const std::string&)> callback) {
    std::string url = fmt::format(
        "https://identitytoolkit.googleapis.com/v1/accounts:signInWithPassword?key={}",
        API_KEY
    );

    matjson::Value body;
    body["email"] = email;
    body["password"] = password;
    body["returnSecureToken"] = true;

    web::WebRequest request = web::WebRequest();
    request.userAgent("");
    request.bodyJSON(body);
    
        request.post(url).listen([this, callback](web::WebResponse* response) {
            if (response->ok()) {
                auto dataString = response->string().unwrapOr("{}");
                this->handleAuthResponse(dataString, callback);
            } else {
                auto error = response->string().unwrapOr("Unknown error");
                BetterSaveLogger::get()->error("Auth", fmt::format("Login failed: {}", error));
            
            // Parse error message
            std::string errorMsg = "Sign in failed";
            auto jsonResult = matjson::parse(error);
            if (jsonResult.isOk()) {
                auto json = jsonResult.unwrap();
                if (json.contains("error") && json["error"].contains("message")) {
                    auto msgResult = json["error"]["message"].asString();
                    if (msgResult.isOk()) {
                        errorMsg = msgResult.unwrap();
                    }
                }
            }
            
            callback(false, errorMsg);
        }
    });
}

void FirebaseAuth::signOut() {
    m_idToken = "";
    m_refreshToken = "";
    m_userId = "";
    m_email = "";
    m_isLoggedIn = false;
    
    // Delete saved credentials
    auto credentialsPath = geode::dirs::getSaveDir() / "bettersave_credentials.json";
    if (std::filesystem::exists(credentialsPath)) {
        std::filesystem::remove(credentialsPath);
    }
    
    BetterSaveLogger::get()->info("Auth", "User signed out and credentials removed");
}

void FirebaseAuth::saveCredentials() {
    if (!m_isLoggedIn) return;
    
    try {
        auto credentialsPath = geode::dirs::getSaveDir() / "bettersave_credentials.json";
        
        matjson::Value creds;
        creds["idToken"] = m_idToken;
        creds["refreshToken"] = m_refreshToken;
        creds["userId"] = m_userId;
        creds["email"] = m_email;
        
        std::ofstream file(credentialsPath);
        file << creds.dump();
        file.close();
        
        BetterSaveLogger::get()->info("Auth", fmt::format("Credentials saved for: {}", m_email));
    } catch (const std::exception& e) {
        BetterSaveLogger::get()->error("Auth", fmt::format("Failed to save credentials: {}", e.what()));
    }
}

bool FirebaseAuth::loadCredentials() {
    try {
        auto credentialsPath = geode::dirs::getSaveDir() / "bettersave_credentials.json";
        
        if (!std::filesystem::exists(credentialsPath)) {
            return false;
        }
        
        std::ifstream file(credentialsPath);
        std::stringstream buffer;
        buffer << file.rdbuf();
        file.close();
        
        auto jsonResult = matjson::parse(buffer.str());
        if (!jsonResult.isOk()) {
            BetterSaveLogger::get()->error("Auth", "Failed to parse credentials file");
            return false;
        }
        
        auto creds = jsonResult.unwrap();
        
        if (creds.contains("idToken")) {
            auto result = creds["idToken"].asString();
            if (result.isOk()) m_idToken = result.unwrap();
        }
        if (creds.contains("refreshToken")) {
            auto result = creds["refreshToken"].asString();
            if (result.isOk()) m_refreshToken = result.unwrap();
        }
        if (creds.contains("userId")) {
            auto result = creds["userId"].asString();
            if (result.isOk()) m_userId = result.unwrap();
        }
        if (creds.contains("email")) {
            auto result = creds["email"].asString();
            if (result.isOk()) m_email = result.unwrap();
        }
        
        m_isLoggedIn = true;
        
        BetterSaveLogger::get()->success("Auth", fmt::format("Credentials loaded for: {}", m_email));
        return true;
        
    } catch (const std::exception& e) {
        BetterSaveLogger::get()->error("Auth", fmt::format("Failed to load credentials: {}", e.what()));
        return false;
    }
}

void FirebaseAuth::tryAutoLogin(std::function<void(bool, const std::string&)> callback) {
    if (loadCredentials()) {
        BetterSaveLogger::get()->success("Auth", "Auto-login successful");
        callback(true, "Logged in automatically!");
    } else {
        BetterSaveLogger::get()->info("Auth", "No saved credentials found");
        callback(false, "No saved credentials");
    }
}

void FirebaseAuth::handleAuthResponse(const std::string& response,
                                      std::function<void(bool, const std::string&)> callback) {
    auto jsonResult = matjson::parse(response);
    
    if (!jsonResult.isOk()) {
        BetterSaveLogger::get()->error("Auth", "Error parsing auth response");
        callback(false, "Failed to parse authentication response");
        return;
    }
    
    auto json = jsonResult.unwrap();
    
    if (json.contains("idToken")) {
        auto result = json["idToken"].asString();
        if (result.isOk()) {
            m_idToken = result.unwrap();
        }
    }
    if (json.contains("refreshToken")) {
        auto result = json["refreshToken"].asString();
        if (result.isOk()) {
            m_refreshToken = result.unwrap();
        }
    }
    if (json.contains("localId")) {
        auto result = json["localId"].asString();
        if (result.isOk()) {
            m_userId = result.unwrap();
        }
    }
    if (json.contains("email")) {
        auto result = json["email"].asString();
        if (result.isOk()) {
            m_email = result.unwrap();
        }
    }
    
    m_isLoggedIn = true;
    
    // Save credentials for auto-login
    saveCredentials();
    
    BetterSaveLogger::get()->success("Auth", fmt::format("User authenticated: {}", m_email));
    callback(true, "Authentication successful!");
}
