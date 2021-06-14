#include "game.h"

// https://www.chessprogramming.org/
// https://chess.org/rules

// init
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
			board[i][j] = pieceFromChar(c);
			++i;
		}
	}
	++idx;

	// state conditions
	// field 2: active color
	// <Side to move> ::= {'w' | 'b'}
	playerToMove = sequence[idx] == 'w' ? White : Black;
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
		if (rank == '6') {
			lastMoveFr = { i, 1 };
			lastMoveTo = { i, 3 };
		}
		// else if (j == 5) // white ...
		else if (rank == '3') {
			lastMoveFr = { i, 6 };
			lastMoveTo = { i, 4 };
		}
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


void Game::addPaths(int i, int j) {
	auto newPath = [&](int i, int j) {
		attackingPaths[i][j][++numAttackingPaths[i][j]] = Path{};
		Path& path = attackingPaths[i][j][numAttackingPaths[i][j]];
		path.source = {i, j};
	};

	auto tryAttack = [&](int i, int j, int x, int y) -> bool {
		Cell piece = board[x][y];
		Path& path = attackingPaths[i][j][numAttackingPaths[i][j]];
		if (piece == Empty) {
			if (path.numTargets == 0)
				// attackers[x][y].push_back(&path);
				attackers[x][y][numAttackers[x][y]++] = &path;
		}
		else { // piece exist
			++path.numTargets;
			if (path.numTargets == 1) {
				path.firstTarget = {x, y};
				// attackers[x][y].push_back(&path);
				attackers[x][y][numAttackers[x][y]++] = &path;
			}
			else if (path.numTargets == 2) {
				path.secondTarget = {x, y};
			}
			else {
				path.thirdTarget = {x, y};
			}
			if (kingCellPos.x == x && kingCellPos.y == y)
				kingXrayers.insert(&path);
				// kingXrayers.emplace(olc::vi2d{path.firstTarget.x, path.firstTarget.y}, &path);
				// kingXrayers[path.firstTarget.x][path.firstTarget.y] = &path;
		}
		path.line.emplace_back(path.numTargets, olc::vi2d{x, y});
		return path.numTargets >= 3;
	};

	Cell piece = board[i][j];
	switch (cellType(piece)) {
		case Queen: {
			newPath(i, j);
			for (int x = i + 1, y = j + 1; x < 8 && y < 8; ++x, ++y)
				if (tryAttack(i, j, x, y)) break;
			newPath(i, j);
			for (int x = i + 1, y = j - 1; x < 8 && y >= 0; ++x, --y)
				if (tryAttack(i, j, x, y)) break;
			newPath(i, j);
			for (int x = i - 1, y = j + 1; x >= 0 && y < 8; --x, ++y)
				if (tryAttack(i, j, x, y)) break;
			newPath(i, j);
			for (int x = i - 1, y = j - 1; x >= 0 && y >= 0; --x, --y)
				if (tryAttack(i, j, x, y)) break;
			newPath(i, j);
			for (int x = i + 1; x < 8; ++x)
				if (tryAttack(i, j, x, j)) break;
			newPath(i, j);
			for (int x = i - 1; x >= 0; --x)
				if (tryAttack(i, j, x, j)) break;
			newPath(i, j);
			for (int y = j + 1; y < 8; ++y)
				if (tryAttack(i, j, i, y)) break;
			newPath(i, j);
			for (int y = j - 1; y >= 0; --y)
				if (tryAttack(i, j, i, y)) break;
			break;
		}
		case Bishop: {
			newPath(i, j);
			for (int x = i + 1, y = j + 1; x < 8 && y < 8; ++x, ++y)
				if (tryAttack(i, j, x, y)) break;
			newPath(i, j);
			for (int x = i + 1, y = j - 1; x < 8 && y >= 0; ++x, --y)
				if (tryAttack(i, j, x, y)) break;
			newPath(i, j);
			for (int x = i - 1, y = j + 1; x >= 0 && y < 8; --x, ++y)
				if (tryAttack(i, j, x, y)) break;
			newPath(i, j);
			for (int x = i - 1, y = j - 1; x >= 0 && y >= 0; --x, --y)
				if (tryAttack(i, j, x, y)) break;
			break;
		}
		case Rook: {
			newPath(i, j);
			for (int x = i + 1; x < 8; ++x)
				if (tryAttack(i, j, x, j)) break;
			newPath(i, j);
			for (int x = i - 1; x >= 0; --x)
				if (tryAttack(i, j, x, j)) break;
			newPath(i, j);
			for (int y = j + 1; y < 8; ++y)
				if (tryAttack(i, j, i, y)) break;
			newPath(i, j);
			for (int y = j - 1; y >= 0; --y)
				if (tryAttack(i, j, i, y)) break;
			break;
		}
		case Pawn: {
			int direction = cellColor(piece) == White ? -1 : 1;
			if (i - 1 >= 0) {
				newPath(i, j);
				tryAttack(i, j, i - 1, j + direction);
			}
			if (i + 1 <  8) {
				newPath(i, j);
				tryAttack(i, j, i + 1, j + direction);
			}
			break;
		}
		case Knight: {
			for (int idx = 0; idx < 8; ++idx) {
				int x = i + knightOffsets[idx],
					y = j + knightOffsets[(idx + 2) % 8];
				if (0 <= x && x < 8 &&
					0 <= y && y < 8) {
					newPath(i, j);
					tryAttack(i, j, x, y);
				}
			}
			break;
		}
		case King: {
			for (int x = i - 1; x <= i + 1; ++x)
				for (int y = j - 1; y <= j + 1; ++y) {
					if (i != x && j == y)
						continue;
					if (0 <= x && x < 8 &&
						0 <= y && y < 8) {
						newPath(i, j);
						tryAttack(i, j, x, y);
					}
				}
			break;
		}
		default: break;
	}
}

