#pragma once

#if eLibraryFeature(Multimedia)

#include <Multimedia/AudioUtility.hpp>

namespace eLibrary::Multimedia {
    class MediaChannelLayout final {
    private:
        uint8_t LayoutChannelCount;
        uint32_t LayoutChannelMask;
    public:
        constexpr MediaChannelLayout(uint8_t LayoutChannelCountSource, uint32_t LayoutChannelMaskSource) noexcept : LayoutChannelCount(LayoutChannelCountSource), LayoutChannelMask(LayoutChannelMaskSource) {}

        constexpr MediaChannelLayout(const AVChannelLayout &LayoutSource) noexcept : LayoutChannelCount(LayoutSource.nb_channels), LayoutChannelMask(LayoutSource.u.mask) {
            if (!LayoutChannelMask) LayoutChannelMask = AV_CH_LAYOUT_STEREO;
        }

        uint8_t getChannelCount() const noexcept {
            return LayoutChannelCount;
        }

        uint32_t getChannelMask() const noexcept {
            return LayoutChannelMask;
        }

        AVChannelLayout toFFMpegFormat() const noexcept {
            return AV_CHANNEL_LAYOUT_MASK(LayoutChannelCount, LayoutChannelMask);
        }

        auto toOpenALFormat() const noexcept {
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
            std::unreachable();
        }
    };

    static MediaChannelLayout Layout51{6, AV_CH_LAYOUT_5POINT1};
    static MediaChannelLayout Layout61{7, AV_CH_LAYOUT_6POINT1};
    static MediaChannelLayout Layout71{8, AV_CH_LAYOUT_7POINT1};
    static MediaChannelLayout LayoutMono{1, AV_CH_LAYOUT_MONO};
    static MediaChannelLayout LayoutQuad{4, AV_CH_LAYOUT_QUAD};
    static MediaChannelLayout LayoutStereo{2, AV_CH_LAYOUT_STEREO};

    class AudioSegment final : public Object {
    private:
        MediaChannelLayout AudioChannelLayout;
        uint8_t **AudioData;
        uint32_t AudioDataSize;
        uint32_t AudioSampleRate;

        AudioSegment(const std::vector<std::vector<uint8_t>> &AudioDataSource, const MediaChannelLayout &AudioChannelLayoutSource, uint32_t AudioSampleRateSource) : AudioChannelLayout(AudioChannelLayoutSource), AudioDataSize(AudioDataSource[0].size()), AudioSampleRate(AudioSampleRateSource) {
            AudioData = MemoryAllocator::newArray<uint8_t*>(AudioDataSource.size());
            for (uint8_t AudioChannel = 0;AudioChannel < AudioChannelLayout.getChannelCount();++AudioChannel) {
                AudioData[AudioChannel] = MemoryAllocator::newArray<uint8_t>(AudioDataSource[AudioChannel].size());
                memcpy(AudioData[AudioChannel], AudioDataSource[AudioChannel].data(), sizeof(uint8_t) * AudioDataSource[AudioChannel].size());
            }
        }

        AudioSegment(uint8_t **AudioDataSource, uint32_t AudioDataSourceSize, const MediaChannelLayout &AudioChannelLayoutSource, uint32_t AudioSampleRateSource) : AudioChannelLayout(AudioChannelLayoutSource), AudioDataSize(AudioDataSourceSize), AudioSampleRate(AudioSampleRateSource) {
            AudioData = MemoryAllocator::newArray<uint8_t*>(AudioChannelLayoutSource.getChannelCount());
            for (uint8_t AudioChannel = 0;AudioChannel < AudioChannelLayout.getChannelCount();++AudioChannel) {
                AudioData[AudioChannel] = MemoryAllocator::newArray<uint8_t>(AudioDataSourceSize);
                memcpy(AudioData[AudioChannel], AudioDataSource[AudioChannel], sizeof(uint8_t) * AudioDataSourceSize);
            }
            for (uint8_t AudioChannel = 0;AudioChannel < AudioChannelLayout.getChannelCount();++AudioChannel)
                delete[] AudioDataSource[AudioChannel];
            delete[] AudioDataSource;
        }
    public:
        AudioSegment(const AudioSegment &AudioSource) : AudioChannelLayout(AudioSource.AudioChannelLayout), AudioDataSize(AudioSource.AudioDataSize), AudioSampleRate(AudioSource.AudioSampleRate) {
            AudioData = MemoryAllocator::newArray<uint8_t*>(AudioChannelLayout.getChannelCount());
            for (uint8_t AudioChannel = 0;AudioChannel < AudioChannelLayout.getChannelCount();++AudioChannel) {
                AudioData[AudioChannel] = MemoryAllocator::newArray<uint8_t>(AudioDataSize);
                memcpy(AudioData[AudioChannel], AudioSource.AudioData[AudioChannel], sizeof(uint8_t) * AudioDataSize);
            }
        }

