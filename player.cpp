//#include <arpa/inet.h>
//#include <netdb.h>
#include <getopt.h>
#include <vector>
#include <string>
#include <cstdlib>
#include <ctime>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fstream>

#define BUFFER_SIZE 128

// Global variables
std::string gsip = "127.0.0.1";
int gsport = 58078;
int udp_socket;
int tcp_socket;

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

std::string receiveTCPMessage() {
    char buffer[BUFFER_SIZE];
    ssize_t n = recv(tcp_socket, buffer, sizeof(buffer) - 1, 0);
    if (n <= 0) {
        std::cerr << "Failed to receive response via TCP." << std::endl;
        return "";
    }

    buffer[n] = '\0';
    return std::string(buffer);
}

bool sendTCPMessage(const std::string& message) {
    ssize_t n = send(tcp_socket, message.c_str(), message.size(), 0);
    if (n == -1) {
        std::cerr << "Failed to send message via TCP." << std::endl;
        return false;
    }

    return true;
}

void handleShowTrials(int plid) {
    std::string message = "STR " + std::to_string(plid) + "\n";

    if (!sendTCPMessage(message)) {
        return;
    }

    std::string response = receiveTCPMessage();
    std::cout << "Server response: " << response << std::endl;

    if (response.substr(0, 3) == "RST") {
        std::string status = response.substr(4, 3);
        if (status == "ACT" || status == "FIN") {
            std::string fname;
            int fsize;
            std::istringstream iss(response.substr(8));
            iss >> fname >> fsize;

            std::ofstream outfile(fname, std::ios::binary);
            char buffer[BUFFER_SIZE];
            int bytes_received = 0;
            while (bytes_received < fsize) {
                ssize_t n = recv(tcp_socket, buffer, std::min(BUFFER_SIZE, fsize - bytes_received), 0);
                if (n <= 0) {
                    std::cerr << "Failed to receive file data via TCP." << std::endl;
                    return;
                }
                outfile.write(buffer, n);
                bytes_received += n;
            }
            outfile.close();

            std::cout << "File received: " << fname << " (" << fsize << " bytes)" << std::endl;
        } else if (status == "NOK") {
            std::cout << "No ongoing or finished game found for player." << std::endl;
        } else {
            std::cerr << "Unknown response status: " << status << std::endl;
        }
    } else {
        std::cerr << "Unexpected response: " << response << std::endl;
        sendTCPMessage("ERR\n");
    }
}

void handleScoreboard() {
    std::string message = "SSB\n";

    if (!sendTCPMessage(message)) {
        return;
    }

    std::string response = receiveTCPMessage();
    std::cout << "Server response: " << response << std::endl;

    if (response.substr(0, 3) == "RSS") {
        std::string status = response.substr(4, 5);
        if (status == "EMPTY") {
            std::cout << "Scoreboard is empty." << std::endl;
        } else if (status == "OK") {
            std::string fname;
            int fsize;
            std::istringstream iss(response.substr(10));
            iss >> fname >> fsize;

            std::ofstream outfile(fname, std::ios::binary);
            char buffer[BUFFER_SIZE];
            int bytes_received = 0;
            while (bytes_received < fsize) {
                ssize_t n = recv(tcp_socket, buffer, std::min(BUFFER_SIZE, fsize - bytes_received), 0);
                if (n <= 0) {
                    std::cerr << "Failed to receive file data via TCP." << std::endl;
                    return;
                }
                outfile.write(buffer, n);
                bytes_received += n;
            }
            outfile.close();

            std::cout << "File received: " << fname << " (" << fsize << " bytes)" << std::endl;
        } else {
            std::cerr << "Unknown response status: " << status << std::endl;
        }
    } else {
        std::cerr << "Unexpected response: " << response << std::endl;
        sendTCPMessage("ERR\n");
    }
}

// Funções para lidar com os comandos do jogador por UDP
void handleStart(int plid, int max_playtime) {
    char time_str[4];
    snprintf(time_str, sizeof(time_str), "%03d", max_playtime);
    std::string message = "SNG " + std::to_string(plid) + " " + time_str + "\n";

    if (!sendUDPMessage(message)) {
        std::cerr << "Failed to send start message via UDP." << std::endl;
    } else {
        std::string response = receiveUDPMessage();
        std::cout << "Server response: " << response << std::endl;

        if (response.substr(0, 3) == "RSG") {
            std::string status = response.substr(4);
            if (status == "OK") {
                std::cout << "Game started successfully." << std::endl;
            } else if (status == "NOK") {
                std::cout << "Failed to start game: ongoing game exists." << std::endl;
            } else if (status == "ERR") {
                std::cout << "Failed to start game: invalid request." << std::endl;
            } else {
                std::cout << "Unknown response status: " << status << std::endl;
            }
        } else {
            std::cerr << "Unexpected response: " << response << std::endl;
            sendUDPMessage("ERR\n");
        }
    }
}

