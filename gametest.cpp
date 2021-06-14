#include "game.h"
#include <chrono>

struct GameTest {
	Game game;

	uint64_t perft(int depth /* >= 1*/) {
		uint64_t nodes = 0ULL;
		if (game.isCheckmate()) // works without this ??
			return uint64_t(game.numLegalMoves + game.extraMoves);

		if (depth == 1)
			return uint64_t(game.numLegalMoves + game.extraMoves);

		for (int i = 0; i < game.numLegalMoves; ++i) {
			const auto& [fr, to] = game.allLegalMoves[i];
			Game copy = game;
			if (game.getCell(fr) == (game.getPlayerToMove() | Pawn) &&
				to.y == (game.getPlayerToMove() == White ? 0 : 7)) {
				for (uint8_t pieceType = Queen; pieceType <= Rook; ++pieceType) {
					// game.extraMoves -= 3; // ??????
					game.setPromotionPiece(Cell{pieceType});
					game.performMove(fr, to);
					nodes += perft(depth - 1);
					game = copy;
				}
			}
			else {
				game.performMove(fr, to);
				nodes += perft(depth - 1);
				game = copy;
			}
		}
		return nodes;
	}

	void test(int maxDepth, const std::string& fen = "") {
		for (int i = 1; i <= maxDepth; ++i) {
			game = fen == "" ? Game{} : Game{fen};

			auto start = std::chrono::high_resolution_clock::now();
			uint64_t result = perft(i);
			auto stop = std::chrono::high_resolution_clock::now();
			auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);

			std::cout << "depth " << i << ": " << result << "\t\t in " << (duration.count() / 1000000.0f) << " second\n";
		}
	}

	void run() {
		// test(5);
		test(4, "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 0");
	}
};