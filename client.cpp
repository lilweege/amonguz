#include "common.h"

#define OLC_PGE_APPLICATION
#include "olcPixelGameEngine.h"

#include "game.h"
#include <unordered_map>


class AmonguzClient : public olc::PixelGameEngine, olc::net::client_interface<GameMsg> {
public:
	AmonguzClient(std::string addr, uint16_t port)
		// : connectionFailed(!Connect(addr, port))
	{
		// if (connectionFailed)
		// 	return;
		
		sAppName = "Amonguz Client";
		return;

		std::cout << "Connection initialized...\n";

		// std::string name;
		// std::cout << "Enter your name [<10 chars]: ";
		// std::cin.ignore();
		// std::getline(std::cin, name);

		// if (name.empty())
		// 	std::strcpy(self.name, "gilbert");
		// else
		// 	std::strncpy(self.name, name.c_str(), sizeof(self.name));

		// self.posX = 1.0f;
		// self.posY = 1.0f;
	}

private:
	bool connectionFailed;
	bool waitingToConnect = true;
	
	std::unordered_map<uint32_t, Player> players;
	uint32_t selfID = 0; Player self;

	olc::Sprite* atlasSprite = nullptr;
	olc::Decal* atlasDecal = nullptr;
	olc::vi2d sheetSize;
	olc::vi2d cellSize;

	int scale;
	Game game;
	bool isPlayerWhite = true;
	// Color playerColor = White;
	bool promotionPrompt = false;
	Cell promotionPiece = Queen;
	olc::vi2d promotionCellPos;

	olc::vi2d selectedCellPos;
	Cell selectedCell = Empty;

	olc::Pixel
		darkColor = olc::VERY_DARK_CYAN,
		lightColor = olc::WHITE,
		hoverColor = olc::YELLOW,
		validColor = { 0, 255, 0, 128 };
		// invalidColor = { 255, 0, 0, 128 };
	
	// constant for all players
	// TODO: store these somewhere elses
	// const int worldScale = 100;
	// const olc::vf2d maxVel = { 1.0f, 1.0f };
	// const float walkAnimSpeed = 20.0f;
	// const float playerRadius = 0.5f * worldScale;

private:

	void getUpdate() {
		// Check for incoming network messages
		if (!IsConnected())
			return;
		
		auto& q = Incoming();
		while (!q.empty()) {
			auto msg = q.pop_front().msg;

			switch (msg.header.id) {
			case GameMsg::Client_Accepted: {
				std::cout << "Server accepted client - you're in!\n";
				olc::net::message<GameMsg> msg;
				msg.header.id = GameMsg::Client_RegisterWithServer;
				msg << self; // set up in constructor
				Send(msg);
				break;
			}

			case GameMsg::Client_AssignID: {
				// Server is assigning us OUR id
				msg >> selfID;
				std::cout << "Assigned Client ID = " << selfID << "\n";
				break;
			}

			case GameMsg::Game_AddPlayer: {
				Player player;
				msg >> player; // expect a player
				players.insert_or_assign(player.uid, player);

				if (player.uid == selfID)
					waitingToConnect = false;
				break;
			}

			case GameMsg::Game_RemovePlayer: {
				uint32_t playerID = 0;
				msg >> playerID; // expect an ID
				players.erase(playerID);
				break;
			}

			case GameMsg::Game_UpdatePlayer: {
				Player desc;
				msg >> desc;
				players.insert_or_assign(desc.uid, desc);
				break;
			}
			}
		}
	}
	
	void doUpdate(float fElapsedTime) {
		Player& me = players[selfID];
		
		// olc::vf2d selfVel;
		// bool
		// 	w = GetKey(olc::Key::W).bHeld,
		// 	a = GetKey(olc::Key::A).bHeld,
		// 	s = GetKey(olc::Key::S).bHeld,
		// 	d = GetKey(olc::Key::D).bHeld;
		// // for now no acceleration
		// if (s != w) selfVel.y = s ? maxVel.y : -maxVel.y;
		// if (d != a) selfVel.x = d ? maxVel.x : -maxVel.x;

		// // moving at all
		// if (selfVel.mag2() > 0) {
		// 	selfVel = selfVel.norm() * 4.0f;
		// 	me.walkCycle = fmod(me.walkCycle + walkAnimSpeed * fElapsedTime, sheetSize.x);
		// }
		// else {
		// 	me.walkCycle = 0;
		// }
		// me.velX = selfVel.x;
		// me.velY = selfVel.y;

		// // all player move clientside
		// for (auto& [id, player] : players) {
		// 	player.posX += player.velX * fElapsedTime;
		// 	player.posY += player.velY * fElapsedTime;
		// 	// TODO: fix this
		// 	// only positive positions pls
		// 	if (player.posX <= 0.0f) player.posX = 0.1f;
		// 	if (player.posY <= 0.0f) player.posY = 0.1f;
		// }

		olc::net::message<GameMsg> msg;
		msg.header.id = GameMsg::Game_UpdatePlayer;
		msg << me;
		Send(msg);
	}

