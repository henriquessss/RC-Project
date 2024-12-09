#include <iostream>
#include <string>
#include <vector>
#include "game.cpp"


// Função para lidar com conexões UDP 
void handleUDPRequest();

// Função para lidar com conexões TCP
void handleTCPRequest();

// Função para processar mensagens do protocolo
void processStart(const std::string& message);
void processTry(const std::string& message);
void processShowTrials(const std::string& message);
void processScoreboard(const std::string& message);

// Funções Protocolos
void sendUDPResponse(const std::string& response);
void sendTCPResponse(const std::string& response);

// Funções para criar sockets
int create_udp_socket(struct addrinfo **res);
int create_tcp_socket(struct addrinfo **res);

// Função principal do servidor
int main(int argc, char *argv[]);