#ifndef _SDL3_CONTEXT_HPP_
#define _SDL3_CONTEXT_HPP_

#include <SDL3/SDL.h> // SDL_Init, SDL_CreateWindow etc.
#include <stdexcept>  // std::runtime_error

class SDLContext
{
public:
  explicit SDLContext(Uint32 flags = SDL_INIT_VIDEO)
  {
    if (!SDL_Init(flags))
    {
      throw std::runtime_error(SDL_GetError());
    }
  }

  SDLContext(const SDLContext &) = delete;
  SDLContext &operator=(const SDLContext &) = delete;

  ~SDLContext()
  {
    SDL_Quit();
  }
};

#endif // _SDL3_CONTEXT_HPP_
