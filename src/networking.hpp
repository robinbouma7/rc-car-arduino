#pragma once
#include <SDL_Net.h>
#include "tinyxml2.hpp"

class Networking {
public:
    Networking();
    ~Networking();

    bool connect(const char* address);
    void sendgas(Sint16 gas);
    void sendsteer(Sint16 steer);
    void ping(int type);

    Uint16 gasport, steerport;

    IPaddress gasip, steerip;
    UDPsocket socket;
    UDPpacket *gaspacket, *steerpacket, *packetin;

    Uint64 gaspingtime = -1;
    Uint64 steerpingtime = -1;
    bool gasconnected = false;
    bool steerconnected = false;

private:
    tinyxml2::XMLDocument doc;
};