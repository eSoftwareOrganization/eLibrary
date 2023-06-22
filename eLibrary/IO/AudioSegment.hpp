#pragma once

#include <IO/FFMpeg.hpp>

extern "C" {
#include <AL/alext.h>
#include <AL/efx.h>
}

namespace eLibrary::IO {
    class AudioBuffer final : public Object {
    private:
        ALuint AudioBufferIndex;
    public:
        AudioBuffer() noexcept {
            alGenBuffers(1, &AudioBufferIndex);
        }

        ~AudioBuffer() noexcept {
            alDeleteBuffers(1, &AudioBufferIndex);
        }

        ALuint getBufferIndex() const noexcept {
            return AudioBufferIndex;
        }

        void setBufferData(ALenum AudioBufferFormat, const void *AudioBufferData, ALsizei AudioBufferSize, ALsizei AudioSampleRate) const noexcept {
            alBufferData(AudioBufferIndex, AudioBufferFormat, AudioBufferData, AudioBufferSize, AudioSampleRate);
        }
    };

    class AudioSource final : public Object {
    private:
        AudioBuffer AudioBufferObject;
        ALuint AudioSourceIndex;
    public:
        AudioSource() {
            alGenSources(1, &AudioSourceIndex);
        }

        ~AudioSource() noexcept {
            alDeleteSources(1, &AudioSourceIndex);
        }

        void doPause() noexcept {
            alSourcePause(AudioSourceIndex);
        }

        void doPlay() noexcept {
            alSourcePlay(AudioSourceIndex);
        }

        void doRewind() noexcept {
            alSourceRewind(AudioSourceIndex);
        }

        void doStop() noexcept {
            alSourceStop(AudioSourceIndex);
        }

        void setAudioBuffer(const AudioBuffer &AudioBufferSource) noexcept {
            AudioBufferObject = AudioBufferSource;
            alSourcei(AudioSourceIndex, AL_BUFFER, AudioBufferObject.getBufferIndex());
        }

        void setAudioGain(float AudioGainSource) noexcept {
            alSourcef(AudioSourceIndex, AL_GAIN, AudioGainSource);
        }

        void setAudioLoop(bool AudioLoopSource) noexcept {
            alSourcef(AudioSourceIndex, AL_LOOPING, (ALboolean) AudioLoopSource);
        }

        void setAudioPitch(float AudioPitchSource) noexcept {
            alSourcef(AudioSourceIndex, AL_PITCH, AudioPitchSource);
        }
    };


#define getIntegerX(T, cp, i)  (*(T *)((unsigned char *)(cp) + (i)))
#define setIntegerX(T, cp, i, val)  do {                \
        *(T *)((unsigned char *)(cp) + (i)) = (T)(val); \
    } while (0)


#define getInteger8(cp, i)          getIntegerX(char8_t, (cp), (i))
#define getInteger16(cp, i)         getIntegerX(int16_t, (cp), (i))
#define getInteger32(cp, i)         getIntegerX(int32_t, (cp), (i))
#define setInteger8(cp, i, val)     setIntegerX(char8_t, (cp), (i), (val))
#define setInteger16(cp, i, val)    setIntegerX(int16_t, (cp), (i), (val))
#define setInteger32(cp, i, val)    setIntegerX(int32_t, (cp), (i), (val))

#define getRawSample(size, cp, i)  (                    \
        (size == 1) ? (int)getInteger8((cp), (i)) :     \
        (size == 2) ? (int)getInteger16((cp), (i)) :    \
                      (int)getInteger32((cp), (i)))

#define setRawSample(size, cp, i, val)  do {    \
        if (size == 1)                          \
            setInteger8((cp), (i), (val));      \
        else if (size == 2)                     \
            setInteger16((cp), (i), (val));     \
        else if (size == 4)                     \
            setInteger32((cp), (i), (val));     \
    } while(0)


#define getSample32(size, cp, i)  (                       \
        (size == 1) ? (int)getInteger8((cp), (i)) << 24 : \
        (size == 2) ? (int)getInteger16((cp), (i)) << 16 :\
                      (int)getInteger32((cp), (i)))

