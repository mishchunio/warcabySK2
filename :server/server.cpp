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
#include <set>
#include <queue>
#include <chrono>

class Game {
public:
    // Udostępnione wartości, aby można było je używać poza klasą.
    enum Piece {
        BOARD_SIZE = 8,
        EMPTY = 0,
        WHITE_PIECE = 1,
        BLACK_PIECE = 2,
        WHITE_KING = 3,
        BLACK_KING = 4
    };

private:
    std::string gameId;
    int whiteCount = 12;
    int blackCount = 12;
    std::vector<std::vector<int>> board;
    int currentPlayer;
    std::string player1, player2;

    void initializeBoard();

public:
    Game(const std::string& p1, const std::string& p2);
    bool checkGameEnd();
    void setCurrentPlayer(int player);
    void printBoard();
    std::string getBoardState() const;
    std::vector<std::pair<int, int>> getAvailableCaptures(int x, int y, bool isWhite);
    std::vector<std::pair<int, int>> getAllAvailableCaptures(bool isWhite);
    bool isValidMove(int fromX, int fromY, int toX, int toY, bool isWhite);
    void makeMove(int fromX, int fromY, int toX, int toY, const std::string& playerName);
    int getCurrentPlayer() const;
    std::string getOpponent(const std::string& player);
    std::string getPlayer1() const;
    std::vector<std::pair<int, int>> getKingCaptures(int x, int y, bool isWhite, std::set<std::pair<int, int>>& capturedPieces);
    bool isKingAt(int x, int y);
    std::pair<int, int> getCapturedCoordinatesForKing(int fromX, int fromY, int toX, int toY);
    int getPieceAt(int x, int y);
    int getWhiteCount() const { return whiteCount; };
    int getBlackCount() const { return blackCount; };

};

Game::Game(const std::string& p1, const std::string& p2)
    : gameId(p1 + "_vs_" + p2),
      board(BOARD_SIZE, std::vector<int>(BOARD_SIZE, EMPTY)),
      player1(p1), player2(p2), currentPlayer(1) {
    initializeBoard();
}

void Game::initializeBoard() {
    for (int row = 0; row < 3; row++) {
        for (int col = 0; col < BOARD_SIZE; col++) {
            if ((row + col) % 2 == 1) {
                board[row][col] = BLACK_PIECE;
            }
        }
    }
    for (int row = 5; row < BOARD_SIZE; row++) {
        for (int col = 0; col < BOARD_SIZE; col++) {
            if ((row + col) % 2 == 1) {
                board[row][col] = WHITE_PIECE;
            }
        }
    }
}

bool Game::checkGameEnd() {
    if (whiteCount == 0) {
        std::cout << "Gra zakończona: Czarny wygrywa!" << std::endl;
        return true;
    }
    if (blackCount == 0) {
        std::cout << "Gra zakończona: Biały wygrywa!" << std::endl;
        return true;
    }
    return false;
}

void Game::setCurrentPlayer(int player) {
    currentPlayer = player;
}

