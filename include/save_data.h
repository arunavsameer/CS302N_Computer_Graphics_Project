#ifndef SAVE_DATA_H
#define SAVE_DATA_H

#include <cstdint> // Required for exact 64-bit integer types
#include <string>

class SaveManager {
public:
    // Initialize the save manager with the executable's directory path
    // This should be called once at program startup with argv[0]
    static void initialize(const char* executablePath);

    // Loads the 128-bit save file. 
    // Returns true if successful, false if the file doesn't exist yet.
    static bool loadData(uint64_t& outCoins, uint64_t& outHighScore);

    // Saves exactly 128 bits (64 bits for coins, 64 bits for high score) to the file.
    static void saveData(uint64_t totalCoins, uint64_t highScore);
};

#endif // SAVE_DATA_H