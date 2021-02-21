#include "common.h"

#include <iostream>
#include <unordered_map>


class AmonguzServer : public olc::net::server_interface<GameMsg> {
public:
	AmonguzServer(uint16_t port)
		: olc::net::server_interface<GameMsg>(port)
	{
		Start();
	}

private:
	std::unordered_map<uint32_t, Player> players;
	std::vector<uint32_t> deadIDs;

protected:
	bool OnClientConnect(std::shared_ptr<olc::net::connection<GameMsg>> client) override { return true; }

	void OnClientValidated(std::shared_ptr<olc::net::connection<GameMsg>> client) override {
		// Client passed validation check, so send them a message informing
		// them they can continue to communicate
		olc::net::message<GameMsg> msg;
		msg.header.id = GameMsg::Client_Accepted;
		client->Send(msg);
	}

	void OnClientDisconnect(std::shared_ptr<olc::net::connection<GameMsg>> client) override {
		if (!client)
			return;
			
		auto id = client->GetID();
		if (players.find(id) == players.end())
			return;
		
		auto& desc = players[id];
		std::cout << "[UNGRACEFUL REMOVAL]:" + std::to_string(desc.nUniqueID) + "\n";
		players.erase(id);
		deadIDs.push_back(id);
	}

	void OnMessage(std::shared_ptr<olc::net::connection<GameMsg>> client, olc::net::message<GameMsg>& msg) override {
		while (!deadIDs.empty()) {
			auto deadID = deadIDs.back();
			olc::net::message<GameMsg> m;
			m.header.id = GameMsg::Game_RemovePlayer;
			m << deadID;
			std::cout << "Removing " << deadID << "\n";
			MessageAllClients(m);
			deadIDs.pop_back();
		}
		
		switch (msg.header.id) {
		case GameMsg::Client_RegisterWithServer: {
			Player desc;
			msg >> desc;
			desc.nUniqueID = client->GetID();
			players.insert_or_assign(desc.nUniqueID, desc);

			olc::net::message<GameMsg> msgSendID;
			msgSendID.header.id = GameMsg::Client_AssignID;
			msgSendID << desc.nUniqueID;
			MessageClient(client, msgSendID);

			olc::net::message<GameMsg> msgAddPlayer;
			msgAddPlayer.header.id = GameMsg::Game_AddPlayer;
			msgAddPlayer << desc;
			MessageAllClients(msgAddPlayer);

			for (const auto& [id, player] : players) {
				olc::net::message<GameMsg> msgAddOtherPlayers;
				msgAddOtherPlayers.header.id = GameMsg::Game_AddPlayer;
				msgAddOtherPlayers << player;
				MessageClient(client, msgAddOtherPlayers);
			}
			break;
		}
		case GameMsg::Client_UnregisterWithServer: {
			break;
		}
		case GameMsg::Game_UpdatePlayer: {
			// Simply bounce update to everyone except incoming client
			MessageAllClients(msg, client);
			break;
		}
		}
	}
};


int main() {
	int SERVER_PORT;
	std::cout << "Enter port number to host: ";
	std::cin >> SERVER_PORT;
	
	AmonguzServer server(SERVER_PORT);
	
	while (true)
		server.Update(-1, true);
}