#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <vector>
#include <fstream>
#include <sstream>
#include <string>
#include <ctime>
#include <unordered_map>

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
int start_game(const int player_id, const int max_playtime) {
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

int quit_Game() {
    if (!currentGameFilename.empty()) {
        currentGameFilename.clear();
        return 0;
    }
    return 1;
}

int debug_Game(const int player_id, const int max_playtime, const std::vector<std::string> key) {
    
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
std::string checkGameStatus(int plid, const std::vector<std::string>& guess, int& nB, int& nW) {
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

        char newFileName[21]; //YYYYMMDD_HHMMSS_(code).txt
        sprintf(newFileName, "GAMES/%s/%s_%s_%s.txt", player_id, endDate, endTimeStr, game_status[0]);

         // Criar diretoria do jogador se não existir e mover o ficheiro para a nova localização

        std::ofstream outFinal(newFileName, std::ios::app);
        outFinal << endDate << " " << endTimeStr << " " << elapsedTime << "\n";
        outFinal.close();
    }

    return game_status;
}

// Processa uma tentativa do jogador
// Função para processar uma tentativa do jogador
std::string play_trial(int plid, const std::vector<std::string>& guess) {
    int nB = 0, nW = 0;

    std::string status = checkGameStatus(plid, guess, nB, nW);

    if (status == "WIN") {
        std::cout << "WELL DONE! You guessed the key in x trials!\n";
    } else if (status == "FAIL") {
        std::cout << "Game over! Maximum number of trials reached.\n";
    } else if (status == "TIMEOUT") {
        std::cout << "Game over! Time limit exceeded.\n";
    } else if (status == "ONGOING") {
        std::cout << "nB: " << nB << ", nW:" << nW << std::endl;
    } else {
        std::cout << "Error: " << status << "\n";
    }

    return status;
}

int main() {
    std::cout << "Digite o comando (ex: start 123456 120 ou try R G B Y):\n";

    std::string command;
    while (true) {
        std::getline(std::cin, command);
        int plid;

        if (command.rfind("start", 0) == 0) {
            int player_id;
            int max_playtime;
            char mode = 'P';

            std::istringstream iss(command);
            std::string cmd;
            iss >> cmd >> player_id >> max_playtime;

            start_game(player_id, max_playtime);
            std::cout << "Jogo iniciado, tens "<< max_playtime << " segundos.\n";
            plid = player_id;

        } else if (command.rfind("try", 0) == 0) {
            std::vector<std::string> guess(4);

            std::istringstream iss(command);
            std::string cmd;
            iss >> cmd >> guess[0] >> guess[1] >> guess[2] >> guess[3];

            std::string status = play_trial(plid, guess);

            if (status == "WIN" || status == "FAIL" || status == "TIMEOUT") {
                break;
            }

        } else {
            std::cout << "Comando inválido. Tente novamente.\n";
        }
    }

    return 0;
}