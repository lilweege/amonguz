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

	Game game;
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
	
	void redraw() {
		Clear(olc::BLACK);
		
		int scale = std::min(ScreenWidth(), ScreenHeight()) / 8;

		for (int i = 0; i < 8; ++i)
			for (int j = 0; j < 8; ++j) {
				if (((i + j) & 1) == 0)
					FillRect(i * scale, j * scale, scale, scale);
				Cell cell = game.getCell(i, j);
				if (cell == Empty)
					continue;
				int cellType = (cell & 0b0111) - 1;
				bool isBlack = (cell & 0b1000);
				DrawPartialDecal(
					{i * scale, j * scale},
					{scale, scale},
					atlasDecal,
					olc::vf2d{float(cellType * cellSize.x), float(isBlack * cellSize.y)},
					cellSize
				);
			}
		
		olc::vi2d mousePos{ GetMouseX(), GetMouseY() };
		olc::vi2d cellPos{ mousePos.x / scale, mousePos.y / scale };
		FillRect(cellPos * scale, {scale, scale}, olc::GREEN);

		// TODO: mouse dragging



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
	
	if (client.Construct(480, 480, 1, 1))
		client.Start();
}