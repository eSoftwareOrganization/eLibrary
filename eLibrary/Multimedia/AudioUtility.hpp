#pragma once

#if eLibraryFeature(Multimedia)

#include <Multimedia/Exception.hpp>

extern "C" {
#include <AL/alext.h>
#include <libavcodec/avcodec.h>
#include <libavdevice/avdevice.h>
#include <libavformat/avformat.h>
#include <libswresample/swresample.h>
}

namespace eLibrary::Multimedia {
    namespace FFMpeg {
        class MediaCodec final : public Object {
        private:
            const AVCodec *CodecObject = nullptr;

            constexpr MediaCodec(const AVCodec *CodecSource) noexcept: CodecObject(CodecSource) {}

            doDisableCopyAssignConstruct(MediaCodec)
            friend class MediaCodecContext;
        public:
            doEnableMoveAssignConstruct(MediaCodec)

            ~MediaCodec() noexcept {
                if (CodecObject) CodecObject = nullptr;
            }

            void doAssign(MediaCodec &&CodecSource) noexcept {
                if (Objects::getAddress(CodecSource) == this) return;
                CodecObject = CodecSource.CodecObject;
                CodecSource.CodecObject = nullptr;
            }

            static MediaCodec doFindDecoder(enum AVCodecID CodecID) {
                const AVCodec *CodecObject = avcodec_find_decoder(CodecID);
                if (!CodecObject)
                    throw MediaException(String(u"MediaCodec::doFindDecoder(AVCodecID) avcodec_find_decoder"));
                return {CodecObject};
            }

            static MediaCodec doFindEncoder(enum AVCodecID CodecID) {
                const AVCodec *CodecObject = avcodec_find_encoder(CodecID);
                if (!CodecObject)
                    throw MediaException(String(u"MediaCodec::doFindEncoder(AVCodecID) avcodec_find_encoder"));
                return {CodecObject};
            }

            explicit operator const AVCodec*() const noexcept {
                return CodecObject;
            }

            const AVCodec *operator->() noexcept {
                return CodecObject;
            }

            String toString() const noexcept override {
                return String(u"MediaCodec[").doConcat({avcodec_get_name(CodecObject->id)}).doConcat(u']');
            }
        };

        class MediaFrame final : public Object {
        private:
            AVFrame *FrameObject = nullptr;

            constexpr MediaFrame(AVFrame *FrameSource) noexcept: FrameObject(FrameSource) {}

            doDisableCopyAssignConstruct(MediaFrame)
            friend class MediaCodecContext;
        public:
            doEnableMoveAssignConstruct(MediaFrame)

            ~MediaFrame() noexcept {
                if (FrameObject) {
                    av_frame_free(&FrameObject);
                    FrameObject = nullptr;
                }
            }

            static MediaFrame doAllocate() {
                AVFrame *FrameObject = av_frame_alloc();
                if (!FrameObject) throw MediaException(String(u"MediaFrame::doAllocate() av_frame_alloc"));
                return {FrameObject};
            }

            void doAssign(MediaFrame &&FrameSource) noexcept {
                if (Objects::getAddress(FrameSource) == this) return;
                FrameObject = FrameSource.FrameObject;
                FrameSource.FrameObject = nullptr;
            }

            void getFrameBuffer() const {
                if (av_frame_get_buffer(FrameObject, 0))
                    throw MediaException(String(u"MediaFrame::getFrameBuffer() av_frame_get_buffer"));
            }

            explicit operator AVFrame*() const noexcept {
                return FrameObject;
            }

            AVFrame *operator->() noexcept {
                return FrameObject;
            }
        };

        class MediaPacket final : public Object {
        private:
            AVPacket *PacketObject = nullptr;

            constexpr MediaPacket(AVPacket *PacketSource) noexcept: PacketObject(PacketSource) {}

            doDisableCopyAssignConstruct(MediaPacket)
            friend class MediaCodecContext;
            friend class MediaFormatContext;
        public:
            doEnableMoveAssignConstruct(MediaPacket)

            ~MediaPacket() noexcept {
                if (PacketObject) {
                    av_packet_free(&PacketObject);
                    PacketObject = nullptr;
                }
            }

            static MediaPacket doAllocate() {
                AVPacket *PacketObject = av_packet_alloc();
                if (!PacketObject) throw MediaException(String(u"MediaPacket::doAllocate() av_packet_alloc"));
                return {PacketObject};
            }

            void doAssign(MediaPacket &&PacketSource) noexcept {
                if (Objects::getAddress(PacketSource) == this) return;
                PacketObject = PacketSource.PacketObject;
                PacketSource.PacketObject = nullptr;
            }

