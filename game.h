#ifndef GAME_H
#define GAME_H

#include <string>
#include <vector>
#include <utility>

namespace Game {

    #define MAX_SCORES 10

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
     * @return A vector of strings representing the key.
     */
    std::vector<std::string> generateKey();

    /**
     * @brief Starts a new game for the player.
     * 
     * @param player_id The ID of the player.
     * @param max_playtime The maximum playtime for the game.
     * @return A vector of strings representing the key.
     */
    std::vector<std::string> startGame(const int player_id, const int max_playtime);

    /**
     * @brief Quits the current game for the player.
     * 
     * @param player_id The ID of the player.
     * @return A vector of strings representing the key.
     */
    std::vector<std::string> quitGame(int player_id);

    /**
     * @brief Starts a debug game for the player.
     * 
     * @param player_id The ID of the player.
     * @param max_playtime The maximum playtime for the game.
     * @param key The secret key for the game.
     * @return 0 if the game was started successfully, 1 otherwise.
     */
    int debugGame(const int player_id, const int max_playtime, const std::vector<std::string> key);

    /**
     * @brief Validates the player's guess against the key.
     * 
     * @param key The secret key.
     * @param guess The player's guess.
     * @return A pair of integers representing the number of correct positions and correct colors.
     */
    std::pair<int, int> validateGuess(const std::vector<std::string>& key, const std::vector<std::string>& guess);

    /**
     * @brief Finalizes the game and saves the results.
     * 
     * @param game_status The status of the game (e.g., WIN, FAIL).
     * @param player_id The ID of the player.
     * @param gameFile The game file.
     * @param elapsedTime The elapsed time of the game.
     * @param maxTime The maximum time allowed for the game.
     * @param mode The mode of the game (e.g., PLAY, DEBUG).
     * @param secretKey The secret key of the game.
     * @param trials The number of trials made by the player.
     * @return 1 if the game was finalized successfully.
     */
    int finalizeGame(std::string game_status, std::string player_id, std::string gameFile, int elapsedTime, int maxTime, std::string mode, std::string secretKey, int trials);

    /**
     * @brief Processes a player's attempt to guess the key.
     * 
     * @param plid The ID of the player.
     * @param nT The number of tries the player has made.
     * @param guess The player's guess.
     * @return A response string indicating the result of the attempt.
     */
    std::string playAttempt(int plid, int nT, const std::vector<std::string>& guess);

    /**
     * @brief Finds the last game file for the player.
     * 
     * @param PLID The ID of the player.
     * @param fname The name of the game file.
     * @return 1 if the game file was found, 0 otherwise.
     */
    int FindLastGame(std::string PLID, char* fname);

    /**
     * @brief Creates a summary file for the player's game.
     * 
     * @param player_id The ID of the player.
     * @param guesses The player's guesses.
     * @param remainingTime The remaining time for the game.
     * @param nB The number of correct positions for each guess.
     * @param nW The number of correct colors for each guess.
     * @return The name of the summary file.
     */
    std::string createSummaryFile(int player_id, std::vector<std::string>& guesses, int remainingTime, std::vector<std::string>& nB, std::vector<std::string>& nW);

    /**
     * @brief Reads the game file for summary creation.
     * 
     * @param fname The name of the game file.
     * @param guesses The player's guesses.
     * @param nB The number of correct positions for each guess.
     * @param nW The number of correct colors for each guess.
     * @return The remaining time for the game.
     */
    int readForSummary(const std::string& fname, std::vector<std::string>& guesses, std::vector<std::string>& nB, std::vector<std::string>& nW);

    /**
     * @brief Shows the trials made by the player.
     * 
     * @param player_id The ID of the player.
     * @return A response string indicating the result of the command.
     */
    std::string showTrials(int player_id);

    /**
     * @brief Finds the top scores.
     * 
     * @param list The list of top scores.
     * @return The number of top scores found.
     */
    int FindTopScores(SCORELIST* list);
}

#endif // GAME_H
