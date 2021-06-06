#pragma once

#include "olcPixelGameEngine.h"
#include <array>


enum Color : bool { White, Black };

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

static unsigned char cellPiece(Cell cell) {
	return (cell & 0b0111) - 1;
}

static Color cellColor(Cell cell) {
	return (cell & 0b1000) ? Black : White;
}

class Game {
private:
	std::array<std::array<Cell, 8>, 8> board;
	Color playerTurn = White;


	olc::vi2d lastMove; // for en passant



	bool
		WKsCastled = false,
		WQsCastled = false,
		BKsCastled = false,
		BQsCastled = false;


private:
	void initBoard();

public:
	void performMove(olc::vi2d fr, olc::vi2d to);
	
	unsigned long long getLegalMoves(int i, int j);
	unsigned long long getLegalMoves(olc::vi2d pos) { return getLegalMoves(pos.x, pos.y); }


	Game() {
		initBoard();
	}

	Cell getCell(int i, int j) { return board[i][j]; }
	Cell getCell(olc::vi2d pos) { return getCell(pos.x, pos.y); }
};