            explicit operator AVPacket*() const noexcept {
                return PacketObject;
            }

            AVPacket *operator->() noexcept {
                return PacketObject;
            }
        };

        class MediaSWRContext final : public Object {
        private:
            SwrContext *ContextObject = nullptr;

            constexpr MediaSWRContext(SwrContext *ContextSource) noexcept: ContextObject(ContextSource) {}

            doDisableCopyAssignConstruct(MediaSWRContext)
        public:
            doEnableMoveAssignConstruct(MediaSWRContext)

            ~MediaSWRContext() noexcept {
                if (ContextObject) {
                    swr_free(&ContextObject);
                    ContextObject = nullptr;
                }
            }

            static MediaSWRContext doAllocate(AVChannelLayout *ContextChannelLayoutInput, AVChannelLayout *ContextChannelLayoutOutput, enum AVSampleFormat ContextSampleFormatInput, enum AVSampleFormat ContextSampleFormatOutput, int ContextSampleRateInput, int ContextSampleRateOutput) {
                SwrContext *ContextObject = nullptr;
                if (swr_alloc_set_opts2(&ContextObject, ContextChannelLayoutOutput, ContextSampleFormatOutput, ContextSampleRateOutput, ContextChannelLayoutInput, ContextSampleFormatInput, ContextSampleRateInput, 0, nullptr))
                    throw MediaException(String(u"MediaSWRContext::doAllocate(AVChannelLayout*, AVChannelLayout, enum AVSampleFormat, enum AVSampleFormat, int, int) swr_alloc_set_opts2"));
                return {ContextObject};
            }

            void doAssign(MediaSWRContext &&ContextSource) noexcept {
                if (Objects::getAddress(ContextSource) == this) return;
                ContextObject = ContextSource.ContextObject;
                ContextSource.ContextObject = nullptr;
            }

            void doConvert(const uint8_t **ContextInput, uint8_t **ContextOutput, int ContextSize) {
                if (swr_convert(ContextObject, ContextOutput, ContextSize, ContextInput, ContextSize) < 0)
                    throw MediaException(String(u"MediaSWRContext::doConvert(const uint8_t**, uint8_t**, int) swr_convert"));
            }

            void doInitialize() {
                if (swr_init(ContextObject))
                    throw MediaException(String(u"MediaSWRContext::doInitialize() swr_init"));
            }

            explicit operator SwrContext*() const noexcept {
                return ContextObject;
            }
        };

        class MediaCodecContext final : public Object {
        private:
            AVCodecContext *ContextObject = nullptr;

            constexpr MediaCodecContext(AVCodecContext *ContextSource) noexcept: ContextObject(ContextSource) {}

            doDisableCopyAssignConstruct(MediaCodecContext)
        public:
            doEnableMoveAssignConstruct(MediaCodecContext)

            ~MediaCodecContext() noexcept {
                if (ContextObject) {
                    avcodec_close(ContextObject);
                    avcodec_free_context(&ContextObject);
                    ContextObject = nullptr;
                }
            }

            static MediaCodecContext doAllocate() {
                AVCodecContext *CodecContextObject = avcodec_alloc_context3(nullptr);
                if (!CodecContextObject)
                    throw MediaException(String(u"MediaCodecContext::doAllocate() avcodec_alloc_context3"));
                return {CodecContextObject};
            }

            static MediaCodecContext doAllocate(const MediaCodec &ContextCodecSource) {
                AVCodecContext *CodecContextObject = avcodec_alloc_context3(ContextCodecSource.CodecObject);
                if (!CodecContextObject)
                    throw MediaException(String(u"MediaCodecContext::doAllocate(const MediaCodec&) avcodec_alloc_context3"));
                return {CodecContextObject};
            }

            void doAssign(MediaCodecContext &&ContextSource) noexcept {
                if (Objects::getAddress(ContextSource) == this) return;
                ContextObject = ContextSource.ContextObject;
                ContextSource.ContextObject = nullptr;
            }

            void doOpen(const MediaCodec &ContextCodecSource) {
                if (avcodec_open2(ContextObject, ContextCodecSource.CodecObject, nullptr))
                    throw MediaException(String(u"MediaCodecContext::doOpen(const MediaCodec&) avcodec_open2"));
            }

            void doSendFrame(const MediaFrame &ContextFrameSource) {
                if (avcodec_send_frame(ContextObject, ContextFrameSource.FrameObject))
                    throw MediaException(String(u"MediaCodecContext::doSendFrame(const MediaFrame&) avcodec_send_frame"));
            }

