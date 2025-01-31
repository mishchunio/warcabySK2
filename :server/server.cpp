#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <thread>
#include <vector>
#include <map>
#include <mutex>
#include <iostream>
#include <sstream>

class Game {
private:
    enum {
        BOARD_SIZE = 8,
        EMPTY = 0,
        WHITE_PIECE = 1,
        BLACK_PIECE = 2
    };

    std::vector<std::vector<int>> board;
    int currentPlayer;
    std::string player1, player2;

    void initializeBoard();

public:
    Game(const std::string& p1, const std::string& p2);
    bool isValidMove(int fromX, int fromY, int toX, int toY, bool isWhite);
    void makeMove(int fromX, int fromY, int toX, int toY);
    int getCurrentPlayer() const;
    std::string getOpponent(const std::string& player);
    std::string getPlayer1() const;
    void printBoard();
    std::string getBoardState() const;
};

Game::Game(const std::string& p1, const std::string& p2)
    : board(BOARD_SIZE, std::vector<int>(BOARD_SIZE, EMPTY)),
      player1(p1), player2(p2), currentPlayer(1) {
    initializeBoard();
}

void Game::initializeBoard() {
    for (int row = 0; row < 3; row++) {
        for (int col = 0; col < BOARD_SIZE; col++) {
            if ((row + col) % 2 == 1) {
                board[row][col] = WHITE_PIECE;
            }
        }
    }

    for (int row = 5; row < BOARD_SIZE; row++) {
        for (int col = 0; col < BOARD_SIZE; col++) {
            if ((row + col) % 2 == 1) {
                board[row][col] = BLACK_PIECE;
            }
        }
    }
}

void Game::printBoard() {
    std::cout << "\nAktualna plansza:\n";
    for (int i = 0; i < BOARD_SIZE; i++) {
        for (int j = 0; j < BOARD_SIZE; j++) {
            if (board[i][j] == WHITE_PIECE) std::cout << "○ ";
            else if (board[i][j] == BLACK_PIECE) std::cout << "● ";
            else std::cout << ". ";
        }
        std::cout << std::endl;
    }
}

std::string Game::getBoardState() const {
    std::stringstream ss;
    for (int i = 0; i < BOARD_SIZE; i++) {
        for (int j = 0; j < BOARD_SIZE; j++) {
            ss << board[i][j] << " ";
        }
    }
    return ss.str();
}

bool Game::isValidMove(int fromX, int fromY, int toX, int toY, bool isWhite) {
    if (fromX < 0 || fromX >= BOARD_SIZE || fromY < 0 || fromY >= BOARD_SIZE ||
        toX < 0 || toX >= BOARD_SIZE || toY < 0 || toY >= BOARD_SIZE) {
        std::cout << "Błąd: Ruch poza planszą" << std::endl;
        return false;
    }

    if (board[toX][toY] != EMPTY) {
        std::cout << "Błąd: Pole docelowe nie jest puste" << std::endl;
        return false;
    }

    int piece = board[fromX][fromY];
    if (piece == EMPTY) {
        std::cout << "Błąd: Brak pionka na polu startowym" << std::endl;
        return false;
    }

    if ((isWhite && piece != BLACK_PIECE) || (!isWhite && piece != WHITE_PIECE)) {
        std::cout << "Błąd: Nieprawidłowy kolor pionka (isWhite=" << isWhite 
                  << ", piece=" << piece << ")" << std::endl;
        return false;
    }

    int dx = toX - fromX;
    int dy = toY - fromY;

    if (abs(dx) != 1 || abs(dy) != 1) {
        std::cout << "Błąd: Nieprawidłowy dystans ruchu (dx=" << dx 
                  << ", dy=" << dy << ")" << std::endl;
        return false;
    }

    if (isWhite && dx != -1) {
        std::cout << "Błąd: Nieprawidłowy kierunek ruchu dla białych" << std::endl;
        return false;
    }
    
    if (!isWhite && dx != 1) {
        std::cout << "Błąd: Nieprawidłowy kierunek ruchu dla czarnych" << std::endl;
        return false;
    }

    std::cout << "Ruch prawidłowy!" << std::endl;
    return true;
}

