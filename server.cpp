#include <iostream>
#include <string>
#include <vector>
#include <cstring>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include "game.h"

// Função para processar mensagens do protocolo
std::string processStart(int player_id, int max_playtime){
    std::string response;
    //TO-DO: Checkar se este player_id tem um jogo ativo
    /*if (plid_has_activeGame){
            response = "RSG NOK"}*/

    std::vector<std::string> key;
    key = Game::startGame(player_id, max_playtime);

    std::cout << "PLID=" << player_id << ": new game (max " << max_playtime << " sec); Colors: ";
    for ( int i = 0; i < 4; i++){
        std::cout << key[i] << " ";
    }
    std::cout std::endl;

    return "RSG OK\n";
}

std::string processTry( int player_id, std::string guess, int num_tries){
    std::string response;
    
    std::vector<std::string> hints = Game::playAttempt(player_id, guess);
    /*TO-DO: if the secret key guess repeats a previous trial’s guess -> RTR DUP
    Adicionar o código no playAttempt para fazer essa verificação e adicionar
    uma string ao return do playAttempt a sinalizar caso seja Dup */
    if (1){
        response = "RTR DUP";
    } else if (atoi(hints[2]) != num_tries){ /*TO-DO: adicionar este caso INV, if the trial number 
        nT is the expected value minus 1, but the secretkey guess is different from the 
        one of the previous message?*/
        response = "RTR INV";
    } else if(/*TO-DO: !plid_has_activeGame*/){
        response = "RTR NOK\n";
    } else if(atoi(hints[2]) = 8 && hints[3] == "FAIL"){
        response = "RTR ENT ";
        for (int i = 0; i < 4; i++){
            response += hints[4][i] + " ";
        }
        response += "\n"
    } else if(hints[3] == "TIMEOUT"){
        response = "RTR ETM ";
        for (int i = 0; i < 4; i++){
            response += hints[4][i] + " ";
        }
        response += "\n";
    } else {
        response = "RTR OK " + num_tries + " " + hints[0] + " " + hints[1] + " \n";
    }

    std::cout << "PLID=" << player_id << ": try ";
    for ( int i = 0; i < 4; i++){
        std::cout << guess[i] << " ";
    }
    std::cout << "- nB = " << hints[0] << ", nW = " << hints[1]
    if (hints[3] == "ONGOING"){
        std::cout << "; not guessed" << std::endl;
    } else if (hints[3] == "WIN"){
        std::cout << ": WIN (game ended)";
    }

    return response;
}

std::string processQuit(int player_id){
    std::string response;
    if (player_has_activeGame){
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

std::string processDebug(int player_id, int max_playtime, std::vector<std::string> key){
    std::string response;
    if (!player_has_activeGame){
        Game::debugGame(player_id,max_playtime, key);
        response = "RDB OK\n";
    } else {
        response = "RDB NOK\n";
    }
    return response;
}

std::string processShowTrials(const std::string& message);
std::string processScoreboard(const std::string& message);

// Funções Protocolos
void sendUDPResponse(const std::string& response);
void sendTCPResponse(const std::string& response);

int create_udp_socket(struct addrinfo **res, int portNumber) {
    struct addrinfo hints = {};
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_PASSIVE;

    int status = getaddrinfo(NULL, std::to_string(portNumber).c_str(), &hints, res);
    if (status != 0) {
        std::cerr << "getaddrinfo error: " << gai_strerror(status) << std::endl;
        return -1;
    }

    int udp_socket = socket((*res)->ai_family, (*res)->ai_socktype, (*res)->ai_protocol);
    if (udp_socket == -1) {
        std::cerr << "UDP socket creation error." << std::endl;
        return -1;
    }

    if (bind(udp_socket, (*res)->ai_addr, (*res)->ai_addrlen) == -1) {
        std::cerr << "UDP socket bind error." << std::endl;
        close(udp_socket);
        return -1;
    }

    return udp_socket;
}

int create_tcp_socket(struct addrinfo **res, int portNumber) {
    struct addrinfo hints = {};
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    int status = getaddrinfo(NULL, std::to_string(portNumber).c_str(), &hints, res);
    if (status != 0) {
        std::cerr << "getaddrinfo error: " << gai_strerror(status) << std::endl;
        return -1;
    }

    int tcp_socket = socket((*res)->ai_family, (*res)->ai_socktype, (*res)->ai_protocol);
    if (tcp_socket == -1) {
        std::cerr << "TCP socket creation error." << std::endl;
        return -1;
    }

    if (bind(tcp_socket, (*res)->ai_addr, (*res)->ai_addrlen) == -1) {
        std::cerr << "TCP socket bind error." << std::endl;
        close(tcp_socket);
        return -1;/
    }

    return tcp_socket;
}

std::string cmdHandler(std::string command(buffer)){
    std::istringstream iss(command);
    std::string cmd_type;
    iss >> cmd_type;

    std::string response;

    if (cmd_type == "SNG"){
        int player_id, max_playtime;
        iss >> player_id >> max_playtime;

        if (player_id < 100000|| player_id >= 999999 || max_playtime <= 0 ||max_playtime > 600){
            response = "RSG ERR\n";
        } else {
            response = processStart(player_id, max_playtime);
        }
    } else if (cmd_type == "TRY"){
        int player_id, num_tries;
        int guess[4];

        iss >> player_id;
        if (player_id < 100000|| player_id >= 999999){
            response = "RTR ERR\n";
        }
        for (int i = 0; i < 4; i++){
            if ((iss >> guess[i]) == 0){
                response = "RTR ERR\n";
                break;
            } else if(guess[i] not in {"R", "G", "B", "Y", "O", "P"}){
                response = "RTR ERR\n"
            }
        }
        if ((iss >> num_tries) == 0 || num_tries <= 0){
            response = "TRY ERR\n";
        } else if (response == ""){
            response = processTry(player_id, guess, num_tries);
        }
    } else if (cmd_type == "QUT"){
        int player_id;
        iss >> player_id;
        
        if (player_id < 100000|| player_id >= 999999){
            response = "RQT ERR\n";
        } else {
            response = processQuit(player_id);
        }
    } else if (cmd_type == "DBG"){
        int player_id, max_playtime;
        std::vector<std::string> key;
        iss >> player_id >> max_playtime;
        for (int i = 0; i < 4; i++){
            iss >> key[i];
            if (key[i].length() != 1){
                response = "RDB ERR\n"
            }
        }
        if (player_id < 100000|| player_id >= 999999 ||
         max_playtime <= 0 ||max_playtime > 600 ||
         ){
            response = "RDB ERR\n";
         } else if (response = ""){
            response = processDebug(player_id, max_playtime, key);
         }
    } else if (cmd_type == "STR"){
        /* TO-DO: */
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
            std::string response = cmdHandler(command(buffer));

            // Envia resposta
            sendto(udp_socket, response.c_str(), response.size(), 0,
                   (struct sockaddr*)&client_addr, addrlen);
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
    std::string response = cmdHandler(command(buffer));

    // Envia a resposta ao cliente
    if (write(client_socket, response.c_str(), response.size()) < 0) {
        perror("TCP write failed");
    }

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