	void drawCell(olc::vi2d cellPos, olc::Pixel color) {
		FillRect(cellPos * scale, { scale, scale }, color);
	}

	void drawPiece(int x, int y, Cell cell) {
		// assert(cell != Empty);
		int piece = cellType(cell) - 1;
		int color = cellColor(cell) == White ? 0 : 1;
		DrawPartialDecal(
			{ float(x), float(y) },
			{ float(scale), float(scale) },
			atlasDecal,
			{ float(piece * cellSize.x), float(color * cellSize.y) },
			cellSize
		);
	}

	void drawBoard() {
		for (int i = 0; i < 8; ++i)
			for (int j = 0; j < 8; ++j) {
				if (((i + j) & 1) == 0)
					drawCell({i, j}, lightColor);
				
				olc::vi2d cellPos = { i, j };
				if (promotionPrompt) {
					if (cellPos == promotionCellPos) {
						drawPiece(i * scale, j * scale, Cell{uint8_t(game.getPlayerToMove() | promotionPiece)});
						continue;
					}
					if (cellPos == selectedCellPos)
						continue;
				}
				if (cellPos == selectedCellPos &&
					selectedCell != Empty)
					continue;
				
				Cell piece = game.getCell(cellPos);
				if (piece == Empty)
					continue;
				drawPiece(i * scale, j * scale, piece);
			}
		
		if (selectedCell != Empty)
			drawPiece(GetMouseX() - scale / 2, GetMouseY() - scale / 2, selectedCell);
	}

	void drawSidebar() {
		{
			bool isWhiteTurn = game.getPlayerToMove() == White;
			olc::Pixel turnPromptColor = isWhiteTurn ? olc::WHITE : olc::BLACK;
			// std::string turnPromptText = isWhiteTurn ? "White Turn" : "Black Turn";
			std::string turnPromptText = isWhiteTurn == isPlayerWhite ? "Your Turn" : "Waiting For\nOpponent...";
			auto strWidth = [] (const std::string& str) {
				int best = 0,
					curr = 0;
				for (char c : str) {
					if (c == '\n') {
						curr = 0;
						continue;
					}
					++curr;
					if (best < curr)
						best = curr;
				}
				return best;
			};
			int turnPromptScale = 2;
			DrawString(
				{8 * scale + (ScreenWidth() - 8 * scale - int(strWidth(turnPromptText) * 8 * turnPromptScale)) / 2, 0},
				turnPromptText, turnPromptColor, turnPromptScale);
		}
		
		if (promotionPrompt) {
			promotionPiece = Pawn;
			for (uint8_t pieceType = Queen; pieceType <= Rook; ++pieceType) {
				Cell piece = Cell{uint8_t(game.getPlayerToMove() | pieceType)};
				int sx = 8 * scale + (ScreenWidth() - 8 * scale - scale) / 2,
					sy = (pieceType - King) * scale,
					mx = GetMouseX(),
					my = GetMouseY();
				FillRect({sx, sy}, {scale, scale}, lightColor);
				if (sx < mx && mx < sx + scale && sy < my && my < sy + scale) {
					SetPixelMode(olc::Pixel::ALPHA);
					FillRect({sx, sy}, {scale, scale}, validColor);
					SetPixelMode(olc::Pixel::NORMAL);
					promotionPiece = piece;
					if (GetMouse(0).bReleased) {
						promotionPrompt = false;
						game.setPromotionPiece(Cell{pieceType});
						game.performMove(selectedCellPos, promotionCellPos);
						// break;
					}
				}

				drawPiece(sx, sy, piece);
			}
		}
	}

