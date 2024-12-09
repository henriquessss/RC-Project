//#include <arpa/inet.h>
//#include <netdb.h>
#include <getopt.h>
#include <vector>
#include <string>
#include <cstdlib>
#include <ctime>

#define BUFFER_SIZE 128

// Função para lidar com os comandos do jogador
void handleStart(int plid, int max_playtime);
void handleTry(int plid, const std::vector<std::string>& guess);
void handleShowTrials(int plid);
void handleScoreboard();
void handleQuit(int plid);

// Funções Protocolos
bool sendUDPMessage(const std::string& message);
std::string receiveUDPMessage();

// Funções para criar sockets
int create_udp_socket(struct addrinfo **res);
int create_tcp_socket(struct addrinfo **res);

// Função principal da aplicação do jogador
int main(int argc, char *argv[]);