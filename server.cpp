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
void processStart(const std::string& message);
void processTry(const std::string& message);
void processShowTrials(const std::string& message);
void processScoreboard(const std::string& message);

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
        return -1;
    }

    return tcp_socket;
}

void handleUDPRequest(int udp_socket) {
    // Handle incoming UDP request
    // ...existing code...
}

void handleTCPRequest(int tcp_socket) {
    // Handle incoming TCP request
    // ...existing code...
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
