#include <iostream>
#include <string>
#include <vector>
#include <cstring>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <netdb.h>
#include <unistd.h>
#include <sstream> 
#include <fstream>
#include "game.h"

// Funções auxiliares
bool playerHasActiveGame(int player_id) {
    std::string filename = "GAMES/GAME_" + std::to_string(player_id) + ".txt";
    std::ifstream file(filename);
    return file.good();
}

// Função para processar mensagens do protocolo
std::string processStart(int player_id, int max_playtime){
    std::string response;
    if (playerHasActiveGame(player_id)) {
        return "RSG NOK\n";
    }

    std::vector<std::string> key;
    key = Game::startGame(player_id, max_playtime);

    std::cout << "PLID=" << player_id << ": new game (max " << max_playtime << " sec); Colors: ";
    for ( int i = 0; i < 4; i++){
        std::cout << key[i] << " ";
    }
    std::cout << std::endl; 

    return "RSG OK\n";
}

std::string processTry(int player_id, const std::vector<std::string>& guess, int num_tries) {
    std::string response;
    std::string result = Game::playAttempt(player_id, num_tries, guess);

    if (result == "NOK") {
        response = "RTR NOK\n";
    } else if (result.substr(0, 3) == "ETM") {
        response = "RTR ETM " + result.substr(4) + "\n";
    } else if (result == "INV") {
        response = "RTR INV\n";
    } else if (result == "DUP") {
        response = "RTR DUP\n";
    } else if (result.substr(0, 2) == "OK") {
        response = "RTR " + result + "\n";
    } else if (result.substr(0, 3) == "ENT") {
        response = "RTR ENT " + result.substr(4) + "\n";
    }

    std::cout << "PLID=" << player_id << ": try ";
    for (const auto& color : guess) {
        std::cout << color << " ";
    }
    std::cout << "- Result: " << result << std::endl;

    return response;
}

std::string processQuit(int player_id){
    std::string response;
    if (playerHasActiveGame(player_id)){
        std::vector<std::string> key = Game::quitGame(player_id);
        response = "RQT OK ";
        for (int i = 0; i < 4; i++) {
            response += key[i] + " ";
        }
        response += "\n";
    } else {
        response = "RQT NOK\n";
    }
    return response;
}

std::string processDebug(int player_id, int max_playtime, const std::vector<std::string>& key){
    std::string response;
    if (!playerHasActiveGame(player_id)){
        Game::debugGame(player_id,max_playtime, key);
        response = "RDB OK\n";
    } else {
        response = "RDB NOK\n";
    }
    return response;
}

std::string processShowTrials(int player_id){
    return Game::showTrials(player_id);
}

std::string processScoreboard(){
    Game::SCORELIST topScores;
    int n_scores = Game::FindTopScores(&topScores);

    if (n_scores == 0){
        return "RSS EMPTY\n";
    }

    std::string filename = "scoreboard.txt";
    std::ofstream scoreBoardFile(filename);

    for ( int i = 0; i < n_scores; i++) {
        scoreBoardFile << topScores.PLID[i] << " "
                        << topScores.col_code[i] << " "
                        << topScores.no_tries[i] << "\n";
    }
    scoreBoardFile.close();

    struct stat fileStat;
    stat(filename.c_str(), &fileStat);
    int fileSize = fileStat.st_size;

    std::ifstream scoreBoardRead(filename);
    std::stringstream fileData;
    fileData << scoreBoardRead.rdbuf();
    scoreBoardRead.close();

    std::stringstream response;
    response << "RSS OK " << filename << " " << fileSize << "\n" << fileData.str();

    return response.str();
}

// Funções Protocolos
void sendUDPResponse(const std::string& response, int udp_socket, struct sockaddr_in* client_addr, socklen_t addrlen) {
    sendto(udp_socket, response.c_str(), response.size(), 0, (struct sockaddr*)client_addr, addrlen);
}

void sendTCPResponse(const std::string& response, int client_socket) {
    write(client_socket, response.c_str(), response.size());
}

