//
// Created by user on 7/12/22.
//

#ifndef FFMPEG_PRINCIPLE_FFMPEG_VIDEO_DECODER_H
#define FFMPEG_PRINCIPLE_FFMPEG_VIDEO_DECODER_H

#pragma once
#ifdef __cplusplus
extern "C" {
#endif
#include <libavformat/avformat.h>
#ifdef __cplusplus
}
#endif

#include <iostream>
namespace ffmpeg_utils {
class FFMPEGVideoDecoder {
public:
    ~FFMPEGVideoDecoder() {
        close();
    }
    int open(const std::string &file_path) {
        int ret = avformat_open_input(&fmt_ctx, file_path.c_str(), nullptr, nullptr);
        if (ret != 0) {
            std::cerr << "avformat_open_input failed\n";
            return ret;
        }
        video_stream_index = findVideoStreamIndex(fmt_ctx);

        codec_ctx = avcodec_alloc_context3(nullptr);
        ret = copyCodecParametersToContext();
        if (ret != 0) {
            std::cerr << "avcodec_parameters_to_context failed\n";
        }

        codec = avcodec_find_decoder(codec_ctx->codec_id);
        if (codec == nullptr) {
            std::cerr << "avcodec_find_decoder failed\n";
            return -1;
        }

        ret = avcodec_open2(codec_ctx, codec, nullptr);
        if (ret != 0) {
            std::cerr << "avcodec_open2 failed\n";
            return -1;
        }

        pkt = av_packet_alloc();
        frame = av_frame_alloc();

        is_open = true;

        return ret;
    }

    void close() {
        if (fmt_ctx != nullptr) {
            avformat_close_input(&fmt_ctx);
            fmt_ctx = nullptr;
        }

        if (codec_ctx != nullptr) {
            avcodec_close(codec_ctx);
            avcodec_free_context(&codec_ctx);
            codec_ctx = nullptr;
        }

        if (pkt != nullptr) {
            av_packet_free(&pkt);
            pkt = nullptr;
        }

        if (frame != nullptr) {
            av_frame_free(&frame);
            frame = nullptr;
        }

        is_open = false;
    }

    bool isOpen() const {
        return is_open;
    }

    std::pair<int, AVFrame *> readFrame() const {
        auto [ret, p] = readPacket();

        if (ret == 0) {
            ret = avcodec_send_packet(codec_ctx, p);
            av_packet_unref(p);

            // receive from codec
            for (;;) {
                ret = avcodec_receive_frame(codec_ctx, frame);

                // receive success
                if (ret == 0) {
                    break;
                } else if (ret == AVERROR(EAGAIN)) {
                    // need more packet data
                    return readFrame();
                } else if (ret == AVERROR_EOF) {
                    break;
                }
            }
        } else {
            return {ret, nullptr};
        }

        return {ret, frame};
    }

    std::pair<int, AVPacket *> readPacket() const {
        for (;;) {
            auto ret = av_read_frame(fmt_ctx, pkt);

            if (pkt->stream_index != video_stream_index) {
                av_packet_unref(pkt);
                continue;
            }

            if (ret == AVERROR_EOF) {
                std::cerr << "read packet eof\n";
                av_packet_unref(pkt);
            }

            return {ret, pkt};
        }
    }

    AVFormatContext *fmt_ctx{nullptr};
    AVCodecContext *codec_ctx{nullptr};
    AVCodec *codec{nullptr};
    AVPacket *pkt{nullptr};
    AVFrame *frame{nullptr};
    int video_stream_index = {-1};
    bool is_open{false};

private:
    int findVideoStreamIndex(const AVFormatContext *ctx) {
        for (int i = 0; i < ctx->nb_streams; ++i) {
            if (ctx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
                return i;
            }
        }

        return -1;
    }

    int copyCodecParametersToContext() {
        const AVCodecParameters *video_codec_par = fmt_ctx->streams[video_stream_index]->codecpar;
        return avcodec_parameters_to_context(codec_ctx, video_codec_par);
    };

    int tryReceiveOneFrame() const {
        // receive from codec
        for (;;) {
            int ret = avcodec_receive_frame(codec_ctx, frame);
            // got one frame, just return
            if (ret == 0) {
                return ret;
            } else if (ret == AVERROR(EAGAIN)) {
            }
        }
    }
};
} // namespace ffmpeg_utils

#endif // FFMPEG_PRINCIPLE_FFMPEG_VIDEO_DECODER_H
