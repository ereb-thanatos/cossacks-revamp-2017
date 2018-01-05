#include "UdpHolePuncher.h"
#include "../CommCore.h"

extern CCommCore IPCORE;

//Assigns host address and fills udp packet contents
void UdpHolePuncher::Init(const char* server_addr, const unsigned short port,
                          const unsigned interval, const long player_id, const char* access_key)
{
	ready_for_punching_ = false;

	if (0 == port || 0 == interval || 0 == player_id || nullptr == server_addr)
	{
		return;
	}

	interval_ = interval;

	int res = InetPton(AF_INET, server_addr, &server_addr_.sin_addr);
	if (1 != res)
	{
		//Invaild ip address string
		return;
	}
	else
	{
		server_addr_.sin_family = AF_INET;
		server_addr_.sin_port = htons(port);
	}

	/*
	UDP packet layout:
	0x00	"CSHP"
	0x04	char version
	0x05	long player_id
	0x09	char access_key[16]
	*/

	packet_size_ = 0x19;
	const char packet_tag[4] = {'C', 'S', 'H', 'P'};
	const char packet_version = 0x01;
	unsigned long player_id_nbo = htonl(player_id);

	packet_ = std::vector<char>(packet_size_, 0);
	std::memcpy(&packet_[0x00], packet_tag, 0x04);
	std::memcpy(&packet_[0x04], &packet_version, 0x01);
	std::memcpy(&packet_[0x05], &player_id_nbo, 0x04);
	std::strcpy(&packet_[0x09], access_key);

	ready_for_punching_ = true;
}

//Checks readiness and delta time
//Sends udp packet via CommCore
void UdpHolePuncher::KeepAlive()
{
	if (!ready_for_punching_)
	{
		return;
	}

	unsigned long current_time = GetTickCount();
	if (interval_ < current_time - last_send_time_)
	{
		IPCORE.SendUdpHolePunch((sockaddr *)&server_addr_, packet_.data(), packet_size_);
		last_send_time_ = current_time;
	}
}
