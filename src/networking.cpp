#include "networking.hpp"
#include "log.hpp"
#include <string>

Networking::Networking() {
    if (!(socket = SDLNet_UDP_Open(0))) {
        Log::log(("error with opening udp socket: " + std::string(SDL_GetError())), 0);
        return;
    }
    Log::log("opened udp socket", 1);

    if (!(gaspacket = SDLNet_AllocPacket(512))) {
        Log::log(("error with allocating gas packet: " + std::string(SDL_GetError())), 0);
    }
    if (!(steerpacket = SDLNet_AllocPacket(512))) {
        Log::log(("error with allocating steer packet: " + std::string(SDL_GetError())), 0);
    }
    if (!(packetin = SDLNet_AllocPacket(512))) {
        Log::log(("error with allocating in packet: " + std::string(SDL_GetError())), 0);
    }
    Log::log("allocated packets", 1);
}
Networking::~Networking() {
    SDLNet_FreePacket(gaspacket);
    SDLNet_FreePacket(steerpacket);
    SDLNet_FreePacket(packetin);
    SDLNet_UDP_Close(socket);
}
bool Networking::connect(const char* address) {
    doc.LoadFile("./config.xml");
    int gastemp, steertemp;
    doc.FirstChildElement("gasport")->QueryIntText(&gastemp);
    doc.FirstChildElement("steerport")->QueryIntText(&steertemp);

    gasport = (Uint16)gastemp;
    steerport = (Uint16)steertemp;

    if(SDLNet_ResolveHost(&gasip, address, gasport)) {
        Log::log(("error with resolving gas host: " + std::string(SDL_GetError())), 0);
        return false;
    }
    Log::log(("gas address: " + std::to_string(gasip.host) + " port: " + std::to_string(gasip.port)), 2);
    gaspacket->address.host = gasip.host; 
    gaspacket->address.port = gasip.port;
    ping(1);

    if(SDLNet_ResolveHost(&steerip, address, steerport)) {
        Log::log(("error with resolving steer host: " + std::string(SDL_GetError())), 0);
        return false;
    }
    Log::log(("steer address: " + std::to_string(steerip.host) + " port: " + std::to_string(steerip.port)), 2);
    steerpacket->address.host = steerip.host; 
    steerpacket->address.port = steerip.port;
    ping(2);

    return true;
}
void Networking::sendgas(Sint16 gas) {
    int gasvalue = ((int)gas / 32767.0F) * 255;
    std::string gasstring = std::to_string(gasvalue);

    gaspacket->data = (unsigned char*)(gasstring.c_str());
    gaspacket->len = strlen((char *)gaspacket->data) + 1;

    SDLNet_UDP_Send(socket, -1, gaspacket);
}
void Networking::sendsteer(Sint16 steervalue) {
    std::string steerstring = std::to_string((int)steervalue);

    steerpacket->data = (unsigned char*)(steerstring.c_str());
    steerpacket->len = strlen((char *)steerpacket->data) + 1;

    SDLNet_UDP_Send(socket, -1, steerpacket);
}
void Networking::ping(int type) {
    const char* pingmsg = "PING";
    Uint64 starttime;
    if(type == 1) {
        Log::log("pinging throttle port", 1);
        gaspacket->data = (unsigned char*)pingmsg;
        gaspacket->len = strlen((char *)gaspacket->data) + 1;

        SDLNet_UDP_Send(socket, -1, gaspacket);
        starttime = SDL_GetTicks64();

    }
    else if(type == 2) {
        Log::log("pinging steering port", 1);
        steerpacket->data = (unsigned char*)pingmsg;
        steerpacket->len = strlen((char *)steerpacket->data) + 1;

        SDLNet_UDP_Send(socket, -1, steerpacket);
        starttime = SDL_GetTicks64();
    }
    else {
        Log::log("port type is incorrect", 2);
        return;
    }

    
    bool gotresponse = false;
    bool waiting = true;
    while(waiting) {
        if(SDLNet_UDP_Recv(socket, packetin)) {
            if(type == 1) {
                gaspingtime = SDL_GetTicks64() - starttime;
                gasconnected = true;
            }
            else {
                steerpingtime = SDL_GetTicks64() - starttime;
                steerconnected = true;
            }
            Log::log("arduino responded to ping", 1);
            Log::log("message: " + std::string((const char*)packetin->data), 2);
            gotresponse = true;
            waiting = false;
        }
        if(starttime + 1000 <= SDL_GetTicks64()) {
            waiting = false;
        }
    }
    if(!gotresponse) {
        Log::log("ping timed out", 0);
        if(type == 1) {
            gaspingtime = -1;
            gasconnected = false;
        }
        else {
            steerpingtime = -1;
            steerconnected = false;
        }
    }
}
void Networking::setled() {
    //will maybe add this
}