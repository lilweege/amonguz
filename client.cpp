#include "common.h"

#define OLC_PGE_APPLICATION
#include "olcPixelGameEngine.h"

#include <unordered_map>


class AmonguzClient : public olc::PixelGameEngine, olc::net::client_interface<GameMsg> {
public:
	AmonguzClient(std::string addr, uint16_t port)
		: connectionFailed(!Connect(addr, port))
	{
		sAppName = "Amonguz Client";
		std::cout << "Connection initialized...\n";
	}

private:
	bool connectionFailed;
	bool waitingToConnect = true;
	
	std::unordered_map<uint32_t, Player> players;
	uint32_t selfID = 0; Player self;

	olc::Sprite* playerAtlasSprite = nullptr;
	olc::Decal* playerAtlasDecal = nullptr;

private:

	void getUpdate() {
		// Check for incoming network messages
		if (!IsConnected())
			return;
		
		auto& q = Incoming();
		while (!q.empty()) {
			auto msg = q.pop_front().msg;

			switch (msg.header.id)
			{
			case GameMsg::Client_Accepted: {
				std::cout << "Server accepted client - you're in!\n";
				olc::net::message<GameMsg> msg;
				msg.header.id = GameMsg::Client_RegisterWithServer;
				self.fPosX = 1.0f;
				self.fPosY = 1.0f;
				
				msg << self;
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
				Player desc;
				msg >> desc;
				players.insert_or_assign(desc.nUniqueID, desc);

				if (desc.nUniqueID == selfID)
					waitingToConnect = false;
				break;
			}

			case GameMsg::Game_RemovePlayer: {
				uint32_t nRemovalID = 0;
				msg >> nRemovalID;
				players.erase(nRemovalID);
				break;
			}

			case GameMsg::Game_UpdatePlayer: {
				Player desc;
				msg >> desc;
				players.insert_or_assign(desc.nUniqueID, desc);
				break;
			}
			}
		}
	}
	
	void doUpdate(float fElapsedTime) {
		// self vel
		olc::vf2d vVel;
		if (GetKey(olc::Key::W).bHeld) vVel += {  0.0f, -1.0f };
		if (GetKey(olc::Key::S).bHeld) vVel += {  0.0f, +1.0f };
		if (GetKey(olc::Key::A).bHeld) vVel += { -1.0f,  0.0f };
		if (GetKey(olc::Key::D).bHeld) vVel += { +1.0f,  0.0f };
		if (vVel.mag2() > 0)
			vVel = vVel.norm() * 4.0f;
		players[selfID].fVelX = vVel.x;
		players[selfID].fVelY = vVel.y;
		
		// all player move clientside
		for (auto& [id, player] : players) {
			player.fPosX += player.fVelX * fElapsedTime;
			player.fPosY += player.fVelY * fElapsedTime;
			if (player.fPosX <= 0.0f) player.fPosX = 0.1f;
			if (player.fPosY <= 0.0f) player.fPosY = 0.1f;
		}
		
		olc::net::message<GameMsg> msg;
		msg.header.id = GameMsg::Game_UpdatePlayer;
		msg << players[selfID];
		Send(msg);
	}
	
	void redraw() {
		Clear(olc::BLACK);
		
		for (auto& [id, player]: players) {
			float scl = 100;
			float radius = player.fRadius * scl;
			
			olc::vf2d vPos = { player.fPosX, player.fPosY }; vPos *= scl;
			olc::vf2d vVel = { player.fVelX, player.fVelY }; vVel *= scl;

			olc::vf2d size = { radius, radius }; size *= 2;
			olc::vi2d relSize = { playerAtlasSprite->width / 8, playerAtlasSprite->height / 8 };
			int row;
			if (vVel.x == 0 && vVel.y == 0) row = 4; // not moving
			else if	(vVel.x >  0 && vVel.y == 0) row = 0; // r
			else if (vVel.x == 0 && vVel.y <  0) row = 1; // u
			else if (vVel.x >  0 && vVel.y <  0) row = 2; // ur
			else if (vVel.x <  0 && vVel.y <  0) row = 3; // ul
			else if (vVel.x == 0 && vVel.y >  0) row = 4; // d
			else if (vVel.x >  0 && vVel.y >  0) row = 5; // dr
			else if (vVel.x <  0 && vVel.y >  0) row = 6; // dl
			else if (vVel.x <  0 && vVel.y == 0) row = 7; // l
			olc::vi2d srcPos = { 0, row * relSize.x };

			/* row
			0: right
			1: up
			2: up right
			3: up left
			4: down
			5: down right
			6: down left
			7: left
			*/
			DrawPartialDecal(vPos - size * 0.5, size, playerAtlasDecal, srcPos, relSize);
			//DrawCircle(vPos, radius);
			//DrawLine(vPos, vPos + vVel.norm() * radius, olc::MAGENTA);
			
			olc::vi2d vNameSize = GetTextSizeProp("ID: " + std::to_string(id));
			DrawStringPropDecal(vPos - olc::vf2d{ vNameSize.x * 0.5f, radius * 1.25f }, "ID: " + std::to_string(id));
		}
	}

public:
	bool OnUserCreate() override {
		if (connectionFailed)
			return false;
		std::cout << "Waiting To Connect...\n";

		playerAtlasSprite = new olc::Sprite("./assets/isometric-spritesheet.png");
		playerAtlasDecal = new olc::Decal(playerAtlasSprite);


		return true;
	}
	
	bool OnUserUpdate(float fElapsedTime) override {
		getUpdate();
		if (waitingToConnect)
			return true;
		doUpdate(fElapsedTime);
		redraw();
		
		return true;
	}
};

int main() {
	std::string SERVER_ADDR;
	uint16_t SERVER_PORT;
	std::cout << "Enter server IP address: ";
	std::cin >> SERVER_ADDR;
	std::cout << "Enter server port number: ";
	std::cin >> SERVER_PORT;
	
	AmonguzClient client(SERVER_ADDR, SERVER_PORT);
	
	if (client.Construct(480, 480, 1, 1))
		client.Start();
}