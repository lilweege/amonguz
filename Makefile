CC = g++
COMMON_FLAGS = -static -std=c++17

CFLAGS_OPTIMIZED = -Ofast -Os -fdata-sections -ffunction-sections $(COMMON_FLAGS)
CFLAGS_DEBUGGING = -g $(COMMON_FLAGS)

ASIO = -Iasio-1.18.1/include
ASIO_WINDOWS = $(ASIO) -lws2_32 -lmswsock
ASIO_LINUX = $(ASIO) -pthread

# server - cross platform
ifeq ($(OS), Windows_NT)
	EXT = .exe
	SERVER_LIBS = $(ASIO_WINDOWS)
else
	EXT = .out
	SERVER_LIBS = $(ASIO_LINUX)
endif
SERVER_TARGET = server/amonguz_server$(EXT)


# windows only for client
# at least for now cause
# pge on linux is a bit tricky
# and not really worth rn imo
CLIENT_LIBS = \
	$(ASIO_WINDOWS) \
	-lgdi32 \
	-lopengl32 \
	-lgdiplus \
	-lShlwapi \
	-ldwmapi \
	-lstdc++fs
CLIENT_TARGET = client/amonguz_client.exe


COMMON_DEPEND = \
	olcPGEX_Network.h \
	common.h
CLIENT_DEPEND = $(COMMON_DEPEND) \
	olcPixelGameEngine.h \
	amonguz_client.res \
	client.cpp
SERVER_DEPEND = $(COMMON_DEPEND) \
	server.cpp
	




all: server amonguz_client.res client

amonguz_client.res: amonguz_client.rc amonguz.ico
	windres amonguz_client.rc -O coff -o amonguz_client.res

client: $(CLIENT_DEPEND)
	$(CC) $(CFLAGS_DEBUGGING) -o $(CLIENT_TARGET) client.cpp amonguz_client.res $(CLIENT_LIBS)

release: $(CLIENT_DEPEND)
	$(CC) $(CFLAGS_OPTIMIZED) -o $(CLIENT_TARGET) client.cpp amonguz_client.res $(CLIENT_LIBS)

server: $(SERVER_DEPEND)
	$(CC) $(CFLAGS_DEBUGGING) -o $(SERVER_TARGET) server.cpp $(SERVER_LIBS)