        ~AudioSegment() noexcept {
            if (AudioData) {
                for (uint8_t AudioChannel = 0;AudioChannel < AudioChannelLayout.getChannelCount();++AudioChannel)
                    delete[] AudioData[AudioChannel];
                delete[] AudioData;
                AudioData = nullptr;
            }
        }

        void doAssign(const AudioSegment &AudioSource) {
            if (Objects::getAddress(AudioSource) == this) return;
            if (AudioData) {
                for (uint8_t AudioChannel = 0; AudioChannel < AudioChannelLayout.getChannelCount(); ++AudioChannel)
                    delete[] AudioData[AudioChannel];
                delete[] AudioData;
            }
            AudioData = MemoryAllocator::newArray<uint8_t*>((AudioChannelLayout = AudioSource.AudioChannelLayout).getChannelCount());
            AudioDataSize = AudioSource.AudioDataSize;
            for (uint8_t AudioChannel = 0;AudioChannel < AudioChannelLayout.getChannelCount();++AudioChannel) {
                AudioData[AudioChannel] = MemoryAllocator::newArray<uint8_t>(AudioDataSize);
                memcpy(AudioData[AudioChannel], AudioSource.AudioData[AudioChannel], sizeof(uint8_t) * AudioDataSize);
            }
        }

        void doExport(const String &AudioPath) const {
            MediaFormatContext AudioFormatContext(MediaFormatContext::doAllocateOutput(AudioPath));
            if (avio_open(&AudioFormatContext->pb, AudioPath.toU8String().c_str(), AVIO_FLAG_WRITE) < 0)
                throw MediaException(String(u"AudioSegment::doExport(const String&) avio_open"));
            MediaCodec AudioCodec(MediaCodec::doFindEncoder(AudioFormatContext->oformat->audio_codec));
            MediaCodecContext AudioCodecContext(MediaCodecContext::doAllocate(AudioCodec));
            AVChannelLayout AudioChannelLayoutSource(AudioChannelLayout.toFFMpegFormat());
            av_channel_layout_copy(&AudioCodecContext->ch_layout, &AudioChannelLayoutSource);
            AudioCodecContext->sample_fmt = AudioCodec->sample_fmts[0];
            AudioCodecContext->sample_rate = AudioSampleRate;
            AVStream *AudioStreamObject;
            AudioCodecContext.doOpen(AudioCodec);
            if (!(AudioStreamObject = avformat_new_stream((AVFormatContext*) AudioFormatContext, (const AVCodec*) AudioCodec)))
                throw MediaException(String(u"AudioSegment::doExport(const String&) avformat_new_stream"));
            if (avcodec_parameters_from_context(AudioStreamObject->codecpar, (AVCodecContext*) AudioCodecContext))
                throw MediaException(String(u"AudioSegment::doExport(const String&) avcodec_parameters_from_context"));
            AudioFormatContext.doWriteHeader();
            MediaSWRContext AudioSWRContext(MediaSWRContext::doAllocate(&AudioCodecContext->ch_layout, &AudioCodecContext->ch_layout, AV_SAMPLE_FMT_U8, AudioCodecContext->sample_fmt, AudioCodecContext->sample_rate, AudioCodecContext->sample_rate));
            AudioSWRContext.doInitialize();
            MediaFrame AudioFrame(MediaFrame::doAllocate());
            if (AudioCodecContext->frame_size <= 0) AudioCodecContext->frame_size = 2048;
            av_channel_layout_copy(&AudioFrame->ch_layout, &AudioCodecContext->ch_layout);
            AudioFrame->format = AudioCodecContext->sample_fmt;
            AudioFrame->nb_samples = AudioCodecContext->frame_size;
            AudioFrame->sample_rate = AudioCodecContext->sample_rate;
            AudioFrame.getFrameBuffer();
            MediaPacket AudioPacket(MediaPacket::doAllocate());
            uint32_t AudioSampleCurrent = 0;
            uint8_t AudioDataSample[AudioChannelLayout.getChannelCount() * AudioCodecContext->frame_size];
            for (;;) {
                if (AudioSampleCurrent < AudioDataSize) {
                    uint32_t AudioFrameSize = Objects::getMinimum(uint32_t(AudioCodecContext->frame_size), uint32_t(AudioDataSize - AudioSampleCurrent));
                    AudioFrame->nb_samples = AudioFrameSize;
                    AudioFrame->pts = AudioSampleCurrent;
                    for (uint32_t AudioSample = 0;AudioSample < AudioFrameSize;++AudioSample)
                        for (uint8_t AudioChannel = 0;AudioChannel < AudioChannelLayout.getChannelCount();++AudioChannel)
                            AudioDataSample[AudioChannelLayout.getChannelCount() * AudioSample + AudioChannel] = AudioData[AudioChannel][AudioSampleCurrent + AudioSample];
                    AudioSampleCurrent += AudioFrameSize;
                    auto *AudioDataPointer = reinterpret_cast<uint8_t*>(AudioDataSample);
                    AudioSWRContext.doConvert((const uint8_t**) &AudioDataPointer, AudioFrame->extended_data, AudioFrameSize);
                } else AudioFrame.~MediaFrame();
                AudioCodecContext.doSendFrame(AudioFrame);
                int AudioStatus;
                while (!(AudioStatus = avcodec_receive_packet((AVCodecContext*) AudioCodecContext, (AVPacket*) AudioPacket)))
                    AudioFormatContext.doWriteFrame(AudioPacket);
                if (AudioStatus == AVERROR_EOF) break;
                else if (AudioStatus != AVERROR(EAGAIN))
                    throw MediaException(String(u"AudioSegment::doExport(const String&) avcodec_receive_packet"));
            }
            AudioFormatContext.doWriteTrailer();
        }

