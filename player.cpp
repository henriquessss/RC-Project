//#include <arpa/inet.h>
//#include <netdb.h>
#include <getopt.h>
#include <vector>
#include <string>
#include <cstdlib>
#include <ctime>

#define BUFFER_SIZE 128

// Global variables
std::string gsip = "127.0.0.1";
int gsport = 58078;

// Função para lidar com os comandos do jogador por TCP
void handleShowTrials(int plid) {
    std::string message = "STR " + std::to_string(plid);

    if (!sendUDPMessage(message)) {
        std::cerr << "Failed to send show message via UDP." << std::endl;
    }
}

void handleScoreboard() {
    std::string message = "SSB";

    if (!sendUDPMessage(message)) {
        std::cerr << "Failed to send scoreboard message via UDP." << std::endl;
    }
}


// Funções Protocolos
std::string receiveUDPMessage() {
    struct sockaddr_in server_addr;
    socklen_t addrlen = sizeof(server_addr);
    char buffer[BUFFER_SIZE];

    memset(buffer, 0, sizeof(buffer));
    ssize_t n = recvfrom(udp_socket, buffer, sizeof(buffer) - 1, 0,
                         (struct sockaddr*)&server_addr, &addrlen);

    if (n <= 0) {
        std::cerr << "UDP receive failed." << std::endl;
        return "";
    }

    buffer[n] = '\0';
    return std::string(buffer);
}

bool sendUDPMessage(const std::string& message) {
    struct addrinfo hints = {};
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;

    struct addrinfo *res;
    int status = getaddrinfo(gsip.c_str(), std::to_string(gsport).c_str(), &hints, &res);
    if (status != 0) {
        std::cerr << "getaddrinfo error: " << gai_strerror(status) << std::endl;
        return false;
    }

    ssize_t n = sendto(udp_socket, message.c_str(), message.size(), 0, res->ai_addr, res->ai_addrlen);
    freeaddrinfo(res);

    if (n == -1) {
        std::cerr << "UDP send failed." << std::endl;
        return false;
    }

    return true;
}

// Funções para lidar com os comandos do jogador por UDP
void handleStart(int plid, int max_playtime) {
    char time_str[4];
    snprintf(time_str, sizeof(time_str), "%03d", max_playtime);
    std::string message = "SNG " + std::to_string(plid) + " " + time_str;

    if (!sendUDPMessage(message)) {
        std::cerr << "Failed to send start message via UDP." << std::endl;
    }
}

void handleTry(int plid, const std::vector<std::string>& guess) {
    static int trial_number = 0;
    trial_number++;

    std::string message = "TRY " + std::to_string(plid);
    for (const auto& color : guess) {
        message += " " + color;
    }
    message += " " + std::to_string(trial_number);

    if (!sendUDPMessage(message)) {
        std::cerr << "Failed to send try message via UDP." << std::endl;
    }
}

void handleQuit(int plid) {
    std::string message = "QUT " + std::to_string(plid);

    if (!sendUDPMessage(message)) {
        std::cerr << "Failed to send quit message via UDP." << std::endl;
    }
}

void handleDebug(int plid, int max_playtime, const std::vector<std::string>& key) {
    char time_str[4];
    snprintf(time_str, sizeof(time_str), "%03d", max_playtime);
    std::string message = "DBG " + std::to_string(plid) + " " + time_str;

    for (const auto& color : key) {
        message += " " + color;
    }

    if (!sendUDPMessage(message)) {
        std::cerr << "Failed to send debug message via UDP." << std::endl;
    }
}

