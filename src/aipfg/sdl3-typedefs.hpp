#ifndef _SDL3_TYPEDEFS_HPP_
#define _SDL3_TYPEDEFS_HPP_

#include <SDL3/SDL.h> // SDL_Init, SDL_CreateWindow etc.
#include <memory>     // std::unique_ptr

using WindowPtr = std::unique_ptr<SDL_Window, decltype(&SDL_DestroyWindow)>;
using RendererPtr = std::unique_ptr<SDL_Renderer, decltype(&SDL_DestroyRenderer)>;
using TexturePtr = std::unique_ptr<SDL_Texture, decltype(&SDL_DestroyTexture)>;
using SurfacePtr = std::unique_ptr<SDL_Surface, decltype(&SDL_DestroySurface)>;

#endif // _SDL3_TYPEDEFS_HPP_