        static AudioSegment doOpen(const String &AudioPath) {
            MediaFormatContext AudioFormatContext(MediaFormatContext::doOpen(AudioPath));
            AudioFormatContext.doFindStreamInformation();
            int AudioStreamIndex = AudioFormatContext.doFindBestStream(AVMEDIA_TYPE_AUDIO);
            MediaCodecContext AudioCodecContext(MediaCodecContext::doAllocate());
            AudioCodecContext.setParameter(AudioFormatContext->streams[AudioStreamIndex]->codecpar);
            MediaCodec AudioCodec(MediaCodec::doFindDecoder(AudioCodecContext->codec_id));
            AudioCodecContext.doOpen(AudioCodec);
            if (AudioCodecContext->sample_rate <= 0)
                throw MediaException(String(u"AudioSegment::doOpen(const String&) AudioCodecContext->sample_rate"));
            MediaSWRContext AudioSWRContext(MediaSWRContext::doAllocate(&AudioCodecContext->ch_layout, &AudioCodecContext->ch_layout, AudioCodecContext->sample_fmt, AV_SAMPLE_FMT_U8, AudioCodecContext->sample_rate, AudioCodecContext->sample_rate));
            AudioSWRContext.doInitialize();
            MediaFrame AudioFrame(MediaFrame::doAllocate());
            MediaPacket AudioPacket(MediaPacket::doAllocate());
            std::vector<std::vector<uint8_t>> AudioDataOutput(AudioCodecContext->ch_layout.nb_channels);
            for (;;) {
                int AudioStatus = av_read_frame((AVFormatContext*) AudioFormatContext, (AVPacket*) AudioPacket);
                if (AudioStatus == AVERROR_EOF) break;
                else if (AudioStatus < 0) throw MediaException(String(u"AudioSegment::doOpen(const String&) av_read_frame"));
                if (AudioPacket->stream_index != AudioStreamIndex) continue;
                AudioCodecContext.doSendPacket(AudioPacket);
                while (!(AudioStatus = avcodec_receive_frame((AVCodecContext*) AudioCodecContext, (AVFrame*) AudioFrame))) {
                    uint8_t AudioDataBuffer[AudioCodecContext->ch_layout.nb_channels * AudioFrame->nb_samples];
                    auto *AudioDataPointer = reinterpret_cast<uint8_t*>(AudioDataBuffer);
                    AudioSWRContext.doConvert((const uint8_t**) AudioFrame->extended_data, (uint8_t**) &AudioDataPointer, AudioFrame->nb_samples);
                    for (int AudioSample = 0; AudioSample < AudioFrame->nb_samples; ++AudioSample)
                        for (int AudioChannel = 0;AudioChannel < AudioCodecContext->ch_layout.nb_channels;++AudioChannel)
                            AudioDataOutput[AudioChannel].push_back(AudioDataBuffer[AudioCodecContext->ch_layout.nb_channels * AudioSample + AudioChannel]);
                }
                if (AudioStatus != AVERROR(EAGAIN))
                    throw MediaException(String(u"AudioSegment::doOpen(const String&) avcodec_receive_frame"));
            }
            return {AudioDataOutput, AudioCodecContext->ch_layout, (uint32_t) AudioCodecContext->sample_rate};
        }