            void doSendPacket(const MediaPacket &ContextPacketSource) {
                if (avcodec_send_packet(ContextObject, ContextPacketSource.PacketObject))
                    throw MediaException(String(u"MediaCodecContext::doSendPacket(const MediaPacket&) avcodec_send_packet"));
            }

            explicit operator AVCodecContext*() const noexcept {
                return ContextObject;
            }

            AVCodecContext *operator->() noexcept {
                return ContextObject;
            }

            void setParameter(AVCodecParameters *ContextParameterSource) {
                if (avcodec_parameters_to_context(ContextObject, ContextParameterSource))
                    throw MediaException(String(u"MediaCodecContext::setParameter(AVCodecParameters*) avcodec_parameters_to_context"));
            }
        };

        class MediaFormatContext final : public Object {
        private:
            AVFormatContext *ContextObject = nullptr;

            constexpr MediaFormatContext(AVFormatContext *ContextSource) noexcept: ContextObject(ContextSource) {}

            doDisableCopyAssignConstruct(MediaFormatContext)
        public:
            doEnableMoveAssignConstruct(MediaFormatContext)

            ~MediaFormatContext() noexcept {
                if (ContextObject) {
                    avformat_close_input(&ContextObject);
                    ContextObject = nullptr;
                }
            }

            static MediaFormatContext doAllocate() {
                AVFormatContext *ContextObject = avformat_alloc_context();
                if (!ContextObject)
                    throw MediaException(String(u"MediaFormatContext::doAllocate() avformat_alloc_context"));
                return {ContextObject};
            }

            static MediaFormatContext doAllocateOutput(const String &MediaPath) {
                AVFormatContext *ContextObject = nullptr;
                if (avformat_alloc_output_context2(&ContextObject, nullptr, nullptr, MediaPath.toU8String().c_str()) < 0)
                    throw MediaException(String(u"MediaFormatContext::doAllocateOutput(const String&) avformat_alloc_output_context2"));
                return {ContextObject};
            }

            void doAssign(MediaFormatContext &&ContextSource) noexcept {
                if (Objects::getAddress(ContextSource) == this) return;
                ContextObject = ContextSource.ContextObject;
                ContextSource.ContextObject = nullptr;
            }

            int doFindBestStream(enum AVMediaType StreamType) const {
                int MediaStreamIndex = av_find_best_stream(ContextObject, StreamType, -1, -1, nullptr, 0);
                if (MediaStreamIndex < 0)
                    throw MediaException(String(u"MediaFormatContext::doFindBestStream(AVMediaType) av_find_best_stream"));
                return MediaStreamIndex;
            }

            void doFindStreamInformation() const {
                if (avformat_find_stream_info(ContextObject, nullptr) < 0)
                    throw MediaException(String(u"MediaFormatContext::doFindStreamInformation() avformat_find_stream_info"));
            }

            static MediaFormatContext doOpen(const String &MediaSource) {
                AVFormatContext *ContextObject = nullptr;
                if (avformat_open_input(&ContextObject, MediaSource.toU8String().c_str(), nullptr, nullptr))
                    throw MediaException(String(u"MediaFormatContext::doOpen(const String&) avformat_open_input"));
                return {ContextObject};
            }

            void doWriteFrame(const MediaPacket &ContextPacketSource) const {
                if (av_write_frame(ContextObject, ContextPacketSource.PacketObject) < 0)
                    throw MediaException(String(u"MediaFormatContext::doWriteFrame(const MediaPacket&) av_write_frame"));
            }

            void doWriteHeader() const {
                if (avformat_write_header(ContextObject, nullptr) < 0)
                    throw MediaException(String(u"MediaFormatContext::doWriteHeader() avformat_write_header"));
            }

            void doWriteTrailer() const {
                if (av_write_trailer(ContextObject) < 0)
                    throw MediaException(String(u"MediaFormatContext::doWriteTrailer() av_write_trailer"));
            }

            explicit operator AVFormatContext*() const noexcept {
                return ContextObject;
            }

            AVFormatContext *operator->() noexcept {
                return ContextObject;
            }

            void setOutputFormat(const AVOutputFormat *FormatSource) {
                if (!FormatSource)
                    throw MediaException(String(u"MediaFormatContext::setOutputFormat(const AVOutputFormat*) FormatSource"));
                ContextObject->oformat = FormatSource;
            }
        };
    }

    namespace OpenAL {
        class MediaBuffer final : public Object {
        private:
            ALuint BufferIndex;

            doDisableCopyAssignConstruct(MediaBuffer)
            friend class MediaSource;
        public:
            MediaBuffer() noexcept {
                alGenBuffers(1, &BufferIndex);
            }