int create_udp_socket(struct addrinfo **res, int portNumber) {
    struct addrinfo hints = {};
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_PASSIVE;

    int status = getaddrinfo("0.0.0.0", std::to_string(portNumber).c_str(), &hints, res); // Change back to wildcard address
    if (status != 0) {
        std::cerr << "getaddrinfo error: " << gai_strerror(status) << std::endl;
        return -1;
    }

    int udp_socket = socket((*res)->ai_family, (*res)->ai_socktype, (*res)->ai_protocol);
    if (udp_socket == -1) {
        std::cerr << "UDP socket creation error: " << strerror(errno) << std::endl;
        return -1;
    }

    if (bind(udp_socket, (*res)->ai_addr, (*res)->ai_addrlen) == -1) {
        std::cerr << "UDP socket bind error: " << strerror(errno) << std::endl;
        close(udp_socket);
        return -1;
    }

    std::cout << "UDP socket bound to 0.0.0.0:" << portNumber << std::endl; // Add logging

    return udp_socket;
}

int create_tcp_socket(struct addrinfo **res, int portNumber) {
    struct addrinfo hints = {};
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    int status = getaddrinfo("0.0.0.0", std::to_string(portNumber).c_str(), &hints, res); // Change back to wildcard address
    if (status != 0) {
        std::cerr << "getaddrinfo error: " << gai_strerror(status) << std::endl;
        return -1;
    }

    int tcp_socket = socket((*res)->ai_family, (*res)->ai_socktype, (*res)->ai_protocol);
    if (tcp_socket == -1) {
        std::cerr << "TCP socket creation error: " << strerror(errno) << std::endl;
        return -1;
    }

    if (bind(tcp_socket, (*res)->ai_addr, (*res)->ai_addrlen) == -1) {
        std::cerr << "TCP socket bind error: " << strerror(errno) << std::endl;
        close(tcp_socket);
        return -1; 
    }

    std::cout << "TCP socket bound to 0.0.0.0:" << portNumber << std::endl; // Add logging

    return tcp_socket;
}

std::string cmdHandler(const std::string& command){ 
    std::istringstream iss(command);
    std::string cmd_type;
    iss >> cmd_type;

    std::string response;

    if (cmd_type == "SNG"){
        int player_id, max_playtime;
        iss >> player_id >> max_playtime;

        if (player_id < 100000 || player_id >= 999999 || max_playtime <= 0 || max_playtime > 600){
            response = "RSG ERR\n";
        } else {
            response = processStart(player_id, max_playtime);
        }
    } else if (cmd_type == "TRY"){
        int player_id, num_tries;
        std::vector<std::string> guess(4); 

        iss >> player_id;
        if (player_id < 100000 || player_id >= 999999){
            response = "RTR ERR\n";
        }
        for (int i = 0; i < 4; i++){
            if (!(iss >> guess[i])){
                response = "RTR ERR\n";
                break;
            } else if(guess[i] != "R" && guess[i] != "G" && guess[i] != "B" && guess[i] != "Y" && guess[i] != "O" && guess[i] != "P"){ // Fix syntax
                response = "RTR ERR\n";
            }
        }
        if (!(iss >> num_tries) || num_tries <= 0){
            response = "TRY ERR\n";
        } else if (response.empty()){
            response = processTry(player_id, guess, num_tries);
        }
    } else if (cmd_type == "QUT"){
        int player_id;
        iss >> player_id;
        
        if (player_id < 100000 || player_id >= 999999){
            response = "RQT ERR\n";
        } else {
            response = processQuit(player_id);
        }
    } else if (cmd_type == "DBG"){
        int player_id, max_playtime;
        std::vector<std::string> key(4); 
        iss >> player_id >> max_playtime;
        for (int i = 0; i < 4; i++){
            iss >> key[i];
            if (key[i].length() != 1){
                response = "RDB ERR\n";
            }
        }
        if (player_id < 100000 || player_id >= 999999 || max_playtime <= 0 || max_playtime > 600){
            response = "RDB ERR\n";
        } else if (response.empty()){
            response = processDebug(player_id, max_playtime, key);
        }
    } else if (cmd_type == "STR"){
        int player_id;
        iss >> player_id;

        if (player_id < 100000 || player_id >= 999999){
            response = "RST NOK\n";
        } else {
            response = processShowTrials(player_id);
        }
    } else if (cmd_type == "SSB"){
        response = processScoreboard();
    } else {
        response = "ERR\n";
    }
    return response;
}

