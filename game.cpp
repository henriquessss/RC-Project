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

    // Função para verificar o estado do jogo
    std::vector<std::string> playAttempt(int plid, const std::vector<std::string>& guess) {
        int nB, nW;
        std::vector<std::string> hints;

        std::string gameFile = "GAMES/GAME_" + std::to_string(plid) + ".txt";

        // Abrir o ficheiro do jogo para leitura
        std::ifstream file(gameFile);

        // Leitura
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

        // Verificar tempo restante
        int maxTime, startTime;
        maxTime = stoi(maxTimeStr);
        startTime = stoi(startTimeStr);
        std::time_t currentTime = std::time(nullptr);
        std::string game_status = "ONGOING";

        int elapsedTime = currentTime - startTime;
        if (elapsedTime > maxTime) {
            game_status = "TIMEOUT";
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

        // Verificar tentativas já feitas
        file.open(gameFile);

        int currentLine = 0;

        while (currentLine < 7 && std::getline(file, line)) {
            currentLine++;
        }

        int trials = 0;
        std::vector<std::string> attempts;
        while (std::getline(file, line)) {
            attempts.push_back(line);
            trials++;
        }
        file.close();

        // Verificar vitória ou número máximo de tentativas
        if (nB == 4) {
            game_status = "WIN";
        } else if (trials >= 8 && game_status != "WIN") {
            game_status = "FAIL";
        }

        // Se o jogo for encerrado
        if (game_status != "ONGOING") {
            std::time_t endTime = std::time(nullptr);
            std::tm* endTm = std::localtime(&endTime);

            char endDate[11];
            std::strftime(endDate, sizeof(endDate), "%Y-%m-%d", endTm);

            char endTimeStr[9];
            std::strftime(endTimeStr, sizeof(endTimeStr), "%H:%M:%S", endTm);

            std::stringstream ss;
            ss << "GAMES/" << player_id << "/" << endDate << "_" << endTimeStr << "_" << game_status[0] << ".txt";
            std::string newFileName = ss.str();

            // Criar diretoria do jogador se não existir
            std::system(("mkdir -p GAMES/" + player_id).c_str());

            std::rename(gameFile.c_str(), newFileName.c_str());

            std::ofstream outFinal(newFileName, std::ios::app);
            outFinal << endDate << " " << endTimeStr << " " << elapsedTime << "\n";
            outFinal.close();

            // Se o jogo for terminado com sucesso
            if (game_status == "WIN") {
                std::string score = "0";

                // Calcular score
                std::string scoreFileName = "SCORES/" + score + "_" + player_id + "_" + endDate + "_" + endTimeStr + ".txt";
                std::ofstream scoreFile(scoreFileName);

                scoreFile << score << " " << player_id << " " << secretKey << " " << trials << " ";
                scoreFile << (mode == "P" ? "PLAY" : "DEBUG") << std::endl;

                scoreFile.close();
            }
        }
        hints = {std::to_string(nB), std::to_string(nW), std::to_string(trials), game_status, secretKey};
        return hints;
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

        if (elapsedTime > maxTime) {
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

        if (nB == 4) {
            return "OK " + std::to_string(nT) + " " + std::to_string(nB) + " " + std::to_string(nW) + " WIN";
        } else if (trials >= 7) {
            return "ENT " + secretKey;
        }

        return "OK " + std::to_string(nT) + " " + std::to_string(nB) + " " + std::to_string(nW);
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

    /*int FindPlayerTopScores(SCORELIST *list, int plid){
        struct dirent **filelist;
        int n_entries, i_file;
        char fname[300];
        FILE *fp;
        char mode[8];

        n_entries = scandir("SCORES/", &filelist, 0, alphasort);

        if (n_entries <= 0)
            return(0);
        else {
            i_file = 0;
            while (n_entries--){
                if(filelist[n_entries]->d_name[0]!='.' && i_file < 10){
                    sprintf(fname, "SCORES/%s", filelist[n_entries]->d_name);
                    fp = fopen(fname, "r");
                    if (fp!= NULL){

                        int temp_id, temp_score, temp_tries;
                        std::string temp_key, temp_mode;

                        fscanf(fp, "%d_%s_%s_%d_%s", &temp_id, &temp_score, temp_key, &temp_tries, temp_mode);

                        if (temp_id == plid) {
                            list-> score[i_file];
                            list-> PLID[i_file].c_str();
                            list-> col_code[i_file].c_str();
                            list-> no_tries[i_file];
                            if (!strcmp(mode, "PLAY"))  
                                list->mode[i_file] = "PLAY";
                            if (!strcmp(mode, "DEBUG")) 
                                list->mode[i_file] = "DEBUG";
                            ++i_file;
                        }
                        fclose(fp);
                    }
                }
                free(filelist[n_entries]);
            }
            free(filelist);
        }

        list->n_scores=i_file;
        return(i_file);
    }

    std::vector<std::string> viewScoreboard(SCORELIST *list, int player_id){
        
        int scores = FindPlayerTopScores(SCORELIST list, player_id);
    }*/

}