void handleTry(int plid, const std::vector<std::string>& guess) {
    static int trial_number = 0;
    trial_number++;

    std::string message = "TRY " + std::to_string(plid);
    for (const auto& color : guess) {
        message += " " + color;
    }
    message += " " + std::to_string(trial_number) + "\n";

    if (!sendUDPMessage(message)) {
        std::cerr << "Failed to send try message via UDP." << std::endl;
    } else {
        std::string response = receiveUDPMessage();
        std::cout << "Server response: " << response << std::endl;

        if (response.substr(0, 3) == "RTR") {
            std::string status = response.substr(4, 2);
            if (status == "OK") {
                int nT, nB, nW;
                sscanf(response.c_str() + 7, "%d %d %d", &nT, &nB, &nW);
                std::cout << "Trial " << nT << ": " << nB << " correct positions, " << nW << " correct colors." << std::endl;
                if (nB == 4) {
                    std::cout << "Congratulations! You've guessed the secret key!" << std::endl;
                }
            } else if (status == "DUP") {
                std::cout << "Duplicate trial." << std::endl;
            } else if (status == "INV") {
                std::cout << "Invalid trial." << std::endl;
            } else if (status == "NOK") {
                std::cout << "No ongoing game." << std::endl;
            } else if (status == "ENT") {
                std::string key = response.substr(7);
                std::cout << "No more attempts. The secret key was: " << key << std::endl;
            } else if (status == "ETM") {
                std::string key = response.substr(7);
                std::cout << "Time exceeded. The secret key was: " << key << std::endl;
            } else if (status == "ERR") {
                std::cout << "Error in request." << std::endl;
            } else {
                std::cout << "Unknown response status: " << status << std::endl;
            }
        } else {
            std::cerr << "Unexpected response: " << response << std::endl;
            sendUDPMessage("ERR\n");
        }
    }
}

void handleQuit(int plid) {
    std::string message = "QUT " + std::to_string(plid) + "\n";

    if (!sendUDPMessage(message)) {
        std::cerr << "Failed to send quit message via UDP." << std::endl;
    } else {
        std::string response = receiveUDPMessage();
        std::cout << "Server response: " << response << std::endl;

        if (response.substr(0, 3) == "RQT") {
            std::string status = response.substr(4, 2);
            if (status == "OK") {
                std::string key = response.substr(7);
                std::cout << "Game terminated. The secret key was: " << key << std::endl;
            } else if (status == "NOK") {
                std::cout << "No ongoing game to terminate." << std::endl;
            } else if (status == "ERR") {
                std::cout << "Error in request." << std::endl;
            } else {
                std::cout << "Unknown response status: " << status << std::endl;
            }
        } else {
            std::cerr << "Unexpected response: " << response << std::endl;
            sendUDPMessage("ERR\n");
        }
    }
}

void handleDebug(int plid, int max_playtime, const std::vector<std::string>& key) {
    char time_str[4];
    snprintf(time_str, sizeof(time_str), "%03d", max_playtime);
    std::string message = "DBG " + std::to_string(plid) + " " + time_str;

    for (const auto& color : key) {
        message += " " + color;
    }
    message += "\n";

    if (!sendUDPMessage(message)) {
        std::cerr << "Failed to send debug message via UDP." << std::endl;
    } else {
        std::string response = receiveUDPMessage();
        std::cout << "Server response: " << response << std::endl;

        if (response.substr(0, 3) == "RDB") {
            std::string status = response.substr(4);
            if (status == "OK") {
                std::cout << "Debug game started successfully." << std::endl;
            } else if (status == "NOK") {
                std::cout << "Failed to start debug game: ongoing game exists." << std::endl;
            } else if (status == "ERR") {
                std::cout << "Failed to start debug game: invalid request." << std::endl;
            } else {
                std::cout << "Unknown response status: " << status << std::endl;
            }
        } else {
            std::cerr << "Unexpected response: " << response << std::endl;
            sendUDPMessage("ERR\n");
        }
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
    udp_socket = create_udp_socket(&udp_res);

    if (udp_socket == -1) {
        std::cerr << "Failed to create UDP socket." << std::endl;
        return 1;
    }

    struct addrinfo *tcp_res;
    tcp_socket = create_tcp_socket(&tcp_res);

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

