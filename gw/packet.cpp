#include <list>
#include <string>

#include "packet.h"


using namespace std;

TraitsPacket::TraitsPacket()
{
	code = 1;
}

TraitsPacket::~TraitsPacket()
{

}

TraitsInitPacket::TraitsInitPacket()
{

}

TraitsInitPacket::~TraitsInitPacket()
{

}

bool TraitsInitPacket::handle()
{
	return false;
}


TraitsDataPacket::TraitsDataPacket()
{

}

TraitsDataPacket::~TraitsDataPacket()
{

}

bool TraitsDataPacket::handle()
{
	return false;
}


TraitsHBPacket::TraitsHBPacket()
{

}

TraitsHBPacket::~TraitsHBPacket()
{

}


bool TraitsHBPacket::handle()
{
	return false;
}


