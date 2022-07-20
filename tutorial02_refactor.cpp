//
// Created by user on 7/17/22.
//
#ifdef __cplusplus
extern "C" {
#endif
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>

#include <SDL.h>
#include <SDL_thread.h>
#ifdef __cplusplus
};
#endif

#include "ffmpeg_utils/ffmpeg_video_decoder.h"
#include "ffmpeg_utils/ffmpeg_image_converter.h"
#include <iostream>
#include <string>
#include <stdint.h>

using namespace ffmpeg_utils;
using namespace std;

int main(int argc, char *argv[]) {
    if (argc < 2) {
        cerr << "Usage " << argv[0] << " input_file\n";
        return -1;
    }

    string input_file = argv[1];
    FFMPEGVideoDecoder decoder;
    int ret = decoder.open(input_file);
    if (ret != 0) {
        cerr << "open file failed\n";
        return -1;
    }
    av_dump_format(decoder.fmt_ctx, 0, input_file.c_str(), 0);

    ret = SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER);
    if (ret != 0) {
        cerr << "SDL_Init failed\n";
        return -1;
    }

    SDL_Window *screen = SDL_CreateWindow("SDL Video Player",
                                          SDL_WINDOWPOS_UNDEFINED,
                                          SDL_WINDOWPOS_UNDEFINED,
                                          decoder.codec_ctx->width / 2,
                                          decoder.codec_ctx->height / 2,
                                          SDL_WINDOW_OPENGL | SDL_WINDOW_ALLOW_HIGHDPI);

    if (screen == nullptr) {
        cerr << "SDL: could not set video mode - exiting.\n";
        return -1;
    }

    //
    SDL_GL_SetSwapInterval(1);

    SDL_Renderer *renderer = nullptr;
    renderer = SDL_CreateRenderer(screen, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC |
                                              SDL_RENDERER_TARGETTEXTURE);   // [3]

    SDL_Texture *texture = nullptr;
    texture = SDL_CreateTexture(renderer,
                                SDL_PIXELFORMAT_YV12,
                                SDL_TEXTUREACCESS_STREAMING,
                                decoder.codec_ctx->width,
                                decoder.codec_ctx->height);

    FFMPEGImageConverter img_conv;
    img_conv.prepare(
        decoder.codec_ctx->width,
        decoder.codec_ctx->height,
        decoder.codec_ctx->pix_fmt,
        decoder.codec_ctx->width,
        decoder.codec_ctx->height,
        AV_PIX_FMT_YUV420P,
        SWS_BILINEAR,
        NULL,
        NULL,
        NULL);

    int max_frames_to_decode = 10000;
    int i = 0;
    for (;;) {
        if (i++ >= max_frames_to_decode) {
            break;
        }

        auto[ret, in_frame] = decoder.readFrame();
        if (ret == AVERROR_EOF) {
            break;
        }

        auto [_, pict] = img_conv.convert(in_frame);

        double fps = av_q2d(decoder.video_stream->r_frame_rate);
        double sleep_time = 1.0 / fps;

        SDL_Delay((1000 * sleep_time) - 10);

        SDL_Rect rect;
        rect.x = 0;
        rect.y = 0;
        rect.w = decoder.codec_ctx->width;
        rect.h = decoder.codec_ctx->height;

        SDL_UpdateYUVTexture(
                texture,            // the texture to update
                &rect,              // a pointer to the rectangle of pixels to update, or NULL to update the entire texture
                pict->data[0],      // the raw pixel data for the Y plane
                pict->linesize[0],  // the number of bytes between rows of pixel data for the Y plane
                pict->data[1],      // the raw pixel data for the U plane
                pict->linesize[1],  // the number of bytes between rows of pixel data for the U plane
                pict->data[2],      // the raw pixel data for the V plane
                pict->linesize[2]   // the number of bytes between rows of pixel data for the V plane
        );

        SDL_RenderClear(renderer);

        SDL_RenderCopy(
                renderer,   // the rendering context
                texture,    // the source texture
                NULL,       // the source SDL_Rect structure or NULL for the entire texture
                NULL        // the destination SDL_Rect structure or NULL for the entire rendering
                // target; the texture will be stretched to fill the given rectangle
        );

        SDL_RenderPresent(renderer);

        // handle Ctrl + C event
        SDL_Event event;
        SDL_PollEvent(&event);
        switch (event.type) {
            case SDL_QUIT: {
                SDL_Quit();
                exit(0);
            }
            default: {
                break;
            }
        }
    }

    SDL_DestroyTexture(texture);
    SDL_DestroyWindow(screen);
    SDL_Quit();
    return 0;
}