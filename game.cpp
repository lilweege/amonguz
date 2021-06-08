#include "game.h"

// https://www.chessprogramming.org/
// https://chess.org/rules

Cell Game::pieceFromChar(char pieceChar) {
	switch (pieceChar) {
		case 'K': return WhiteKing;
		case 'k': return BlackKing;
		case 'Q': return WhiteQueen;
		case 'q': return BlackQueen;
		case 'B': return WhiteBishop;
		case 'b': return BlackBishop;
		case 'N': return WhiteKnight;
		case 'n': return BlackKnight;
		case 'R': return WhiteRook;
		case 'r': return BlackRook;
		case 'P': return WhitePawn;
		case 'p': return BlackPawn;
		default: return Empty;
	}
}

void Game::fromFEN(const std::string& sequence) {
	// assume sequence is valid (because I'm lazy)
	// fields are space-separated subsequences

	// field 1: piece placement
	for (int i = 0; i < 8; ++i)
		for (int j = 0; j < 8; ++j)
			board[i][j] = Empty;

	size_t idx = 0;
	for (int i = 0, j = 0; idx < sequence.size(); ++idx) {
		char c = sequence[idx];
		if (c == ' ')
			break;
		if (c == '/') {
			++j;
			i = 0;
			continue;
		}
		if ('1' <= c && c <= '8') {
			i += c - '0';
		}
		else {
			board[i][j] = Game::pieceFromChar(c);
			++i;
		}
	}
	++idx;

	// state conditions
	// field 2: active color
	// <Side to move> ::= {'w' | 'b'}
	isWhiteTurn = sequence[idx] == 'w';
	idx += 2;
	
	// field 3: castling rights
	// <Castling ability> ::= '-' | ['K'] ['Q'] ['k'] ['q'] (1..4)
	WKsCanCastle = false;
	WQsCanCastle = false;
	BKsCanCastle = false;
	BQsCanCastle = false;
	if (sequence[idx] == '-') {
		idx += 2;
	}
	else {
		// the field should be ordered anyways, but
		// this way of doing it is simple enough
		for (; idx < sequence.size(); ++idx) {
			char c = sequence[idx];
			if (c == ' ')
				break;
			switch (c) {
				case 'K': WKsCanCastle = true; break;
				case 'Q': WQsCanCastle = true; break;
				case 'k': BKsCanCastle = true; break;
				case 'q': BQsCanCastle = true; break;
				default: break;
			}
		}
		++idx;
	}

	// field 4: possible en passant targets
	// <En passant target square> ::= '-' | <epsquare>
	// <epsquare>   ::= <fileLetter> <eprank>
	// <fileLetter> ::= 'a' | 'b' | 'c' | 'd' | 'e' | 'f' | 'g' | 'h'
	// <eprank>     ::= '3' | '6'
	if (sequence[idx] == '-') {
		idx += 2;
	}
	else {
		// position 'behind' the moved pawn
		char file = sequence[idx++]; // a single letter
		char rank = sequence[idx++]; // a single digit
		int i = file - 'a';
		// int j = 8 - (rank - '0');
		// if (j == 2) // black pawn moved
		if (rank == '6')
			lastMove = { i, 1 };
		// else if (j == 5) // white ...
		else if (rank == '3')
			lastMove = { i, 6 };
		++idx;
	}

	// field 5: halfmove clock
	// <Halfmove Clock> ::= <digit> {<digit>}
	// <digit> ::= '0' | '1' | '2' | '3' | '4' | '5' | '6' | '7' | '8' | '9'
	int end = sequence.find(' ', idx);
	halfmoveClock = atoi(sequence.substr(idx, end - idx).c_str());
	idx = end;

	// field 6: fullmove counter
	// <Fullmove counter> ::= <digit19> {<digit>}
	// <digit19> ::= '1' | '2' | '3' | '4' | '5' | '6' | '7' | '8' | '9'
	// <digit>   ::= '0' | <digit19>
	fullmoveCounter = atoi(sequence.substr(idx).c_str());
}

int Game::addBoardToHistory() {
	// faux FEN code as string for hashable type
	std::string currentBoardHash;
	currentBoardHash.reserve(64 + 5); // nice
	for (int i = 0; i < 8; ++i)
		for (int j = 0; j < 8; ++j)
			currentBoardHash.push_back(board[i][j]);
	currentBoardHash.push_back(isWhiteTurn);
	currentBoardHash.push_back(BQsCanCastle);
	currentBoardHash.push_back(BKsCanCastle);
	currentBoardHash.push_back(WQsCanCastle);
	currentBoardHash.push_back(WKsCanCastle);
	// pawn stucture can't ever the same so
	// en passant doesn't have to be included (I hope)
	// however there may be some other weird edge cases

	previousBoards.insert(currentBoardHash);
	return previousBoards.count(currentBoardHash);
}

