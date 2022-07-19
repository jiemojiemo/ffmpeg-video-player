//
// Created by user on 7/19/22.
//

#ifdef __cplusplus
extern "C" {
#endif
#include <SDL.h>
#ifdef __cplusplus
};
#endif

#include <iostream>

using namespace std;

int main() {
    SDL_Window *window;
    SDL_Renderer *renderer;

    int ret = SDL_Init(SDL_INIT_VIDEO);
    if (ret < 0) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                     "Couldn't initialize SDL %s", SDL_GetError());
        return -1;
    }

    window = SDL_CreateWindow("An SDL2 window",
                              512,
                              512,
                              512,
                              512,
                              SDL_WINDOW_OPENGL
    );
    if (window == nullptr) {
        cerr << "Could not create window " << SDL_GetError() << endl;
        return -1;
    }

    renderer = SDL_CreateRenderer(window, -1, 0);

    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);

    SDL_RenderClear(renderer);

    SDL_RenderPresent(renderer);

    bool quit = false;
    SDL_Event event;
    for (; !quit;) {
        SDL_WaitEvent(&event);

        switch (event.type) {
            case SDL_QUIT: {
                quit = true;
                break;
            }
        }
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}