void cmdParser() {
    std::string command;
    while (true) {
        std::cin >> command;

        if (command == "start") {
            int plid, max_playtime;
            std::cin >> plid >> max_playtime;
            handleStart(plid, max_playtime);
        } else if (command == "try") {
            int plid;
            std::cin >> plid;

            std::vector<std::string> guess;
            for (int i = 0; i < 4; i++) {
                std::string color;
                std::cin >> color;
                guess.push_back(color);
            }

            handleTry(plid, guess);
        } else if (command == "show") {
            int plid;
            std::cin >> plid;
            handleShowTrials(plid);
        } else if (command == "scoreboard") {
            handleScoreboard();
        } else if (command == "quit") {
            int plid;
            std::cin >> plid;
            handleQuit(plid);
        } else if (command == "debug") {
            int plid, max_playtime;
            std::cin >> plid >> max_playtime;

            std::vector<std::string> key;
            for (int i = 0; i < 4; i++) {
                std::string color;
                std::cin >> color;
                key.push_back(color);
            }

            handleDebug(plid, max_playtime, key);
        } else if (command == "exit") {
            break;
        } else {
            std::cerr << "Invalid command." << std::endl;
        }
    }
}

// Funções para criar sockets
int create_udp_socket(struct addrinfo **res) {
    struct addrinfo hints = {};
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;

    int status = getaddrinfo(NULL, std::to_string(gsport).c_str(), &hints, res);
    if (status != 0) {
        std::cerr << "getaddrinfo error: " << gai_strerror(status) << std::endl;
        return -1;
    }

    int udp_socket = socket((*res)->ai_family, (*res)->ai_socktype, (*res)->ai_protocol);
    if (udp_socket == -1) {
        std::cerr << "UDP socket creation error." << std::endl;
        freeaddrinfo(*res);
        return -1;
    }

    if (bind(udp_socket, (*res)->ai_addr, (*res)->ai_addrlen) == -1) {
        std::cerr << "UDP socket bind error." << std::endl;
        close(udp_socket);
        freeaddrinfo(*res);
        return -1;
    }

    return udp_socket;
}

int create_tcp_socket(struct addrinfo **res) {
    struct addrinfo hints = {};
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    int status = getaddrinfo(NULL, std::to_string(gsport).c_str(), &hints, res);
    if (status != 0) {
        std::cerr << "getaddrinfo error: " << gai_strerror(status) << std::endl;
        return -1;
    }

    int tcp_socket = socket((*res)->ai_family, (*res)->ai_socktype, (*res)->ai_protocol);
    if (tcp_socket == -1) {
        std::cerr << "TCP socket creation error." << std::endl;
        freeaddrinfo(*res);
        return -1;
    }

    if (bind(tcp_socket, (*res)->ai_addr, (*res)->ai_addrlen) == -1) {
        std::cerr << "TCP socket bind error." << std::endl;
        close(tcp_socket);
        freeaddrinfo(*res);
        return -1;
    }

    return tcp_socket;
}

// Função principal da aplicação do jogador
int main(int argc, char *argv[]) {
    int opt;
    while ((opt = getopt(argc, argv, "n:p:")) != -1) {
        switch (opt) {
            case 'n':
                gsip = optarg;
                break;
            case 'p':
                gsport = std::atoi(optarg);
                break;
            default:
                std::cerr << "Usage: " << argv[0] << " [-n GSIP] [-p GSPort]" << std::endl;
                exit(EXIT_FAILURE);
        }
    }

    std::cout << "GSIP: " << gsip << ", GSPort: " << gsport << std::endl;

    // Criar sockets
    struct addrinfo *udp_res;
    int udp_socket = create_udp_socket(&udp_res);

    if (udp_socket == -1) {
        std::cerr << "Failed to create UDP socket." << std::endl;
        return 1;
    }

    struct addrinfo *tcp_res;
    int tcp_socket = create_tcp_socket(&tcp_res);

    if (tcp_socket == -1) {
        std::cerr << "Failed to create TCP socket." << std::endl;
        freeaddrinfo(udp_res);
        return 1;
    }

    // parse commands
    cmdParser();

    freeaddrinfo(udp_res);
    freeaddrinfo(tcp_res);

    return 0;
}

