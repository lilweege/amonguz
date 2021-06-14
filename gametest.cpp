#include "game.h"

struct GameTest {
	Game game;

	uint64_t lastNodes = 0;
	uint64_t nodes = 0;
	uint64_t perft(int depth) {

		if (depth == 0) {
			if (nodes - lastNodes > 10000ULL) {
				std::cout << nodes << std::endl;
				lastNodes = nodes;
			}
			return 1ULL;
		}

		// rip ram
		uint64_t num = 0;
		for (int i = 0; i < game.numLegalMoves; ++i) {
			const auto& [fr, to] = game.allLegalMoves[i];
			Game copy = game;
			game.performMove(fr, to);
			num += perft(depth - 1);
			game = copy;
		}
		return num;
	}

	void test(int depth, const std::string& fen = "") {
		for (int i = 0; i < depth; ++i) {
			nodes = 0;
			game = fen == "" ? Game{} : Game{fen};
			std::cout << "depth " << i << ": " << perft(i) << std::endl;
		}
	}

	void run() {
		test(5, "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 0");
		test(6);
	}
};