#ifndef PLAYER_H
#define PLAYER_H

#include <string>
#include <vector>

/**
 * @brief Receives a message via UDP.
 * 
 * @return The received message as a string.
 */
std::string receiveUDPMessage();

/**
 * @brief Sends a message via UDP.
 * 
 * @param message The message to send.
 * @return true if the message was sent successfully, false otherwise.
 */
bool sendUDPMessage(const std::string& message);

/**
 * @brief Receives a message via TCP.
 * 
 * @return The received message as a string.
 */
std::string receiveTCPMessage();

/**
 * @brief Sends a message via TCP.
 * 
 * @param message The message to send.
 * @return true if the message was sent successfully, false otherwise.
 */
bool sendTCPMessage(const std::string& message);

/**
 * @brief Reconnects the TCP socket.
 * 
 * @return true if the reconnection was successful, false otherwise.
 */
bool reconnectTCP();

/**
 * @brief Handles the show trials command.
 * 
 * @param plid The player ID.
 */
void handleShowTrials(int plid);

/**
 * @brief Handles the scoreboard command.
 */
void handleScoreboard();

/**
 * @brief Handles the start game command.
 * 
 * @param plid The player ID.
 * @param max_playtime The maximum playtime for the game.
 */
void handleStart(int plid, int max_playtime);

/**
 * @brief Handles the try command.
 * 
 * @param plid The player ID.
 * @param guess The player's guess.
 */
void handleTry(int plid, const std::vector<std::string>& guess);

/**
 * @brief Handles the quit game command.
 * 
 * @param plid The player ID.
 */
void handleQuit(int plid);

/**
 * @brief Handles the debug game command.
 * 
 * @param plid The player ID.
 * @param max_playtime The maximum playtime for the game.
 * @param key The secret key for the game.
 */
void handleDebug(int plid, int max_playtime, const std::vector<std::string>& key);

/**
 * @brief Validates the start command.
 * 
 * @param plid The player ID.
 * @param max_playtime The maximum playtime for the game.
 * @return true if the command is valid, false otherwise.
 */
bool validateStartCommand(int plid, int max_playtime);

/**
 * @brief Validates the try command.
 * 
 * @param guess The player's guess.
 * @return true if the command is valid, false otherwise.
 */
bool validateTryCommand(const std::vector<std::string>& guess);

/**
 * @brief Validates the debug command.
 * 
 * @param plid The player ID.
 * @param max_playtime The maximum playtime for the game.
 * @param key The secret key for the game.
 * @return true if the command is valid, false otherwise.
 */
bool validateDebugCommand(int plid, int max_playtime, const std::vector<std::string>& key);

/**
 * @brief Parses and handles player commands.
 */
void cmdParser();

/**
 * @brief Creates a UDP socket.
 * 
 * @param res The address info structure.
 * @return The created UDP socket, or -1 on error.
 */
int create_udp_socket(struct addrinfo **res);

/**
 * @brief Creates a TCP socket.
 * 
 * @param res The address info structure.
 * @return The created TCP socket, or -1 on error.
 */
int create_tcp_socket(struct addrinfo **res);

#endif // PLAYER_H