#define setSample32(size, cp, i, val)  do {      \
        if (size == 1)                           \
            setInteger8((cp), (i), (val) >> 24); \
        else if (size == 2)                      \
            setInteger16((cp), (i), (val) >> 16);\
        else if (size == 4)                      \
            setInteger32((cp), (i), (val));      \
    } while(0)

    class AudioSegment final : public Object {
    private:
        struct AudioFMTHeader final {
            uint32_t AudioFormatSize;
            uint16_t AudioFormatType;
            uint16_t AudioChannelCount;
            uint32_t AudioSampleRate;
            uint32_t AudioTransferRate;
            uint16_t AudioBlockAlignment;
            uint16_t AudioBitSample;
        };
        uint16_t AudioBitSample;
        uint16_t AudioChannelCount;
        uint8_t *AudioData;
        uint32_t AudioDataSize;
        uint32_t AudioSampleRate;

        AudioSegment(FILE *AudioOperator, uint16_t AudioBitSampleSource, uint16_t AudioChannelCountSource, uint32_t AudioSampleRateSource) noexcept : AudioBitSample(AudioBitSampleSource), AudioChannelCount(AudioChannelCountSource), AudioSampleRate(AudioSampleRateSource) {
            fread(&AudioDataSize, sizeof(uint32_t), 1, AudioOperator);
            AudioData = new uint8_t[AudioDataSize / sizeof(uint8_t)];
            fread(AudioData, sizeof(uint8_t), AudioDataSize / sizeof(uint8_t), AudioOperator);
            fclose(AudioOperator);
        }

        AudioSegment(uint8_t *AudioDataSource, uint32_t AudioDataSizeSource, uint16_t AudioBitSampleSource, uint16_t AudioChannelCountSource, uint32_t AudioSampleRateSource) noexcept : AudioBitSample(AudioBitSampleSource), AudioChannelCount(AudioChannelCountSource), AudioDataSize(AudioDataSizeSource), AudioSampleRate(AudioSampleRateSource) {
            AudioData = new uint8_t[AudioDataSize];
            memcpy(AudioData, AudioDataSource, AudioDataSize);
            delete[] AudioDataSource;
        }

        AudioSegment(const std::vector<uint8_t> &AudioDataSource, uint16_t AudioBitSampleSource, uint16_t AudioChannelCountSource, uint32_t AudioSampleRateSource) noexcept : AudioBitSample(AudioBitSampleSource), AudioChannelCount(AudioChannelCountSource), AudioSampleRate(AudioSampleRateSource) {
            AudioData = new uint8_t[AudioDataSize = AudioDataSource.size()];
            std::copy(AudioDataSource.begin(), AudioDataSource.end(), AudioData);
        }

        template<typename OperationExceptionHandleType>
        static int doHandleOperation(int OperationResult, const String &OperationMethod, OperationExceptionHandleType OperationExceptionHandle) {
            if (OperationResult < 0) {
                char OperationExceptionBuffer[256];
                av_strerror(OperationResult, OperationExceptionBuffer, 256);
                OperationExceptionHandle();
                throw IOException(String(u"AudioSegment::doHandleOperation<OperationExceptionHandleType>(int, const String&, OperationExceptionHandleType) ").doConcat(OperationMethod).doConcat(u':').doConcat({OperationExceptionBuffer}));
            }
            return OperationResult;
        }

        template<typename OperationReturnType, typename OperationExceptionHandleType>
        static OperationReturnType doHandleOperation(OperationReturnType OperationResult, const String &OperationMethod, OperationExceptionHandleType OperationExceptionHandle) {
            if (!OperationResult) {
                OperationExceptionHandle();
                throw IOException(String(u"AudioSegment::doHandleOperation<OperationReturnType, OperationExceptionHandleType>(OperationReturnType, const String&, OperationExceptionHandleType) ").doConcat(OperationMethod));
            }
            return OperationResult;
        }
    public:
        constexpr AudioSegment() noexcept : AudioBitSample(1), AudioChannelCount(1), AudioDataSize(1), AudioSampleRate(1) {
            AudioData = new uint8_t;
        }

        AudioSegment(const AudioSegment &AudioSource) noexcept {
            doAssign(AudioSource);
        }

        ~AudioSegment() noexcept {
            if (AudioData && AudioDataSize) {
                AudioDataSize = 0;
                delete[] AudioData;
                AudioData = nullptr;
            }
        }

        void doAssign(const AudioSegment &AudioSource) noexcept {
            if (std::addressof(AudioSource) == this) return;
            delete[] AudioData;
            AudioData = new uint8_t[AudioDataSize = AudioSource.AudioDataSize];
            memcpy(AudioData, AudioSource.AudioData, AudioDataSize);
            AudioBitSample = AudioSource.AudioBitSample;
            AudioChannelCount = AudioSource.AudioChannelCount;
            AudioSampleRate = AudioSource.AudioSampleRate;
        }

        void doExport(const String &AudioPath) const {
            MediaIOContext *AudioIOContext = MediaIOContext::doOpen(AudioPath, AVIO_FLAG_WRITE);
            MediaFormatContext *AudioFormatContext = MediaFormatContext::doAllocate();
            AudioFormatContext->setIOContext(AudioIOContext);
            AudioFormatContext->setOutputFormat(av_guess_format(nullptr, AudioPath.toU8String().c_str(), nullptr));
            std::string AudioPathStd = AudioPath.toU8String();
            if (!(AudioFormatContext->getContextObject()->url = av_strdup(AudioPathStd.c_str())))
                throw IOException(String(u"AudioSegment::doExport(const String&) av_strdup"));
            MediaCodec *AudioCodec = MediaCodec::doFindEncoder(av_guess_codec(AudioFormatContext->getContextObject()->oformat, nullptr, AudioPathStd.c_str(), nullptr, AVMEDIA_TYPE_AUDIO));
            MediaCodecContext *AudioCodecContext = MediaCodecContext::doAllocate(AudioCodec);
            AVStream *AudioStreamObject;
            if (!(AudioStreamObject = avformat_new_stream(AudioFormatContext->getContextObject(), nullptr)))
                throw IOException(String(u"AudioSegment::doExport(const String&) avformat_new_stream"));
            AudioStreamObject->time_base.den = AudioSampleRate;
            AudioStreamObject->time_base.num = 1;
            AudioCodecContext->getContextObject()->bit_rate = AudioChannelCount * AudioSampleRate * AudioBitSample / 8;
            AudioCodecContext->getContextObject()->channels = AudioChannelCount;
            AudioCodecContext->getContextObject()->channel_layout = av_get_default_channel_layout(AudioChannelCount);
            AudioCodecContext->getContextObject()->sample_fmt = AudioCodec->getCodecObject()->sample_fmts[0];
            AudioCodecContext->getContextObject()->sample_rate = AudioSampleRate;
            if (AudioFormatContext->getContextObject()->oformat->flags & AVFMT_GLOBALHEADER)
                AudioCodecContext->getContextObject()->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
            if (avcodec_open2(AudioCodecContext->getContextObject(), AudioCodec->getCodecObject(), nullptr))
                throw IOException(String(u"AudioSegment::doExport(const String&) avcodec_open2"));
            if (avcodec_parameters_from_context(AudioStreamObject->codecpar, AudioCodecContext->getContextObject()))
                throw IOException(String(u"AudioSegment::doExport(const String&) avcodec_parameters_from_context"));
            SwrContext *AudioSWRContext;
            if (!(AudioSWRContext = swr_alloc_set_opts(nullptr, AudioCodecContext->getContextObject()->channel_layout, AudioCodecContext->getContextObject()->sample_fmt, AudioSampleRate, AudioCodecContext->getContextObject()->channel_layout, AV_SAMPLE_FMT_U8, AudioSampleRate, 0, nullptr)))
                throw IOException(String(u"AudioSegment::doExport(const String&) swr_alloc_set_opts"));
            if (swr_init(AudioSWRContext) < 0) {
                swr_free(&AudioSWRContext);
                throw IOException(String(u"AudioSegment::doExport(const String&) swr_init"));
            }
            if (avformat_write_header(AudioFormatContext->getContextObject(), nullptr) < 0) {
                swr_free(&AudioSWRContext);
                throw IOException(String(u"AudioSegment::doExport(const String&) avformat_write_header"));
            }
            MediaFrame *AudioFrame = MediaFrame::doAllocate();
            if (AudioCodecContext->getContextObject()->frame_size <= 0) AudioCodecContext->getContextObject()->frame_size = 2048;
            AudioFrame->getFrameObject()->ch_layout = AudioCodecContext->getContextObject()->ch_layout;
            AudioFrame->getFrameObject()->format = AudioCodecContext->getContextObject()->sample_fmt;
            AudioFrame->getFrameObject()->nb_samples = AudioCodecContext->getContextObject()->frame_size;
            AudioFrame->getFrameObject()->sample_rate = AudioCodecContext->getSampleRate();
            if (av_frame_get_buffer(AudioFrame->getFrameObject(), 0))
                throw IOException(String(u"AudioSegment::doExport(const String&) av_frame_get_buffer"));
            MediaPacket *AudioPacket = MediaPacket::doAllocate();
            int AudioSampleCurrent = 0;
            uint8_t AudioDataSample[AudioChannelCount * AudioCodecContext->getContextObject()->frame_size];
            for (;;) {
                if (AudioSampleCurrent < AudioDataSize / AudioChannelCount) {
                    int AudioFrameSize = std::min(AudioCodecContext->getContextObject()->frame_size, int(AudioDataSize / AudioChannelCount - AudioSampleCurrent));
                    AudioFrame->getFrameObject()->nb_samples = AudioFrameSize;
                    AudioFrame->getFrameObject()->pts = AudioSampleCurrent;
                    ::memcpy(AudioDataSample, AudioData + (AudioSampleCurrent * 2), AudioFrameSize);
                    AudioSampleCurrent += AudioFrameSize;
                    if (swr_convert(AudioSWRContext, AudioFrame->getFrameObject()->extended_data, AudioFrameSize, (const uint8_t**) &AudioDataSample, AudioFrameSize) < 0) {
                        swr_free(&AudioSWRContext);
                        throw IOException(String(u"AudioSegment::doExport(const String&) swr_convert"));
                    }
                } else AudioFrame->~MediaFrame();
                if (avcodec_send_frame(AudioCodecContext->getContextObject(), AudioFrame->getFrameObject()) < 0) {
                    swr_free(&AudioSWRContext);
                    throw IOException(String(u"AudioSegment::doExport(const String&) avcodec_send_frame"));
                }
                int AudioStatus;
                while ((AudioStatus = avcodec_receive_packet(AudioCodecContext->getContextObject(), AudioPacket->getPacketObject())) == 0) {
                    if (av_write_frame(AudioFormatContext->getContextObject(), AudioPacket->getPacketObject()) < 0) {
                        swr_free(&AudioSWRContext);
                        throw IOException(String(u"AudioSegment::doExport(const String&) av_write_frame"));
                    }
                }
                if (AudioStatus == AVERROR_EOF) break;
                else if (AudioStatus != AVERROR(EAGAIN)) {
                    swr_free(&AudioSWRContext);
                    throw IOException(String(u"AudioSegment::doExport(const String&) avcodec_receive_packet"));
                }
            }
            swr_free(&AudioSWRContext);
            if (av_write_trailer(AudioFormatContext->getContextObject()) < 0)
                throw IOException(String(u"AudioSegment::doExport(const String&) av_write_trailer"));
        }

        void doExportWAV(const String &AudioPath) const {
            FILE *AudioOperator = fopen(AudioPath.toU8String().c_str(), "wb");
            fwrite("RIFF", sizeof(uint8_t), 4, AudioOperator);
            uint8_t AudioFormatBuffer[4];
            *((uint32_t*) AudioFormatBuffer) = AudioDataSize + 14;
            fwrite(AudioFormatBuffer, sizeof(uint8_t), 4, AudioOperator);
            fwrite("WAVE", sizeof(uint8_t), 4, AudioOperator);
            fwrite("fmt ", sizeof(uint8_t), 4, AudioOperator);
            *((uint32_t*) AudioFormatBuffer) = 16;
            fwrite(AudioFormatBuffer, sizeof(uint8_t), 4, AudioOperator);
            *((uint16_t*) AudioFormatBuffer) = 1;
            fwrite(AudioFormatBuffer, sizeof(uint8_t), 2, AudioOperator);
            *((uint16_t*) AudioFormatBuffer) = AudioChannelCount;
            fwrite(AudioFormatBuffer, sizeof(uint8_t), 2, AudioOperator);
            *((uint32_t*) AudioFormatBuffer) = AudioSampleRate;
            fwrite(AudioFormatBuffer, sizeof(uint8_t), 4, AudioOperator);
            *((uint32_t*) AudioFormatBuffer) = AudioChannelCount * AudioSampleRate * AudioBitSample / 8;
            fwrite(AudioFormatBuffer, sizeof(uint8_t), 4, AudioOperator);
            *((uint16_t*) AudioFormatBuffer) = AudioChannelCount * AudioBitSample / 8;
            fwrite(AudioFormatBuffer, sizeof(uint8_t), 2, AudioOperator);
            *((uint16_t*) AudioFormatBuffer) = AudioBitSample;
            fwrite(AudioFormatBuffer, sizeof(uint8_t), 2, AudioOperator);
            fwrite("data", sizeof(uint8_t), 4, AudioOperator);
            *((uint32_t*) AudioFormatBuffer) = AudioDataSize;
            fwrite(AudioFormatBuffer, sizeof(uint8_t), 4, AudioOperator);
            fwrite(AudioData, sizeof(uint8_t), AudioDataSize / sizeof(uint8_t), AudioOperator);
        }

        static AudioSegment doOpen(const String &AudioPath) {
            int MediaOperationResult;
            AVPacket MediaPacketObject;
            av_init_packet(&MediaPacketObject);
            MediaPacketObject.data = nullptr;
            MediaPacketObject.size = 0;
            AVFormatContext *MediaFormatContextObject = nullptr;
            doHandleOperation(avformat_open_input(&MediaFormatContextObject, AudioPath.toU8String().c_str(), nullptr, nullptr), {u"avformat_open_input"}, []{});
            doHandleOperation(avformat_find_stream_info(MediaFormatContextObject, nullptr), {u"avformat_find_stream_info"}, [&]{
                avformat_close_input(&MediaFormatContextObject);
            });
            const AVCodec *MediaCodecObject = nullptr;
            int MediaStreamIndex = doHandleOperation(av_find_best_stream(MediaFormatContextObject, AVMEDIA_TYPE_AUDIO, -1, -1, &MediaCodecObject, 0), {u"av_find_best_stream"}, [&]{
                avformat_close_input(&MediaFormatContextObject);
            });
            AVCodecContext *MediaCodecContextObject = doHandleOperation(avcodec_alloc_context3(MediaCodecObject), {u"avcodec_alloc_context3"}, [&]{
                avformat_close_input(&MediaFormatContextObject);
            });
            doHandleOperation(avcodec_parameters_to_context(MediaCodecContextObject, MediaFormatContextObject->streams[MediaStreamIndex]->codecpar), {u"avcodec_parameters_to_context"}, [&] {
                avcodec_close(MediaCodecContextObject);
                avcodec_free_context(&MediaCodecContextObject);
                avformat_close_input(&MediaFormatContextObject);
            });
            doHandleOperation(avcodec_open2(MediaCodecContextObject, MediaCodecObject, nullptr), {u"avcodec_open2"}, [&]{
                avcodec_close(MediaCodecContextObject);
                avcodec_free_context(&MediaCodecContextObject);
                avformat_close_input(&MediaFormatContextObject);
            });
            MediaCodecContextObject -> channel_layout = MediaCodecContextObject -> channel_layout || av_get_default_channel_layout(MediaCodecContextObject -> channels);
            doHandleOperation(-(MediaCodecContextObject->sample_rate <= 0), {u"MediaCodecContextObject->sample_rate"}, [&]{
                avcodec_close(MediaCodecContextObject);
                avcodec_free_context(&MediaCodecContextObject);
                avformat_close_input(&MediaFormatContextObject);
            });
            SwrContext *MediaSWRContext = doHandleOperation(swr_alloc_set_opts(nullptr, MediaCodecContextObject->channel_layout, AV_SAMPLE_FMT_U8, MediaCodecContextObject->sample_rate, MediaCodecContextObject->channel_layout, MediaCodecContextObject->sample_fmt, MediaCodecContextObject->sample_rate, 0, nullptr), {u"swr_alloc_set_opts"}, [&]{
                avcodec_close(MediaCodecContextObject);
                avcodec_free_context(&MediaCodecContextObject);
                avformat_close_input(&MediaFormatContextObject);
            });
            doHandleOperation(swr_init(MediaSWRContext), {u"swr_init"}, [&]{
                swr_free(&MediaSWRContext);
                avcodec_close(MediaCodecContextObject);
                avcodec_free_context(&MediaCodecContextObject);
                avformat_close_input(&MediaFormatContextObject);
            });
            AVFrame *MediaFrameObject = doHandleOperation(av_frame_alloc(), {u"av_frame_alloc"}, [&]{
                swr_free(&MediaSWRContext);
                avcodec_close(MediaCodecContextObject);
                avcodec_free_context(&MediaCodecContextObject);
                avformat_close_input(&MediaFormatContextObject);
            });
            std::vector<uint8_t> AudioDataOutput;
            int MediaSampleCurrent = 0;
            while (MediaSampleCurrent < std::floor((MediaFormatContextObject -> duration) / (double) AV_TIME_BASE * MediaCodecContextObject -> sample_rate)) {
                MediaOperationResult = av_read_frame(MediaFormatContextObject, &MediaPacketObject);
                if (MediaOperationResult == AVERROR_EOF) {
                    break;
                } else if (MediaOperationResult < 0) doHandleOperation(MediaOperationResult, {u"av_read_frame"}, [&]{
                        av_frame_free(&MediaFrameObject);
                        av_packet_unref(&MediaPacketObject);
                        swr_free(&MediaSWRContext);
                        avcodec_close(MediaCodecContextObject);
                        avcodec_free_context(&MediaCodecContextObject);
                        avformat_close_input(&MediaFormatContextObject);
                    });
                if (MediaPacketObject.stream_index != MediaStreamIndex) continue;
                doHandleOperation(avcodec_send_packet(MediaCodecContextObject, &MediaPacketObject), {u"avcodec_send_packet"}, [&]{
                    av_frame_free(&MediaFrameObject);
                    av_packet_unref(&MediaPacketObject);
                    swr_free(&MediaSWRContext);
                    avcodec_close(MediaCodecContextObject);
                    avcodec_free_context(&MediaCodecContextObject);
                    avformat_close_input(&MediaFormatContextObject);
                });
                while ((MediaOperationResult = avcodec_receive_frame(MediaCodecContextObject, MediaFrameObject)) == 0) {
                    uint8_t audio_data[MediaCodecContextObject->channels * MediaFrameObject -> nb_samples];
                    uint8_t * audio_data_ = reinterpret_cast<uint8_t *>(audio_data);
                    doHandleOperation(swr_convert(MediaSWRContext, (uint8_t**) &audio_data_, MediaFrameObject->nb_samples, (const uint8_t**) MediaFrameObject->extended_data, MediaFrameObject->nb_samples), {u"swr_convert"}, [&]{
                        av_frame_free(&MediaFrameObject);
                        av_packet_unref(&MediaPacketObject);
                        swr_free(&MediaSWRContext);
                        avcodec_close(MediaCodecContextObject);
                        avcodec_free_context(&MediaCodecContextObject);
                        avformat_close_input(&MediaFormatContextObject);
                    });
                    for (int s = 0; s < MediaFrameObject -> nb_samples; s++)
                        for (int channel = 0; channel < MediaCodecContextObject->channels; channel++)
                            AudioDataOutput.push_back(audio_data[MediaCodecContextObject->channels * s + channel]);
                    MediaSampleCurrent += MediaFrameObject -> nb_samples;
                }
                if (MediaOperationResult != AVERROR(EAGAIN))
                    doHandleOperation(MediaOperationResult, {u"avcodec_receive_frame"}, [&]{
                        av_frame_free(&MediaFrameObject);
                        av_packet_unref(&MediaPacketObject);
                        swr_free(&MediaSWRContext);
                        avcodec_close(MediaCodecContextObject);
                        avcodec_free_context(&MediaCodecContextObject);
                        avformat_close_input(&MediaFormatContextObject);
                    });
            }
            uint16_t AudioBitSampleSource = av_get_bytes_per_sample(MediaCodecContextObject->sample_fmt) << 3, AudioChannelCountSource = MediaCodecContextObject->channels;
            uint32_t AudioSampleRateSource = MediaCodecContextObject->sample_rate;
            if (MediaCodecContextObject->sample_fmt == AV_SAMPLE_FMT_FLTP && (strcmp(avcodec_get_name(MediaCodecContextObject->codec_id), "mp3") == 0 || strcmp(avcodec_get_name(MediaCodecContextObject->codec_id), "vorbis") == 0)) AudioBitSampleSource = 16;
            av_frame_free(&MediaFrameObject);
            av_packet_unref(&MediaPacketObject);
            swr_free(&MediaSWRContext);
            avcodec_close(MediaCodecContextObject);
            avcodec_free_context(&MediaCodecContextObject);
            avformat_close_input(&MediaFormatContextObject);
            return {AudioDataOutput, AudioBitSampleSource, AudioChannelCountSource, AudioSampleRateSource};
        }

        static AudioSegment doOpenWAV(const String &AudioPath) {
            FILE *AudioOperator = fopen(AudioPath.toU8String().c_str(), "rb");
            if (!AudioOperator) throw IOException(String(u"AudioSegment::doOpenWAV(const String&) fopen"));
            uint8_t AudioFormatBuffer[4];
            fread(AudioFormatBuffer, sizeof(uint8_t), 4, AudioOperator);
            if (AudioFormatBuffer[0] != 'R' || AudioFormatBuffer[1] != 'I' || AudioFormatBuffer[2] != 'F' || AudioFormatBuffer[3] != 'F') {
                fclose(AudioOperator);
                throw IOException(String(u"AudioSegment::doOpenWAV(const String&) AudioFormatBuffer"));
            }
            uint32_t AudioFileSize;
            fread(&AudioFileSize, sizeof(uint32_t), 1, AudioOperator);
            fread(AudioFormatBuffer, sizeof(uint8_t), 4, AudioOperator);
            if (AudioFormatBuffer[0] != 'W' || AudioFormatBuffer[1] != 'A' || AudioFormatBuffer[2] != 'V' || AudioFormatBuffer[3] != 'E') {
                fclose(AudioOperator);
                throw IOException(String(u"AudioSegment::doOpenWAV(const String&) AudioFormatBuffer"));
            }
            fread(AudioFormatBuffer, sizeof(uint8_t), 4, AudioOperator);
            if (AudioFormatBuffer[0] != 'f' || AudioFormatBuffer[1] != 'm' || AudioFormatBuffer[2] != 't' || AudioFormatBuffer[3] != ' ') {
                fclose(AudioOperator);
                throw IOException(String(u"AudioSegment::doOpenWAV(const String&) AudioFormatBuffer"));
            }
            AudioFMTHeader AudioHeader;
            fread(&AudioHeader, sizeof(AudioFMTHeader), 1, AudioOperator);
            if (AudioHeader.AudioTransferRate != AudioHeader.AudioChannelCount * AudioHeader.AudioSampleRate * AudioHeader.AudioBitSample / 8 || AudioHeader.AudioBlockAlignment != AudioHeader.AudioChannelCount * AudioHeader.AudioBitSample / 8 || (AudioHeader.AudioBitSample != 8 && AudioHeader.AudioBitSample != 16 && AudioHeader.AudioBitSample != 32)) {
                fclose(AudioOperator);
                throw IOException(String(u"AudioSegment::doOpenWAV(const String&) AudioTransferRate || AudioBlockAlignment || AudioBitSample"));
            }
            while (!feof(AudioOperator) && fgetc(AudioOperator) != 'd');
            if (feof(AudioOperator)) throw IOException(String(u"AudioSegment::doOpenWAV(const String&) feof"));
            fread(AudioFormatBuffer, sizeof(uint8_t), 3, AudioOperator);
            if (AudioFormatBuffer[0] != 'a' || AudioFormatBuffer[1] != 't' || AudioFormatBuffer[2] != 'a') {
                fclose(AudioOperator);
                throw IOException(String(u"AudioSegment::doOpenWAV(const String&) AudioFormatBuffer"));
            }
            return {AudioOperator, AudioHeader.AudioBitSample, AudioHeader.AudioChannelCount, AudioHeader.AudioSampleRate};
        }

        ArrayList<AudioSegment> doSplitChannel() const noexcept {
            if (AudioChannelCount == 1) return {*this};
            ArrayList<AudioSegment> AudioResultList;
            for (uint16_t AudioChannel = 0;AudioChannel < AudioChannelCount;++AudioChannel) {
                std::vector<uint8_t> AudioDataResult;
                for (uint16_t AudioSample = AudioChannel;AudioSample < AudioDataSize;AudioSample += AudioChannelCount)
                    AudioDataResult.push_back(AudioData[AudioSample]);
                AudioResultList.addElement({AudioDataResult, AudioBitSample, 1, AudioSampleRate});
            }
            return AudioResultList;
        }

        static ArrayList<AudioSegment> doSynchronize(const std::vector<AudioSegment> &AudioList) {
            ArrayList<AudioSegment> AudioResultList;
            uint16_t AudioBitSampleMaximum = std::max_element(AudioList.begin(), AudioList.end(), [](const AudioSegment &Audio1, const AudioSegment &Audio2) {
                return Audio1.getBitSample() > Audio2.getBitSample();
            })->getBitSample(), AudioChannelCountMaximum = std::max_element(AudioList.begin(), AudioList.end(), [](const AudioSegment &Audio1, const AudioSegment &Audio2) {
                return Audio1.getChannelCount() > Audio2.getChannelCount();
            })->getChannelCount();
            uint32_t AudioSampleRateMaximum = std::max_element(AudioList.begin(), AudioList.end(), [](const AudioSegment &Audio1, const AudioSegment &Audio2) {
                return Audio1.getSampleRate() > Audio2.getSampleRate();
            })->getSampleRate();
            std::for_each(AudioList.begin(), AudioList.end(), [&](const AudioSegment &SegmentCurrent) {
                AudioResultList.addElement(SegmentCurrent.setBitSample(AudioBitSampleMaximum).setChannelCount(AudioChannelCountMaximum).setSampleRate(AudioSampleRateMaximum));
            });
            return AudioResultList;
        }

        uint16_t getBitSample() const noexcept {
            return AudioBitSample;
        }

        uint16_t getChannelCount() const noexcept {
            return AudioChannelCount;
        }

        uint32_t getSampleRate() const noexcept {
            return AudioSampleRate;
        }

        AudioSegment &operator=(const AudioSegment &SegmentSource) noexcept {
            doAssign(SegmentSource);
            return *this;
        }

        AudioSegment setBitSample(uint16_t AudioBitSampleSource) const {
            if (AudioBitSample == AudioBitSampleSource) return *this;
            if (AudioBitSampleSource != 8 && AudioBitSampleSource != 16 && AudioBitSampleSource != 32) throw IOException(String(u"AudioSegment::setBitSample(uint16_t) AudioBitSampleSource"));
            auto *AudioDataResult = new uint8_t[AudioDataSize / AudioBitSample * AudioBitSampleSource];
            uint32_t AudioSample1, AudioSample2;
            for (AudioSample1 = AudioSample2 = 0;AudioSample1 < AudioDataSize;AudioSample1 += AudioBitSample, AudioSample2 += AudioBitSampleSource) {
                int32_t AudioSampleValue = getSample32(AudioBitSample / 8, AudioData, AudioSample1);
                setSample32(AudioBitSampleSource / 8, AudioDataResult, AudioSample2, AudioSampleValue);
            }
            return {AudioDataResult, AudioDataSize / AudioBitSample * AudioBitSampleSource, AudioBitSampleSource, AudioChannelCount, AudioSampleRate};
        }

        AudioSegment setChannelCount(uint16_t AudioChannelCountSource) const {
            if (AudioChannelCount == AudioChannelCountSource) return *this;
            throw IOException(String(u"AudioSegment::setChannelCount(uint16_t) AudioChannelCount != AudioChannelCountSource"));
        }

        AudioSegment setSampleRate(uint32_t AudioSampleRateSource) const {
            if (AudioSampleRate == AudioSampleRateSource) return *this;
            throw IOException(String(u"AudioSegment::setSampleRate(uint16_t) AudioSampleRate != AudioSampleRateSource"));
        }

        AudioBuffer toAudioBuffer() const {
            if ((AudioBitSample != 8 && AudioBitSample != 16) || (AudioChannelCount != 1 && AudioChannelCount != 2))
                throw IOException(String(u"AudioSegment::toAudioBuffer() AudioBitSample || AudioChannelCount"));
            AudioBuffer AudioBufferResult;
            AudioBufferResult.setBufferData(AudioBitSample == 16 ? (AudioChannelCount == 1 ? AL_FORMAT_MONO16 : AL_FORMAT_STEREO16) : (AudioChannelCount == 1 ? AL_FORMAT_MONO8 : AL_FORMAT_STEREO8), AudioData, AudioDataSize, AudioSampleRate);
            return AudioBufferResult;
        }
    };

    class OpenALInitializer final : public Object {
    private:
        static ALCcontext *AudioContext;
        static ALCdevice *AudioDevice;
    public:
        static void doDestroy() noexcept {
            alcMakeContextCurrent(nullptr);
            alcDestroyContext(AudioContext);
            alcCloseDevice(AudioDevice);
        }

        static void doInitialize() noexcept {
            AudioDevice = alcOpenDevice(nullptr);
            AudioContext = alcCreateContext(AudioDevice, nullptr);
            alcMakeContextCurrent(AudioContext);
        }

        constexpr OpenALInitializer &operator=(const OpenALInitializer&) noexcept = delete;
    };

    ALCcontext *OpenALInitializer::AudioContext = nullptr;
    ALCdevice *OpenALInitializer::AudioDevice = nullptr;
}