void Game::printBoard() {
    std::cout << "\nAktualna plansza:\n";
    for (int i = 0; i < BOARD_SIZE; i++) {
        for (int j = 0; j < BOARD_SIZE; j++) {
            if (board[i][j] == WHITE_PIECE) std::cout << "○ ";
            else if (board[i][j] == BLACK_PIECE) std::cout << "● ";
            else if (board[i][j] == WHITE_KING) std::cout << "♚ ";
            else if (board[i][j] == BLACK_KING) std::cout << "♔ ";
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

std::vector<std::pair<int, int>> Game::getAvailableCaptures(int x, int y, bool isWhite) {
    std::vector<std::pair<int, int>> captures;
    int piece = board[x][y];
    bool isKing = (piece == WHITE_KING || piece == BLACK_KING);
    if (isKing) {
        std::vector<std::pair<int, int>> directions = {{-1, -1}, {-1, 1}, {1, -1}, {1, 1}};
        for (const auto& dir : directions) {
            int currentX = x + dir.first;
            int currentY = y + dir.second;
            while (currentX >= 0 && currentX < BOARD_SIZE && currentY >= 0 && currentY < BOARD_SIZE) {
                if (board[currentX][currentY] != EMPTY) {
                    bool isEnemy = false;
                    if (isWhite && (board[currentX][currentY] == BLACK_PIECE || board[currentX][currentY] == BLACK_KING))
                        isEnemy = true;
                    if (!isWhite && (board[currentX][currentY] == WHITE_PIECE || board[currentX][currentY] == WHITE_KING))
                        isEnemy = true;
                    if (isEnemy) {
                        int nextX = currentX + dir.first;
                        int nextY = currentY + dir.second;
                        while (nextX >= 0 && nextX < BOARD_SIZE && nextY >= 0 && nextY < BOARD_SIZE) {
                            if (board[nextX][nextY] == EMPTY) {
                                captures.push_back({nextX, nextY});
                            } else {
                                break;
                            }
                            nextX += dir.first;
                            nextY += dir.second;
                        }
                    }
                    break;
                }
                currentX += dir.first;
                currentY += dir.second;
            }
        }
    } else {
        std::vector<std::pair<int, int>> directions = {{-2, -2}, {-2, 2}, {2, -2}, {2, 2}};
        for (const auto& dir : directions) {
            int newX = x + dir.first;
            int newY = y + dir.second;
            int midX = x + dir.first/2;
            int midY = y + dir.second/2;
            if (newX >= 0 && newX < BOARD_SIZE && newY >= 0 && newY < BOARD_SIZE) {
                if (board[newX][newY] == EMPTY) {
                    int midPiece = board[midX][midY];
                    if (isWhite && (midPiece == BLACK_PIECE || midPiece == BLACK_KING))
                        captures.push_back({newX, newY});
                    if (!isWhite && (midPiece == WHITE_PIECE || midPiece == WHITE_KING))
                        captures.push_back({newX, newY});
                }
            }
        }
    }
    return captures;
}

std::vector<std::pair<int, int>> Game::getAllAvailableCaptures(bool isWhite) {
    std::vector<std::pair<int, int>> allCaptures;
    for (int x = 0; x < BOARD_SIZE; x++) {
        for (int y = 0; y < BOARD_SIZE; y++) {
            int piece = board[x][y];
            if ((isWhite && (piece == WHITE_PIECE || piece == WHITE_KING)) ||
                (!isWhite && (piece == BLACK_PIECE || piece == BLACK_KING))) {
                auto captures = getAvailableCaptures(x, y, isWhite);
                allCaptures.insert(allCaptures.end(), captures.begin(), captures.end());
            }
        }
    }
    return allCaptures;
}

bool Game::isValidMove(int fromX, int fromY, int toX, int toY, bool isWhite) {
    std::cout << "\nSprawdzanie ruchu: (" << fromX << "," << fromY << ") -> (" 
              << toX << "," << toY << ") dla " << (isWhite ? "białego" : "czarnego") << std::endl;
    if (fromX < 0 || fromX >= BOARD_SIZE || fromY < 0 || fromY >= BOARD_SIZE ||
        toX < 0 || toX >= BOARD_SIZE || toY < 0 || toY >= BOARD_SIZE) {
        std::cout << "Błąd: Ruch poza planszą" << std::endl;
        return false;
    }
    if (board[toX][toY] != EMPTY) {
        std::cout << "Błąd: Pole docelowe nie jest puste" << std::endl;
        return false;
    }
    std::cout << "Wartości enum:" << std::endl;
    std::cout << "WHITE_PIECE = " << WHITE_PIECE << std::endl;
    std::cout << "BLACK_PIECE = " << BLACK_PIECE << std::endl;
    std::cout << "WHITE_KING = " << WHITE_KING << std::endl;
    std::cout << "BLACK_KING = " << BLACK_KING << std::endl;
    int piece = board[fromX][fromY];
    std::cout << "Wartość pionka na polu (" << fromX << "," << fromY << ") = " << piece << std::endl;
    if (piece == EMPTY) {
        std::cout << "Błąd: Brak pionka na polu startowym" << std::endl;
        return false;
    }
    bool isKing = (piece == WHITE_KING || piece == BLACK_KING);
    std::cout << "Typ pionka: " << (isKing ? "damka" : "zwykły")
              << " (wartość=" << piece << ")" << std::endl;
    if ((isWhite && (piece != WHITE_PIECE && piece != WHITE_KING)) ||
        (!isWhite && (piece != BLACK_PIECE && piece != BLACK_KING))) {
        std::cout << "Błąd: Nieprawidłowy kolor pionka (piece=" << piece 
                  << ", isWhite=" << isWhite << ")" << std::endl;
        return false;
    }
    int dx = toX - fromX;
    int dy = toY - fromY;
    auto allCaptures = getAllAvailableCaptures(isWhite);
    if (!allCaptures.empty()) {
        std::cout << "Dostępne bicia:" << std::endl;
        for (const auto& capture : allCaptures) {
            std::cout << "Możliwe bicie na pozycję (" 
                      << capture.first << "," << capture.second << ")" << std::endl;
        }
        std::cout << "Przed sprawdzeniem czy to damka czy nie - isKing: " << isKing << std::endl;
        if (isKing) {
            std::cout << "Sprawdzamy bicie damki" << std::endl;
            std::cout << "abs(dx) i abs(dy): " << abs(dx) << "   " << abs(dy) << std::endl;
            if (abs(dx) != abs(dy)) {
                std::cout << "Błąd: Ruch damki musi być po przekątnej" << std::endl;
                return false;
            }
            int stepX = (dx > 0) ? 1 : -1;
            int stepY = (dy > 0) ? 1 : -1;
            std::cout << "stepX, stepY: " << stepX << "   " << stepY << std::endl;
            int currentX = fromX + stepX;
            int currentY = fromY + stepY;
            int enemyCount = 0;
            while (currentX != toX && currentY != toY) {
                std::cout << "enemyCount: " << enemyCount << std::endl;
                if (board[currentX][currentY] != EMPTY) {
                    bool isEnemy = false;
                    if (isWhite && (board[currentX][currentY] == BLACK_PIECE || board[currentX][currentY] == BLACK_KING))
                        isEnemy = true;
                    if (!isWhite && (board[currentX][currentY] == WHITE_PIECE || board[currentX][currentY] == WHITE_KING))
                        isEnemy = true;
                    if (!isEnemy) {
                        std::cout << "Błąd: Na drodze damki znajduje się pionek własny" << std::endl;
                        return false;
                    }
                    enemyCount++;
                    if (enemyCount > 1) {
                        std::cout << "Błąd: Na drodze damki jest więcej niż jeden pionek przeciwnika" << std::endl;
                        return false;
                    }
                }
                currentX += stepX;
                currentY += stepY;
            }
            if (enemyCount == 1) {
                std::cout << "check if (enemyCount == 1): " << enemyCount << std::endl;
                auto currentCaptures = getAvailableCaptures(fromX, fromY, isWhite);
                for (const auto& cap : currentCaptures) {
                    if (cap.first == toX && cap.second == toY) {
                        std::cout << "Prawidłowe bicie damką" << std::endl;
                        return true;
                    }
                }
                std::cout << "Błąd: Ruch damki nie znajduje się na liście dostępnych bić" << std::endl;
                return false;
            } else {
                std::cout << "Błąd: Ruch damki musi przebiegać przez dokładnie jeden pionek przeciwnika" << std::endl;
                return false;
            }
        } else {
            std::cout << "else - nie sprawdzamy bicia damki tylko bicie piona" << std::endl;
            if (abs(dx) == 2 && abs(dy) == 2) {
                int midX = (fromX + toX) / 2;
                int midY = (fromY + toY) / 2;
                int midPiece = board[midX][midY];
                std::cout << "Próba bicia - sprawdzam pionek na (" << midX << "," << midY 
                          << "), wartość=" << midPiece << std::endl;
                if (isWhite && (midPiece == BLACK_PIECE || midPiece == BLACK_KING)) {
                    auto currentCaptures = getAvailableCaptures(fromX, fromY, isWhite);
                    for (const auto& cap : currentCaptures) {
                        if (cap.first == toX && cap.second == toY) {
                            std::cout << "Prawidłowe bicie dla białego" << std::endl;
                            return true;
                        }
                    }
                }
                if (!isWhite && (midPiece == WHITE_PIECE || midPiece == WHITE_KING)) {
                    auto currentCaptures = getAvailableCaptures(fromX, fromY, isWhite);
                    for (const auto& cap : currentCaptures) {
                        if (cap.first == toX && cap.second == toY) {
                            std::cout << "Prawidłowe bicie dla czarnego" << std::endl;
                            return true;
                        }
                    }
                }
            }
            std::cout << "Błąd: Musisz wykonać dostępne bicie" << std::endl;
            return false;
        }
    }
    if (isKing) {
        std::cout << "Sprawdzanie ruchu damki" << std::endl;
        if (abs(dx) != abs(dy)) {
            std::cout << "Błąd: Ruch damki musi być po przekątnej" << std::endl;
            return false;
        }
        int stepX = (dx > 0) ? 1 : -1;
        int stepY = (dy > 0) ? 1 : -1;
        int currentX = fromX + stepX;
        int currentY = fromY + stepY;
        while (currentX != toX && currentY != toY) {
            if (board[currentX][currentY] != EMPTY) {
                std::cout << "Błąd: Droga ruchu damki jest zablokowana" << std::endl;
                return false;
            }
            currentX += stepX;
            currentY += stepY;
        }
        std::cout << "Prawidłowy ruch damki bez bicia" << std::endl;
        return true;
    } else {
        std::cout << "Sprawdzanie ruchu zwykłego pionka" << std::endl;
        if (abs(dx) != 1 || abs(dy) != 1) {
            std::cout << "Błąd: Ruch musi być o jedno pole po przekątnej" << std::endl;
            return false;
        }
        if (isWhite && dx != -1) {
            std::cout << "Błąd: Nieprawidłowy kierunek ruchu dla białego pionka" << std::endl;
            return false;
        }
        if (!isWhite && dx != 1) {
            std::cout << "Błąd: Nieprawidłowy kierunek ruchu dla czarnego pionka" << std::endl;
            return false;
        }
        std::cout << "Prawidłowy ruch zwykłego pionka" << std::endl;
        return true;
    }
}

void Game::makeMove(int fromX, int fromY, int toX, int toY, const std::string& playerName) {
    int movedPiece = board[fromX][fromY];
    std::cout << "Poruszany pionek (movedPiece) = " << movedPiece << std::endl;
    bool isKing = (movedPiece == WHITE_KING || movedPiece == BLACK_KING);
    bool isWhite = (playerName == player1);
    board[toX][toY] = movedPiece;
    board[fromX][fromY] = EMPTY;
    if (abs(toX - fromX) > 1) {
        if (isKing) {
            int stepX = (toX - fromX) > 0 ? 1 : -1;
            int stepY = (toY - fromY) > 0 ? 1 : -1;
            int currentX = fromX + stepX;
            int currentY = fromY + stepY;
            bool foundEnemy = false;
            while (currentX != toX && currentY != toY) {
                if (board[currentX][currentY] != EMPTY) {
                    std::cout << "Zbicie pionka na pozycji (" << currentX << "," << currentY << ")" << std::endl;
                    board[currentX][currentY] = EMPTY;
                    if (isWhite) blackCount--; else whiteCount--;
                    foundEnemy = true;
                    break;
                }
                currentX += stepX;
                currentY += stepY;
            }
        } else {
            int midX = (fromX + toX) / 2;
            int midY = (fromY + toY) / 2;
            board[midX][midY] = EMPTY;
            if (isWhite) blackCount--; else whiteCount--;
        }
    }
    std::cout << "Sprawdzanie promocji:" << std::endl;
    std::cout << "isKing = " << isKing << std::endl;
    std::cout << "isWhite = " << isWhite << std::endl;
    std::cout << "toX = " << toX << std::endl;
    std::cout << "BOARD_SIZE - 1 = " << (BOARD_SIZE - 1) << std::endl;
    if (!isKing) {
        if ((isWhite && toX == 0) || (!isWhite && toX == BOARD_SIZE - 1)) {
            board[toX][toY] = isWhite ? WHITE_KING : BLACK_KING;
            std::cout << "Promocja na damkę! Kolor: " << (isWhite ? "biały" : "czarny") << std::endl;
        }
    }
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

std::vector<std::pair<int, int>> Game::getKingCaptures(int x, int y, bool isWhite, std::set<std::pair<int, int>>& capturedPieces) {
    std::vector<std::pair<int, int>> possibleMoves;
    std::vector<std::pair<int, int>> directions = {{-1, -1}, {-1, 1}, {1, -1}, {1, 1}};
    for (const auto& dir : directions) {
        int currentX = x + dir.first;
        int currentY = y + dir.second;
        while (currentX >= 0 && currentX < BOARD_SIZE && currentY >= 0 && currentY < BOARD_SIZE) {
            int piece = board[currentX][currentY];
            if ((isWhite && (piece == WHITE_PIECE || piece == WHITE_KING)) ||
                (!isWhite && (piece == BLACK_PIECE || piece == BLACK_KING))) {
                break;
            }
            if ((!isWhite && (piece == WHITE_PIECE || piece == WHITE_KING)) ||
                (isWhite && (piece == BLACK_PIECE || piece == BLACK_KING))) {
                if (capturedPieces.find({currentX, currentY}) == capturedPieces.end()) {
                    int nextX = currentX + dir.first;
                    int nextY = currentY + dir.second;
                    while (nextX >= 0 && nextX < BOARD_SIZE && nextY >= 0 && nextY < BOARD_SIZE) {
                        if (board[nextX][nextY] == EMPTY) {
                            std::set<std::pair<int, int>> newCaptured = capturedPieces;
                            newCaptured.insert({currentX, currentY});
                            std::vector<std::pair<int, int>> furtherCaptures = getKingCaptures(nextX, nextY, isWhite, newCaptured);
                            if (furtherCaptures.empty()) {
                                possibleMoves.push_back({nextX, nextY});
                            }
                            possibleMoves.insert(possibleMoves.end(), furtherCaptures.begin(), furtherCaptures.end());
                        }
                        nextX += dir.first;
                        nextY += dir.second;
                    }
                }
                break;
            }
            currentX += dir.first;
            currentY += dir.second;
        }
    }
    return possibleMoves;
}

bool Game::isKingAt(int x, int y) {
    int piece = board[x][y];
    return (piece == WHITE_KING || piece == BLACK_KING);
}

std::pair<int, int> Game::getCapturedCoordinatesForKing(int fromX, int fromY, int toX, int toY) {
    int stepX = (toX - fromX) > 0 ? 1 : -1;
    int stepY = (toY - fromY) > 0 ? 1 : -1;
    int currentX = fromX + stepX;
    int currentY = fromY + stepY;
    while (currentX != toX && currentY != toY) {
         if (board[currentX][currentY] != EMPTY) {
              return {currentX, currentY};
         }
         currentX += stepX;
         currentY += stepY;
    }
    return { (fromX + toX) / 2, (fromY + toY) / 2 };
}

int Game::getPieceAt(int x, int y) {
    return board[x][y];
}

class GameServer {
private:
    int serverSocket;
    std::map<std::string, int> connectedPlayers;
    std::map<std::string, Game*> activeGames;
    std::map<std::string, std::string> playerToGameId;
    std::map<std::string, std::set<std::string>> gamePlayerMap;
    std::map<std::string, bool> playerMultiCaptureMode;
    std::mutex playersMutex, gamesMutex;
    std::queue<std::string> waitingPlayers;
    std::string generateGameId(const std::string& player1, const std::string& player2) {
        return player1 + "_vs_" + player2 + "_" + std::to_string(std::chrono::system_clock::now().time_since_epoch().count());
    }
public:
    GameServer(int port);
    void start();
private:
    void setupServer(int port);
    void handleClient(int clientSocket);
    void processCommand(const std::string& cmd, int clientSocket, std::string& playerName);
    void sendMessage(const std::string& player, const std::string& message);
    void removePlayer(const std::string& playerName);
    void createGame(const std::string& player1, const std::string& player2);
    void removeGame(const std::string& gameId);
};

GameServer::GameServer(int port) {
    setupServer(port);
}

void GameServer::createGame(const std::string& player1, const std::string& player2) {
    std::string gameId = generateGameId(player1, player2);
    Game* game = new Game(player1, player2);
    {
        std::lock_guard<std::mutex> lock(gamesMutex);
        activeGames[gameId] = game;
        gamePlayerMap[gameId] = {player1, player2};
    }
    std::string msg;
    msg = "COLOR white\n";
    send(connectedPlayers[player1], msg.c_str(), msg.length(), 0);
    msg = "COLOR black\n";
    send(connectedPlayers[player2], msg.c_str(), msg.length(), 0);
    msg = "GAME_START\n";
    send(connectedPlayers[player1], msg.c_str(), msg.length(), 0);
    send(connectedPlayers[player2], msg.c_str(), msg.length(), 0);
    msg = "YOUR_TURN\n";
    send(connectedPlayers[player1], msg.c_str(), msg.length(), 0);
    msg = "WAIT_TURN\n";
    send(connectedPlayers[player2], msg.c_str(), msg.length(), 0);
}

void GameServer::removeGame(const std::string& gameId) {
    std::lock_guard<std::mutex> lock(gamesMutex);
    auto gameIt = activeGames.find(gameId);
    if (gameIt != activeGames.end()) {
        delete gameIt->second;
        activeGames.erase(gameId);
        gamePlayerMap.erase(gameId);
    }
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
        std::string msg = message + "\n";
        send(clientSocket, msg.c_str(), msg.length(), 0);
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
        // Pobieramy gameId dla rozłączającego się gracza
        auto it = playerToGameId.find(playerName);
        if (it != playerToGameId.end()) {
            std::string gameId = it->second;
            // Pobieramy listę graczy tej gry
            if (gamePlayerMap.find(gameId) != gamePlayerMap.end()) {
                auto players = gamePlayerMap[gameId];
                std::string opponent;
                for (const auto& p : players) {
                    if (p != playerName) {
                        opponent = p;
                        break;
                    }
                }
                if (!opponent.empty()) {
                    sendMessage(opponent, "OPPONENT_DISCONNECTED");
                }
                // Usuwamy mapowanie gry dla obu graczy
                for (const auto& p : players) {
                    playerToGameId.erase(p);
                }
                gamePlayerMap.erase(gameId);
            }
            if (activeGames.find(gameId) != activeGames.end()) {
                delete activeGames[gameId];
                activeGames.erase(gameId);
            }
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
            waitingPlayers.push(playerName);
            if (waitingPlayers.size() >= 2) {
                std::string player1 = waitingPlayers.front(); waitingPlayers.pop();
                std::string player2 = waitingPlayers.front(); waitingPlayers.pop();
                std::string gameId = player1 + "_vs_" + player2;
                std::cout << "Rozpoczynanie gry: " << player1 << " vs " << player2 << std::endl;
                Game* game = new Game(player1, player2);
                {
                    std::lock_guard<std::mutex> gamesLock(gamesMutex);
                    activeGames[gameId] = game;
                    gamePlayerMap[gameId] = {player1, player2};
                    playerToGameId[player1] = gameId;
                    playerToGameId[player2] = gameId;
                }
                std::string msg;
                msg = "COLOR white\n";
                send(connectedPlayers[player1], msg.c_str(), msg.length(), 0);
                std::cout << "Wysłano do " << player1 << ": COLOR white" << std::endl;
                msg = "COLOR black\n";
                send(connectedPlayers[player2], msg.c_str(), msg.length(), 0);
                std::cout << "Wysłano do " << player2 << ": COLOR black" << std::endl;
                msg = "GAME_START\n";
                send(connectedPlayers[player1], msg.c_str(), msg.length(), 0);
                send(connectedPlayers[player2], msg.c_str(), msg.length(), 0);
                std::cout << "Wysłano GAME_START do obu graczy" << std::endl;
                msg = "YOUR_TURN\n";
                send(connectedPlayers[player1], msg.c_str(), msg.length(), 0);
                std::cout << "Wysłano YOUR_TURN do " << player1 << std::endl;
                msg = "WAIT_TURN\n";
                send(connectedPlayers[player2], msg.c_str(), msg.length(), 0);
                std::cout << "Wysłano WAIT_TURN do " << player2 << std::endl;
                std::cout << "Wszystkie wiadomości inicjalizacyjne zostały wysłane" << std::endl;
            }
        }
   }
   else if (command == "MOVE") {
        int fromX, fromY, toX, toY;
        ss >> fromX >> fromY >> toX >> toY;
        std::cout << "Próba ruchu: " << playerName << " (" << fromX << "," << fromY << ") -> (" << toX << "," << toY << ")" << std::endl;
        std::lock_guard<std::mutex> lock(gamesMutex);
        auto gameIdIt = playerToGameId.find(playerName);
        if (gameIdIt == playerToGameId.end()) {
            std::cout << "Błąd: Gracz " << playerName << " nie ma przypisanej gry!" << std::endl;
            sendMessage(playerName, "NO_GAME_FOUND");
            return;
        }
        std::string gameId = gameIdIt->second;
        auto gameIt = activeGames.find(gameId);
        if (gameIt != activeGames.end()) {
            Game* game = gameIt->second;
            bool isWhite = (playerName == game->getPlayer1());
            std::cout << "isWhite: " << isWhite << ", currentPlayer: " << game->getCurrentPlayer() << std::endl;
            auto allCaptures = game->getAllAvailableCaptures(isWhite);
            if (game->getCurrentPlayer() == (isWhite ? 1 : 2)) {
                if (!allCaptures.empty()) {
                    auto currentCaptures = game->getAvailableCaptures(fromX, fromY, isWhite);
                    if (currentCaptures.empty()) {
                        std::cout << "Musisz wykonać bicie innym pionkiem!" << std::endl;
                        sendMessage(playerName, "INVALID_MOVE");
                        return;
                    }
                }
                if (game->isValidMove(fromX, fromY, toX, toY, isWhite)) {
                    std::cout << "Ruch wykonany przez " << playerName << ": " 
                            << fromX << "," << fromY << " -> " << toX << "," << toY << std::endl;
                    
                    bool isCapture = abs(toX - fromX) > 1;
                    std::string moveUpdate;
                    int capturedX, capturedY;
                    bool promotion = false;  // nowa zmienna do sprawdzenia promocji
                    
                    if (isCapture) {
                        // Pobieramy typ pionka przed wykonaniem ruchu
                        int piece = game->getPieceAt(fromX, fromY);
                        if (piece == Game::WHITE_KING || piece == Game::BLACK_KING) {
                            std::pair<int, int> captured = game->getCapturedCoordinatesForKing(fromX, fromY, toX, toY);
                            capturedX = captured.first;
                            capturedY = captured.second;
                        } else {
                            // Dla zwykłego pionka używamy standardowego wzoru
                            capturedX = (fromX + toX) / 2;
                            capturedY = (fromY + toY) / 2;
                            // Sprawdzamy, czy nastąpi promocja (dla białych linia promocji to 0, dla czarnych to BOARD_SIZE - 1)
                            if ((isWhite && toX == 0) || (!isWhite && toX == Game::BOARD_SIZE - 1)) {
                                promotion = true;
                            }
                        }
                        
                        game->makeMove(fromX, fromY, toX, toY, playerName);
                        
                        moveUpdate = "MOVE_UPDATE " + std::to_string(fromX) + " " + 
                                    std::to_string(fromY) + " " + std::to_string(toX) + " " + 
                                    std::to_string(toY) + " CAPTURE " + std::to_string(capturedX) + " " + 
                                    std::to_string(capturedY);
                        if (game->isKingAt(toX, toY)) {
                            moveUpdate += " KING";
                        }
                        
                        sendMessage(playerName, moveUpdate);
                        sendMessage(game->getOpponent(playerName), moveUpdate);
                        
                        // Sprawdzenie, czy są kolejne możliwe bicia z pola docelowego
                        auto nextCaptures = game->getAvailableCaptures(toX, toY, isWhite);
                        // Jeśli nastąpiła promocja lub nie ma kolejnych bić – kończymy turę
                        if (promotion || nextCaptures.empty()) {
                            game->setCurrentPlayer(isWhite ? 2 : 1);
                            sendMessage(playerName, "WAIT_TURN");
                            sendMessage(game->getOpponent(playerName), "YOUR_TURN");
                        } else {
                            sendMessage(playerName, "YOUR_TURN");
                            sendMessage(game->getOpponent(playerName), "WAIT_TURN");
                        }
                    } else {
                        // Ruch bez bicia
                        game->makeMove(fromX, fromY, toX, toY, playerName);
                        
                        moveUpdate = "MOVE_UPDATE " + std::to_string(fromX) + " " + 
                                    std::to_string(fromY) + " " + std::to_string(toX) + " " + 
                                    std::to_string(toY);
                        if (game->isKingAt(toX, toY)) {
                            moveUpdate += " KING";
                        }
                        
                        sendMessage(playerName, moveUpdate);
                        sendMessage(game->getOpponent(playerName), moveUpdate);
                        
                        game->setCurrentPlayer(isWhite ? 2 : 1);
                        sendMessage(playerName, "WAIT_TURN");
                        sendMessage(game->getOpponent(playerName), "YOUR_TURN");
                    }
                } else {
                    std::cout << "Nieprawidłowy ruch!" << std::endl;
                    sendMessage(playerName, "INVALID_MOVE");
                }

                if (game->checkGameEnd()) {
                    std::string winner;
                    if (game->getWhiteCount() == 0)
                        winner = "black";
                    else if (game->getBlackCount() == 0)
                        winner = "white";
                    sendMessage(playerName, "GAME_OVER " + winner);
                    sendMessage(game->getOpponent(playerName), "GAME_OVER " + winner);
                    return;
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
