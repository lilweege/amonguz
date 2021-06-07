#pragma once

#include "olcPixelGameEngine.h"
#include <array>


enum Cell : uint8_t {
	// 'abstract' cells
	White		= 0b01000,
	Black		= 0b10000,
	King		= 0b00001,
	Queen		= 0b00010,
	Bishop		= 0b00011,
	Knight		= 0b00100,
	Rook		= 0b00101,
	Pawn		= 0b00110,

	// valid cells
	Empty		= 0,

	WhiteKing		= White | King,
	WhiteQueen		= White | Queen,
	WhiteBishop		= White | Bishop,
	WhiteKnight		= White | Knight,
	WhiteRook		= White | Rook,
	WhitePawn		= White | Pawn,

	BlackKing		= Black | King,
	BlackQueen		= Black | Queen,
	BlackBishop		= Black | Bishop,
	BlackKnight		= Black | Knight,
	BlackRook		= Black | Rook,
	BlackPawn		= Black | Pawn,
};

static Cell cellPiece(Cell cell) {
	return Cell{(uint8_t)((cell & 0b00111))};
}

static bool cellIsWhite(Cell cell) {
	return cell & White;
}

class Game {
private:
	std::array<std::array<Cell, 8>, 8> board;
	std::array<std::array<Cell, 8>, 8> tempBoard; // for king check
	bool isWhiteTurn = true;
	// Color playerTurn = White;


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
	
	uint64_t getLegalMoves(int i, int j) const;
	uint64_t getLegalMoves(olc::vi2d pos) const { return getLegalMoves(pos.x, pos.y); }


	Game() {
		initBoard();
	}

	Cell getCell(int i, int j) { return board[i][j]; }
	Cell getCell(olc::vi2d pos) { return getCell(pos.x, pos.y); }
	void setCell(int i, int j, Cell val)  { board[i][j] = val; }
	void setCell(olc::vi2d pos, Cell val) { setCell(pos.x, pos.y, val); }
	bool getWhiteTurn() { return isWhiteTurn; }
};