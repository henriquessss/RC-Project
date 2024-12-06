#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <getopt.h>

#define BUFFER_SIZE 128

int create_udp_socket(struct addrinfo **res) {
    int fd;
    struct addrinfo hints;

    fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd == -1) {
        perror("Erro ao criar socket UDP");
        exit(1);
    }

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;

    if (getaddrinfo(server_name, server_port, &hints, res) != 0) {
        perror("Erro em getaddrinfo para UDP");
        exit(1);
    }

    return fd;
}

int create_tcp_socket(struct addrinfo **res) {
    int fd;
    struct addrinfo hints;

    fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd == -1) {
        perror("Erro ao criar socket TCP");
        exit(1);
    }

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    if (getaddrinfo(server_name, server_port, &hints, res) != 0) {
        perror("Erro em getaddrinfo para TCP");
        exit(1);
    }

    if (connect(fd, (*res)->ai_addr, (*res)->ai_addrlen) == -1) {
        perror("Erro ao conectar socket TCP");
        exit(1);
    }

    return fd;
}

void handle_udp_command(const char *command, const char *plid, const char *arg) {
    int fd;
    struct addrinfo *res;
    char buffer[BUFFER_SIZE];

    // Construção da mensagem baseada no comando
    if (strcmp(command, "start") == 0) {
        snprintf(buffer, sizeof(buffer), "SNG %s %03d\n", plid, atoi(arg));
    } else if (strcmp(command, "try") == 0) {
        snprintf(buffer, sizeof(buffer), "TRY %s %c\n", plid, arg[0]);
    } else if (strcmp(command, "quit") == 0 || strcmp(command, "exit") == 0)

    else {
        fprintf(stderr, "Comando UDP desconhecido: %s\n", command);
        close(fd);
        return;
    }

    // Enviar comando ao servidor
    if (sendto(fd, buffer, strlen(buffer), 0, res->ai_addr, res->ai_addrlen) == -1) {
        perror("Erro ao enviar comando UDP");
        exit(1);
    }

    // Receber resposta
    socklen_t addrlen = sizeof(struct sockaddr_in);
    struct sockaddr_in addr;
    ssize_t n = recvfrom(fd, buffer, BUFFER_SIZE, 0, (struct sockaddr *)&addr, &addrlen);
    if (n == -1) {
        perror("Erro ao receber resposta UDP");
    }

    freeaddrinfo(res);
    close(fd);
}

// Função para lidar com comandos TCP
void handle_tcp_command(const char *command, const char *plid) {
    int fd;
    struct addrinfo *res;
    char buffer[BUFFER_SIZE];


    if (strcmp(command, "show_trials") == 0 || strcmp(command, "st") == 0) {
        snprintf(buffer, sizeof(buffer), "STR %s\n", plid);
    } else if(strcmp(command, "scoreboard") == 0) {
        snprintf(buffer, sizeof(buffer), "SSB\n");
    } else {
        fprintf(stderr, "Comando TCP desconhecido: %s\n", command);
        freeaddrinfo(res);
        close(fd);
        return;
    }

    // Enviar comando ao servidor
    if (write(fd, buffer, strlen(buffer)) == -1) {
        perror("Erro ao enviar comando TCP");
        exit(1);
    }

    // Receber resposta
    ssize_t n = read(fd, buffer, BUFFER_SIZE - 1);
    if (n == -1) {
        perror("Erro ao receber resposta TCP");
    }

    freeaddrinfo(res);
    close(fd);
}

void parse_arguments(){

    return 0;
}

int main(int argc, char *argv[]) {
    int fd1, fd2;
    parse_arguments(argc, argv);

    fd1 = create_udp_socket(&res);
    fd2 = create_tcp_socket(&res);

    const char *plid = "103403";
    const char *command = argv[0];

    while (1){

        if (fgets(input, sizeof(input), stdin) == NULL) {
            fprintf(stderr, "Erro ao ler o comando.\n");
            continue;
        }
    }
    // Determinar se o comando é UDP ou TCP
    if (strcmp(command, "start") == 0 || strcmp(command, "try") == 0 
    || strcmp(command, "quit") == 0 || strcmp(command, "exit") == 0) || strcmp(command, "debug") == 0{
        handle_udp_command(command, plid, arg);
    } else if (strcmp(command, "show_trials") == 0 || strcmp(command, "scoreboard") == 0) {
        handle_tcp_command(command, plid);
    } else {
        fprintf(stderr, "Comando desconhecido: %s\n", command);
        exit(1);
    }

    return 0;
}