#pragma once
#include <Windows.h>

class PlayerData
{
public:
	UINT8 R;
	UINT8 G;
	UINT8 B;
	UINT32 X;
	UINT32 Y;
	SOCKET ClientSocket;

	//proto buffer, flat buffer
	//0	  1 	2	3	4	5	6	7	8	9	10	11	        12	13	14
	//R   G	    B	X				Y				ClientNumber
	void MakeData(char* Data) //Deserialze(역직렬화), json -> Data
	{
		R = (UINT8)Data[0];
		G = (UINT8)Data[1];
		B = (UINT8)Data[2];
		X = (UINT32)Data[3];
		Y = (UINT32)Data[7];
		ClientSocket = (UINT32)Data[11];
	}

	void MakePacket(char* Packet) //Serialize(직렬화), Data -> json
	{
		Packet[0] = (UINT8)R;
		Packet[1] = (UINT8)G;
		Packet[2] = (UINT8)B;
		memcpy(&Packet[3], &(X), 4);
		memcpy(&Packet[7], &(Y), 4);
		memcpy(&Packet[11], &(ClientSocket), 4);
	}
};