void Game::performMove(olc::vi2d fr, olc::vi2d to) {
	++halfmoveClock;
	if (board[to.x][to.y] != Empty)
		halfmoveClock = 0;
	if (!isWhiteTurn)
		++fullmoveCounter;
	
	board[to.x][to.y] = board[fr.x][fr.y];
	board[fr.x][fr.y] = Empty;

	// en passant, promotion
	int i = to.x,
		j = to.y;
	if (cellPiece(board[i][j]) == Pawn) {
		halfmoveClock = 0;
		if (board[i][j] == WhitePawn) {
			if (j == 2 && lastMove.y == 1 && board[i][3] == BlackPawn) {
				board[i][3] = Empty;
			}
		}
		else {
			if (j == 5 && lastMove.y == 6 && board[i][4] == WhitePawn) {
				board[i][4] = Empty;
			}
		}
	}
	else if (cellPiece(board[i][j]) == King) {
		if (board[i][j] == WhiteKing) {
			WQsCanCastle = WKsCanCastle = false;
			if (fr.x == 4 && fr.y == 7) {
				if (to.x == 2 && to.y == 7) {
					board[0][7] = Empty;
					board[3][7] = WhiteRook;
				}
				else if (to.x == 6 && to.y == 7) {
					board[7][7] = Empty;
					board[5][7] = WhiteRook;
				}
			}
		}
		else {
			BQsCanCastle = BKsCanCastle = false;
			if (fr.x == 4 && fr.y == 0) {
				if (to.x == 2 && to.y == 0) {
					board[0][0] = Empty;
					board[3][0] = BlackRook;
				}
				else if (to.x == 6 && to.y == 0) {
					board[7][0] = Empty;
					board[5][0] = BlackRook;
				}
			}
		}
	}
	else if (cellPiece(board[i][j]) == Rook) {
		// B > Qs (0, 0) | (7, 0) Ks
		// W > Qs (0, 7) | (7, 7) Ks
		if (board[i][j] == BlackRook) {
			if (BKsCanCastle && fr.x == 7 && fr.y == 0)
				BKsCanCastle = false;
			else if (BQsCanCastle && fr.x == 0 && fr.y == 0)
				BQsCanCastle = false;
		}
		else {
			if (WKsCanCastle && fr.x == 7 && fr.y == 7)
				WKsCanCastle = false;
			else if (WQsCanCastle && fr.x == 0 && fr.y == 7)
				WQsCanCastle = false;
		}
	}


	// TODO: online stuff
	lastMove = fr;
	isWhiteTurn ^= true;
	
	// fifty-move rule
	if (halfmoveClock >= 100) {
		// TODO: end game draw
		// std::cout << "DRAW\n";
	}
	
	// 3rd repetition rule
	if (addBoardToHistory() >= 3) {
		// TODO: end game draw
		// std::cout << "DRAW\n";
	}

	// TODO: insuficcient material
	// TODO: stalemate

	// TODO: checkmate
}

