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
#include <iostream>
#include <sstream>
#include <cstring>
#include <netdb.h> 

#define BUFFER_SIZE 128

// Global variables
std::string gsip = "127.0.0.1";
int gsport = 58078;
int udp_socket;
int tcp_socket;
int currentPlayerID;

// Funções Protocolos
std::string receiveUDPMessage() {
    struct sockaddr_in server_addr;
    socklen_t addrlen = sizeof(server_addr);
    char buffer[BUFFER_SIZE];

    fd_set read_fds;
    struct timeval timeout;
    timeout.tv_sec = 30;
    timeout.tv_usec = 0;

    FD_ZERO(&read_fds);
    FD_SET(udp_socket, &read_fds);

    int ret = select(udp_socket + 1, &read_fds, NULL, NULL, &timeout);
    if (ret == -1) {
        std::cerr << "Select error: " << strerror(errno) << std::endl;
        return "";
    } else if (ret == 0) {
        std::cerr << "No server response. Request timed out." << std::endl;
        return "";
    }

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
        std::cerr << "UDP send failed: " << strerror(errno) << std::endl;
        return false;
    }

    return true;
}

std::string receiveTCPMessage() {
    char buffer[BUFFER_SIZE];

    fd_set read_fds;
    struct timeval timeout;
    timeout.tv_sec = 30;
    timeout.tv_usec = 0;

    FD_ZERO(&read_fds);
    FD_SET(tcp_socket, &read_fds);

    int ret = select(tcp_socket + 1, &read_fds, NULL, NULL, &timeout);
    if (ret == -1) {
        std::cerr << "Select error: " << strerror(errno) << std::endl;
        return "";
    } else if (ret == 0) {
        std::cerr << "No server response. Request timed out." << std::endl;
        return "";
    }

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
        std::cerr << "Failed to send message via TCP: " << strerror(errno) << std::endl;
        return false;
    }
    return true;
}

