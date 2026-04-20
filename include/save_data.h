#ifndef SAVE_DATA_H
#define SAVE_DATA_H

#include <cstdint> // Required for exact 64-bit integer types
#include <string>

class SaveManager {
public:
    // Initialize the save manager with the executable's directory path
    // This should be called once at program startup with argv[0]
    static void initialize(const char* executablePath);

    // Loads the 192-bit save file (coins + high score + purchased characters bitmask).
    // Returns true if successful, false if the file doesn't exist yet.
    // outPurchasedChars bitmask: bit 0 = Dino, bit 1 = Cat, bit 2 = Dog
    static bool loadData(uint64_t& outCoins, uint64_t& outHighScore, uint64_t& outPurchasedChars);

    // Saves exactly 192 bits (64 bits each for coins, high score, purchased characters) to the file.
    static void saveData(uint64_t totalCoins, uint64_t highScore, uint64_t purchasedChars);
};

#endif // SAVE_DATA_H