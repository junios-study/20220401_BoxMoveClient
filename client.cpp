#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include "SDL.h"
#include "MSGPacket.h"
#include <WinSock2.h>

#pragma comment(lib, "ws2_32.lib")

int SDL_main(int argc, char* argv[])
{
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

	//code length  r   g   b
	//[1]   [1]   [1] [1] [1]
	//Login
	SendData[0] = (UINT8)MSGPacket::Login;
	SendData[1] = (UINT8)3;
	SendData[2] = (UINT8)0xff;
	SendData[3] = (UINT8)0x00;
	SendData[4] = (UINT8)0x00;
	int sendLength = send(ServerSocket, SendData, 5, 0);


	bool bIsRunning = true;
	int PlayerX = 100;
	int PlayerY = 100;

	SDL_Init(SDL_INIT_EVERYTHING);

	SDL_Window* MyWindow = SDL_CreateWindow("Client", 100, 100, 800, 600, SDL_WINDOW_OPENGL);
	SDL_Renderer* MyRenderer = SDL_CreateRenderer(MyWindow, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC
		| SDL_RENDERER_TARGETTEXTURE);
	SDL_Event MyEvent;

	SDL_Rect MyRect = { PlayerX, PlayerY, 30, 30 };

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
					PlayerX--;
					break;
				case SDLK_RIGHT:
					PlayerX++;
					break;
				case SDLK_UP:
					PlayerY--;
					break;
				case SDLK_DOWN:
					PlayerY++;
					break;
			}
		}

		SDL_SetRenderDrawColor(MyRenderer, 0xff, 0xff, 0xff, 0xff);
		SDL_RenderClear(MyRenderer);

		MyRect.x = PlayerX;
		MyRect.y = PlayerY;

		SDL_SetRenderDrawColor(MyRenderer, 0xff, 0x00, 0x00, 0xff);
		SDL_RenderFillRect(MyRenderer, &MyRect);

		SDL_RenderPresent(MyRenderer);
	}

	SDL_DestroyRenderer(MyRenderer);
	SDL_DestroyWindow(MyWindow);
	SDL_Quit();

	closesocket(ServerSocket);

	WSACleanup();

	return 0;
}