	void redraw() {
		Clear(darkColor);

		drawBoard();
		drawSidebar();
		
		// TODO: move some of this stuff to another function
		olc::vi2d mousePos = GetMousePos();
		olc::vi2d currentCellPos{ mousePos.x / scale, mousePos.y / scale };
		if (!promotionPrompt && mousePos.x < ScreenHeight() && mousePos.y < ScreenHeight()) {
			drawCell(currentCellPos, hoverColor);
			
			if (GetMouse(0).bPressed) {
				Cell target = game.getCell(currentCellPos);
				if (target != Empty) {
					selectedCellPos = currentCellPos;
					selectedCell = target;
				}
			}
			uint64_t validMoves = game.getLegalMoves((selectedCell == Empty) ? currentCellPos : selectedCellPos);
			SetPixelMode(olc::Pixel::ALPHA);
			for (int i = 0; i < 8; ++i)
				for (int j = 0; j < 8; ++j)
					if ((validMoves >> (i * 8 + j)) & 1)
						drawCell({i, j}, validColor);
			SetPixelMode(olc::Pixel::NORMAL);
			if (selectedCell != Empty) {
				if (GetMouse(0).bReleased && !promotionPrompt) {
					bool isLegal = (validMoves >> (currentCellPos.x * 8 + currentCellPos.y)) & 1;
					if (isLegal) {
						if (cellType(selectedCell) == Pawn &&
							currentCellPos.y == (cellColor(selectedCell) == White ? 0 : 7)) {
							promotionCellPos = currentCellPos;
							promotionPrompt = true;
						}
						else
							game.performMove(selectedCellPos, currentCellPos);
					}
					selectedCell = Empty;
				}
			}
		}
		

		// for (const auto& [id, player]: players) {
		// 	olc::vf2d pos = { player.posX, player.posY }; pos *= worldScale;
		// 	olc::vf2d vel = { player.velX, player.velY }; vel *= worldScale;
		// 	// TODO: make this better
		// 	int row;
		// 	if (vel.x == 0 && vel.y == 0) row = 4; // not moving
		// 	else if	(vel.x >  0 && vel.y == 0) row = 0; // r
		// 	else if (vel.x == 0 && vel.y <  0) row = 1; // u
		// 	else if (vel.x >  0 && vel.y <  0) row = 2; // ur
		// 	else if (vel.x <  0 && vel.y <  0) row = 3; // ul
		// 	else if (vel.x == 0 && vel.y >  0) row = 4; // d
		// 	else if (vel.x >  0 && vel.y >  0) row = 5; // dr
		// 	else if (vel.x <  0 && vel.y >  0) row = 6; // dl
		// 	else if (vel.x <  0 && vel.y == 0) row = 7; // l
		// 	olc::vi2d srcPos = { int(player.walkCycle), row }; srcPos *= cellSize;
		// 	// DrawCircle(pos, 5);
		// 	// DrawRect(pos, { playerRadius * 2, playerRadius * 2});
		// 	DrawPartialDecal(pos, { playerRadius * 2, playerRadius * 2 }, atlasDecal, srcPos, cellSize, { player.rCol, player.gCol, player.bCol, });
		// 	DrawStringPropDecal(pos + olc::vf2d(playerRadius - GetTextSizeProp(player.name).x * 0.5f, 0), player.name);
		// }
	}

public:
	bool OnUserCreate() override {
		// if (connectionFailed)
		// 	return false;
		// std::cout << "Waiting To Connect...\n";

		// atlasSprite = new olc::Sprite("./assets/isometric-spritesheet.png");
		// atlasDecal = new olc::Decal(atlasSprite);
		// sheetSize = { 8, 8 };
		// cellSize = { atlasSprite->width / sheetSize.x, atlasSprite->height / sheetSize.y};

		atlasSprite = new olc::Sprite("./assets/pieces.png");
		atlasDecal = new olc::Decal(atlasSprite);
		sheetSize = { 6, 2 };
		cellSize = { atlasSprite->width / sheetSize.x, atlasSprite->height / sheetSize.y };
		scale = ScreenHeight() / 8;

		return true;
	}
	
	bool OnUserUpdate(float fElapsedTime) override {
		getUpdate();
		// if (waitingToConnect)
		// 	return true;
		doUpdate(fElapsedTime);
		redraw();
		
		return true;
	}
};

int main() {
	std::string SERVER_ADDR;
	uint16_t SERVER_PORT;


#ifdef DEBUG
	// I don't want to type this every time
	// same in server
	SERVER_ADDR = "127.0.0.1";
	SERVER_PORT = 42000;
#else
	std::cout << "Enter <address:port>: ";
	std::cin >> SERVER_ADDR;
	// assume input format is correct
	
	std::size_t location = SERVER_ADDR.find(":");
	SERVER_PORT = (location == std::string::npos) ? 42000 :
		atoi(SERVER_ADDR.substr(location + 1).c_str());
	SERVER_ADDR = SERVER_ADDR.substr(0, location);


	std::cout << SERVER_ADDR << ":" << SERVER_PORT << "\n";

#endif

	AmonguzClient client(SERVER_ADDR, SERVER_PORT);
	
	if (client.Construct(660, 480, 1, 1))
		client.Start();
}