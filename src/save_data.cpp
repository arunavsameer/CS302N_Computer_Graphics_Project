#include "save_data.h"
#include <fstream>
#include <filesystem>
#include <iostream>

namespace fs = std::filesystem;

// Static variable to store the save file path
static std::string g_saveFilePath;

void SaveManager::initialize(const char* executablePath) {
    // Get the directory containing the executable
    fs::path exePath(executablePath);
    fs::path saveDir = exePath.parent_path();
    
    // Construct the full path to the save file
    g_saveFilePath = (saveDir / "savedata.dat").string();
    
    // Debug output
    // std::cout << "SaveManager initialized with save path: " << g_saveFilePath << std::endl;
}

bool SaveManager::loadData(uint64_t& outCoins, uint64_t& outHighScore, uint64_t& outPurchasedChars) {
    // Open in binary mode to ensure exactly 192 raw bits are read
    std::ifstream file(g_saveFilePath, std::ios::binary);
    
    if (!file.is_open()) {
        return false; // File likely doesn't exist yet (first time playing)
    }

    // Read the first 64 bits (8 bytes) into coins
    file.read(reinterpret_cast<char*>(&outCoins), sizeof(uint64_t));
    
    // Read the next 64 bits (8 bytes) into high score
    file.read(reinterpret_cast<char*>(&outHighScore), sizeof(uint64_t));

    // Read the last 64 bits (8 bytes) into purchased characters bitmask
    // Bit 0 = Dino purchased, Bit 1 = Cat purchased, Bit 2 = Dog purchased
    if (file.read(reinterpret_cast<char*>(&outPurchasedChars), sizeof(uint64_t))) {
        // Successfully read all 3 fields
    } else {
        // Old save file (only 128 bits) – no purchased characters yet
        outPurchasedChars = 0;
    }

    file.close();
    return true;
}

void SaveManager::saveData(uint64_t totalCoins, uint64_t highScore, uint64_t purchasedChars) {
    // Open in binary mode, truncating to overwrite any previous data cleanly
    std::ofstream file(g_saveFilePath, std::ios::binary | std::ios::trunc);
    
    if (file.is_open()) {
        // Write the first 64 bits
        file.write(reinterpret_cast<const char*>(&totalCoins), sizeof(uint64_t));
        
        // Write the next 64 bits
        file.write(reinterpret_cast<const char*>(&highScore), sizeof(uint64_t));

        // Write the last 64 bits (purchased characters bitmask)
        file.write(reinterpret_cast<const char*>(&purchasedChars), sizeof(uint64_t));
        
        file.close();
    }
}