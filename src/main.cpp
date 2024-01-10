#include <SDL.h>
#include <SDL_ttf.h>
#include <SDL_image.h>
#include <SDL_net.h>
#include <iostream>
#include <vector>
#include <array>
#include "tinyxml2.hpp"
#include "log.hpp"
#include "networking.hpp"
#include "texturemanager.hpp"

Networking* net;
TextureManager tex;
tinyxml2::XMLDocument doc;


SDL_Window* window;
SDL_Renderer* renderer;
SDL_Event event;

bool isrunning = true;
int loglevel;
bool filelogging;
int screenWidth, screenHeight;
float gamescreenWidth = 1300;
float gamescreenHeight = 800;
int oldScreenWidth, oldScreenHeight;

int connectedjoysticks;
SDL_GameController* controller;
bool hasrumble;
bool hastriggerrumble;
int deadzone;

int fpslimit;
bool limitfps;

Sint16 throttleaxis;
Sint16 brakeaxis;
Sint16 xaxis;
Sint16 enginedelta;
Sint16 correctedgas;

Sint16 oldengine;
Sint16 oldsteer;

bool sentzerogas = false;
bool sentzero = false;
int senttimesgas = 0;
int senttimes = 0;

Uint64 totalgasspeed;
float gasspeed;
bool dogasspeed;
Uint64 totalsteerspeed = 250;
float steerspeed;
bool dosteerspeed;

Uint64 oldframestart;
Uint64 framestart;
Uint64 frametime;
float framedelay;

bool usingkeyboard = true;

bool pingpressed;

int frame = 1;



//steering triangles
std::vector<std::array<int, 2>> leftborder;
std::vector<SDL_Vertex> lefttri = {
        { SDL_FPoint{ 600, 400 }, SDL_Color{ 0, 0, 255, 255 }, SDL_FPoint{ 0 }, }, //top
        { SDL_FPoint{ 600, 550 }, SDL_Color{ 0, 0, 255, 255 }, SDL_FPoint{ 0 }, }, //left
        { SDL_FPoint{ 600, 700 }, SDL_Color{ 0, 0, 255, 255 }, SDL_FPoint{ 0 }, }, //bottom
};
std::vector<int> rightborder = {};
std::vector<SDL_Vertex> righttri = {
        { SDL_FPoint{ 701, 400 }, SDL_Color{ 0, 0, 255, 255 }, SDL_FPoint{ 0 }, }, //top
        { SDL_FPoint{ 701, 550 }, SDL_Color{ 0, 0, 255, 255 }, SDL_FPoint{ 0 }, }, //right
        { SDL_FPoint{ 701, 700 }, SDL_Color{ 0, 0, 255, 255 }, SDL_FPoint{ 0 }, }, //bottom
};
SDL_Rect throttlerect;

SDL_Texture* inputmap;
SDL_Rect src, dst;

TTF_Font* font;
TTF_Font* bigfont;

SDL_Color white = {255, 255, 255, 255};
SDL_Color red = {255, 0, 0, 255};


void handleEvents();
void update();
void render();
void controllercheck();

