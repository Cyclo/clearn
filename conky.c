#include <stdio.h>
#include <stdlib.h>
#include <SDL/SDL.h>


int main()
{
  int running = 1;
  SDL_Event event;
  SDL_Surface *screen;

  putenv("SDL_VIDEO_WINDOW_POS=center");
  putenv("SDL_VIDEO_CENTERED=1");

  SDL_Init(SDL_INIT_EVERYTHING);
  SDL_WM_SetCaption("Hello World!",NULL);
  screen = SDL_SetVideoMode(800,600,32,SDL_HWSURFACE);
  
  SDL_FillRect(screen, NULL, SDL_MapRGB(screen->format,0,0,255));

  boxRGBA(screen,10,10,100,100,0,255,0,255);
  SDL_Flip(screen);

  while(running){

    SDL_WaitEvent(&event);
    if(event.type == SDL_QUIT)
      running = 0;

  }

  SDL_Quit();
  return 0;
}
