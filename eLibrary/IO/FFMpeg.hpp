#pragma once

#include <IO/Exception.hpp>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavdevice/avdevice.h>
#include <libavformat/avformat.h>
#include <libswresample/swresample.h>
}

namespace eLibrary::IO {
    class MediaCodec final : public Object {
    private:
        const AVCodec *CodecObject;

        MediaCodec(const AVCodec *CodecSource) noexcept : CodecObject(CodecSource) {}
    public:
        ~MediaCodec() noexcept {
            if (CodecObject) CodecObject = nullptr;
        }

        static MediaCodec *doFindDecoder(enum AVCodecID CodecID) {
            const AVCodec *CodecObject = avcodec_find_decoder(CodecID);
            if (!CodecObject) throw IOException(String(u"MediaCodec::doFindDecoder(AVCodecID) avcodec_find_decoder"));
            return new MediaCodec(CodecObject);
        }

        static MediaCodec *doFindEncoder(enum AVCodecID CodecID) {
            const AVCodec *CodecObject = avcodec_find_encoder(CodecID);
            if (!CodecObject) throw IOException(String(u"MediaCodec::doFindEncoder(AVCodecID) avcodec_find_encoder"));
            return new MediaCodec(CodecObject);
        }

        const AVCodec *getCodecObject() const noexcept {
            return CodecObject;
        }
    };

    class MediaCodecContext final : public Object {
    private:
        AVCodecContext *ContextObject;

        MediaCodecContext(AVCodecContext *ContextSource) noexcept : ContextObject(ContextSource) {}
    public:
        ~MediaCodecContext() noexcept {
            if (ContextObject) {
                avcodec_close(ContextObject);
                avcodec_free_context(&ContextObject);
                ContextObject = nullptr;
            }
        }

        static MediaCodecContext *doAllocate(const MediaCodec *CodecSource) {
            AVCodecContext *CodecContextObject = avcodec_alloc_context3(CodecSource->getCodecObject());
            if (!CodecContextObject) throw IOException(String(u"MediaCodecContext::doAllocate(const MediaCodec&) avcodec_alloc_context3"));
            return new MediaCodecContext(CodecContextObject);
        }

        int getChannelCount() const noexcept {
            return ContextObject->ch_layout.nb_channels;
        }

        AVCodecContext *getContextObject() const noexcept {
            return ContextObject;
        }

        int getSampleRate() const noexcept {
            return ContextObject->sample_rate;
        }
    };

    class MediaFrame final : public Object {
    private:
        AVFrame *FrameObject;

        MediaFrame(AVFrame *FrameSource) noexcept : FrameObject(FrameSource) {}
    public:
        ~MediaFrame() noexcept {
            if (FrameObject) {
                av_frame_free(&FrameObject);
                FrameObject = nullptr;
            }
        }

        static MediaFrame *doAllocate() {
            AVFrame *FrameObject = av_frame_alloc();
            if (!FrameObject) throw IOException(String(u"MediaFrame::doAllocate() av_frame_alloc"));
            return new MediaFrame(FrameObject);
        }

        AVFrame *getFrameObject() const noexcept {
            return FrameObject;
        }
    };

    class MediaIOContext final : public Object {
    private:
        AVIOContext *ContextObject;

        MediaIOContext(AVIOContext *ContextSource) noexcept : ContextObject(ContextSource) {}
    public:
        ~MediaIOContext() noexcept {
            if (ContextObject) {
                avio_close(ContextObject);
                avio_context_free(&ContextObject);
                ContextObject = nullptr;
            }
        }

        static MediaIOContext *doOpen(const String &ContextSource, int ContextFlag) {
            AVIOContext *ContextObject;
            if (avio_open(&ContextObject, ContextSource.toU8String().c_str(), ContextFlag) < 0)
                throw IOException(String(u"MediaIOContext::doOpen(const String&, int) avio_open"));
            return new MediaIOContext(ContextObject);
        }

        AVIOContext *getContextObject() const noexcept {
            return ContextObject;
        }
    };

    class MediaPacket final : public Object {
    private:
        AVPacket *PacketObject;

        MediaPacket(AVPacket *PacketSource) noexcept : PacketObject(PacketSource) {}
    public:
        ~MediaPacket() noexcept {
            if (PacketObject) {
                av_packet_free(&PacketObject);
                PacketObject = nullptr;
            }
        }

        static MediaPacket *doAllocate() {
            AVPacket *PacketObject = av_packet_alloc();
            if (!PacketObject) throw IOException(String(u"MediaPacket::doAllocate() av_packet_alloc"));
            return new MediaPacket(PacketObject);
        }

        AVPacket *getPacketObject() const noexcept {
            return PacketObject;
        }
    };

    class MediaFormatContext final : public Object {
    private:
        AVFormatContext *ContextObject;

        MediaFormatContext(AVFormatContext *ContextSource) noexcept : ContextObject(ContextSource) {}
    public:
        ~MediaFormatContext() noexcept {
            if (ContextObject) {
                avformat_close_input(&ContextObject);
                ContextObject = nullptr;
            }
        }

        int doFindBestStream(enum AVMediaType StreamType) const {
            int MediaStreamIndex = av_find_best_stream(ContextObject, StreamType, -1, -1, nullptr, 0);
            if (MediaStreamIndex < 0) throw IOException(String(u"MediaFormatContext::doFindBestStream(AVMediaType) av_find_best_stream"));
            return MediaStreamIndex;
        }

        void doFindStreamInformation() const {
            if (avformat_find_stream_info(ContextObject, nullptr) < 0)
                throw IOException(String(u"MediaFormatContext::doFindStreamInformation() avformat_find_stream_info"));
        }

        static MediaFormatContext *doAllocate() {
            AVFormatContext *ContextObject = avformat_alloc_context();
            if (!ContextObject) throw IOException(String(u"MediaFormatContext::doAllocate() avformat_alloc_context"));
            return new MediaFormatContext(ContextObject);
        }

        static MediaFormatContext *doOpen(const String &MediaSource) {
            AVFormatContext *ContextObject;
            if (avformat_open_input(&ContextObject, MediaSource.toU8String().c_str(), nullptr, nullptr))
                throw IOException(String(u"MediaFormatContext::doOpen(const String&) avformat_open_input"));
            return new MediaFormatContext(ContextObject);
        }

        AVFormatContext *getContextObject() const noexcept {
            return ContextObject;
        }

        void setIOContext(const MediaIOContext *IOContext) noexcept {
            ContextObject->pb = IOContext->getContextObject();
        }

        void setOutputFormat(const AVOutputFormat *FormatSource) {
            if (!FormatSource) throw IOException(String(u"MediaFormatContext::setOutputFormat(const AVOutputFormat*) FormatSource"));
            ContextObject->oformat = FormatSource;
        }
    };

    class FFMpegInitializer final : public Object {
    public:
        static void doDestroy() {
            avformat_network_deinit();
        }

        static void doInitialize() {
            avdevice_register_all();
            avformat_network_init();
        }
    };
}