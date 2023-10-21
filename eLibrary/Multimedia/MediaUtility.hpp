#pragma once

#ifndef eLibraryHeaderMultimediaAudioUtility
#define eLibraryHeaderMultimediaAudioUtility

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
    class MediaChannelLayout final : public Object {
    private:
        uint8_t LayoutChannelCount;
        uint64_t LayoutChannelMask;
    public:
        constexpr MediaChannelLayout() noexcept : LayoutChannelCount(0), LayoutChannelMask(0) {};

        constexpr MediaChannelLayout(uint8_t LayoutChannelCountSource, uint64_t LayoutChannelMaskSource) noexcept : LayoutChannelCount(LayoutChannelCountSource), LayoutChannelMask(LayoutChannelMaskSource) {}

        constexpr MediaChannelLayout(const AVChannelLayout &LayoutSource) noexcept : LayoutChannelCount((uint8_t) LayoutSource.nb_channels), LayoutChannelMask(LayoutSource.u.mask) {
            if (!LayoutChannelMask) LayoutChannelMask = AV_CH_LAYOUT_STEREO;
        }

        uint8_t getChannelCount() const noexcept {
            return LayoutChannelCount;
        }

        uint64_t getChannelMask() const noexcept {
            return LayoutChannelMask;
        }

        const char *getClassName() const noexcept override {
            return "MediaChannelLayout";
        }

        AVChannelLayout toFFMpegFormat() const noexcept {
            return AV_CHANNEL_LAYOUT_MASK(LayoutChannelCount, LayoutChannelMask);
        }

        auto toOpenALFormat() const {
            switch (LayoutChannelMask) {
                case AV_CH_LAYOUT_5POINT1:
                    return AL_FORMAT_51CHN8;
                case AV_CH_LAYOUT_6POINT1:
                    return AL_FORMAT_61CHN8;
                case AV_CH_LAYOUT_7POINT1:
                    return AL_FORMAT_71CHN8;
                case AV_CH_LAYOUT_MONO:
                    return AL_FORMAT_MONO8;
                case AV_CH_LAYOUT_QUAD:
                    return AL_FORMAT_QUAD8;
                case AV_CH_LAYOUT_STEREO:
                    return AL_FORMAT_STEREO8;
            }
            throw MediaException(String(u"MediaChannelLayout::toOpenALFormat() LayoutChannelMask"));
        }
    };

    static const MediaChannelLayout Layout51{6, AV_CH_LAYOUT_5POINT1};
    static const MediaChannelLayout Layout61{7, AV_CH_LAYOUT_6POINT1};
    static const MediaChannelLayout Layout71{8, AV_CH_LAYOUT_7POINT1};
    static const MediaChannelLayout LayoutMono{1, AV_CH_LAYOUT_MONO};
    static const MediaChannelLayout LayoutQuad{4, AV_CH_LAYOUT_QUAD};
    static const MediaChannelLayout LayoutStereo{2, AV_CH_LAYOUT_STEREO};

    namespace FFMpeg {
        class MediaCodec final : public Object, public NonCopyable {
        private:
            const AVCodec *CodecObject = nullptr;

            constexpr MediaCodec(const AVCodec *CodecSource) noexcept: CodecObject(CodecSource) {}

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

            const char *getClassName() const noexcept override {
                return "MediaCodec";
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

        class MediaFrame final : public Object, public NonCopyable {
        private:
            AVFrame *FrameObject = nullptr;

            constexpr MediaFrame(AVFrame *FrameSource) noexcept: FrameObject(FrameSource) {}

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

        class MediaPacket final : public Object, public NonCopyable {
        private:
            AVPacket *PacketObject = nullptr;

            constexpr MediaPacket(AVPacket *PacketSource) noexcept: PacketObject(PacketSource) {}

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

        class MediaSWRContext final : public Object, public NonCopyable {
        private:
            SwrContext *ContextObject = nullptr;

            constexpr MediaSWRContext(SwrContext *ContextSource) noexcept: ContextObject(ContextSource) {}
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

        class MediaCodecContext final : public Object, public NonCopyable {
        private:
            AVCodecContext *ContextObject = nullptr;

            constexpr MediaCodecContext(AVCodecContext *ContextSource) noexcept: ContextObject(ContextSource) {}
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

        class MediaFormatContext final : public Object, public NonCopyable {
        private:
            AVFormatContext *ContextObject = nullptr;

            constexpr MediaFormatContext(AVFormatContext *ContextSource) noexcept: ContextObject(ContextSource) {}
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
        class MediaBuffer final : public Object, public NonCopyable {
        private:
            ALuint BufferIndex;
            IO::ByteBuffer BufferObject;

            friend class MediaSource;
        public:
            MediaBuffer() {
                alGenBuffers(1, &BufferIndex);
                if (alGetError() != AL_NO_ERROR) throw MediaException(String(u"MediaBuffer::MediaBuffer() alGenBuffers"));
            }

            MediaBuffer(const MediaChannelLayout &AudioBufferLayout, const IO::ByteBuffer &AudioBuffer, ALsizei AudioSampleRate) : BufferObject(AudioBuffer) {
                alGenBuffers(1, &BufferIndex);
                if (alGetError() != AL_NO_ERROR) throw MediaException(String(u"MediaSource::MediaSource(const MediaChannelLayout&, const IO::ByteBuffer&, ALsizei) alGenSources"));
                alBufferData(BufferIndex, AudioBufferLayout.toOpenALFormat(), AudioBuffer.getBufferContainer(), BufferObject.getBufferLimit(), AudioSampleRate);
            }

            ~MediaBuffer() noexcept {
                alDeleteBuffers(1, &BufferIndex);
            }
        };

        class MediaCaptureDevice final : public Object, public NonCopyable {
        private:
            ALCdevice *DeviceObject;

            friend class MediaContext;
        public:
            MediaCaptureDevice(const String &DeviceName, ALCsizei DeviceFrequency, const MediaChannelLayout &DeviceLayout) {
                DeviceObject = alcCaptureOpenDevice(DeviceName.isEmpty() ? nullptr : DeviceName.toU8String().c_str(), DeviceFrequency, DeviceLayout.toOpenALFormat(), 0);
                if (!DeviceObject) throw MediaException(String(u"MediaCaptureDevice::MediaCaptureDevice(const String&) alcCaptureOpenDevice"));
            }

            ~MediaCaptureDevice() noexcept {
                if (DeviceObject) doClose();
            }

            void doCapture() {
                if (!DeviceObject) throw MediaException(String(u"MediaCaptureDevice::doCapture() DeviceObject"));
                alcGetIntegerv(DeviceObject, ALC_CAPTURE_SAMPLES, 0, nullptr);
                alcCaptureSamples(DeviceObject, nullptr, 0);
                if (alGetError() != AL_NO_ERROR) throw MediaException(String(u"MediaCaptureDevice::doCapture() alcCaptureStop"));
            }

            void doClose() {
                if (!DeviceObject) throw MediaException(String(u"MediaCaptureDevice::doClose() DeviceObject"));
                alcCaptureCloseDevice(DeviceObject);
                DeviceObject = nullptr;
            }

            void doStart() {
                if (!DeviceObject) throw MediaException(String(u"MediaCaptureDevice::doStart() DeviceObject"));
                alcCaptureStart(DeviceObject);
                if (alGetError() != AL_NO_ERROR) throw MediaException(String(u"MediaCaptureDevice::doStart() alcCaptureStart"));
            }

            void doStop() {
                if (!DeviceObject) throw MediaException(String(u"MediaCaptureDevice::doStop() DeviceObject"));
                alcCaptureStop(DeviceObject);
                if (alGetError() != AL_NO_ERROR) throw MediaException(String(u"MediaCaptureDevice::doStart() alcCaptureStop"));
            }
        };

        class MediaDevice final : public Object, public NonCopyable {
        private:
            ALCdevice *DeviceObject;

            friend class MediaContext;
        public:
            MediaDevice(const String &DeviceName) {
                DeviceObject = alcOpenDevice(DeviceName.isEmpty() ? nullptr : DeviceName.toU8String().c_str());
                if (!DeviceObject) throw MediaException(String(u"MediaDevice::MediaDevice(const String&) alcOpenDevice"));
            }

            ~MediaDevice() noexcept {
                if (DeviceObject) doClose();
            }

            void doClose() {
                if (!DeviceObject) throw MediaException(String(u"MediaDevice::doClose() DeviceObject"));
                alcCloseDevice(DeviceObject);
                DeviceObject = nullptr;
            }
        };

        class MediaContext final : public Object, public NonCopyable {
        private:
            ALCcontext *ContextObject;
        public:
            enum class MediaDistanceModel : ALenum {
                ModelExponent = AL_EXPONENT_DISTANCE,
                ModelExponentClamped = AL_EXPONENT_DISTANCE_CLAMPED,
                ModelInverse = AL_INVERSE_DISTANCE,
                ModelInverseClamped = AL_INVERSE_DISTANCE_CLAMPED,
                ModelLinear = AL_LINEAR_DISTANCE,
                ModelLinearClamped = AL_LINEAR_DISTANCE_CLAMPED,
                ModelNone = AL_NONE
            };

            MediaContext(const MediaDevice &ContextDevice) {
                ContextObject = alcCreateContext(ContextDevice.DeviceObject, nullptr);
                if (!ContextObject) throw MediaException(String(u"MediaContext::MediaContext(const MediaDevice&) alcCreateContext"));
            }

            ~MediaContext() noexcept {
                if (ContextObject) doDestroy();
            }

            void doDestroy() {
                if (!ContextObject) throw MediaException(String(u"MediaContext::doDestroy() ContextObject"));
                alcDestroyContext(ContextObject);
                ContextObject = nullptr;
            }

            void setContextCurrent() const {
                alcMakeContextCurrent(ContextObject);
            }

            static void setContextCurrentNull() noexcept {
                alcMakeContextCurrent(nullptr);
            }

            static void setDistanceModel(MediaDistanceModel ModelType) noexcept {
                alDistanceModel((ALenum) ModelType);
            }

            static void setDopplerFactor(float FactorValue) noexcept {
                alDopplerFactor(FactorValue);
            }

            static void setSoundVelocity(float VelocityValue) noexcept {
                alSpeedOfSound(VelocityValue);
            }
        };

        class MediaSource final : public Object, public NonCopyable {
        private:
            ALuint SourceIndex;
        public:
            MediaSource() {
                alGenSources(1, &SourceIndex);
                if (alGetError() != AL_NO_ERROR) throw MediaException(String(u"MediaSource::MediaSource() alGenSources"));
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

            void setSourceBuffer(const MediaBuffer &SourceBufferSource) const noexcept {
                alSourcei(SourceIndex, AL_BUFFER, (ALint) SourceBufferSource.BufferIndex);
            }

            void setSourceDirection(float DirectionX, float DirectionY, float DirectionZ) const {
                alSource3f(SourceIndex, AL_DIRECTION, DirectionX, DirectionY, DirectionZ);
                if (alGetError() != AL_NO_ERROR) throw MediaException(String(u"MediaSource::setSourceDirection(float, float, float) alSource3f"));
            }

            void setSourceDistanceMaximum(float OptionValue) const {
                alSourcef(SourceIndex, AL_MAX_DISTANCE, OptionValue);
                if (alGetError() != AL_NO_ERROR) throw MediaException(String(u"MediaSource::setSourceDistanceMaximum(float) alSourcef"));
            }

            void setSourceDistanceReference(float OptionValue) const {
                alSourcef(SourceIndex, AL_REFERENCE_DISTANCE, OptionValue);
                if (alGetError() != AL_NO_ERROR) throw MediaException(String(u"MediaSource::setSourceDistanceReference(float) alSourcef"));
            }

            void setSourceGain(float OptionValue) const {
                alSourcef(SourceIndex, AL_GAIN, OptionValue);
                if (alGetError() != AL_NO_ERROR) throw MediaException(String(u"MediaSource::setSourceGain(float) alSourcef"));
            }

            void setSourceGainMaximum(float OptionValue) const {
                alSourcef(SourceIndex, AL_MAX_GAIN, OptionValue);
                if (alGetError() != AL_NO_ERROR) throw MediaException(String(u"MediaSource::setSourceGainMaximum(float) alSourcef"));
            }

            void setSourceGainMinimum(float OptionValue) const {
                alSourcef(SourceIndex, AL_MIN_GAIN, OptionValue);
                if (alGetError() != AL_NO_ERROR) throw MediaException(String(u"MediaSource::setSourceGainMinimum(float) alSourcef"));
            }

            void setSourceLoop(bool OptionValue) const {
                alSourcei(SourceIndex, AL_LOOPING, OptionValue);
                if (alGetError() != AL_NO_ERROR) throw MediaException(String(u"MediaSource::setSourceLoop(bool) alSourcei"));
            }

            void setSourcePitch(float OptionValue) const {
                alSourcef(SourceIndex, AL_PITCH, OptionValue);
                if (alGetError() != AL_NO_ERROR) throw MediaException(String(u"MediaSource::setSourcePitch(float) alSourcef"));
            }

            void setSourceRelative(bool OptionValue) const {
                alSourcei(SourceIndex, AL_SOURCE_RELATIVE, OptionValue);
                if (alGetError() != AL_NO_ERROR) throw MediaException(String(u"MediaSource::setSourceRelative(bool) alSourcei"));
            }

            void setSourceRolloffFactor(float OptionValue) const {
                alSourcef(SourceIndex, AL_ROLLOFF_FACTOR, OptionValue);
                if (alGetError() != AL_NO_ERROR) throw MediaException(String(u"MediaSource::setSourceRolloffFactor(float) alSourcef"));
            }

            void setSourceVelocity(float VelocityX, float VelocityY, float VelocityZ) const {
                alSource3f(SourceIndex, AL_VELOCITY, VelocityX, VelocityY, VelocityZ);
                if (alGetError() != AL_NO_ERROR) throw MediaException(String(u"MediaSource::setSourceVelocity(float, float, float) alSource3f"));
            }
        };

#define doDestroyOpenAL() Multimedia::OpenAL::MediaContext::setContextCurrentNull();
#define doInitializeOpenAL() Multimedia::OpenAL::MediaDevice MediaDeviceObject({u""});Multimedia::OpenAL::MediaContext MediaContextObject(MediaDeviceObject);MediaContextObject.setContextCurrent();
    }
}
#endif

#endif
