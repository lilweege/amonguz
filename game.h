#pragma once

// olc::vi2d is used as a drop in replacement for std::pair of int, int
// for a standalone application, but here use vi2d for ease of interfacing
#include "olcPixelGameEngine.h"
#include <array>
#include <unordered_set>
#include <map>

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

static Cell cellType (Cell cell) { return Cell{uint8_t(cell & 0b00111)}; }
static Cell cellColor(Cell cell) { return Cell{uint8_t(cell & 0b11000)}; }

struct Path {
	olc::vi2d source;
	// for small vector sizes, linear search faster than hashtable lookup (probably)
	std::vector<std::pair<uint8_t, olc::vi2d>> line;

	int numTargets = 0;
	olc::vi2d firstTarget;
	olc::vi2d secondTarget; // sliding piece pins
	olc::vi2d thirdTarget; // en passant double reveal edge case ?
};

class Game {
private:
	static constexpr int knightOffsets[8] = {-2, -2, -1,  1,  2,  2,  1, -1};

	std::array<std::array<Cell, 8>, 8> board;
	std::unordered_multiset<std::string> previousBoards; // 3rd repetition rule
	int lastBoardCount;
	std::array<std::array<uint64_t, 8>, 8> legalMoves; // player

	// std::array<std::array<std::array<Path*, 2>, 8>, 8> attackers; // opponents
	std::array<std::array<std::array<Path*, 14>, 8>, 8> attackers; // opponents
	std::unordered_set<Path*> kingXrayers; // first target, path
	// std::map<olc::vi2d, Path*> kingXrayers; // first target, path
	// std::array<std::array<Path*, 8>, 8> kingXrayers;
	// std::array<std::array<std::array<Path, 8>, 8>, 8> attackingPaths; // opponent
	std::array<std::array<std::array<Path, 14>, 8>, 8> attackingPaths; // opponent
	std::array<std::array<uint8_t, 8>, 8> numAttackers;
	std::array<std::array<uint8_t, 8>, 8> numAttackingPaths;

	olc::vi2d kingCellPos;
	bool kingInCheck = false;
	bool checkmate = false;

	int fullmoveCounter = 0; // this is not really useful for anything
	int halfmoveClock = 0; // fifty-move rule
	Cell playerToMove = White;

	olc::vi2d lastMoveFr;
	olc::vi2d lastMoveTo;
	Cell promotionPiece;
	bool
		WKsCanCastle = true,
		WQsCanCastle = true,
		BKsCanCastle = true,
		BQsCanCastle = true;


private:
	static Cell pieceFromChar(char c);

	void fromFEN(const std::string& sequence);
	void addPaths(int i, int j);
	void computePosition();
	bool setMove(uint64_t& moves, int i, int j, int x, int y) const;
	Path* isBlockingCheck(int i, int j) const;
	uint64_t kingMoves(int i, int j) const;
	uint64_t queenMoves(int i, int j) const;
	uint64_t bishopMoves(int i, int j) const;
	uint64_t knightMoves(int i, int j) const;
	uint64_t rookMoves(int i, int j) const;
	uint64_t pawnMoves(int i, int j) const;
	uint64_t (Game::*pieceMoves[6])(int i, int j) const = { &kingMoves, &queenMoves, &bishopMoves, &knightMoves, &rookMoves, &pawnMoves };
	uint64_t enPassantMoves(int i, int j) const;
	uint64_t castleMoves(int i, int j) const;

public:
	static bool boardGetBit(uint64_t board, int x, int y) { return (board >> (x * 8 + y)) & 1; }
	static void boardSetBit(uint64_t& board, int x, int y) { board |= (1ULL << (x * 8 + y)); }
	static void boardUnsetBit(uint64_t& board, int x, int y) { board &= ~(1ULL << (x * 8 + y)); }
	// static void boardToggleBit(uint64_t& board, int x, int y) { board ^= (1ULL << (x * 8 + y)); }

	void setPromotionPiece(Cell type) { promotionPiece = Cell{uint8_t(type | playerToMove)}; }
	void performMove(olc::vi2d fr, olc::vi2d to);
	uint64_t getLegalMoves(int i, int j) const { return legalMoves[i][j]; }
	uint64_t getLegalMoves(olc::vi2d pos) const { return getLegalMoves(pos.x, pos.y); }
	Cell getCell(int i, int j) const { return board[i][j]; }
	Cell getCell(olc::vi2d pos) const { return getCell(pos.x, pos.y); }
	Cell getPlayerToMove() const { return playerToMove; }
	bool isCheckmate() const { return checkmate; }

	mutable std::array<std::pair<olc::vi2d, olc::vi2d>, 256> allLegalMoves;
	mutable int numLegalMoves = 0;
	mutable int extraMoves = 0;

	Game(const std::string& startingPosition = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1") {
		fromFEN(startingPosition);
		computePosition();
	}
};