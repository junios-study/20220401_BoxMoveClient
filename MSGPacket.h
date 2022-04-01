#pragma once

enum class MSGPacket
{
	Login				= 10,
	LoginAck			= 20,
	MakePlayer			= 30,
	DeletePlayer		= 40,
	MovePlayer			= 50,
};
