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

struct Move {
	// x (i) => file (col)
	// y (j) => rank (row)
	// from -> to
	olc::vi2d fr, to;
};

class Game {
private:
	std::array<std::array<Cell, 8>, 8> board;
	Color playerTurn = White;

private:
	void initBoard();
	bool isValidMove(Move move);
	// void performMove(Move move);

public:
	// bool tryMove(Move move);
	// TODO: perhaps use a different container
	std::vector<olc::vi2d> getLegalMoves(int i, int j);
	std::vector<olc::vi2d> getLegalMoves(olc::vi2d pos) { return getLegalMoves(pos.x, pos.y); }


	Game() {
		initBoard();
	}

	Cell getCell(int i, int j) { return board[i][j]; }
	Cell getCell(olc::vi2d pos) { return getCell(pos.x, pos.y); }
	void setCell(int i, int j, Cell val) { board[i][j] = val; }
	void setCell(olc::vi2d pos, Cell val) { setCell(pos.x, pos.y, val); }
};