void handleShowTrials(int plid) {
    std::string message = "STR " + std::to_string(plid) + "\n";

    if (!sendTCPMessage(message)) {
        std::cerr << "Failed to send start message via TCP." << std::endl;
        return;
    }

    std::string response = receiveTCPMessage();

    std::cout << "Response: " << response << std::endl; 

    printf("Server response: %s\n", response.c_str());

    if (response.substr(0, 3) == "RST") {
        std::string status = response.substr(4, 3);
        if (status == "ACT\n" || status == "FIN\n") {
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

            printf("File received: %s (%d bytes)\n", fname.c_str(), fsize);
        } else if (status == "NOK\n") {
            printf("No ongoing or finished game found for player.\n");
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
        std::cerr << "Failed to send start message via TCP." << std::endl;
        return;
    }

    std::string response = receiveTCPMessage();
    printf("Server response: %s\n", response.c_str());

    if (response.substr(0, 3) == "RSS") {
        std::string status = response.substr(4, 5);
        if (status == "EMPTY\n") {
            printf("Scoreboard is empty.\n");
        } else if (status == "OK\n") {
            // O PLAYER GRAVA-O LOCALMENTE E DE SEGUIDA imprime o file no terminal linha a linha
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

            printf("File received: %s (%d bytes)\n", fname.c_str(), fsize);
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
        printf("Server response: %s\n", response.c_str());

        if (response.substr(0, 3) == "RSG") {
            std::string status = response.substr(4);
            if (status == "OK\n") {
                printf("Game started successfully.\n");
            } else if (status == "NOK\n") {
                printf("Failed to start game: ongoing game exists.\n");
            } else if (status == "ERR\n") {
                printf("Failed to start game: invalid request.\n");
            } else {
                printf("Unknown response status: %s\n", status.c_str());
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
        printf("Server response: %s\n", response.c_str());

        std::istringstream iss(response);
        std::string prefix, status;
        iss >> prefix >> status;

        if (prefix == "RTR") {

            if (strcmp(status.c_str(), "OK") == 0) {
                int nT, nB, nW;
                /*sscanf(response.c_str() + 7, "%d %d %d", &nT, &nB, &nW);*/
                iss >> nT >> nB >> nW;
                printf("Trial %d: %d correct positions, %d correct colors.\n", nT, nB, nW);
                if (nB == 4) {
                    trial_number = 0;
                    printf("Congratulations! You've guessed the secret key!\n");
                }
            } else if (strcmp(status.c_str(), "DUP") == 0) {
                trial_number--;
                printf("Duplicate trial.\n");
            } else if (strcmp(status.c_str(), "INV") == 0) {
                trial_number--;
                printf("Invalid trial.\n");
            } else if (strcmp(status.c_str(), "NOK") == 0) {
                trial_number--;
                printf("No ongoing game.\n");
            } else if (strcmp(status.c_str(), "ENT") == 0) {
                std::string key;
                iss >> key;
                printf("No more attempts. The secret key was: %s\n", key.c_str());
            } else if (strcmp(status.c_str(), "ETM") == 0) {
                std::string key;
                iss >> key;
                printf("Time exceeded. The secret key was: %s\n", key.c_str());
            } else if (strcmp(status.c_str(), "ERR") == 0) {
                trial_number--;
                printf("Error in request.\n");
            } else {
                trial_number--;
                printf("Unknown response status: %s\n", status.c_str());
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
        printf("Server response: %s\n", response.c_str());

        if (response.substr(0, 3) == "RQT") {
            std::string status = response.substr(4, 2);
            if (status == "OK\n") {
                std::string key = response.substr(7);
                printf("Game terminated. The secret key was: %s\n", key.c_str());
            } else if (status == "NOK\n") {
                printf("No ongoing game to terminate.\n");
            } else if (status == "ERR\n") {
                printf("Error in request.\n");
            } else {
                printf("Unknown response status: %s\n", status.c_str());
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
        printf("Server response: %s\n", response.c_str());

        if (response.substr(0, 3) == "RDB") {
            std::string status = response.substr(4);
            if (status == "OK\n") {
                printf("Debug game started successfully.\n");
            } else if (status == "NOK\n") {
                printf("Failed to start debug game: ongoing game exists.\n");
            } else if (status == "ERR\n") {
                printf("Failed to start debug game: invalid request.\n");
            } else {
                printf("Unknown response status: %s\n", status.c_str());
            }
        } else {
            std::cerr << "Unexpected response: " << response << std::endl;
            sendUDPMessage("ERR\n");
        }
    }
}

bool validateStartCommand(int plid, int max_playtime) {
    if (plid < 100000 || plid > 999999) {
        std::cerr << "Invalid player ID. It must be a 6-digit number." << std::endl;
        return false;
    }

    if (max_playtime < 1 || max_playtime > 600) {
        std::cerr << "Invalid max playtime. It must be between 1 and 600." << std::endl;
        return false;
    }

    return true;
}

bool validateTryCommand(int plid, const std::vector<std::string>& guess) {
    if (plid < 100000 || plid > 999999) {
        std::cerr << "Invalid player ID. It must be a 6-digit number." << std::endl;
        return false;
    }

    if (guess.size() != 4) {
        std::cerr << "Invalid guess. It must have 4 colors." << std::endl;
        return false;
    }

    for (const auto& color : guess) {
        if (color != "R" && color != "G" && color != "B" && color != "Y" &&
            color != "O" && color != "P") {
            std::cerr << "Invalid color: " << color << std::endl;
            return false;
        }
    }

    return true;
}

bool validateDebugCommand(int plid, int max_playtime, const std::vector<std::string>& key) {
    if (plid < 100000 || plid > 999999) {
        std::cerr << "Invalid player ID. It must be a 6-digit number." << std::endl;
        return false;
    }

    if (max_playtime < 1 || max_playtime > 600) {
        std::cerr << "Invalid max playtime. It must be between 1 and 600." << std::endl;
        return false;
    }

    if (key.size() != 4) {
        std::cerr << "Invalid key. It must have 4 colors." << std::endl;
        return false;
    }

    for (const auto& color : key) {
        if (color != "R" && color != "G" && color != "B" && color != "Y" &&
            color != "O" && color != "P") {
            std::cerr << "Invalid color: " << color << std::endl;
            return false;
        }
    }

    return true;
}

void cmdParser() {
    char command[BUFFER_SIZE];
    while (true) {
        scanf("%s", command);

        if (strcmp(command, "start") == 0) {
            int plid, max_playtime;
            scanf("%d %d", &plid, &max_playtime);
            if (validateStartCommand(plid, max_playtime)) {
                currentPlayerID = plid;
                handleStart(plid, max_playtime);
            }
        } else if (strcmp(command, "try") == 0) {
            int plid;
            scanf("%d", &plid);

            std::vector<std::string> guess;
            for (int i = 0; i < 4; i++) {
                char color[BUFFER_SIZE];
                scanf("%s", color);
                guess.push_back(color);
            }

            if (validateTryCommand(plid, guess)) {
                handleTry(plid, guess);
            }
        } else if (strcmp(command, "show_trials") == 0) {
            handleShowTrials(currentPlayerID);
        } else if (strcmp(command, "scoreboard") == 0 || strcmp(command, "sb") == 0) {
            handleScoreboard();
        } else if (strcmp(command, "quit") == 0) {
            handleQuit(currentPlayerID);
        } else if (strcmp(command, "debug") == 0) {
            int plid, max_playtime;
            scanf("%d %d", &plid, &max_playtime);

            std::vector<std::string> key;
            for (int i = 0; i < 4; i++) {
                char color[BUFFER_SIZE];
                scanf("%s", color);
                key.push_back(color);
            }

            if (validateDebugCommand(plid, max_playtime, key)) {
                handleDebug(plid, max_playtime, key);
            }
        } else if (strcmp(command, "exit") == 0) {
            break;
        } else {
            fprintf(stderr, "Invalid command.\n");
        }
    }
}

// Funções para criar sockets
int create_udp_socket(struct addrinfo **res) {
    struct addrinfo hints = {};
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;

    int status = getaddrinfo(gsip.c_str(), std::to_string(gsport).c_str(), &hints, res);
    if (status != 0) {
        std::cerr << "getaddrinfo error: " << gai_strerror(status) << std::endl;
        return -1;
    }

    int udp_socket = socket((*res)->ai_family, (*res)->ai_socktype, (*res)->ai_protocol);
    if (udp_socket == -1) {
        std::cerr << "UDP socket creation error: " << strerror(errno) << std::endl;
        freeaddrinfo(*res);
        return -1;
    }

    // No need to bind to a specific port, let the OS choose
    return udp_socket;
}

int create_tcp_socket(struct addrinfo **res) {
    struct addrinfo hints = {};
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    int status = getaddrinfo(gsip.c_str(), std::to_string(gsport).c_str(), &hints, res);
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

    // No need to bind to a specific port, let the OS choose
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

    printf("GSIP: %s, GSPort: %d\n", gsip.c_str(), gsport);

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