            MediaBuffer(ALenum AudioBufferFormat, const void *AudioBufferData, ALsizei AudioBufferSize, ALsizei AudioSampleRate) noexcept {
                alGenBuffers(1, &BufferIndex);
                alBufferData(BufferIndex, AudioBufferFormat, AudioBufferData, AudioBufferSize, AudioSampleRate);
            }

            ~MediaBuffer() noexcept {
                alDeleteBuffers(1, &BufferIndex);
            }
        };

        class MediaDevice final : public Object {
        private:
            ALCdevice *DeviceObject;

            doDisableCopyAssignConstruct(MediaDevice)
            friend class MediaContext;
        public:
            MediaDevice(const String &DeviceName) {
                DeviceObject = alcOpenDevice(DeviceName.isEmpty() ? nullptr : DeviceName.toU8String().c_str());
                if (!DeviceObject)
                    throw MediaException(String(u"MediaDevice::MediaDevice(const String&) alcOpenDevice"));
            }

            ~MediaDevice() noexcept {
                if (DeviceObject) {
                    alcCloseDevice(DeviceObject);
                    DeviceObject = nullptr;
                }
            }

            void doClose() {
                if (!DeviceObject) throw MediaException(String(u"MediaDevice::doClose() DeviceObject"));
                alcCloseDevice(DeviceObject);
                DeviceObject = nullptr;
            }
        };

        class MediaContext final : public Object {
        private:
            ALCcontext *ContextObject;

            doDisableCopyAssignConstruct(MediaContext)
        public:
            MediaContext(const MediaDevice &ContextDevice) {
                ContextObject = alcCreateContext(ContextDevice.DeviceObject, nullptr);
                if (!ContextObject)
                    throw MediaException(String(u"MediaContext::MediaContext(const MediaDevice&) alcCreateContext"));
            }

            ~MediaContext() noexcept {
                if (ContextObject) {
                    alcDestroyContext(ContextObject);
                    ContextObject = nullptr;
                }
            }

            void doDestroy() {
                if (!ContextObject) throw MediaException(String(u"MediaContext::doDestroy() ContextObject"));
                alcDestroyContext(ContextObject);
            }

            void setContextCurrent() const {
                alcMakeContextCurrent(ContextObject);
            }

            static void setContextCurrentNull() noexcept {
                alcMakeContextCurrent(nullptr);
            }
        };

        class MediaSource final : public Object {
        private:
            ALuint SourceIndex;

            doDisableCopyAssignConstruct(MediaSource)
        public:
            MediaSource() noexcept {
                alGenSources(1, &SourceIndex);
            }

            ~MediaSource() noexcept {
                alDeleteSources(1, &SourceIndex);
            }

            void doPause() const noexcept {
                alSourcePause(SourceIndex);
            }

            void doPlay() const noexcept {
                alSourcePlay(SourceIndex);
            }

            void doRewind() const noexcept {
                alSourceRewind(SourceIndex);
            }

            void doStop() const noexcept {
                alSourceStop(SourceIndex);
            }

            void setAudioGain(float OptionValue) const noexcept {
                alSourcef(SourceIndex, AL_GAIN, OptionValue);
            }

            void setAudioPitch(float OptionValue) const noexcept {
                alSourcef(SourceIndex, AL_PITCH, OptionValue);
            }

            void setAudioVelocity(float VelocityX, float VelocityY, float VelocityZ) const noexcept {
                alSource3f(SourceIndex, AL_VELOCITY, VelocityX, VelocityY, VelocityZ);
            }

            void setSourceBuffer(const MediaBuffer &SourceBufferSource) const noexcept {
                alSourcei(SourceIndex, AL_BUFFER, (ALint) SourceBufferSource.BufferIndex);
            }

            void setSourceDirection(float DirectionX, float DirectionY, float DirectionZ) const noexcept {
                alSource3f(SourceIndex, AL_DIRECTION, DirectionX, DirectionY, DirectionZ);
            }

            void setSourceLoop(bool OptionValue) const noexcept {
                alSourcei(SourceIndex, AL_LOOPING, OptionValue);
            }

            void setSourceRelative(bool OptionValue) const noexcept {
                alSourcei(SourceIndex, AL_SOURCE_RELATIVE, OptionValue);
            }
        };

#define doDestroyOpenAL() MediaContext::setContextCurrentNull();
#define doInitializeOpenAL() Multimedia::MediaDevice MediaDeviceObject(String(u""));Multimedia::MediaContext MediaContextObject(MediaDeviceObject);MediaContextObject.setContextCurrent();
    }
}
#endif
