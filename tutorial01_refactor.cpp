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
#ifdef __cplusplus
};
#endif

#include "ffmpeg_utils/ffmpeg_image_converter.h"
#include "ffmpeg_utils/ffmpeg_video_decoder.h"
#include <iostream>
#include <string>

using namespace std;
using namespace ffmpeg_utils;

void SaveFrame(AVFrame *pFrame, int width, int height, int iFrame) {
    FILE *pFile;
    char szFilename[32];
    int y;

    // Open file
    sprintf(szFilename, "frame%d.ppm", iFrame);
    pFile = fopen(szFilename, "wb");
    if (pFile == NULL)
        return;

    // Write header
    fprintf(pFile, "P6\n%d %d\n255\n", width, height);

    // Write pixel data
    for (y = 0; y < height; y++)
        fwrite(pFrame->data[0] + y * pFrame->linesize[0], 1, width * 3, pFile);

    // Close file
    fclose(pFile);
}

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

    AVPixelFormat dst_format = AVPixelFormat::AV_PIX_FMT_RGB24;
    FFMPEGImageConverter img_conv;
    img_conv.prepare(
        decoder.codec_ctx->width,
        decoder.codec_ctx->height,
        decoder.codec_ctx->pix_fmt,
        decoder.codec_ctx->width,
        decoder.codec_ctx->height,
        dst_format,
        SWS_BILINEAR,
        nullptr,
        nullptr,
        nullptr);

    int num_frame_saved = 0;
    for (;;) {
        if (num_frame_saved >= 5) {
            break;
        }

        auto [ret, in_frame] = decoder.readFrame();
        if (ret == AVERROR_EOF) {
            cout << "read frame eof\n";
            break;
        }

        auto [_, rgb_frame] = img_conv.convert(in_frame);
        SaveFrame(rgb_frame, decoder.codec_ctx->width, decoder.codec_ctx->height, num_frame_saved);
        ++num_frame_saved;
    }
    return 0;
}