        MediaChannelLayout getChannelLayout() const noexcept {
            return AudioChannelLayout;
        }

        uint32_t getSampleRate() const noexcept {
            return AudioSampleRate;
        }

        AudioSegment &operator=(const AudioSegment &AudioSource) noexcept {
            doAssign(AudioSource);
            return *this;
        }

        AudioSegment setChannelLayout(const MediaChannelLayout &AudioChannelLayoutSource) const {
            if (AudioChannelLayout.getChannelMask() == AudioChannelLayoutSource.getChannelMask()) return *this;
            AVChannelLayout AudioChannelLayoutCurrent(AudioChannelLayout.toFFMpegFormat());
            AVChannelLayout AudioChannelLayoutTarget(AudioChannelLayoutSource.toFFMpegFormat());
            MediaSWRContext AudioSWRContext(MediaSWRContext::doAllocate(&AudioChannelLayoutCurrent, &AudioChannelLayoutTarget, AV_SAMPLE_FMT_U8P, AV_SAMPLE_FMT_U8P, AudioSampleRate, AudioSampleRate));
            AudioSWRContext.doInitialize();
            auto **AudioDataOutput = MemoryAllocator::newArray<uint8_t*>(AudioChannelLayout.getChannelCount());
            for (uint8_t AudioChannel = 0;AudioChannel < AudioChannelLayout.getChannelCount();++AudioChannel)
                AudioDataOutput[AudioChannel] = MemoryAllocator::newArray<uint8_t>(AudioDataSize);
            AudioSWRContext.doConvert((const uint8_t**) AudioData, AudioDataOutput, AudioDataSize);
            return {AudioDataOutput, AudioDataSize, AudioChannelLayoutSource, AudioSampleRate};
        }

        AudioSegment setSampleRate(uint32_t AudioSampleRateSource) const {
            if (AudioSampleRate == AudioSampleRateSource) return *this;
            AVChannelLayout AudioChannelLayoutSource(AudioChannelLayout.toFFMpegFormat());
            MediaSWRContext AudioSWRContext(MediaSWRContext::doAllocate(&AudioChannelLayoutSource, &AudioChannelLayoutSource, AV_SAMPLE_FMT_U8P, AV_SAMPLE_FMT_U8P, AudioSampleRate, AudioSampleRateSource));
            AudioSWRContext.doInitialize();
            auto **AudioDataOutput = MemoryAllocator::newArray<uint8_t*>(AudioChannelLayout.getChannelCount());
            for (uint8_t AudioChannel = 0;AudioChannel < AudioChannelLayout.getChannelCount();++AudioChannel)
                AudioDataOutput[AudioChannel] = MemoryAllocator::newArray<uint8_t>(AudioDataSize);
            AudioSWRContext.doConvert((const uint8_t**) AudioData, AudioDataOutput, AudioDataSize);
            return {AudioDataOutput, AudioDataSize, AudioChannelLayout, AudioSampleRate};
        }

        MediaBuffer toMediaBuffer() const {
            auto *AudioDataOutput = MemoryAllocator::newArray<uint8_t>(AudioDataSize * AudioChannelLayout.getChannelCount());
            AVChannelLayout AudioChannelLayoutSource(AudioChannelLayout.toFFMpegFormat());
            MediaSWRContext AudioSWRContext(MediaSWRContext::doAllocate(&AudioChannelLayoutSource, &AudioChannelLayoutSource, AV_SAMPLE_FMT_U8P, AV_SAMPLE_FMT_U8, AudioSampleRate, AudioSampleRate));
            AudioSWRContext.doInitialize();
            AudioSWRContext.doConvert((const uint8_t**) AudioData, &AudioDataOutput, AudioDataSize);
            return {AudioChannelLayout.toOpenALFormat(), AudioDataOutput, (ALsizei) AudioDataSize * AudioChannelLayout.getChannelCount(), (ALsizei) AudioSampleRate};
        }
    };
}
#endif
