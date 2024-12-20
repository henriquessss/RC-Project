#ifndef GAME_H
#define GAME_H

#include <iostream>
#include <vector>
#include <string>
#include <unordered_map>

namespace Game {

    #define MAX_SCORES 10

    // Struct SCORELIST
    struct SCORELIST {
        int score[MAX_SCORES];
        std::string PLID[MAX_SCORES];
        std::string col_code[MAX_SCORES];
        int no_tries[MAX_SCORES];
        std::string mode[MAX_SCORES];
        int n_scores;
    };

    /**
     * @brief Generates a random key for the game.
     * 
     * @return std::vector<std::string> The generated key.
     */
    std::vector<std::string> generateKey();

    /**
     * @brief Starts a new game for the player.
     * 
     * @param player_id The ID of the player.
     * @param max_playtime The maximum playtime allowed for the game.
     * @return int 0 if the game is successfully started, 1 if the game already exists.
     */
    std::vector<std::string> startGame(const int player_id, const int max_playtime);

    /**
     * @brief 
     * 
     */
    int finalizeGame(std::string game_status, std::string player_id,
                    std::string gameFile, int elapsedTime, int maxTime, std::string mode,
                    std::string secretKey, int trials);

    /**
     * @brief Processes a play attempt with number of tries.
     * 
     * @param plid The player ID.
     * @param nT The number of tries.
     * @param guess The guess made by the player.
     * @return std::string The status of the game after the attempt.
     */
    std::string playAttempt(int plid, int nT, const std::vector<std::string>& guess);

    /**
     * @brief 
     * 
     */
    int FindLastGame(std::string PLID, char* fname);

    /**
     * @brief 
     * 
     */
    std::string createSummaryFile(int player_id, std::vector<std::string>& guesses, int remainingTime);

    /**
     * @brief Returns a vector of strings with every try in the game file.
     * 
     * @return std::vector<std::string> A vector containing all the tries.
     */
    std::string showTrials(int player_id);

    /**
     * @brief Returns a integer of the number of scores found for that player
     * 
     * @return int 
     */
    int FindTopScores(SCORELIST* list);

    /**
     * @brief Quits the current game.
     * 
     * @return int 0 if the game is successfully quit, 1 if there is no game to quit.
     */
    std::vector<std::string> quitGame(int player_id);

    /**
     * @brief Starts a debug game with a predefined key.
     * 
     * @param player_id The ID of the player.
     * @param max_playtime The maximum playtime allowed for the game.
     * @param key The predefined key for the debug game.
     * @return int 0 if the game is successfully started, 1 if the game already exists.
     */
    int debugGame(const int player_id, const int max_playtime, const std::vector<std::string> key);

    /**
     * @brief Validates a guess against the key.
     * 
     * @param key The key to validate against.
     * @param guess The guess to validate.
     * @return std::pair<int, int> The number of correct positions and correct colors.
     */
    std::pair<int, int> validateGuess(const std::vector<std::string>& key, const std::vector<std::string>& guess);


}

#endif 