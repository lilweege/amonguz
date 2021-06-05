#pragma once

#include <array>

// should this be a struct?
// member functions could be useful
enum Cell : unsigned char {
	Empty 			= 0b0000,

	WhiteKing		= 0b0001,
	WhiteQueen		= 0b0010,
	WhiteBishop		= 0b0011,
	WhiteKnight		= 0b0100,
	WhiteRook		= 0b0101,
	WhitePawn		= 0b0110,

	BlackKing		= 0b1001,
	BlackQueen		= 0b1010,
	BlackBishop		= 0b1011,
	BlackKnight		= 0b1100,
	BlackRook		= 0b1101,
	BlackPawn		= 0b1110,
};


class Game {
private:
	std::array<std::array<Cell, 8>, 8> board;
private:

	void initBoard() {
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
public:
	Game() {
		initBoard();
	}

	Cell getCell(int i, int j) { return board[i][j]; }
};