void Game::computePosition() {
	// faux FEN code as string for hashable type
	std::string currentBoardHash;
	currentBoardHash.reserve(64 + 5); // nice
	for (int i = 0; i < 8; ++i)
		for (int j = 0; j < 8; ++j)
			currentBoardHash.push_back(board[i][j]);
	currentBoardHash.push_back(playerToMove);
	currentBoardHash.push_back(BQsCanCastle);
	currentBoardHash.push_back(BKsCanCastle);
	currentBoardHash.push_back(WQsCanCastle);
	currentBoardHash.push_back(WKsCanCastle);
	// pawn stucture can't ever the same so
	// en passant doesn't have to be included (I hope)
	// however there may be some other weird edge cases

	previousBoards.insert(currentBoardHash);
	lastBoardCount = previousBoards.count(currentBoardHash);

	numLegalMoves = 0;
	extraMoves = 0;

	// TODO: only update as much as necessary, save much computation

	Cell kingPiece = Cell{uint8_t(playerToMove | King)};
	kingXrayers.clear();
	for (int i = 0; i < 8; ++i)
		for (int j = 0; j < 8; ++j) {
			numAttackers[i][j] = 0;
			numAttackingPaths[i][j] = 0;
			// attackers[i][j].clear();
			// kingXrayers[i][j] = nullptr;
			if (board[i][j] == kingPiece)
				kingCellPos = {i, j};
		}
	
	// enemy attack paths
	for (int i = 0; i < 8; ++i)
		for (int j = 0; j < 8; ++j) {
			Cell piece = board[i][j];
			
			if (piece != Empty && cellColor(piece) != playerToMove) {
				// returns true if sliding path should end
				addPaths(i, j);
			}
		}

	int kx = kingCellPos.x,
		ky = kingCellPos.y;
	kingInCheck = numAttackers[kx][ky];
	checkmate = false;
	if (kingInCheck) {
		checkmate = true;
		for (int x = kx - 1; x <= kx + 1; ++x) {
			for (int y = ky - 1; y <= ky + 1; ++y) {
				if (kx != x && ky == y)
					continue;
				if (0 <= x && x < 8 &&
					0 <= y && y < 8)
					if (numAttackers[x][y] == 0) {
						checkmate = false;
						break;
					}
			}
			if (!checkmate)
				break;
		}
	}
	if (checkmate && numAttackers[kx][ky] >= 2) {
		// unsure if this is correct in all cases
		return;
	}
	// if here, 0 or 1 king attackers
	// allow move if
		// blocks attacker
		// captures attacker
		// king is moving out of path

	
	auto getMoves = [&](int i, int j) -> uint64_t {
		uint64_t moves = (this->*(pieceMoves[cellType(board[i][j]) - King]))(i, j);
		// Path* attackerPath = isBlockingCheck(i, j);
		// if (attackerPath) {
		// }
		return moves;
	};
	// FIXME: test and catch all edge cases (there will be many)
	// player legal moves
	for (int i = 0; i < 8; ++i)
		for (int j = 0; j < 8; ++j) {
			Cell piece = board[i][j];
			if (piece != Empty && cellColor(piece) == playerToMove) {
				legalMoves[i][j] = getMoves(i, j);
				// check attack paths (pins)
				// invalidate king in check moves
				// if (can capture attacking piece) checkmate = false;
			}
			else {
				legalMoves[i][j] = 0ULL;
			}
		}
	// for (int j = 0; j < 8; ++j) {
	// 	for (int i = 0; i < 8; ++i)
	// 		std::cout << int(numAttackers[i][j]);
	// 	std::cout << "\n";
	// }

	
}


