#pragma once

#include "olcPixelGameEngine.h"
#include <array>
#include <unordered_set>

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

static Cell cellType(Cell cell) { return Cell{(uint8_t)(cell & 0b00111)}; }
static Cell cellColor(Cell cell) { return Cell{(uint8_t)(cell & 0b11000)}; }

class Game {
private:
	std::array<std::array<Cell, 8>, 8> board;
	std::array<std::array<Cell, 8>, 8> tempBoard; // for king check
	
	int fullmoveCounter = 0; // this is not really useful for anything
	int halfmoveClock = 0; // fifty-move rule
	bool isWhiteTurn = true;

	olc::vi2d lastMoveFr;
	olc::vi2d lastMoveTo;
	
	// there has to be a better way to do this
	std::unordered_multiset<std::string> previousBoards; // 3rd repetition rule

	bool
		WKsCanCastle = true,
		WQsCanCastle = true,
		BKsCanCastle = true,
		BQsCanCastle = true;


private:
	void fromFEN(const std::string& sequence);
	static Cell pieceFromChar(char c);
	int addBoardToHistory(); // add current board to previousBoards, returns count

	bool setMove(uint64_t& moves, int i, int j, int x, int y) const;
	uint64_t kingMoves(int i, int j) const;
	uint64_t queenMoves(int i, int j) const;
	uint64_t bishopMoves(int i, int j) const;
	uint64_t knightMoves(int i, int j) const;
	uint64_t rookMoves(int i, int j) const;
	uint64_t pawnMoves(int i, int j) const;
	uint64_t (Game::*pieceMoves[6])(int i, int j) const = { &kingMoves, &queenMoves, &bishopMoves, &knightMoves, &rookMoves, &pawnMoves };
	uint64_t enPassantMoves(int i, int j) const;
	uint64_t castleMoves(int i, int j) const;
	
	static bool boardGetBit(uint64_t board, int x, int y) { return (board >> (x * 8 + y)) & 1; }
	static void boardSetBit(uint64_t& board, int x, int y) { board |= (1ULL << (x * 8 + y)); }
	
public:
	void performMove(olc::vi2d fr, olc::vi2d to);
	
	uint64_t getLegalMoves(int i, int j) const;
	uint64_t getLegalMoves(olc::vi2d pos) const { return getLegalMoves(pos.x, pos.y); }


	Game() {
		fromFEN("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"); // starting
		// test
		// fromFEN("r1b1k1nr/p2p1pNp/n2B4/1p1NP2P/6P1/3P1Q2/P1P1K3/q5b1");
		// fromFEN("4k2r/6r1/8/8/8/8/3R4/R3K3 w Qk - 0 1"); // castle
		// fromFEN("rnbqkbnr/pppp1ppp/8/8/3pP3/6PP/PPPP1P11/RNBQKBNR b KQkq e3 0 3"); // en passant
		// fromFEN("8/5k2/3p4/1p1Pp2p/pP2Pp1P/P4P1K/8/8 b - - 99 50"); // draw
		
		addBoardToHistory();
	}

	Cell getCell(int i, int j) { return board[i][j]; }
	Cell getCell(olc::vi2d pos) { return getCell(pos.x, pos.y); }
	// ideally these would not be public
	void setCell(int i, int j, Cell val)  { board[i][j] = val; }
	void setCell(olc::vi2d pos, Cell val) { setCell(pos.x, pos.y, val); }
	bool getWhiteTurn() { return isWhiteTurn; }
};