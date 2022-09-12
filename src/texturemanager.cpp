#include "texturemanager.hpp"
#include "log.hpp"

SDL_Renderer* TextureManager::renderer;

void TextureManager::passrenderer(SDL_Renderer* render) {
	renderer = render;
}
SDL_Texture* TextureManager::loadTexture(const char* texture) {
	
	SDL_Surface* tempSurface = IMG_Load(texture);
	
	if(tempSurface == NULL) {
		Log::log(("tempsurface is null: " + std::string(SDL_GetError())), 0);
	}
	SDL_Texture* tex = SDL_CreateTextureFromSurface(renderer, tempSurface);
	
	if(tex == NULL) {
		Log::log(("tex is null: " + std::string(SDL_GetError())), 0);
	}
	SDL_FreeSurface(tempSurface);
	return tex;
}
void TextureManager::Draw(SDL_Texture* tex, SDL_Rect src, SDL_Rect dest) {
	SDL_RenderCopy(renderer, tex, &src, &dest);
}
void TextureManager::drawFont(const char* text, TTF_Font* font, SDL_Color color, int x, int y, int w) {
	
	SDL_Surface* surfaceMessage = TTF_RenderText_Blended_Wrapped(font, text, color, w);
	
	if (surfaceMessage == NULL) {
		Log::log(("failed to load message surface: " + std::string(SDL_GetError())), 0);
	}
	else {
		SDL_Texture* message = SDL_CreateTextureFromSurface(renderer, surfaceMessage);
		if (message == NULL) {
			Log::log(("failed to load message sprite: " + std::string(SDL_GetError())), 0);
		}
		else {
			src.x = 0;
			src.y = 0;
			src.w = surfaceMessage->w;
			src.h = surfaceMessage->h;

			dst.x = x;
			dst.y = y;
			dst.w = src.w;
			dst.h = src.h;

			SDL_RenderCopy(renderer, message, &src, &dst);
			SDL_FreeSurface(surfaceMessage);
			SDL_DestroyTexture(message);
		}
	}
	
}
void TextureManager::drawFont_NW(const char* text, TTF_Font* font, SDL_Color color, int x, int y) {
	
	SDL_Surface* surfaceMessage = TTF_RenderText_Blended(font, text, color);
	
	if (surfaceMessage == NULL) {
		Log::log(("failed to load message surface: " + std::string(SDL_GetError())), 0);
	}
	else {
		SDL_Texture* message = SDL_CreateTextureFromSurface(renderer, surfaceMessage);
		if (message == NULL) {
			Log::log(("failed to load message sprite: " + std::string(SDL_GetError())), 0);
		}
		else {
			src.x = 0;
			src.y = 0;
			src.w = surfaceMessage->w;
			src.h = surfaceMessage->h;

			dst.x = x;
			dst.y = y;
			dst.w = src.w;
			dst.h = src.h;

			SDL_RenderCopy(renderer, message, &src, &dst);
			SDL_FreeSurface(surfaceMessage);
			SDL_DestroyTexture(message);
		}
	}
	
}
void TextureManager::drawFontCenter(const char* text, TTF_Font* font, SDL_Color color, int x, int y, int w, int h) {
	
	SDL_Surface* surfaceMessage = TTF_RenderText_Blended_Wrapped(font, text, color, w);
	
	if (surfaceMessage == NULL) {
		Log::log(("failed to load message surface: " + std::string(SDL_GetError())), 0);
	}
	else {
		SDL_Texture* message = SDL_CreateTextureFromSurface(renderer, surfaceMessage);
		if (message == NULL) {
			Log::log(("failed to load message sprite: " + std::string(SDL_GetError())), 0);
		}
		else {
			src.x = 0;
			src.y = 0;
			src.w = surfaceMessage->w;
			src.h = surfaceMessage->h;

			dst.x = x + ((w - surfaceMessage->w) / 2);
			dst.y = y + ((h - surfaceMessage->h) / 2);
			dst.w = src.w;
			dst.h = src.h;

			SDL_RenderCopy(renderer, message, &src, &dst);
			SDL_FreeSurface(surfaceMessage);
			SDL_DestroyTexture(message);
		}
	}
	
}
void TextureManager::drawRect(int x, int y, int w, int h, Uint8 r, Uint8 g, Uint8 b, Uint8 a) {
	Uint8 oldr, oldb, oldg, olda;

	SDL_GetRenderDrawColor(renderer, &oldr, &oldb, &oldg, &olda);
	square.x = x;
	square.y = y;
	square.w = w;
	square.h = h;
	SDL_SetRenderDrawColor(renderer, r, g, b, a);
	SDL_RenderFillRect(renderer, &square);
	SDL_SetRenderDrawColor(renderer, oldr, oldg, oldb, olda);

}