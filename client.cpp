#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include "SDL.h"
#include "MSGPacket.h"
#include <WinSock2.h>
#include <process.h>
#include <map>
#include "PlayerData.h"
#include <time.h>
#include <vector>
#include <iostream>

#pragma comment(lib, "ws2_32.lib")

using namespace std;

unsigned WINAPI RecvThread(void* args);

HANDLE RecvThreadHandle;

vector<PlayerData> PlayerList;

CRITICAL_SECTION ClientCriticalSection;

int SDL_main(int argc, char* argv[])
{
	srand((unsigned int)time(nullptr));

	InitializeCriticalSection(&ClientCriticalSection);

	PlayerData MyPlayerData;

	MyPlayerData.R = rand() % 255; // 0 ~ 255
	MyPlayerData.G = rand() % 255;
	MyPlayerData.B = rand() % 255;
	MyPlayerData.X = rand() % 300 + 300; //300 ~ 600
	MyPlayerData.Y = rand() % 200 + 200; //200 ~ 400

	EnterCriticalSection(&ClientCriticalSection);
	PlayerList.push_back(MyPlayerData);
	LeaveCriticalSection(&ClientCriticalSection);

	WSAData wsaData;
	WSAStartup(MAKEWORD(2, 2), &wsaData);

	SOCKET ServerSocket;
	ServerSocket = socket(AF_INET, SOCK_STREAM, 0);

	SOCKADDR_IN ServerAddr;
	memset(&ServerAddr, 0, sizeof(SOCKADDR));
	ServerAddr.sin_family = PF_INET;
	ServerAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
	ServerAddr.sin_port = htons(3000);

	char SendData[1024] = { 0, };

	connect(ServerSocket, (SOCKADDR*)&ServerAddr, sizeof(ServerAddr));

	RecvThreadHandle = (HANDLE)_beginthreadex(0, 0, RecvThread, (void*)&ServerSocket, 0, 0);

	//Login
	SendData[0] = (UINT8)MSGPacket::Login;
	SendData[1] = (UINT8)15;
	PlayerList[0].MakePacket(&SendData[2]);

	SDL_Log("%d\n", MyPlayerData.R);
	SDL_Log("%d\n", MyPlayerData.G);
	SDL_Log("%d\n", MyPlayerData.B);


	int sendLength = send(ServerSocket, SendData, 15 + 2, 0);

	bool bIsRunning = true;

	SDL_Init(SDL_INIT_EVERYTHING);

	SDL_Window* MyWindow = SDL_CreateWindow("Client", 100, 100, 800, 600, SDL_WINDOW_OPENGL);
	SDL_Renderer* MyRenderer = SDL_CreateRenderer(MyWindow, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC
		| SDL_RENDERER_TARGETTEXTURE);
	SDL_Event MyEvent;


	while (bIsRunning)
	{
		SDL_PollEvent(&MyEvent);
		switch (MyEvent.type)
		{
		case SDL_QUIT:
			bIsRunning = false;
			break;
		case SDL_KEYDOWN:
			switch (MyEvent.key.keysym.sym)
			{
			case SDLK_q:
			case SDLK_ESCAPE:
				bIsRunning = false;
				break;
			case SDLK_LEFT:
				PlayerList[0].X--;
				break;
			case SDLK_RIGHT:
				PlayerList[0].X++;
				break;
			case SDLK_UP:
				PlayerList[0].Y--;
				break;
			case SDLK_DOWN:
				PlayerList[0].Y++;
				break;
			}

			char SendData[20] = { 0, };
			SendData[0] = (UINT8)MSGPacket::MovePlayer;
			SendData[1] = 15;
			PlayerList[0].MakePacket(&SendData[2]);
			send(ServerSocket, SendData, 15 + 2, 0);
			break;

		}

		SDL_SetRenderDrawColor(MyRenderer, 0xff, 0xff, 0xff, 0xff);
		SDL_RenderClear(MyRenderer);

		for (int i = 0; i < (int)PlayerList.size(); ++i)
		{
			SDL_Rect MyRect = { (int)PlayerList[i].X, (int)PlayerList[i].Y, 30, 30 };

			SDL_SetRenderDrawColor(MyRenderer, PlayerList[i].R, PlayerList[i].G, PlayerList[i].B, 0xff);
			SDL_RenderFillRect(MyRenderer, &MyRect);
		}


		SDL_RenderPresent(MyRenderer);
	}

	SDL_DestroyRenderer(MyRenderer);
	SDL_DestroyWindow(MyWindow);
	SDL_Quit();

	closesocket(ServerSocket);

	DeleteCriticalSection(&ClientCriticalSection);

	WSACleanup();

	return 0;
}

unsigned __stdcall RecvThread(void* args)
{
	SOCKET ServerSocket = *(SOCKET*)(args);

	char Header[2] = { 0, };

	while (1)
	{
		int recvLength = recv(ServerSocket, Header, 2, 0);
		if (recvLength <= 0)
		{
			break;
		}
		else
		{
			if ((UINT8)Header[0] == (UINT8)MSGPacket::LoginAck)
			{
				char Data[15] = { 0 };
				int recvLength = recv(ServerSocket, Data, Header[1], 0);

				PlayerList[0].MakeData(Data);
			}
			else if ((UINT8)Header[0] == (UINT8)MSGPacket::MakePlayer)
			{
				char Data[15] = { 0 };
				int recvLength = recv(ServerSocket, Data, Header[1], 0);

				PlayerData NewPlayerData;
				NewPlayerData.MakeData(Data);

				bool NewPlayerCheck = true;
				for (int i = 0; i < (int)PlayerList.size(); ++i)
				{
					if (PlayerList[i].ClientSocket == NewPlayerData.ClientSocket)
					{
						NewPlayerCheck = false;
						break;
					}
				}

				if (NewPlayerCheck)
				{
					PlayerList.push_back(NewPlayerData);
				}
				SDL_Log("MakePlayer\n");
			}
			else if ((UINT8)Header[0] == (UINT8)MSGPacket::DeletePlayer)
			{
				char Data[15] = { 0 };
				int recvLength = recv(ServerSocket, Data, Header[1], 0);

				PlayerData DeletePlayerData;
				DeletePlayerData.MakeData(Data);

				for (auto iter = PlayerList.begin(); iter != PlayerList.end(); ++iter)
				{
					if ((*iter).ClientSocket == DeletePlayerData.ClientSocket)
					{
						EnterCriticalSection(&ClientCriticalSection);
						iter = PlayerList.erase(iter);
						LeaveCriticalSection(&ClientCriticalSection);
						break;
					}
				}
			}
			else if ((UINT8)Header[0] == (UINT8)MSGPacket::MovePlayer)
			{
				char Data[15] = { 0 };
				int recvLength = recv(ServerSocket, Data, Header[1], 0);

				PlayerData MovePlayerData;
				MovePlayerData.MakeData(Data);
				for (auto iter = PlayerList.begin(); iter != PlayerList.end(); ++iter)
				{
					if ((*iter).ClientSocket == MovePlayerData.ClientSocket)
					{
						EnterCriticalSection(&ClientCriticalSection);
						SDL_Log("MovePlayer\n");
						(*iter) = MovePlayerData;
						LeaveCriticalSection(&ClientCriticalSection);
						break;
					}
				}
			}
		}
	}



	return 0;
}