// TODO: refactor
// TODO: castling
// TODO: king in check move rules
// FIXME: test and catch all edge cases (there will be many)
uint64_t Game::getLegalMoves(int i, int j) const {
	uint64_t moves = 0ULL;
	auto validateMove = [&moves] (int x, int y) {
		moves |= (1ULL << (x * 8 + y));
	};

	Cell piece = board[i][j];
	// assert(piece != Empty);
	if (cellIsWhite(piece) != isWhiteTurn)
		return moves;
	
	// a pawn should never exist on the first or last rank
	if (piece == BlackPawn) {
		// normal move
		if (board[i][j + 1] == Empty) {
			validateMove(i, j + 1);
			// double move
			if (j == 1 && board[i][3] == Empty) {
				validateMove(i, 3);
			}
		}
		// normal capture
		if (i + 1 < 8) {
			Cell target = board[i + 1][j + 1];
			if (target != Empty && cellIsWhite(target) != cellIsWhite(piece)) {
				validateMove(i + 1, j + 1);
			}
		}
		if (i - 1 >= 0) {
			Cell target = board[i - 1][j + 1];
			if (target != Empty && cellIsWhite(target) != cellIsWhite(piece)) {
				validateMove(i - 1, j + 1);
			}
		}
		// en passant capture
		if (j == 4 && lastMove.y == 6) {
			if (lastMove.x == i + 1 && board[i + 1][j] == WhitePawn)
				validateMove(i + 1, j + 1);
			if (lastMove.x == i - 1 && board[i - 1][j] == WhitePawn)
				validateMove(i - 1, j + 1);
		}
	}
	if (piece == WhitePawn) {
		if (board[i][j - 1] == Empty) {
			validateMove(i, j - 1);
			if (j == 6 && board[i][4] == Empty) {
				validateMove(i, 4);
			}
		}
		if (i + 1 < 8) {
			Cell target = board[i + 1][j - 1];
			if (target != Empty && cellIsWhite(target) != cellIsWhite(piece)) {
				validateMove(i + 1, j - 1);
			}
		}
		if (i - 1 >= 0) {
			Cell target = board[i - 1][j - 1];
			if (target != Empty && cellIsWhite(target) != cellIsWhite(piece)) {
				validateMove(i - 1, j - 1);
			}
		}
		if (j == 3 && lastMove.y == 1) {
			if (lastMove.x == i + 1 && board[i + 1][j] == BlackPawn)
				validateMove(i + 1, j - 1);
			if (lastMove.x == i - 1 && board[i - 1][j] == BlackPawn)
				validateMove(i - 1, j - 1);
		}
	}
	if (cellPiece(piece) == Rook || cellPiece(piece) == Queen) {
		for (int x = i + 1; x < 8; ++x) {
			Cell target = board[x][j];
			if (target == Empty) {
				validateMove(x, j);
				continue;
			}
			if (cellIsWhite(target) != cellIsWhite(piece))
				validateMove(x, j);
			break;
		}
		for (int x = i - 1; x >= 0; --x) {
			Cell target = board[x][j];
			if (target == Empty) {
				validateMove(x, j);
				continue;
			}
			if (cellIsWhite(target) != cellIsWhite(piece))
				validateMove(x, j);
			break;
		}
		for (int y = j + 1; y < 8; ++y) {
			Cell target = board[i][y];
			if (target == Empty) {
				validateMove(i, y);
				continue;
			}
			if (cellIsWhite(target) != cellIsWhite(piece))
				validateMove(i, y);
			break;
		}
		for (int y = j - 1; y >= 0; --y) {
			Cell target = board[i][y];
			if (target == Empty) {
				validateMove(i, y);
				continue;
			}
			if (cellIsWhite(target) != cellIsWhite(piece))
				validateMove(i, y);
			break;
		}
	}
	if (cellPiece(piece) == Bishop || cellPiece(piece) == Queen) {
		for (int x = i + 1, y = j + 1; x < 8 && y < 8; ++x, ++y) {
			Cell target = board[x][y];
			if (target == Empty) {
				validateMove(x, y);
				continue;
			}
			if (cellIsWhite(target) != cellIsWhite(piece))
				validateMove(x, y);
			break;
		}
		for (int x = i + 1, y = j - 1; x < 8 && y >= 0; ++x, --y) {
			Cell target = board[x][y];
			if (target == Empty) {
				validateMove(x, y);
				continue;
			}
			if (cellIsWhite(target) != cellIsWhite(piece))
				validateMove(x, y);
			break;
		}
		for (int x = i - 1, y = j + 1; x >= 0 && y < 8; --x, ++y) {
			Cell target = board[x][y];
			if (target == Empty) {
				validateMove(x, y);
				continue;
			}
			if (cellIsWhite(target) != cellIsWhite(piece))
				validateMove(x, y);
			break;
		}
		for (int x = i - 1, y = j - 1; x >= 0 && y >= 0; --x, --y) {
			Cell target = board[x][y];
			if (target == Empty) {
				validateMove(x, y);
				continue;
			}
			if (cellIsWhite(target) != cellIsWhite(piece))
				validateMove(x, y);
			break;
		}
	}
	if (cellPiece(piece) == Knight) {
		for (int x = i - 1; x <= i + 1; x += 2) {
			for (int y = j - 2; y <= j + 2; y += 4) {
				if (x < 0 || x >= 8 || y < 0 || y >= 8)
					continue;
				Cell target = board[x][y];
				if (target == Empty || cellIsWhite(target) != cellIsWhite(piece))
					validateMove(x, y);
			}
		}
		for (int x = i - 2; x <= i + 2; x += 4) {
			for (int y = j - 1; y <= j + 1; y += 2) {
				if (x < 0 || x >= 8 || y < 0 || y >= 8)
					continue;
				Cell target = board[x][y];
				if (target == Empty || cellIsWhite(target) != cellIsWhite(piece))
					validateMove(x, y);
			}
		}
	}
	if (cellPiece(piece) == King) {
		// TODO: castling
		for (int x = i - 1; x <= i + 1; ++x) {
			for (int y = j - 1; y <= j + 1; ++y) {
				if ((x == i && y == j) || x < 0 || x >= 8 || y < 0 || y >= 8)
					continue;
				Cell target = board[x][y];
				if (target == Empty || cellIsWhite(target) != cellIsWhite(piece))
					validateMove(x, y);
			}
		}
		if (piece == WhiteKing) {
			if (WKsCanCastle) {
				if (board[5][7] == Empty &&
					board[6][7] == Empty) {
					validateMove(6, 7);
				}
			}
			if (WQsCanCastle) {
				if (board[1][7] == Empty &&
					board[2][7] == Empty && 
					board[3][7] == Empty) {
					validateMove(2, 7);
				}
			}
		}
		else {
			if (BKsCanCastle) {
				if (board[5][0] == Empty &&
					board[6][0] == Empty) {
					validateMove(6, 0);
				}
			}
			if (BQsCanCastle) {
				if (board[1][0] == Empty &&
					board[2][0] == Empty && 
					board[3][0] == Empty) {
					validateMove(2, 0);
				}
			}
		}
	}


	return moves;
}