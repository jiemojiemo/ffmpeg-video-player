//
// Created by user on 7/11/22.
//
#include "ffmpeg_utils/ffmpeg_video_decoder.h"
#include <gmock/gmock.h>

using namespace testing;
using namespace ffmpeg_utils;

class AFFMPEGVideoDecoder : public Test {
public:
    FFMPEGVideoDecoder decoder;
    const std::string file_path = "/Users/user/Downloads/encode-v1/juren-30s.mp4";
};

TEST_F(AFFMPEGVideoDecoder, CanOpenLocalFile) {
    int ret = decoder.open(file_path);

    ASSERT_THAT(ret, Eq(0));
}

TEST_F(AFFMPEGVideoDecoder, IsOpenedReturnTrueIfOpened) {
    decoder.open(file_path);

    ASSERT_TRUE(decoder.isOpen());
}

TEST_F(AFFMPEGVideoDecoder, OpenFailedIfInputPathInvalid) {
    const std::string invalid_path = "./xxxx.mp4";

    int ret = decoder.open(invalid_path);

    ASSERT_THAT(ret, Not(0));
}

TEST_F(AFFMPEGVideoDecoder, CreateFormatContextIfOpenSuccess) {
    decoder.open(file_path);

    ASSERT_THAT(decoder.fmt_ctx, NotNull());
}

TEST_F(AFFMPEGVideoDecoder, CanFindVideoStreamIndexAfterOpen) {
    decoder.open(file_path);

    ASSERT_THAT(decoder.video_stream_index, Not(-1));
}

TEST_F(AFFMPEGVideoDecoder, CreateCodecContextIfOpenSucess) {
    decoder.open(file_path);

    ASSERT_THAT(decoder.codec_ctx, NotNull());
}

TEST_F(AFFMPEGVideoDecoder, CanGetCodecIfOpenSucess) {
    decoder.open(file_path);

    ASSERT_THAT(decoder.codec, NotNull());
}

TEST_F(AFFMPEGVideoDecoder, CreatePacketIfOpenSuccess) {
    decoder.open(file_path);

    ASSERT_THAT(decoder.pkt, NotNull());
}

TEST_F(AFFMPEGVideoDecoder, CreateFrameIfOpenSuccess) {
    decoder.open(file_path);

    ASSERT_THAT(decoder.frame, NotNull());
}

TEST_F(AFFMPEGVideoDecoder, CloseWillFreeAllMemories) {
    decoder.open(file_path);

    decoder.close();

    ASSERT_THAT(decoder.fmt_ctx, IsNull());
    ASSERT_THAT(decoder.codec_ctx, IsNull());
    ASSERT_THAT(decoder.pkt, IsNull());
    ASSERT_THAT(decoder.frame, IsNull());
}

TEST_F(AFFMPEGVideoDecoder, IsOpenReturnFalseAfterClose) {
    decoder.open(file_path);

    decoder.close();

    ASSERT_FALSE(decoder.isOpen());
}

TEST_F(AFFMPEGVideoDecoder, CanReadAVideoPacket) {
    decoder.open(file_path);

    auto [error_code, pkt] = decoder.readPacket();

    ASSERT_THAT(error_code, Eq(0));
    ASSERT_THAT(pkt->size, Not(0));
    ASSERT_THAT(pkt->stream_index, Eq(decoder.video_stream_index));
}

TEST_F(AFFMPEGVideoDecoder, CanReadADecodedFrame) {
    decoder.open(file_path);

    auto [error_code, frame] = decoder.readFrame();

    ASSERT_THAT(error_code, Eq(0));
    ASSERT_THAT(frame->pkt_size, Not(0));
    ASSERT_THAT(frame->width, Not(0));
}