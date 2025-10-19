#pragma once
#include <Geode/Geode.hpp>
#include <string>
#include <functional>

using namespace geode::prelude;

class FirebaseAuth {
private:
    static constexpr const char* API_KEY = "placeholder";
    static constexpr const char* AUTH_DOMAIN = "gdbettersave.firebaseapp.com";
    static constexpr const char* PROJECT_ID = "gdbettersave";
    
    static FirebaseAuth* s_instance;
    std::string m_idToken;
    std::string m_refreshToken;
    std::string m_userId;
    std::string m_email;
    bool m_isLoggedIn;

public:
    static FirebaseAuth* get() {
        if (!s_instance) {
            s_instance = new FirebaseAuth();
        }
        return s_instance;
    }

    FirebaseAuth() : m_isLoggedIn(false) {}

    // Sign up with email and password
    void signUp(const std::string& email, const std::string& password, 
                std::function<void(bool, const std::string&)> callback);
    
    // Sign in with email and password
    void signIn(const std::string& email, const std::string& password,
                std::function<void(bool, const std::string&)> callback);
    
    // Sign out
    void signOut();
    
    // Save credentials to file
    void saveCredentials();
    
    // Load credentials from file and restore session
    bool loadCredentials();
    
    // Try to auto-login with saved credentials
    void tryAutoLogin(std::function<void(bool, const std::string&)> callback);
    
    // Check if user is logged in
    bool isLoggedIn() const { return m_isLoggedIn; }
    
    // Get user email
    std::string getEmail() const { return m_email; }
    
    // Get user ID
    std::string getUserId() const { return m_userId; }
    
    // Get ID token for authenticated requests
    std::string getIdToken() const { return m_idToken; }

private:
    void handleAuthResponse(const std::string& response, 
                           std::function<void(bool, const std::string&)> callback);
};

