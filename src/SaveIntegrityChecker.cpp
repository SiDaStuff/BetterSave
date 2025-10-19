/**
 * BetterSave - Save Integrity Checker
 * Created by: sidastuff
 */

#include "SaveIntegrityChecker.hpp"
#include "BetterSaveLogger.hpp"
#include <Geode/loader/Dirs.hpp>
#include <fstream>
#include <sstream>
#include <iomanip>

std::string SaveIntegrityChecker::calculateChecksum(const std::vector<uint8_t>& data) {
    // Simple CRC32-like checksum
    uint32_t checksum = 0xFFFFFFFF;
    
    for (const auto& byte : data) {
        checksum ^= byte;
        for (int i = 0; i < 8; i++) {
            if (checksum & 1) {
                checksum = (checksum >> 1) ^ 0xEDB88320;
            } else {
                checksum >>= 1;
            }
        }
    }
    
    checksum ^= 0xFFFFFFFF;
    
    std::stringstream ss;
    ss << std::hex << std::setw(8) << std::setfill('0') << checksum;
    return ss.str();
}

IntegrityResult SaveIntegrityChecker::checkFile(const std::filesystem::path& filePath) {
    IntegrityResult result;
    result.isValid = false;
    result.fileSize = 0;
    result.checksum = "";
    
    try {
        if (!std::filesystem::exists(filePath)) {
            result.message = "File does not exist";
            return result;
        }
        
        std::ifstream file(filePath, std::ios::binary);
        if (!file.is_open()) {
            result.message = "Failed to open file";
            return result;
        }
        
        // Read file
        std::vector<uint8_t> data((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
        file.close();
        
        result.fileSize = data.size();
        
        if (data.empty()) {
            result.message = "File is empty";
            return result;
        }
        
        // Calculate checksum
        result.checksum = calculateChecksum(data);
        
        // Basic integrity checks
        if (result.fileSize < 100) {
            result.message = "File size too small, likely corrupted";
            return result;
        }
        
        // Check for null bytes ratio (corrupted files often have excessive null bytes)
        size_t nullCount = std::count(data.begin(), data.end(), 0);
        double nullRatio = static_cast<double>(nullCount) / data.size();
        
        if (nullRatio > 0.9) {
            result.message = "File contains too many null bytes, likely corrupted";
            return result;
        }
        
        result.isValid = true;
        result.message = "File integrity check passed";
        
    } catch (const std::exception& e) {
        result.message = fmt::format("Error checking file: {}", e.what());
    }
    
    return result;
}

bool SaveIntegrityChecker::compareChecksums(const std::string& checksum1, const std::string& checksum2) {
    return checksum1 == checksum2;
}

IntegrityResult SaveIntegrityChecker::checkGameManagerSave() {
    auto saveDir = geode::dirs::getSaveDir();
    auto gmPath = saveDir / "CCGameManager.dat";
    
    BetterSaveLogger::get()->info("Integrity", "Checking CCGameManager.dat...");
    auto result = checkFile(gmPath);
    
    if (result.isValid) {
        BetterSaveLogger::get()->success("Integrity", 
            fmt::format("CCGameManager.dat is valid (Size: {} bytes, Checksum: {})", result.fileSize, result.checksum));
    } else {
        BetterSaveLogger::get()->error("Integrity", 
            fmt::format("CCGameManager.dat failed: {}", result.message));
    }
    
    return result;
}

IntegrityResult SaveIntegrityChecker::checkLocalLevelsSave() {
    auto saveDir = geode::dirs::getSaveDir();
    auto llPath = saveDir / "CCLocalLevels.dat";
    
    BetterSaveLogger::get()->info("Integrity", "Checking CCLocalLevels.dat...");
    auto result = checkFile(llPath);
    
    if (result.isValid) {
        BetterSaveLogger::get()->success("Integrity", 
            fmt::format("CCLocalLevels.dat is valid (Size: {} bytes, Checksum: {})", result.fileSize, result.checksum));
    } else {
        BetterSaveLogger::get()->error("Integrity", 
            fmt::format("CCLocalLevels.dat failed: {}", result.message));
    }
    
    return result;
}

std::pair<IntegrityResult, IntegrityResult> SaveIntegrityChecker::checkAllSaves() {
    return {checkGameManagerSave(), checkLocalLevelsSave()};
}

