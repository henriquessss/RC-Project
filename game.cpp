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
#include <cstdio>
#include <unistd.h>
#include <dirent.h>
#include <algorithm>

namespace Game {
    std::string currentGameFilename;

    std::vector<std::string> generateKey() {
        std::vector<std::string> validColors = {"R", "G", "B", "Y", "O", "P"};
        std::vector<std::string> key;

        // Seed the random number generator
        srand(static_cast<unsigned int>(std::time(nullptr)));

        for (int i = 0; i < 4; i++) {
            key.push_back(validColors[rand() % 6]);
        }

        return key;
    }

    // INPUTS ARE ASSUMED TO BE CORRECT AND VALIDATED CLIENT SIDE
    std::vector<std::string> startGame(const int player_id, const int max_playtime) {
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
        
            return key;
        }
        return {}; // Ensure the function returns a value
    }

    std::vector<std::string> quitGame(int player_id) {
        std::vector<std::string> key;
        if (!currentGameFilename.empty()) {
            std::ifstream file(currentGameFilename);

            if (file) {
                std::string line;
                std::getline(file, line);
                int plid = std::stoi(line);

                if (plid == player_id) {
                    std::getline(file, line);
                    std::getline(file, line);

                    for (char c : line) {
                        key.push_back(std::string(1, c));
                    }
                }
                file.close();
            }
            currentGameFilename.clear();
        }
        return key;
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

    int finalizeGame(std::string game_status, std::string player_id,
                    std::string gameFile, int elapsedTime, int maxTime, std::string mode,
                    std::string secretKey, int trials){
        if (game_status != "ONGOING") {
            std::time_t endTime = std::time(nullptr);
            std::tm* endTm = std::localtime(&endTime);

            char endDate[11];
            std::strftime(endDate, sizeof(endDate), "%Y-%m-%d", endTm);

            char endTimeStr[9];
            std::strftime(endTimeStr, sizeof(endTimeStr), "%H:%M:%S", endTm);

            std::stringstream ss;
            ss << "GAMES/" << player_id << "/" << endDate << "_" 
            << endTimeStr << "_" << game_status[0] << ".txt";
            std::string newFileName = ss.str();

            std::system(("mkdir -p GAMES/" + player_id).c_str());

            std::rename(gameFile.c_str(), newFileName.c_str());

            std::ofstream outFinal(newFileName, std::ios::app);
            if (outFinal.is_open()) {
                outFinal << endDate << " " << endTimeStr << " " << elapsedTime << "\n";
                outFinal.close();
            }

            if (game_status == "WIN") {
                int trial_penalty = 5.0;
                int time_scale = 5.0;
                int score_aux = 100 - (trials * trial_penalty + std::abs(elapsedTime - maxTime) / time_scale);
                if (score_aux < 1){
                    score_aux = 1;
                } else if (score_aux > 100) {
                    score_aux = 100;
                } else if(trials == 1){
                    score_aux = 100;
                }
                std::string score = std::to_string(score_aux);
                
                std::string scoreFileName = "SCORES/" + score + "_" + player_id + "_" + endDate + "_" + endTimeStr + ".txt";
                std::ofstream scoreFile(scoreFileName);

                scoreFile << score << " " << player_id << " " << secretKey << " " << trials << " ";
                scoreFile << (mode == "P" ? "PLAY" : "DEBUG") << std::endl;

                scoreFile.close();
            }
        }
        return 1;
    }

    std::string playAttempt(int plid, int nT, const std::vector<std::string>& guess) {
        int nB, nW;
        std::string gameFile = "GAMES/GAME_" + std::to_string(plid) + ".txt";
        std::ifstream file(gameFile);

        if (!file.is_open()) {
            return "NOK";
        }

        std::string line, fileContent;
        int lineCount = 0;

        while (std::getline(file, line) && lineCount < 7) {
            fileContent += line + " ";
            lineCount++;
        }
        file.close();

        std::istringstream iss(fileContent);
        std::string player_id, secretKey, maxTimeStr, startTimeStr;
        std::string mode, startDate, startHour;

        iss >> player_id >> mode >> secretKey >> maxTimeStr >> startDate >> startHour >> startTimeStr;

        int maxTime = stoi(maxTimeStr);
        int startTime = stoi(startTimeStr);
        std::time_t currentTime = std::time(nullptr);
        int elapsedTime = currentTime - startTime;
        std::string gameStatus = "ONGOING";

        if (elapsedTime > maxTime) {
            gameStatus = "FAIL";
            finalizeGame(gameStatus, player_id, gameFile, elapsedTime, 0, "", "", 0);
            return "ETM " + secretKey;
        }

        std::vector<std::string> key = {std::string(1, secretKey[0]), std::string(1, secretKey[1]),
                                        std::string(1, secretKey[2]), std::string(1, secretKey[3])};

        std::ifstream attemptsFile(gameFile);
        int trials = 0;
        std::vector<std::string> attempts;
        while (std::getline(attemptsFile, line)) {
            if (line.substr(0, 2) == "T:") {
                attempts.push_back(line);
                trials++;
            }
        }
        attemptsFile.close();

        if (nT != trials + 1) {
            return "INV";
        }

        for (const auto& attempt : attempts) {
            if (attempt.find(guess[0] + guess[1] + guess[2] + guess[3]) != std::string::npos) {
                return "DUP";
            }
        }

        std::tie(nB, nW) = validateGuess(key, guess);

        std::ofstream outFile(gameFile, std::ios::app);
        outFile << "T: ";
        for (const auto& color : guess) {
            outFile << color;
        }
        outFile << " " << nB << " " << nW << " " << elapsedTime << "\n";
        outFile.close();

        if (nB == 4 && trials <= 8) {
            gameStatus = "WIN";
            finalizeGame(gameStatus, player_id, gameFile, elapsedTime, maxTime, mode, secretKey, trials);
            return "OK " + std::to_string(nT) + " " + std::to_string(nB) + " " + std::to_string(nW) + " WIN";
        } else if (nB != 4 && trials == 7) {
            gameStatus = "FAIL";
            finalizeGame(gameStatus, player_id, gameFile, elapsedTime, 0, "", "", 0);
            return "ENT " + secretKey;
        }

        return "OK " + std::to_string(nT) + " " + std::to_string(nB) + " " + std::to_string(nW);
    }

    int FindLastGame(std::string PLID, char* fname) {
        struct dirent** filelist;
        int n_entries, found = 0;
        char dirname[256];

        sprintf(dirname, "GAMES/%s/", PLID.c_str());

        n_entries = scandir(dirname, &filelist, nullptr, alphasort);
        if (n_entries <= 0) {
            return 0;
        } else {
            while (n_entries--) {
                if (filelist[n_entries]->d_name[0] != '.' && !found) {

                    sprintf(fname, "GAMES/%s/%s", PLID.c_str(), filelist[n_entries]->d_name);
                    found = 1; 
                }
                free(filelist[n_entries]);
            }
            free(filelist);
        }
        return found;
    }

    std::string createSummaryFile(int player_id, std::vector<std::string>& guesses, int remainingTime) {
        std::string summaryFile = "GAMES/SUMMARY_" + std::to_string(player_id) + ".txt";
        std::ofstream outFile(summaryFile);
        std::vector<std::string> result;

        if (!outFile.is_open()) {
            std::cerr << "ERROR: Unable to create summary file." << std::endl;
            return "";
        }

        int trials_counter = 0;
        for (const std::string& trial : guesses) {
            trials_counter++;
            outFile << trials_counter << " - " << trial << "\n";
        }

        outFile << remainingTime << " seconds to go!\n";

        outFile.close();

        std::ifstream inFile(summaryFile);
        std::stringstream fileContent;
        fileContent << inFile.rdbuf();

        std::string content = fileContent.str();

        //Calcular o tamanho do arquivo
        std::ifstream file(summaryFile, std::ios::binary);
        file.seekg(0, std::ios::end);  // Vai até o final do arquivo
        std::string fileSizeStr = std::to_string(file.tellg());

        result.push_back(summaryFile);
        result.push_back(fileSizeStr);
        result.push_back(content);

        std::string response_aux = result[0] + result[1] + result[2];

        return response_aux;
    }

    int readForSummary(const std::string& fname, std::vector<std::string>& guesses) {
        std::ifstream gameFile(fname);
        std::string line;
            int lineCount = 0;
            int maxTime, elapsedTime, remainingTime;

            while (std::getline(gameFile, line)) {
                if (lineCount == 4){
                    maxTime = std::stoi(line);
                }
                lineCount++;
                if (line.substr(0, 2) == "T:"){
                    guesses.push_back(line.substr(3, 12));
                    std::istringstream iss(line);
                    std::string secretKey, nB, nW, elapsedTimeStr;
                    iss >> secretKey >> nB >> nW >> elapsedTimeStr;
                    elapsedTime = stoi(elapsedTimeStr);
                    remainingTime = maxTime - elapsedTime;
                }
            }
            gameFile.close();
        return remainingTime;
    }

    std::string showTrials(int player_id) {
        std::vector<std::string> guesses;
        std::string gameFile = "GAMES/GAME_" + std::to_string(player_id) + ".txt";
        std::ifstream file(gameFile);
        std::string response;

        if (file.is_open()) { //se houver ficheiro ativo
            int remainingTime = readForSummary(gameFile, guesses);

            return "RST ACT " + createSummaryFile(player_id, guesses, remainingTime);
        } else { // se n houver
            char fname[256];
            std::string plid = std::to_string(player_id);

            if (FindLastGame(plid, fname)){
                int remainingTime = readForSummary(fname, guesses);
                return "RST FIN " + createSummaryFile(player_id, guesses, remainingTime);
            }
            return "RST NOK";
        }
    }

    int FindTopScores(SCORELIST* list) {
        struct dirent** filelist;
        int n_entries, i_file = 0;
        char fname[300];

        n_entries = scandir("SCORES/", &filelist, 0, alphasort);
        if (n_entries <= 0) {
            return 0;
        }

        while (n_entries--) {
            if (filelist[n_entries]->d_name[0] != '.' && i_file < MAX_SCORES) {
                sprintf(fname, "SCORES/%s", filelist[n_entries]->d_name);
                std::ifstream file(fname);

                if (file.is_open()) {
                    int temp_score, temp_tries;
                    std::string temp_plid, temp_colcode, temp_mode;

                    file >> temp_score >> temp_plid >> temp_colcode >> temp_tries >> temp_mode;
                    list->score[i_file] = temp_score;
                    list->PLID[i_file] = temp_plid;
                    list->col_code[i_file] = temp_colcode;
                    list->no_tries[i_file] = temp_tries;

                    if (temp_mode == "PLAY") {
                        list->mode[i_file] = "PLAY";
                    } else if (temp_mode == "DEBUG") {
                        list->mode[i_file] = "DEBUG";
                    }

                    file.close();
                    i_file++;
                }
            }
            free(filelist[n_entries]);
        }
        free(filelist);

        list->n_scores = i_file;
        return i_file;
    }
}