void Game::makeMove(int fromX, int fromY, int toX, int toY) {
    std::cout << "Wykonywanie ruchu z (" << fromX << "," << fromY << ") na (" 
              << toX << "," << toY << ")" << std::endl;
    
    board[toX][toY] = board[fromX][fromY];
    board[fromX][fromY] = EMPTY;
    currentPlayer = (currentPlayer == 1) ? 2 : 1;
    printBoard();
}

int Game::getCurrentPlayer() const {
    return currentPlayer;
}

std::string Game::getOpponent(const std::string& player) {
    return (player == player1) ? player2 : player1;
}

std::string Game::getPlayer1() const {
    return player1;
}

class GameServer {
private:
    int serverSocket;
    std::map<std::string, int> connectedPlayers;
    std::map<std::string, Game*> activeGames;
    std::mutex playersMutex, gamesMutex;

public:
    GameServer(int port);
    void start();

private:
    void setupServer(int port);
    void handleClient(int clientSocket);
    void processCommand(const std::string& cmd, int clientSocket, std::string& playerName);
    void sendMessage(const std::string& player, const std::string& message);
    void removePlayer(const std::string& playerName);
};

GameServer::GameServer(int port) {
    setupServer(port);
}

void GameServer::setupServer(int port) {
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket < 0) {
        perror("Tworzenie gniazda nie powiodło się");
        exit(1);
    }

    int opt = 1;
    setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    if (bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        perror("Powiązanie nie powiodło się");
        exit(1);
    }

    if (listen(serverSocket, 10) < 0) {
        perror("Nasłuchiwanie nie powiodło się");
        exit(1);
    }

    std::cout << "Serwer uruchomiony na porcie " << port << std::endl;
}

void GameServer::sendMessage(const std::string& player, const std::string& message) {
    std::lock_guard<std::mutex> lock(playersMutex);
    auto it = connectedPlayers.find(player);
    if (it != connectedPlayers.end()) {
        int clientSocket = it->second;
        send(clientSocket, message.c_str(), message.length(), 0);
        std::cout << "Wysłano do " << player << ": " << message << std::endl;
    }
}

void GameServer::removePlayer(const std::string& playerName) {
    if (playerName.empty()) return;
    
    {
        std::lock_guard<std::mutex> playersLock(playersMutex);
        connectedPlayers.erase(playerName);
        std::cout << "Usunięto gracza: " << playerName << std::endl;
    }
    
    {
        std::lock_guard<std::mutex> gamesLock(gamesMutex);
        auto gameIt = activeGames.find(playerName);
        if (gameIt != activeGames.end()) {
            std::string opponent = gameIt->second->getOpponent(playerName);
            
            sendMessage(opponent, "OPPONENT_DISCONNECTED");
            
            delete gameIt->second;
            activeGames.erase(playerName);
            activeGames.erase(opponent);
            std::cout << "Usunięto grę dla graczy: " << playerName << " i " << opponent << std::endl;
        }
    }
}

