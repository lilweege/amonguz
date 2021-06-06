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

// TODO: refactor
// TODO: en passant and castling
// TODO: king in check move rules
// FIXME: test and catch all edge cases (there will be many)
unsigned long long Game::getLegalMoves(int i, int j) {
	unsigned long long moves = 0ULL;
	auto validateMove = [&moves] (int x, int y) {
		moves |= (1ULL << (x * 8 + y));
	};


	Cell piece = getCell(i, j);
	// assert(piece != Empty);
	if (piece == BlackPawn) {
		// a pawn should never exist on the first or last rank
		// TODO: en passant
		if (getCell(i, j + 1) == Empty) {
			validateMove(i, j + 1);
			if (j == 1 && getCell(i, 3) == Empty) {
				validateMove(i, 3);
			}
		}
		if (i + 1 < 8) {
			Cell target = getCell(i + 1, j + 1);
			if (target != Empty && cellColor(target) == White) {
				validateMove(i + 1, j + 1);
			}
		}
		if (i - 1 >= 0) {
			Cell target = getCell(i - 1, j + 1);
			if (target != Empty && cellColor(target) == White) {
				validateMove(i - 1, j + 1);
			}
		}
	}
	if (piece == WhitePawn) {
		if (getCell(i, j - 1) == Empty) {
			validateMove(i, j - 1);
			if (j == 6 && getCell(i, 4) == Empty) {
				validateMove(i, 4);
			}
		}
		if (i + 1 < 8) {
			Cell target = getCell(i + 1, j - 1);
			if (target != Empty && cellColor(target) == Black) {
				validateMove(i + 1, j - 1);
			}
		}
		if (i - 1 >= 0) {
			Cell target = getCell(i - 1, j - 1);
			if (target != Empty && cellColor(target) == Black) {
				validateMove(i - 1, j - 1);
			}
		}
	}
	if (piece == BlackRook || piece == WhiteRook || piece == BlackQueen || piece == WhiteQueen) {
		for (int x = i + 1; x < 8; ++x) {
			Cell target = getCell(x, j);
			if (target == Empty) {
				validateMove(x, j);
				continue;
			}
			if (cellColor(target) != cellColor(piece))
				validateMove(x, j);
			break;
		}
		for (int x = i - 1; x >= 0; --x) {
			Cell target = getCell(x, j);
			if (target == Empty) {
				validateMove(x, j);
				continue;
			}
			if (cellColor(target) != cellColor(piece))
				validateMove(x, j);
			break;
		}
		for (int y = j + 1; y < 8; ++y) {
			Cell target = getCell(i, y);
			if (target == Empty) {
				validateMove(i, y);
				continue;
			}
			if (cellColor(target) != cellColor(piece))
				validateMove(i, y);
			break;
		}
		for (int y = j - 1; y >= 0; --y) {
			Cell target = getCell(i, y);
			if (target == Empty) {
				validateMove(i, y);
				continue;
			}
			if (cellColor(target) != cellColor(piece))
				validateMove(i, y);
			break;
		}
	}
	if (piece == BlackBishop || piece == WhiteBishop || piece == BlackQueen || piece == WhiteQueen) {
		for (int x = i + 1, y = j + 1; x < 8 && y < 8; ++x, ++y) {
			Cell target = getCell(x, y);
			if (target == Empty) {
				validateMove(x, y);
				continue;
			}
			if (cellColor(target) != cellColor(piece))
				validateMove(x, y);
			break;
		}
		for (int x = i + 1, y = j - 1; x < 8 && y >= 0; ++x, --y) {
			Cell target = getCell(x, y);
			if (target == Empty) {
				validateMove(x, y);
				continue;
			}
			if (cellColor(target) != cellColor(piece))
				validateMove(x, y);
			break;
		}
		for (int x = i - 1, y = j + 1; x >= 0 && y < 8; --x, ++y) {
			Cell target = getCell(x, y);
			if (target == Empty) {
				validateMove(x, y);
				continue;
			}
			if (cellColor(target) != cellColor(piece))
				validateMove(x, y);
			break;
		}
		for (int x = i - 1, y = j - 1; x >= 0 && y >= 0; --x, --y) {
			Cell target = getCell(x, y);
			if (target == Empty) {
				validateMove(x, y);
				continue;
			}
			if (cellColor(target) != cellColor(piece))
				validateMove(x, y);
			break;
		}
	}
	if (piece == BlackKnight || piece == WhiteKnight) {
		for (int x = i - 1; x <= i + 1; x += 2) {
			for (int y = j - 2; y <= j + 2; y += 4) {
				if (x < 0 || x >= 8 || y < 0 || y >= 8)
					continue;
				Cell target = getCell(x, y);
				if (target == Empty || cellColor(target) != cellColor(piece))
					validateMove(x, y);
			}
		}
		for (int x = i - 2; x <= i + 2; x += 4) {
			for (int y = j - 1; y <= j + 1; y += 2) {
				if (x < 0 || x >= 8 || y < 0 || y >= 8)
					continue;
				Cell target = getCell(x, y);
				if (target == Empty || cellColor(target) != cellColor(piece))
					validateMove(x, y);
			}
		}
	}
	if (piece == BlackKing || piece == WhiteKing) {
		// TODO: castling
		for (int x = i - 1; x <= i + 1; ++x) {
			for (int y = j - 1; y <= j + 1; ++y) {
				if ((x == i && y == j) || x < 0 || x >= 8 || y < 0 || y >= 8)
					continue;
				Cell target = getCell(x, y);
				if (target == Empty || cellColor(target) != cellColor(piece))
					validateMove(x, y);
			}
		}
	}


	return moves;
}