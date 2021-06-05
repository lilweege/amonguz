#include "game.h"


void Game::initBoard() {
    board[0][0] = BlackRook;		board[0][7] = WhiteRook;
    board[1][0] = BlackKnight;		board[1][7] = WhiteKnight;
    board[2][0] = BlackBishop;		board[2][7] = WhiteBishop;
    board[3][0] = BlackQueen;		board[3][7] = WhiteQueen;
    board[4][0] = BlackKing;		board[4][7] = WhiteKing;
    board[5][0] = BlackBishop;		board[5][7] = WhiteBishop;
    board[6][0] = BlackKnight;		board[6][7] = WhiteKnight;
    board[7][0] = BlackRook;		board[7][7] = WhiteRook;
    for (int i = 0; i < 8; ++i) {
        board[i][1] = BlackPawn;
        board[i][6] = WhitePawn;
        for (int j = 2; j < 6; ++j)
            board[i][j] = Empty;
    }
}


// bool Game::tryMove(Move move) {
//     bool isValid = isValidMove(move);
//     if (isValid)
//         performMove(move);
//     return isValid;
// }

// void Game::performMove(Move move) {
//     std::swap(board[move.fr.x][move.fr.y], board[move.to.x][move.to.y]);
//     board[move.fr.x][move.fr.y] = Empty;

//     // updating other stuff

//     playerTurn = (playerTurn == White) ? Black : White;
// }


bool Game::isValidMove(Move move) {
    int i = move.fr.x, j = move.fr.y;
    Cell piece = getCell(i, j);
    if (move.fr.x == move.to.x &&
        move.fr.y == move.to.y)
        return false;


    // return false;
    return true;
}

unsigned long long Game::getLegalMoves(int i, int j) {
    unsigned long long board = 0LL;
    // FIXME: this is trash
    for (int x = 0; x < 8; ++x)
        for (int y = 0; y < 8; ++y)
            if (isValidMove({{i, j}, {x, y}}))
                board |= (1 << (x * 8 + y));
    return board;
}