void GameServer::processCommand(const std::string& cmd, int clientSocket, std::string& playerName) {
    std::istringstream ss(cmd);
    std::string command;
    ss >> command;

    std::cout << "\nOtrzymano komendę: " << cmd << std::endl;

    if (command == "CONNECT") {
        ss >> playerName;
        {
            std::lock_guard<std::mutex> lock(playersMutex);
            connectedPlayers[playerName] = clientSocket;
            std::cout << "Gracz połączony: " << playerName << std::endl;

            if (connectedPlayers.size() % 2 == 0) {
                auto it = connectedPlayers.begin();
                std::string player1 = it->first;
                std::string player2 = (++it)->first;

                std::cout << "Rozpoczynanie gry: " << player1 << " vs " << player2 << std::endl;

                Game* game = new Game(player1, player2);
                
                {
                    std::lock_guard<std::mutex> gamesLock(gamesMutex);
                    activeGames[player1] = game;
                    activeGames[player2] = game;
                }

                // Dodajemy znaki końca linii do każdej wiadomości
                std::string msg;
                
                msg = "COLOR white\n";
                send(connectedPlayers[player1], msg.c_str(), msg.length(), 0);
                std::cout << "Wysłano do " << player1 << ": " << msg;

                msg = "COLOR black\n";
                send(connectedPlayers[player2], msg.c_str(), msg.length(), 0);
                std::cout << "Wysłano do " << player2 << ": " << msg;

                msg = "GAME_START\n";
                send(connectedPlayers[player1], msg.c_str(), msg.length(), 0);
                send(connectedPlayers[player2], msg.c_str(), msg.length(), 0);
                std::cout << "Wysłano GAME_START do obu graczy\n";

                msg = "YOUR_TURN\n";
                send(connectedPlayers[player1], msg.c_str(), msg.length(), 0);
                std::cout << "Wysłano YOUR_TURN do " << player1 << std::endl;

                msg = "WAIT_TURN\n";
                send(connectedPlayers[player2], msg.c_str(), msg.length(), 0);
                std::cout << "Wysłano WAIT_TURN do " << player2 << std::endl;
            }
        }
    }
    else if (command == "MOVE") {
        int fromX, fromY, toX, toY;
        ss >> fromX >> fromY >> toX >> toY;
        
        std::cout << "Próba ruchu: " << playerName << " (" << fromX << "," 
                  << fromY << ") -> (" << toX << "," << toY << ")" << std::endl;
        
        std::lock_guard<std::mutex> lock(gamesMutex);
        auto gameIt = activeGames.find(playerName);
        if (gameIt != activeGames.end()) {
            Game* game = gameIt->second;
            
            bool isWhite = (playerName == game->getPlayer1());
            std::cout << "isWhite: " << isWhite << ", currentPlayer: " 
                      << game->getCurrentPlayer() << std::endl;

            if (game->getCurrentPlayer() == (isWhite ? 1 : 2)) {
                if (game->isValidMove(fromX, fromY, toX, toY, isWhite)) {
                    std::cout << "Ruch wykonany przez " << playerName << ": " 
                              << fromX << "," << fromY << " -> " << toX << "," << toY << std::endl;
                    
                    game->makeMove(fromX, fromY, toX, toY);
                    
                    std::string moveUpdate = "MOVE_UPDATE " + std::to_string(fromX) + " " + 
                                          std::to_string(fromY) + " " + std::to_string(toX) + " " + 
                                          std::to_string(toY);
                    
                    sendMessage(playerName, moveUpdate);
                    sendMessage(game->getOpponent(playerName), moveUpdate);
                    
                    sendMessage(playerName, "WAIT_TURN");
                    sendMessage(game->getOpponent(playerName), "YOUR_TURN");
                } else {
                    std::cout << "Nieprawidłowy ruch!" << std::endl;
                    sendMessage(playerName, "INVALID_MOVE");
                }
            } else {
                std::cout << "Nie twoja kolej!" << std::endl;
                sendMessage(playerName, "NOT_YOUR_TURN");
            }
        }
    }
}

void GameServer::handleClient(int clientSocket) {
    char buffer[1024];
    std::string playerName;

    while (true) {
        memset(buffer, 0, sizeof(buffer));
        int bytesRead = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
        
        if (bytesRead <= 0) {
            std::cout << "Klient rozłączony: " << playerName << std::endl;
            break;
        }

        processCommand(std::string(buffer), clientSocket, playerName);
    }

    close(clientSocket);
    removePlayer(playerName);
}

void GameServer::start() {
    std::cout << "Serwer oczekuje na połączenia...\n";
    
    while (true) {
        sockaddr_in clientAddr;
        socklen_t clientLen = sizeof(clientAddr);
        int clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddr, &clientLen);

        if (clientSocket < 0) {
            perror("Akceptacja połączenia nie powiodła się");
            continue;
        }

        std::cout << "Nowe połączenie przyjęte\n";
        std::thread clientThread(&GameServer::handleClient, this, clientSocket);
        clientThread.detach();
    }
}

int main() {
    GameServer server(12345);
    server.start();
    return 0;
}