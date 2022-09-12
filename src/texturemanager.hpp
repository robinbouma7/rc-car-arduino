#pragma once
#include <SDL_ttf.h>
#include <SDL_image.h>
#include <SDL.h>

class TextureManager {

public:
	static void passrenderer(SDL_Renderer* render);
	static SDL_Texture* loadTexture(const char* filename);
	static void Draw(SDL_Texture* tex, SDL_Rect src, SDL_Rect dest);
	void drawFont(const char* text, TTF_Font* font, SDL_Color color, int x, int y, int w);
	void drawFont_NW(const char* text, TTF_Font* font, SDL_Color color, int x, int y);
	void drawFontCenter(const char* text, TTF_Font* font, SDL_Color color, int x, int y, int w, int h);
	void drawRect(int x, int y, int w, int h, Uint8 r, Uint8 g, Uint8 b, Uint8 a);
	SDL_Rect src, dst;
	SDL_Rect square;
	static SDL_Renderer* renderer;
};