int WinMain(int argc, char* argv[]) {
    std::cout << "starting game\n";
    doc.LoadFile("./config.xml");
    doc.FirstChildElement("loglevel")->QueryIntText(&loglevel);
    doc.FirstChildElement("filelogging")->QueryBoolText(&filelogging);

    Log::setlogmode(filelogging, loglevel, "./");
    Log::log("initializing game", 1);
    if(SDL_Init(SDL_INIT_EVENTS | SDL_INIT_GAMECONTROLLER) == 0) {
        Log::log(("initialized SDL " + std::to_string(SDL_MAJOR_VERSION) + "." + std::to_string(SDL_MINOR_VERSION) + "." + std::to_string(SDL_PATCHLEVEL)), 1);
        //image library doesnt need to be initailized but like this it looks better in the console
		Log::log(("initialized SDL_Image " + std::to_string(SDL_IMAGE_MAJOR_VERSION) + "." + std::to_string(SDL_IMAGE_MINOR_VERSION) + "." + std::to_string(SDL_IMAGE_PATCHLEVEL)), 1);
        
        if(TTF_Init() == -1) {
            Log::log(("error initializing ttf library: " + std::string(SDL_GetError())), 0);
        }
        else { 
            Log::log(("initialized SDL_TTF " + std::to_string(SDL_TTF_MAJOR_VERSION) + "." + std::to_string(SDL_TTF_MINOR_VERSION) + "." + std::to_string(SDL_TTF_PATCHLEVEL)), 1);
        }
        if(SDLNet_Init()==-1) {
            Log::log(("error initializing networking library: " + std::string(SDL_GetError())), 0);
        }
        else {
            
            Log::log(("initialized SDL_Net " + std::to_string(SDL_NET_MAJOR_VERSION) + "." + std::to_string(SDL_NET_MINOR_VERSION) + "." + std::to_string(SDL_NET_PATCHLEVEL)), 1);
        }
        //SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1");
        //window maken
        Uint32 flags = SDL_WINDOW_MAXIMIZED | SDL_WINDOW_RESIZABLE;
        window = SDL_CreateWindow("rc car controller", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 1300, 800, flags);

        if (window) {
            //std::cout << "window created." << std::endl;
            Log::log("window created.", 1);
        }
        else {
            //fout bij het maken van de window
            Log::log(("error creating window: " + std::string(SDL_GetError())), 0);
            isrunning = false;
        }

        //renderer creeren om op het scherm dingen te renderen
        renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
        if (renderer) {

            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
            Log::log("renderer created.", 1);

            //krijg de schermgrootte
            SDL_GetRendererOutputSize(renderer, &screenWidth, &screenHeight);
            Log::log("renderer size:", 2);
            Log::log(("width: "+ std::to_string(screenWidth)), 2);
            Log::log(("height: " + std::to_string(screenHeight)), 2);
            //be able to draw transparent rectangles
            SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
        }
        else {
            //fout bij maken renderer
            Log::log(("error creating renderer: " + std::string(SDL_GetError())), 0);
            isrunning = false;
        }
        isrunning = true;
    
    }
    else {
        Log::log("sdl init failed: " + std::string(SDL_GetError()) + "\n", 0);
        std::cout << "sdl init failed: " << SDL_GetError() << "\n";
        isrunning = false;
    }
    SDL_RenderSetScale(renderer, screenWidth / gamescreenWidth, screenHeight / gamescreenHeight);
    Log::log("base initialized", 1);

    Log::log("initializing controllers.", 2);

	//mapping file last updated: 03-09-2022
	
	SDL_GameControllerAddMappingsFromFile("./mappings.txt");
	
	connectedjoysticks = SDL_NumJoysticks();

	
	if(connectedjoysticks > 0) {
		if(SDL_IsGameController(0)) {
			controller = SDL_GameControllerOpen(0);

			if(controller) {
				hasrumble = SDL_GameControllerHasRumble(controller);
				hastriggerrumble = SDL_GameControllerHasRumbleTriggers(controller);
				Log::log(("opened controller: " + std::string(SDL_GameControllerName(controller))), 1);
				if(hasrumble) {
					Log::log("controller has rumble", 2);
				}
				if(hastriggerrumble) {
					Log::log("controller has rumble triggers", 2);
				}
				Log::log("type: " + std::to_string((int)SDL_GameControllerGetType(controller)), 2);
			}
			else {
				Log::log("couldn't open controller", 0);
			}
		}
		else {
			//controller has no mapping so can only be used qith the SDL_Joystick module
			//that's to much work and most controllers are probably supported
			Log::log("game controller is not supported", 0);
		}
		
	}
	Log::log("controllers initialized.", 2);

    doc.FirstChildElement("limitfps")->QueryBoolText(&limitfps);
    doc.FirstChildElement("fpslimit")->QueryIntText(&fpslimit);
    doc.FirstChildElement("deadzone")->QueryIntText(&deadzone);
    framedelay = 1000.0f / fpslimit;

    int tempgs;
    doc.FirstChildElement("limitgas")->QueryBoolText(&dogasspeed);
    doc.FirstChildElement("gasspeed")->QueryIntText(&tempgs);
    totalgasspeed = (Uint64)tempgs;

    int tempss;
    doc.FirstChildElement("dosteerspeed")->QueryBoolText(&dosteerspeed);
    doc.FirstChildElement("steerspeed")->QueryIntText(&tempss);
    totalsteerspeed = (Uint64)tempss;
    if(totalgasspeed <= framedelay || !dogasspeed) {
        if(!dogasspeed) {
            Log::log("throttle speed limit deactivated", 1);
        }
        else {
            Log::log("throttle speed limit deactivated because time was smaller then a frame", 1);
            std::cout << "frame: " << framedelay << " speed: " << totalgasspeed << "\n";
        }
        gasspeed = 1;
    }
    else {
        gasspeed = framedelay / totalgasspeed;
    }
    if(totalsteerspeed <= framedelay || !dosteerspeed) {
        if(!dosteerspeed) {
            Log::log("steer speed limit deactivated", 1);
        }
        else {
            Log::log("steer speed limit deactivated because time was smaller then a frame", 1);
            std::cout << "frame: " << framedelay << " speed: " << totalsteerspeed << "\n";
        }
        steerspeed = 1;
    }
    else {
        steerspeed = framedelay / totalsteerspeed;
    }
    std::cout << "steerspeed: " << steerspeed << "\n";
    std::cout << "gasspeed: " << gasspeed << "\n";


    throttlerect.w = 89;
    throttlerect.x = 606;
    throttlerect.y = 550;
    throttlerect.h = 0;

    SDL_Surface* tempSurface = IMG_Load("./inputmap.png");
	
	if(tempSurface == NULL) {
		Log::log(("tempsurface is null: " + std::string(SDL_GetError())), 0);
	}
	inputmap = SDL_CreateTextureFromSurface(renderer, tempSurface);
	
	if(inputmap == NULL) {
		Log::log(("input texture is null: " + std::string(SDL_GetError())), 0);
	}
	SDL_FreeSurface(tempSurface);

    src.x = 0;
    src.y = 0;
    src.w = 700;
    src.h = 300;

    dst.x = 300;
    dst.y = 400;
    dst.w = 700;
    dst.h = 300;

    TextureManager::passrenderer(renderer);

    font = TTF_OpenFont("font.ttf", 16);
	if(font == NULL) {
		Log::log(("font is null: " + std::string(SDL_GetError())), 0);
	}
	bigfont = TTF_OpenFont("font.ttf", 64);
	if(bigfont == NULL) {
		Log::log(("font is null: " + std::string(SDL_GetError())), 0);
	}

    net = new Networking();
    SDL_SetRenderDrawColor( renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
    tex.drawFontCenter("loading", bigfont, white, 0, 0, 1300, 800);
    SDL_RenderPresent(renderer);
    net->connect(doc.FirstChildElement("ip")->GetText());

    

    Log::log("initialized", 1);
    std::cout << "framedelay: " << framedelay << "\n";
    oldframestart = SDL_GetTicks64();
    while(isrunning) {
        //main loop
        //krijg de tijd van het begin van de loop
		framestart = SDL_GetTicks64();
        
        handleEvents();
        update();
        render();

        frame++;
        if(frame == 59) {
            frame = 0;
        }
        if(limitfps) {
            frametime = SDL_GetTicks64() - framestart;

            //als de code sneller is dan 1/60e van een seconde wacht daarop
            if (framedelay > frametime) {
                SDL_Delay(framedelay - frametime);
            }
        }
    }
    Log::log("closing game", 0);
    SDLNet_Quit();
    TTF_Quit();
    IMG_Quit();
    SDL_Quit();
    
    return 0;
}



void handleEvents() {
    oldScreenWidth = screenWidth;
	oldScreenHeight = screenHeight;
	SDL_GetRendererOutputSize(renderer, &screenWidth, &screenHeight);

	//kijk of de schermgrootte is veranderd
	if (oldScreenWidth != screenWidth || oldScreenHeight != screenHeight) {
		//screen size changed
		SDL_RenderSetScale(renderer, screenWidth / gamescreenWidth, screenHeight / gamescreenHeight);
	}

    while(SDL_PollEvent(&event)) {
        switch (event.type) {
            //als de gebruiker het heeft afgesloten dan word isrunning veranderd naar false en sluit het spel
            case SDL_QUIT:
                isrunning = false;
            break;
            default:

            break;

        }
    }
    
    controllercheck();
    
    const Uint8* keys = SDL_GetKeyboardState(NULL);
    if(keys[SDL_SCANCODE_ESCAPE]) {
        isrunning = false;
    }
}
void update() {
    const Uint8* keys = SDL_GetKeyboardState(NULL);
    oldengine = enginedelta;
    oldsteer = xaxis;
    if(keys[SDL_SCANCODE_P] && !pingpressed) {
        net->ping(1);
        net->ping(2);
        pingpressed = true;
        usingkeyboard = true;
    }
    if(connectedjoysticks > 0) {
        if(!keys[SDL_SCANCODE_LEFT] && !keys[SDL_SCANCODE_RIGHT] && !keys[SDL_SCANCODE_UP] && !keys[SDL_SCANCODE_DOWN]) {
            Sint16 tempthrottle = SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_TRIGGERRIGHT);
            Sint16 tempbrake = SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_TRIGGERLEFT);
            Sint16 tempx = SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_LEFTX);
            if(tempthrottle > 0 || tempbrake > 0 || tempx <= 0 - deadzone || tempx > deadzone) {
                usingkeyboard = false;
            }
        }
        else {
            usingkeyboard = true;
        }
        if(!usingkeyboard) {
            throttleaxis = SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_TRIGGERRIGHT);
            brakeaxis = SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_TRIGGERLEFT);
            if(throttleaxis + brakeaxis > 0 && throttleaxis + brakeaxis >= oldengine + (32767 *gasspeed)) {
                throttleaxis = oldengine + (32767 *gasspeed) - brakeaxis;
            }
            if(throttleaxis + brakeaxis < 0 && throttleaxis + brakeaxis <= oldengine - (32767 *gasspeed)) {
                brakeaxis = oldengine + (32767 *gasspeed) - throttleaxis;
            }
            xaxis = SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_LEFTX);
        }
        if(SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_START) && !pingpressed) {
            net->ping(1);
            net->ping(2);
            pingpressed = true;
            usingkeyboard = false;
        }

    }
    if(usingkeyboard) {
        //not using controller
        if(keys[SDL_SCANCODE_LEFT]) {
            if(xaxis <= -32767 + (32767 * steerspeed)) {
                xaxis = -32767;
            }
            else {
                 xaxis -= 32767 * steerspeed;
            }
        }
        else {
            if(xaxis <= 0) {
                if(xaxis < 0 - 32767 * steerspeed) {
                    xaxis += 32767 * steerspeed;
                }
                else {
                    xaxis = 0;
                }
            }
            
        }
        if(keys[SDL_SCANCODE_RIGHT]) {
            if(xaxis >= 32767 - (32767 * steerspeed)) {
                xaxis = 32767;
            }
            else {
                 xaxis += 32767 * steerspeed;
            } 
        }
        else {
            if(xaxis >= 0) {
                if(xaxis > 0 + 32767 * steerspeed) {
                    xaxis -= 32767 * steerspeed;
                }
                else {
                    xaxis = 0;
                }
            }
            
        }
        if(keys[SDL_SCANCODE_LEFT] && keys[SDL_SCANCODE_RIGHT]) {
            if(xaxis < 0 - 32767 * steerspeed) {
                xaxis += 32767 * steerspeed;
            }
            else if(xaxis > 0 + 32767 * steerspeed) {
                xaxis -= 32767 * steerspeed;
            }
            else {
                xaxis = 0;
            }
            
        }
        if(keys[SDL_SCANCODE_UP]) {
            if(throttleaxis > 32767 - 32767 * gasspeed) {
                throttleaxis = 32767;
            }
            else {
                throttleaxis += 32767 * gasspeed;
            }
            
        }
        else {
            if(throttleaxis > 0 + 32767 * gasspeed) {
                throttleaxis -= 32767 * gasspeed;
            }
            else {
                throttleaxis = 0;
            }
        }
        if(keys[SDL_SCANCODE_DOWN]) {
           if(brakeaxis > 32767 - 32767 * gasspeed) {
                brakeaxis = 32767;
            }
            else {
                brakeaxis += 32767 * gasspeed;
            }
        }
        else {
            if(brakeaxis > 0 + 32767 * gasspeed) {
                brakeaxis -= 32767 * gasspeed;
            }
            else {
                brakeaxis = 0;
            }
        }
    }
    enginedelta = throttleaxis - brakeaxis;
    if(enginedelta > 0) {
        throttlerect.h = (enginedelta / 32767.0f) * 150;
        throttlerect.y = 550 - throttlerect.h;
    }
    else {
        throttlerect.y = 550;
        throttlerect.h = ((0 - enginedelta) / 32768.0f) * 150; 
    }

    //steering
    if(xaxis > deadzone) {
        //right
        righttri[1].position = {(701 + ((xaxis / 32767.0f) * 300)), 550};
        lefttri[1].position = {600, 550};
    }
    else if(xaxis < 0 - deadzone) {
        //left
        lefttri[1].position = {(600 + ((xaxis / 32768.0f) * 300)), 550};
        righttri[1].position = {701, 550};
    }
    else {
        //straight
        lefttri[1].position = {600, 550};
        righttri[1].position = {701, 550};
    }
    
    net->sendgas(enginedelta);

    if(xaxis > deadzone || xaxis < 0 - deadzone) {
        net->sendsteer(xaxis);
        sentzero = false;
    }
    else if(!sentzero || frame == 0 || frame == 20 || frame == 40) {
        net->sendsteer(0);
        sentzero = true;
    }
    if(usingkeyboard && !keys[SDL_SCANCODE_P]) {
        pingpressed = false;
    }
    if(connectedjoysticks > 0 && pingpressed && !usingkeyboard && !SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_START)) {
        pingpressed = false;
    }
}
void render() {
    
    SDL_SetRenderDrawColor( renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    
    if(xaxis < 0) {
        SDL_RenderGeometry(renderer, nullptr, lefttri.data(), lefttri.size(), nullptr, 0);
    }
    
    if(xaxis > 0) {
        SDL_RenderGeometry(renderer, nullptr, righttri.data(), righttri.size(), nullptr, 0);
    }

    if(enginedelta != 0) {
        if(enginedelta > 0) {
            SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
        }
        else {
            SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
        }
        
        SDL_RenderFillRect(renderer, &throttlerect);
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    }



    SDL_RenderCopy(renderer, inputmap, &src, &dst);

    if(net->gasconnected) {
        tex.drawFont_NW(("gas: connected (" + std::to_string(net->gaspingtime) + ")").c_str(), font, white, 5, 5);
    }
    else {
        tex.drawFont_NW("gas: disconnected", font, red, 5, 5);
    }
    if(net->steerconnected) {
        tex.drawFont_NW(("steer: connected (" + std::to_string(net->steerpingtime) + ")").c_str(), font, white, 5, 30);
    }
    else {
        tex.drawFont_NW("steer: disconnected", font, red, 5, 30);
    }
    if(!net->gasconnected || !net->steerconnected) {
        if(connectedjoysticks > 0 && !usingkeyboard) {
            tex.drawFont_NW(("press " + std::string(SDL_GameControllerGetStringForButton(SDL_CONTROLLER_BUTTON_START)) + " to try to (re)connect").c_str(), font, white, 5, 55);
        }
        else {
            tex.drawFont_NW("press p to try to (re)connect", font, white, 5, 55);
        }
    }
    
    SDL_RenderPresent(renderer);
}
void controllercheck() {
    int oldjoysticks = connectedjoysticks;
	connectedjoysticks = SDL_NumJoysticks();

	if(connectedjoysticks == 0 && oldjoysticks > 0) {
		SDL_GameControllerClose(controller);
		hasrumble = false;
		Log::log("joystick disconnected", 1);
        usingkeyboard = true;
	}
	else if(connectedjoysticks > 0 && oldjoysticks == 0) {
		controller = SDL_GameControllerOpen(0);

		if(controller) {
			hasrumble = SDL_GameControllerHasRumble(controller);
			hastriggerrumble = SDL_GameControllerHasRumbleTriggers(controller);
			Log::log(("opened controller: " + std::string(SDL_GameControllerName(controller))), 1);
			if(hasrumble) {
				Log::log("controller has rumble", 1);
			}
			if(hastriggerrumble) {
				Log::log("controller has rumble triggers", 1);
			}
			Log::log("type: " + std::to_string((int)SDL_GameControllerGetType(controller)), 2);
		}
		else {
			Log::log(("couldn't open controller " + std::string(SDL_GetError())), 0);
		}
	}
}