void handleUDPRequest(int udp_socket) {
    struct sockaddr_in client_addr;
    socklen_t addrlen = sizeof(client_addr);
    char buffer[128];

    while (true) {
        memset(buffer, 0, sizeof(buffer));

        // Recebe mensagem
        ssize_t n = recvfrom(udp_socket, buffer, sizeof(buffer) - 1, 0,
                             (struct sockaddr*)&client_addr, &addrlen);
        if (n > 0) {
            buffer[n] = '\0';
            std::string command(buffer);

            //Função Parse + Process do comando
            std::string response = cmdHandler(command);

            // Envia resposta
            sendUDPResponse(response, udp_socket, &client_addr, addrlen);
        }
    }
}

void handleTCPRequest(int tcp_socket) {
    struct sockaddr_in client_addr;
    socklen_t addrlen = sizeof(client_addr);

    char buffer[128];

    // Aceita conexão do cliente
    int client_socket = accept(tcp_socket, (struct sockaddr*)&client_addr, &addrlen);
    if (client_socket < 0) {
        perror("TCP accept failed");
        return;
    }

    std::cout << "Client connected via TCP." << std::endl;

    // Recebe o comando do cliente
    memset(buffer, 0, sizeof(buffer));
    ssize_t n = read(client_socket, buffer, sizeof(buffer) - 1);
    if (n <= 0) {
        perror("TCP read failed");
        close(client_socket);
        return;
    }

    buffer[n] = '\0';
    std::string command(buffer);

    std::cout << "Received TCP command: " << command << std::endl;

    std::string response;

    // Trata os comandos STR e SSB
    response = cmdHandler(command); // Fix duplicate declaration

    // Envia a resposta ao cliente
    sendTCPResponse(response, client_socket);

    std::cout << "Response sent: " << response << std::endl;

    // Fecha a conexão com o cliente
    close(client_socket);
    std::cout << "Client disconnected." << std::endl;
}



// Função principal do servidor
int main(int argc, char *argv[]) {
    int portNumber = 58078; // Default port number
    bool verbose = false;

    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "-p") == 0 && i + 1 < argc) {
            portNumber = std::stoi(argv[++i]);
        } else if (strcmp(argv[i], "-v") == 0) {
            verbose = true;
        }
    }

    if (verbose) {
        std::cout << "Verbose mode enabled." << std::endl;
        std::cout << "Using port number: " << portNumber << std::endl;
    }

    // Criar sockets
    struct addrinfo *udp_res, *tcp_res;
    int udp_socket = create_udp_socket(&udp_res, portNumber);
    int tcp_socket = create_tcp_socket(&tcp_res, portNumber);

    if (udp_socket == -1 || tcp_socket == -1) {
        std::cerr << "Failed to create sockets." << std::endl;
        return 1;
    }

    // Listen for incoming TCP connections
    if (listen(tcp_socket, 10) == -1) {
        std::cerr << "Failed to listen on TCP socket." << std::endl;
        return 1;
    }

    fd_set readfds;
    int max_sd = std::max(udp_socket, tcp_socket);

    while (true) {
        FD_ZERO(&readfds);
        FD_SET(udp_socket, &readfds);
        FD_SET(tcp_socket, &readfds);

        int activity = select(max_sd + 1, &readfds, NULL, NULL, NULL);

        if (activity < 0 && errno != EINTR) {
            std::cerr << "Select error." << std::endl;
        }

        if (FD_ISSET(udp_socket, &readfds)) {
            handleUDPRequest(udp_socket);
        }

        if (FD_ISSET(tcp_socket, &readfds)) {
            handleTCPRequest(tcp_socket);
        }
    }

    freeaddrinfo(udp_res);
    freeaddrinfo(tcp_res);
    close(udp_socket);
    close(tcp_socket);

    return 0;
}
