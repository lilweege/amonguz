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
	uint32_t nUniqueID = 0;

	float fRadius = 0.5f;
	float fPosX = 0, fPosY = 0;
	float fVelX = 0, fVelY = 0;
};