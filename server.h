#ifndef SERVER_H
#define SERVER_H

#include <string>
#include <vector>
#include <netinet/in.h>

/**
 * @brief Checks if a player has an active game.
 * 
 * @param player_id The ID of the player.
 * @return true if the player has an active game, false otherwise.
 */
bool playerHasActiveGame(int player_id);

/**
 * @brief Processes the start game command.
 * 
 * @param player_id The ID of the player.
 * @param max_playtime The maximum playtime for the game.
 * @return A response string indicating the result of the command.
 */
std::string processStart(int player_id, int max_playtime);

/**
 * @brief Processes a player's attempt to guess the key.
 * 
 * @param player_id The ID of the player.
 * @param guess The player's guess.
 * @param num_tries The number of tries the player has made.
 * @return A response string indicating the result of the attempt.
 */
std::string processTry(int player_id, const std::vector<std::string>& guess, int num_tries);

/**
 * @brief Processes the quit game command.
 * 
 * @param player_id The ID of the player.
 * @return A response string indicating the result of the command.
 */
std::string processQuit(int player_id);

/**
 * @brief Processes the debug game command.
 * 
 * @param player_id The ID of the player.
 * @param max_playtime The maximum playtime for the game.
 * @param key The secret key for the game.
 * @return A response string indicating the result of the command.
 */
std::string processDebug(int player_id, int max_playtime, const std::vector<std::string>& key);

/**
 * @brief Processes the show trials command.
 * 
 * @param player_id The ID of the player.
 * @return A response string indicating the result of the command.
 */
std::string processShowTrials(int player_id);

/**
 * @brief Processes the scoreboard command.
 * 
 * @return A response string indicating the result of the command.
 */
std::string processScoreboard();

/**
 * @brief Sends a UDP response to the client.
 * 
 * @param response The response string to send.
 * @param udp_socket The UDP socket.
 * @param client_addr The client's address.
 * @param addrlen The length of the client's address.
 */
void sendUDPResponse(const std::string& response, int udp_socket, struct sockaddr_in* client_addr, socklen_t addrlen);

/**
 * @brief Sends a TCP response to the client.
 * 
 * @param response The response string to send.
 * @param client_socket The TCP socket.
 */
void sendTCPResponse(const std::string& response, int client_socket);

/**
 * @brief Creates a UDP socket.
 * 
 * @param res The address info structure.
 * @param portNumber The port number to bind the socket to.
 * @return The created UDP socket, or -1 on error.
 */
int create_udp_socket(struct addrinfo **res, int portNumber);

/**
 * @brief Creates a TCP socket.
 * 
 * @param res The address info structure.
 * @param portNumber The port number to bind the socket to.
 * @return The created TCP socket, or -1 on error.
 */
int create_tcp_socket(struct addrinfo **res, int portNumber);

/**
 * @brief Handles incoming commands and returns the appropriate response.
 * 
 * @param command The command string.
 * @return The response string.
 */
std::string cmdHandler(const std::string& command);

/**
 * @brief Processes requests from the request queue.
 */
void processRequests();

/**
 * @brief Handles incoming UDP requests.
 * 
 * @param udp_socket The UDP socket.
 */
void handleUDPRequest(int udp_socket);

/**
 * @brief Handles incoming TCP connections.
 * 
 * @param client_socket The client socket.
 * @param client_addr The client's address.
 * @param addrlen The length of the client's address.
 */
void handleTCPConnection(int client_socket, struct sockaddr_in client_addr, socklen_t addrlen);

/**
 * @brief Handles incoming TCP requests.
 * 
 * @param tcp_socket The TCP socket.
 */
void handleTCPRequest(int tcp_socket);

#endif // SERVER_H
