#include "game.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fstream>
#include <sstream>
#include <ctime>
#include <sys/stat.h>
#include <iostream>
#include <vector>
#include <string>
#include <unordered_map>

namespace Game {
    std::string currentGameFilename;

    std::vector<std::string> generateKey() {
        std::vector<std::string> validColors = {"R", "G", "B", "Y", "O", "P"};
        std::vector<std::string> key;

        for (int i = 0; i < 4; i++) {
            key.push_back(validColors[rand() % 6]);
        }

        return key;

    }


    // INPUTS ARE ASSUMED TO BE CORRECT AND VALIDATED CLIENT SIDE
    int startGame(const int player_id, const int max_playtime) {
        currentGameFilename = "GAMES/GAME_" + std::to_string(player_id) + ".txt";
        std::ifstream file(currentGameFilename);
        
        if (!file) {

            std::ofstream newFile(currentGameFilename);
            std::vector<std::string> key = generateKey();

            newFile << std::to_string(player_id) << std::endl;
            newFile << "P" << std::endl;
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

    int quitGame() {
        if (!currentGameFilename.empty()) {
            currentGameFilename.clear();
            return 0;
        }
        return 1;
    }

    int debugGame(const int player_id, const int max_playtime, const std::vector<std::string> key) {
        
        currentGameFilename = "GAMES/GAME_" + std::to_string(player_id) + ".txt";
        std::ifstream file(currentGameFilename);
        
        if (!file) {

            std::ofstream newFile(currentGameFilename);

            newFile << std::to_string(player_id) << std::endl;
            newFile << "D" << std::endl;
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


    std::pair<int, int> validateGuess(const std::vector<std::string>& key, const std::vector<std::string>& guess) {
        int nB = 0; // Cor e posição corretas
        int nW = 0; // Cor correta, posição errada

        std::unordered_map<std::string, int> colorCount; // Conta as cores da chave secreta que ainda não foram usadas

        for (size_t i = 0; i < 4; ++i) {
            if (key[i] == guess[i]) {
                nB++;
            } else {
                // Se não for preto, adicionar a cor da chave ao contador
                colorCount[key[i]]++;
            }
        }

        for (size_t i = 0; i < 4; ++i) {
            if (key[i] != guess[i] && colorCount[guess[i]] > 0) {
                nW++;
                colorCount[guess[i]]--; // Reduzir o contador da cor usada
            }
        }

        return {nB, nW};
    }

    // Function to create directory if it doesn't exist
    void createDirectory(const std::string& path) {
        struct stat info;
        if (stat(path.c_str(), &info) != 0) {
            mkdir(path.c_str());
        }
    }

    // Função para verificar o estado do jogo
    std::string playAttempt(int plid, const std::vector<std::string>& guess) {
        int nB, nW;

        std::string gameFile = "GAMES/GAME_" + std::to_string(plid) + ".txt";

        // Abrir o ficheiro do jogo para leitura
        std::ifstream file(gameFile);
        if (!file.is_open()) {
            return "ERROR: Unable to open game file.";
        }

        // Leitura
        std::string line, fileContent;
        int lineCount = 0;

        while (std::getline(file, line) && lineCount < 7) {
            fileContent += line + "\n";
            lineCount++;
        }

        std::istringstream iss(fileContent);

        std::string player_id, secretKey, maxTimeStr, startTimeStr;
        std::string mode, startDate, startHour;

        iss >> player_id >> mode >> secretKey >> maxTimeStr >> startDate >> startHour >> startTimeStr;

        // Verificar tentativas já feitas
        int trials = 0;
        std::vector<std::string> attempts;
        while (std::getline(file, line)) {
            attempts.push_back(line);
            trials++;
        }
        file.close();

        // Verificar tempo restante
        int maxTime, startTime;
        maxTime = stoi(maxTimeStr);
        startTime = stoi(startTimeStr);
        std::time_t currentTime = std::time(nullptr);

        int elapsedTime = currentTime - startTime;
        if (elapsedTime > maxTime) {
            return "TIMEOUT";
        }


        // Validar a tentativa
        nB = nW = 0;
        std::vector<std::string> key = {std::string(1, secretKey[0]), std::string(1, secretKey[1]),
                                        std::string(1, secretKey[2]), std::string(1, secretKey[3])};
        std::tie(nB, nW) = validateGuess(key, guess);

        // Atualizar o ficheiro com a nova tentativa
        std::ofstream outFile(gameFile, std::ios::app);

        outFile << "T: ";
        for (const auto& color : guess) {
            outFile << color;
        }
        outFile << " " << nB << " " << nW << " " << elapsedTime << "\n";
        outFile.close();

        // Verificar vitória ou número máximo de tentativas
        std::string game_status = "ONGOING";
        if (nB == 4) {
            game_status = "WIN";
        } else if (trials >= 8) {
            game_status = "FAIL";
        }

        // Se o jogo for encerrado
        if (game_status != "ONGOING"){
            std::time_t endTime = std::time(nullptr);
            std::tm* endTm = std::localtime(&endTime);

            char endDate[11];
            std::strftime(endDate, sizeof(endDate), "%Y-%m-%d", endTm);

            char endTimeStr[9];
            std::strftime(endTimeStr, sizeof(endTimeStr), "%H:%M:%S", endTm);

            std::string newFileName = "GAMES/" + player_id + "/" + endDate + "_" + endTimeStr + "_" + game_status[0] + ".txt";

            // Create player directory if it doesn't exist
            createDirectory("GAMES/" + player_id);

            // Move the file to the new location
            std::ofstream outFinal(newFileName, std::ios::app);
            outFinal << endDate << " " << endTimeStr << " " << elapsedTime << "\n";
            outFinal.close();
        }

        return game_status;
    }

    std::vector<std::string> showTrials(int player_id) {
        std::vector<std::string> trials;
        std::string gameFile = "GAMES/GAME_" + std::to_string(player_id) + ".txt";
        std::ifstream file(gameFile);

        if (!file.is_open()) {
            std::cerr << "ERROR: Unable to open game file." << std::endl;
            return trials;
        }

        std::string line;
        int lineCount = 0;

        // Skip the first 7 lines (header information)
        while (std::getline(file, line) && lineCount < 7) {
            lineCount++;
        }

        // Read the trials
        while (std::getline(file, line)) {
            trials.push_back(line);
        }

        file.close();
        return trials;
    }
}
