/**
 * BetterSave - Save Integrity Checker
 * Created by: sidastuff
 */

#pragma once
#include <Geode/Geode.hpp>
#include <string>

using namespace geode::prelude;

struct IntegrityResult {
    bool isValid;
    std::string message;
    size_t fileSize;
    std::string checksum;
};

class SaveIntegrityChecker {
public:
    static IntegrityResult checkFile(const std::filesystem::path& filePath);
    static std::string calculateChecksum(const std::vector<uint8_t>& data);
    static bool compareChecksums(const std::string& checksum1, const std::string& checksum2);
    static IntegrityResult checkGameManagerSave();
    static IntegrityResult checkLocalLevelsSave();
    static std::pair<IntegrityResult, IntegrityResult> checkAllSaves();
};

