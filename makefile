# Variáveis de compilação
CC = g++
CFLAGS = -Wall -g  # Flags para mostrar todos os avisos e debugar

# Ficheiros fonte
SRC = game.cpp game.h player.h server.cpp player.cpp

# Objetos
GAME_OBJ = game.o
SERVER_OBJ = server.o $(GAME_OBJ)
PLAYER_OBJ = player.o $(GAME_OBJ)

# Executáveis
SERVER_BIN = GS
PLAYER_BIN = player

# Alvo padrão: compilar ambos os executáveis
all: $(SERVER_BIN) $(PLAYER_BIN)

# Regras para gerar os executáveis
$(SERVER_BIN): $(SERVER_OBJ)
	$(CC) -o $(SERVER_BIN) $(SERVER_OBJ)

$(PLAYER_BIN): $(PLAYER_OBJ)
	$(CC) -o $(PLAYER_BIN) $(PLAYER_OBJ)

# Regras para gerar os objetos

server.o: server.cpp game.h
	$(CC) $(CFLAGS) -c server.cpp

player.o: player.cpp game.h
	$(CC) $(CFLAGS) -c player.cpp

# Limpar os ficheiros gerados
clean:
	rm -f *.o $(SERVER_BIN) $(PLAYER_BIN)

# Limpar a diretoria GAMES
cleanData:
	rm -rf GAMES
	mkdir GAMES




