#ifndef SDLHELPER_HPP
#define SDLHELPER_HPP

#include "SDL_FontCache.h"
#include "colors.hpp"
#include "logger.hpp"
#include "main.hpp"
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <string>
#include <unordered_map>

#include <coreinit/memory.h>

bool SDLH_Init(void);
void SDLH_Exit(void);

void SDLH_ClearScreen(SDL_Color color);
void SDLH_DrawRect(int x, int y, int w, int h, SDL_Color color);
void SDLH_DrawText(int size, int x, int y, SDL_Color color, const char* text);
void SDLH_LoadImage(SDL_Texture** texture, const char* path);
void SDLH_LoadImage(SDL_Texture** texture, uint8_t* buff, size_t size, bool tga);
void SDLH_DrawImage(SDL_Texture* texture, int x, int y);
void SDLH_DrawImageScale(SDL_Texture* texture, int x, int y, int w, int h);
void SDLH_DrawIcon(std::string icon, int x, int y);
void SDLH_GetTextDimensions(int size, const char* text, uint32_t* w, uint32_t* h);
void SDLH_DrawTextBox(int size, int x, int y, SDL_Color color, int max, const char* text);
void SDLH_Render(void);

void drawOutline(uint32_t x, uint32_t y, uint16_t w, uint16_t h, uint8_t size, SDL_Color color);
void drawPulsingOutline(uint32_t x, uint32_t y, uint16_t w, uint16_t h, uint8_t size, SDL_Color color);
std::string trimToFit(const std::string& text, uint32_t maxsize, size_t textsize);

#endif