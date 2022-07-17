//
// Created by user on 7/17/22.
//
#ifdef __cplusplus
extern "C" {
#endif
#include <SDL.h>
#include <SDL_thread.h>
#ifdef __cplusplus
};
#endif

#include <iostream>
using namespace std;

int main() {
    SDL_Window *window{nullptr};

    int ret = SDL_Init(SDL_INIT_VIDEO);
    if (ret < 0) {
        SDL_LogError(
            SDL_LOG_CATEGORY_APPLICATION,
            "Cannot initialize SDL: %s", SDL_GetError());
        return -1;
    }

    // create an application window the following setting
    int width = 640;
    int height = 480;
    window = SDL_CreateWindow(
        "An SDL2 window",
        SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED,
        width,
        height,
        SDL_WINDOW_OPENGL);

    if (window == nullptr) {
        cerr << "Can not create window " << SDL_GetError() << endl;
        return -1;
    }

    SDL_Delay(3000);

    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}