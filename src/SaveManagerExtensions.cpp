/**
 * BetterSave - Save Manager Extensions
 * Additional functionality for SaveManagerPopup
 * Created by: sidastuff
 */

#include "SaveManagerPopup.hpp"
#include "RateLimiter.hpp"
#include "BetterSaveLogger.hpp"
#include "FirebaseAuth.hpp"

// Delete old user data before uploading (fresh start)
void SaveManagerPopup::deleteOldDataBeforeUpload(std::function<void()> callback) {
    std::string userId = FirebaseAuth::get()->getUserId();
    std::string token = FirebaseAuth::get()->getIdToken();
    
    BetterSaveLogger::get()->info("Upload", "Deleting old data for fresh upload");
    
    // Delete saveData first
    std::string saveDataUrl = fmt::format(
        "https://gdbettersave-default-rtdb.firebaseio.com/users/{}/saveData.json?auth={}",
        userId, token
    );
    
    // Delete by setting to null (Firebase REST API) 
    web::WebRequest req1 = web::WebRequest();
    req1.userAgent("");
    matjson::Value nullValue;
    req1.bodyJSON(nullValue);
    
    req1.patch(saveDataUrl).listen([callback, userId, token](web::WebResponse* resp) {
        if (resp->ok()) {
            BetterSaveLogger::get()->info("Upload", "Deleted old saveData");
            
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
            
            req2.patch(chunksUrl).listen([callback](web::WebResponse* resp2) {
                if (resp2->ok()) {
                    BetterSaveLogger::get()->info("Upload", "Deleted old chunks");
                } else {
                    BetterSaveLogger::get()->warning("Upload", "Chunks didn't exist or couldn't delete");
                }
                
                // Proceed with upload regardless
                callback();
            });
        } else {
            BetterSaveLogger::get()->warning("Upload", "SaveData didn't exist or couldn't delete");
            // Proceed anyway
            callback();
        }
    });
}

// Check if user is banned before allowing upload
bool isBanned(const std::string& email, std::function<void(bool)> callback) {
    std::string emailKey = email;
    std::replace(emailKey.begin(), emailKey.end(), '@', '-');
    std::replace(emailKey.begin(), emailKey.end(), '.', '-');
    
    std::string token = FirebaseAuth::get()->getIdToken();
    std::string url = fmt::format(
        "https://gdbettersave-default-rtdb.firebaseio.com/banned/{}.json?auth={}",
        emailKey, token
    );
    
    web::WebRequest req = web::WebRequest();
    req.userAgent("");
    
    req.get(url).listen([callback](web::WebResponse* resp) {
        if (resp->ok()) {
            auto data = resp->string().unwrapOr("null");
            auto jsonResult = matjson::parse(data);
            
            // If data exists and is not null, user is banned
            bool banned = jsonResult.isOk() && !jsonResult.unwrap().isNull();
            callback(banned);
        } else {
            // If request fails or user not found in banned list, assume not banned
            callback(false);
        }
    });
    
    return false; // Placeholder return
}

