#ifndef PLAYER_H
#define PLAYER_H

#include <string>
#include <vector>

/**
 * @brief Global variables
 */
extern std::string gsip; ///< Game server IP address
extern int gsport; ///< Game server port
extern int udp_socket; ///< UDP socket descriptor
extern int tcp_socket; ///< TCP socket descriptor

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
 * @return True if the message was sent successfully, false otherwise.
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
 * @return True if the message was sent successfully, false otherwise.
 */
bool sendTCPMessage(const std::string& message);

/**
 * @brief Handles the "show trials" command.
 * 
 * @param plid The player ID.
 */
void handleShowTrials(int plid);

/**
 * @brief Handles the "scoreboard" command.
 */
void handleScoreboard();

/**
 * @brief Handles the "start" command.
 * 
 * @param plid The player ID.
 * @param max_playtime The maximum playtime.
 */
void handleStart(int plid, int max_playtime);

/**
 * @brief Handles the "try" command.
 * 
 * @param plid The player ID.
 * @param guess The player's guess.
 */
void handleTry(int plid, const std::vector<std::string>& guess);

/**
 * @brief Handles the "quit" command.
 * 
 * @param plid The player ID.
 */
void handleQuit(int plid);

/**
 * @brief Handles the "debug" command.
 * 
 * @param plid The player ID.
 * @param max_playtime The maximum playtime.
 * @param key The secret key.
 */
void handleDebug(int plid, int max_playtime, const std::vector<std::string>& key);

/**
 * @brief Parses and handles player commands.
 */
void cmdParser();

/**
 * @brief Creates a UDP socket.
 * 
 * @param res The address info result.
 * @return The UDP socket descriptor.
 */
int create_udp_socket(struct addrinfo **res);

/**
 * @brief Creates a TCP socket.
 * 
 * @param res The address info result.
 * @return The TCP socket descriptor.
 */
int create_tcp_socket(struct addrinfo **res);

#endif // PLAYER_H
