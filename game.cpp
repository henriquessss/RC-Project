#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <vector>
#include <fstream>
#include <ctime>
#include <string>

std::string currentGameFilename;

std::vector<std::string> generateKey() {
    std::vector<std::string> validColors = {"R", "G", "B", "Y", "O", "P"};
    std::vector<std::string> key;

    for (int i = 0; i < 4; i++) {
        key.push_back(validColors[rand() % 6]);
    }

    return key;

}

int validatePlaytime(const int max_playtime) {
    if (max_playtime < 1 || max_playtime > 600) {
        return 1;
    }
    return 0;
}

int startGame(const int player_id, const int max_playtime, const char mode) {
    currentGameFilename = "GAMES/GAME_" + std::to_string(player_id) + ".txt";
    std::ifstream file(currentGameFilename);
    
    if (!file) {

        std::ofstream newFile(currentGameFilename);
        std::vector<std::string> key = generateKey();

        if (validatePlaytime(max_playtime)) {
            return 1;
        }

        newFile << std::to_string(player_id) << std::endl;
        newFile << mode << std::endl;
        for (int i = 0; i < 4; i++) {
            newFile << key[i];
        }
        newFile << std::endl;
        newFile << std::to_string(max_playtime) << std::endl;

        std::time_t currentTime = std::time(nullptr);
        std::tm* localTime = std::localtime(&currentTime);

        char date[11];
        std::strftime(date, sizeof(date), "%Y-%m-%d", localTime);

        newFile << date << std::endl;

        char time[9];
        std::strftime(time, sizeof(time), "%H:%M:%S", localTime);

        newFile << time << std::endl;

        newFile << std::to_string(currentTime) << std::endl;

        newFile.close();
    
        return 0;
    } else {
        return 1;
    }
}

int quitGame(const int player_id) {
    if (!currentGameFilename.empty()) {
        currentGameFilename.clear();
        return 0;
    }
    return 1;
}


int main(int argc, char *argv[]) {
    int player_id = std::stoi(argv[1]);
    int max_playtime = std::stoi(argv[2]);
    char mode = argv[3][0];

    startGame(player_id, max_playtime, mode);

    return 0;
}