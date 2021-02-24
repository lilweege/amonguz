#pragma once
#include <cstdint>

#define OLC_PGEX_NETWORK
#include "olcPGEX_Network.h"


enum class GameMsg : uint32_t {
	Server_GetStatus,
	Server_GetPing,

	Client_Accepted,
	Client_AssignID,
	Client_RegisterWithServer,
	Client_UnregisterWithServer,

	Game_AddPlayer,
	Game_RemovePlayer,
	Game_UpdatePlayer,
};

struct Player {
	uint32_t uid = 0;
	char name[10];

	uint8_t rCol = 255, gCol = 255, bCol = 255;
	float posX = 0, posY = 0;
	float velX = 0, velY = 0;
	float walkCycle = 0;
};