void Game::performMove(olc::vi2d fr, olc::vi2d to) {
	++halfmoveClock;
	if (board[to.x][to.y] != Empty)
		halfmoveClock = 0;
	if (playerToMove == Black)
		++fullmoveCounter;
	
	// TODO: refactor/organize
	// handle specific cases
	Cell piece = board[fr.x][fr.y];
	if (cellType(piece) == Pawn) {
		halfmoveClock = 0;
		if (cellColor(piece) == White) {
			if (to.y == 2 && lastMoveFr.y == 1 && lastMoveTo.y == 3 && board[to.x][3] == BlackPawn) {
				board[to.x][3] = Empty;
			}
			else if (to.y == 0) {
				board[fr.x][fr.y] = promotionPiece;
			}
		}
		else {
			if (to.y == 5 && lastMoveFr.y == 6 && lastMoveTo.y == 4 && board[to.x][4] == WhitePawn) {
				board[to.x][4] = Empty;
			}
			else if (to.y == 7) {
				board[fr.x][fr.y] = promotionPiece;
			}
		}
	}
	else if (cellType(piece) == King) {
		if (cellColor(piece) == White) {
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
	else if (cellType(piece) == Rook) {
		// B > Qs (0, 0) | (7, 0) Ks
		// W > Qs (0, 7) | (7, 7) Ks
		if (cellColor(piece) == White) {
			if (WKsCanCastle && fr.x == 7 && fr.y == 7)
				WKsCanCastle = false;
			else if (WQsCanCastle && fr.x == 0 && fr.y == 7)
				WQsCanCastle = false;
		}
		else {
			if (BKsCanCastle && fr.x == 7 && fr.y == 0)
				BKsCanCastle = false;
			else if (BQsCanCastle && fr.x == 0 && fr.y == 0)
				BQsCanCastle = false;
		}
	}

	// do the move (edge cases excluded)
	board[to.x][to.y] = board[fr.x][fr.y];
	board[fr.x][fr.y] = Empty;


	// TODO: online stuff
	// only after everything else actually works


	lastMoveFr = fr;
	lastMoveTo = to;
	playerToMove = playerToMove == White ? Black : White;
	computePosition(); // store results in class variables for use below

	// TODO: check and checkmate position
	// TODO: insuficcient material draw
	// TODO: stalemate position draw

	// fifty-move rule draw
	if (halfmoveClock >= 100) {
		// TODO:
	}
	
	// 3rd repetition rule draw
	if (lastBoardCount >= 3) {
		// TODO:
	}
}

// if a piece is blocking their king, they should not move
// returns attack path if found, otherwise null
Path* Game::isBlockingCheck(int i, int j) const {
	// Path* xray = kingXrayers[i][j];
	// return xray;

	// auto xray = kingXrayers.find(olc::vi2d{i, j});
	// return xray != kingXrayers.end() ? xray->second : nullptr;

	for (Path* path : kingXrayers) {
		// if piece is on path between king and source
		if (path->firstTarget.x == i && path->firstTarget.y == j &&
			path->secondTarget.x == kingCellPos.x && path->secondTarget.y == kingCellPos.y)
			return path;
		// FIXME: the nasty en passant double reveal
	}

	return nullptr;
}

// returns true if sliding path should end
// (i, j) -> (x, y)
bool Game::setMove(uint64_t& moves, int i, int j, int x, int y) const {
	Cell target = board[x][y];
	bool allowed = true;

	if (kingInCheck && !(kingCellPos.x == i && kingCellPos.y == j)) {
		allowed = false;
		// allow move if
			// blocks attacker
			// captures attacker
			// king is moving out of path
		
		Path* attackingPath = attackers[kingCellPos.x][kingCellPos.y][0];
		const olc::vi2d& attackerPos = attackingPath->source;

		// capture attacker
		if (attackerPos.x == x && attackerPos.y == y) {
			// no work for en passant
			allowed = true;
		}

		// if move is on path between king and source
		for (const auto& [numTargets, cellPos] : attackingPath->line) {
			if (numTargets > 0)
				break;
			if (cellPos.x == x && cellPos.y == y) {
				allowed = true;
				break;
			}
		}
	}
	else {
		// auto xrayIter = kingXrayers.find(olc::vi2d{i, j});
		// Path* attackingPath = xrayIter->second;
		// Path* attackingPath = kingXrayers[i][j];
		Path* attackingPath = isBlockingCheck(i, j);
		if (attackingPath) {
			allowed = false;
			const olc::vi2d& attackerPos = attackingPath->source;
			// capture attacker
			if (attackerPos.x == x && attackerPos.y == y) {
				// no work for en passant
				allowed = true;
			}

			// if move is on path between king and source
			for (const auto& [numTargets, cellPos] : attackingPath->line) {
				if (numTargets > 0)
					break;
				if (cellPos.x == x && cellPos.y == y) {
					allowed = true;
					break;
				}
			}
		}
	}

	if (target == Empty) {
		if (allowed) {
			boardSetBit(moves, x, y);
			allLegalMoves[numLegalMoves++] = {{i, j}, {x, y}};
			
			if (cellType(board[i][j]) == Pawn &&
				y == (getPlayerToMove() == White ? 0 : 7)) {
				extraMoves += 3;
			}
		}
		return false;
	}
	if (cellColor(target) != cellColor(board[i][j]))
		if (allowed) {
			boardSetBit(moves, x, y);
			allLegalMoves[numLegalMoves++] = {{i, j}, {x, y}};

			if (cellType(board[i][j]) == Pawn &&
				y == (getPlayerToMove() == White ? 0 : 7)) {
				extraMoves += 3;
			}
		}
	return true;
}

// pseudo-legal move generation
uint64_t Game::bishopMoves(int i, int j) const {
	uint64_t moves = 0ULL;
	for (int x = i + 1, y = j + 1; x < 8 && y < 8; ++x, ++y)
		if (setMove(moves, i, j, x, y)) break;
	for (int x = i + 1, y = j - 1; x < 8 && y >= 0; ++x, --y)
		if (setMove(moves, i, j, x, y)) break;
	for (int x = i - 1, y = j + 1; x >= 0 && y < 8; --x, ++y)
		if (setMove(moves, i, j, x, y)) break;
	for (int x = i - 1, y = j - 1; x >= 0 && y >= 0; --x, --y)
		if (setMove(moves, i, j, x, y)) break;
	return moves;
}
uint64_t Game::rookMoves(int i, int j) const {
	uint64_t moves = 0ULL;
	for (int x = i + 1; x < 8; ++x)
		if (setMove(moves, i, j, x, j)) break;
	for (int x = i - 1; x >= 0; --x)
		if (setMove(moves, i, j, x, j)) break;
	for (int y = j + 1; y < 8; ++y)
		if (setMove(moves, i, j, i, y)) break;
	for (int y = j - 1; y >= 0; --y)
		if (setMove(moves, i, j, i, y)) break;
	return moves;
}
uint64_t Game::queenMoves(int i, int j) const {
	return bishopMoves(i, j) | rookMoves(i, j);
}

uint64_t Game::knightMoves(int i, int j) const {
	uint64_t moves = 0ULL;
	for (int idx = 0; idx < 8; ++idx) {
		int x = i + knightOffsets[idx],
			y = j + knightOffsets[(idx + 2) % 8];
		if (0 <= x && x < 8 &&
			0 <= y && y < 8)
			setMove(moves, i, j, x, y);
	}
	return moves;
}

uint64_t Game::castleMoves(int i, int j) const {
	uint64_t moves = 0ULL;
	if (cellColor(board[i][j]) == White) {
		if (WKsCanCastle) {
			if (board[5][7] == Empty && numAttackers[5][7] == 0 &&
				board[6][7] == Empty && numAttackers[6][7] == 0) {
				setMove(moves, i, j, 6, 7);
			}
		}
		if (WQsCanCastle) {
			if (board[1][7] == Empty &&
				board[2][7] == Empty && numAttackers[2][7] == 0 &&
				board[3][7] == Empty && numAttackers[3][7] == 0) {
				setMove(moves, i, j, 2, 7);
			}
		}
	}
	else {
		if (BKsCanCastle) {
			if (board[5][0] == Empty && numAttackers[5][0] == 0 &&
				board[6][0] == Empty && numAttackers[6][0] == 0) {
				setMove(moves, i, j, 6, 0);
			}
		}
		if (BQsCanCastle) {
			if (board[1][0] == Empty &&
				board[2][0] == Empty && numAttackers[2][0] == 0 &&
				board[3][0] == Empty && numAttackers[3][0] == 0) {
				setMove(moves, i, j, 2, 0);
			}
		}
	}
	return moves;
}
uint64_t Game::kingMoves(int i, int j) const {
	uint64_t moves = 0ULL;
	for (int x = i - 1; x <= i + 1; ++x)
		for (int y = j - 1; y <= j + 1; ++y) {
			if (i == x && j == y)
				continue;
			if (0 <= x && x < 8 &&
				0 <= y && y < 8) {
				if (kingInCheck) {
					for (int idx = 0; idx < numAttackers[i][j]; ++idx) {
						Path* path = attackers[i][j][idx];
						if (path->firstTarget != kingCellPos)
							continue;
						for (const auto& [_, cellPos] : path->line)
							if (cellPos.x == x && cellPos.y == y)
								goto invalid;
					}

				}
				if (numAttackers[x][y] == 0)
					setMove(moves, i, j, x, y);
				invalid:;
			}
		}
	if (!kingInCheck)
		moves |= castleMoves(i, j);
	return moves;
}

uint64_t Game::enPassantMoves(int i, int j) const {
	uint64_t moves = 0ULL;
	Cell pawnColor = cellColor(board[i][j]);
	int firstRank = pawnColor == White ? 6 : 1,
		direction = pawnColor == White ? -1 : 1;
	if (j == firstRank + direction * 3 &&
		lastMoveTo.y == j &&
		lastMoveFr.y == firstRank + direction * 5) {
		Cell targetPawn = Cell{(uint8_t)((pawnColor == White ? Black : White) | Pawn)};
		if (lastMoveFr.x == i + 1 && board[i + 1][j] == targetPawn)
			setMove(moves, i, j, i + 1, j + direction);
		if (lastMoveFr.x == i - 1 && board[i - 1][j] == targetPawn)
			setMove(moves, i, j, i - 1, j + direction);
	}
	return moves;
}
uint64_t Game::pawnMoves(int i, int j) const {
	uint64_t moves = 0ULL;
	Cell pawnColor = cellColor(board[i][j]);
	int firstRank = pawnColor == White ? 6 : 1,
		direction = pawnColor == White ? -1 : 1;
	// a pawn should never exist on the first or last rank
	// normal move
	if (board[i][j + direction] == Empty) {
		setMove(moves, i, j, i, j + direction);
		// double move
		if (board[i][j + direction * 2] == Empty && j == firstRank)
			setMove(moves, i, j, i, j + direction * 2);
	}

	// normal capture
	if (i + 1 < 8 &&
		board[i + 1][j + direction] != Empty &&
		cellColor(board[i + 1][j + direction]) != pawnColor)
		setMove(moves, i, j, i + 1, j + direction);
	if (i - 1 >= 0 &&
		board[i - 1][j + direction] != Empty &&
		cellColor(board[i - 1][j + direction]) != pawnColor)
		setMove(moves, i, j, i - 1, j + direction);
	moves |= enPassantMoves(i, j);
	return moves;
}

