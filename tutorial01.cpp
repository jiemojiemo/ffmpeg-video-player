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

#include <iostream>
#include <string>
using namespace std;

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

    // open video file
    AVFormatContext *fmt_ctx{nullptr};
    int ret = avformat_open_input(&fmt_ctx, input_file.c_str(), nullptr, nullptr);
    if (ret < 0) {
        cerr << "avformat_open_input failed\n";
        return -1;
    }

    // retrieve stream information
    ret = avformat_find_stream_info(fmt_ctx, nullptr);
    if (ret < 0) {
        cerr << "avformat_find_stream_info failed\n";
        return -1;
    }

    // dump information about file
    av_dump_format(fmt_ctx, 0, input_file.c_str(), 0);

    // find the video stream
    int video_stream_index = -1;
    for (int i = 0; i < fmt_ctx->nb_streams; ++i) {
        if (fmt_ctx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            video_stream_index = i;
            break;
        }
    }
    if (video_stream_index == -1) {
        cerr << "cannot fine video stream\n";
        return -1;
    }

    // allocate codec context
    AVCodecContext *codec_ctx = avcodec_alloc_context3(nullptr);
    ret = avcodec_parameters_to_context(codec_ctx, fmt_ctx->streams[video_stream_index]->codecpar);
    if (ret < 0) {
        cerr << "avcodec_parameters_to_context failed\n";
        return -1;
    }

    // find the codec for the video stream
    AVCodec *codec = avcodec_find_decoder(codec_ctx->codec_id);
    if (codec == nullptr) {
        cerr << "avcodec_find_encoder failed\n";
        return -1;
    }

    // open codec
    ret = avcodec_open2(codec_ctx, codec, nullptr);
    if (ret < 0) {
        cerr << "avcodec_open2 failed\n";
        return -1;
    }

    AVFrame *frame = av_frame_alloc();
    AVFrame *rgb_frame = av_frame_alloc();

    AVPixelFormat dst_format = AVPixelFormat::AV_PIX_FMT_RGB24;
    const int align = 1;

    int num_bytes = av_image_get_buffer_size(dst_format,
                                             codec_ctx->width, codec_ctx->height, align);
    uint8_t *buffer = (uint8_t *)av_malloc(num_bytes);

    ret = av_image_fill_arrays(rgb_frame->data,
                               rgb_frame->linesize,
                               buffer,
                               dst_format,
                               codec_ctx->width,
                               codec_ctx->height,
                               align);
    if (ret < 0) {
        cerr << "av_image_fill_arrays failed\n";
        return -1;
    }

    // initialize SWS context for software scaling
    //    sws_alloc_context()
    struct SwsContext *sws_ctx = sws_getContext(
        codec_ctx->width,
        codec_ctx->height,
        codec_ctx->pix_fmt,
        codec_ctx->width,
        codec_ctx->height,
        dst_format,
        SWS_BILINEAR,
        nullptr,
        nullptr,
        nullptr);

    if (sws_ctx == nullptr) {
        cerr << "sws_getContext failed\n";
        return -1;
    }

    // read frames and save first five frames to disk
    AVPacket *pkt = av_packet_alloc();

    int num_frame_saved = 0;
    for (;;) {
        if (num_frame_saved >= 5) {
            break;
        }

        ret = av_read_frame(fmt_ctx, pkt);
        if (pkt->stream_index != video_stream_index) {
            av_packet_unref(pkt);
            continue;
        }

        if (ret == AVERROR_EOF) {
            cout << "read frame eof\n";
            av_packet_unref(pkt);
            break;
        }

        // send packet to decoder
        avcodec_send_packet(codec_ctx, pkt);
        av_packet_unref(pkt);

        // receive decoded data
        for (;;) {
            ret = avcodec_receive_frame(codec_ctx, frame);
            if (ret == 0) {
                sws_scale(sws_ctx,
                          frame->data,
                          frame->linesize,
                          0,
                          codec_ctx->height,
                          rgb_frame->data,
                          rgb_frame->linesize);
                SaveFrame(rgb_frame, codec_ctx->width, codec_ctx->height, num_frame_saved);
                ++num_frame_saved;
            } else {
                break;
            }
        }
    }

    av_packet_free(&pkt);
    sws_freeContext(sws_ctx);
    av_free(buffer);
    av_frame_free(&rgb_frame);
    av_frame_free(&frame);
    avcodec_close(codec_ctx);
    avcodec_free_context(&codec_ctx);
    avformat_close_input(&fmt_ctx);
    return 0;
}