#pragma once

#ifndef eLibraryHeaderMultimediaAudioSegment
#define eLibraryHeaderMultimediaAudioSegment

#if eLibraryFeature(Multimedia)

#include <Multimedia/MediaUtility.hpp>

namespace eLibrary::Multimedia {
    class AudioSegment final : public Object {
    private:
        MediaChannelLayout AudioChannelLayout;
        uint8_t **AudioData = nullptr;
        uintmax_t AudioDataSize = 0;
        int AudioSampleRate = 0;

        AudioSegment(const std::vector<std::vector<uint8_t>> &AudioDataSource, const MediaChannelLayout &AudioChannelLayoutSource, int AudioSampleRateSource) : AudioChannelLayout(AudioChannelLayoutSource), AudioDataSize(AudioDataSource[0].size()), AudioSampleRate(AudioSampleRateSource) {
            AudioData = MemoryAllocator::newArray<uint8_t*>(AudioDataSource.size());
            for (uint8_t AudioChannel = 0;AudioChannel < AudioChannelLayout.getChannelCount();++AudioChannel) {
                AudioData[AudioChannel] = MemoryAllocator::newArray<uint8_t>(AudioDataSource[AudioChannel].size());
                Collections::doCopy(AudioDataSource[AudioChannel].begin(), AudioDataSource[AudioChannel].end(), AudioData[AudioChannel]);
            }
        }

        AudioSegment(uint8_t **AudioDataSource, uintmax_t AudioDataSourceSize, const MediaChannelLayout &AudioChannelLayoutSource, int AudioSampleRateSource) : AudioChannelLayout(AudioChannelLayoutSource), AudioDataSize(AudioDataSourceSize), AudioSampleRate(AudioSampleRateSource) {
            AudioData = MemoryAllocator::newArray<uint8_t*>(AudioChannelLayoutSource.getChannelCount());
            for (uint8_t AudioChannel = 0;AudioChannel < AudioChannelLayout.getChannelCount();++AudioChannel) {
                AudioData[AudioChannel] = MemoryAllocator::newArray<uint8_t>(AudioDataSourceSize);
                Collections::doCopy(AudioDataSource[AudioChannel], AudioDataSize, AudioData[AudioChannel]);
            }
            for (uint8_t AudioChannel = 0;AudioChannel < AudioChannelLayout.getChannelCount();++AudioChannel)
                MemoryAllocator::deleteArray(AudioDataSource[AudioChannel]);
            MemoryAllocator::deleteArray(AudioDataSource);
        }
    public:
        doEnableCopyAssignConstruct(AudioSegment)
        doEnableMoveAssignConstruct(AudioSegment)

        ~AudioSegment() noexcept {
            if (AudioData) {
                for (uint8_t AudioChannel = 0;AudioChannel < AudioChannelLayout.getChannelCount();++AudioChannel)
                    MemoryAllocator::deleteArray(AudioData[AudioChannel]);
                MemoryAllocator::deleteArray(AudioData);
                AudioData = nullptr;
            }
        }

        void doAssign(const AudioSegment &AudioSource) noexcept {
            if (Objects::getAddress(AudioSource) == this) return;
            if (AudioData) {
                for (uint8_t AudioChannel = 0; AudioChannel < AudioChannelLayout.getChannelCount(); ++AudioChannel)
                    MemoryAllocator::deleteArray(AudioData[AudioChannel]);
                MemoryAllocator::deleteArray(AudioData);
            }
            AudioData = MemoryAllocator::newArray<uint8_t*>((AudioChannelLayout = AudioSource.AudioChannelLayout).getChannelCount());
            AudioDataSize = AudioSource.AudioDataSize;
            AudioSampleRate = AudioSource.AudioSampleRate;
            for (uint8_t AudioChannel = 0;AudioChannel < AudioChannelLayout.getChannelCount();++AudioChannel) {
                AudioData[AudioChannel] = MemoryAllocator::newArray<uint8_t>(AudioDataSize);
                Collections::doCopy(AudioSource.AudioData[AudioChannel], AudioDataSize, AudioData[AudioChannel]);
            }
        }

        void doAssign(AudioSegment &&AudioSource) noexcept {
            if (Objects::getAddress(AudioSource) == this) return;
            AudioChannelLayout = AudioSource.AudioChannelLayout;
            AudioData = AudioSource.AudioData;
            AudioDataSize = AudioSource.AudioDataSize;
            AudioSampleRate = AudioSource.AudioSampleRate;
            if (AudioSource.AudioData) {
                for (uint8_t AudioChannel = 0; AudioChannel < AudioSource.AudioChannelLayout.getChannelCount(); ++AudioChannel)
                    MemoryAllocator::deleteArray(AudioSource.AudioData[AudioChannel]);
                MemoryAllocator::deleteArray(AudioSource.AudioData);
            }
            AudioSource.AudioDataSize = 0;
            AudioSource.AudioSampleRate = 0;
        }

        void doExport(const String &AudioPath) const {
            FFMpeg::MediaFormatContext AudioFormatContext(FFMpeg::MediaFormatContext::doAllocateOutput(AudioPath));
            if (avio_open(&AudioFormatContext->pb, AudioPath.toU8String().c_str(), AVIO_FLAG_WRITE) < 0)
                throw MediaException(String(u"AudioSegment::doExport(const String&) avio_open"));
            FFMpeg::MediaCodec AudioCodec(FFMpeg::MediaCodec::doFindEncoder(AudioFormatContext->oformat->audio_codec));
            FFMpeg::MediaCodecContext AudioCodecContext(FFMpeg::MediaCodecContext::doAllocate(AudioCodec));
            AVChannelLayout AudioChannelLayoutSource(AudioChannelLayout.toFFMpegFormat());
            av_channel_layout_copy(&AudioCodecContext->ch_layout, &AudioChannelLayoutSource);
            AudioCodecContext->sample_fmt = AudioCodec->sample_fmts[0];
            AudioCodecContext->sample_rate = (int) AudioSampleRate;
            AVStream *AudioStreamObject;
            AudioCodecContext.doOpen(AudioCodec);
            if (!(AudioStreamObject = avformat_new_stream((AVFormatContext*) AudioFormatContext, (const AVCodec*) AudioCodec)))
                throw MediaException(String(u"AudioSegment::doExport(const String&) avformat_new_stream"));
            if (avcodec_parameters_from_context(AudioStreamObject->codecpar, (AVCodecContext*) AudioCodecContext))
                throw MediaException(String(u"AudioSegment::doExport(const String&) avcodec_parameters_from_context"));
            AudioFormatContext.doWriteHeader();
            FFMpeg::MediaSWRContext AudioSWRContext(FFMpeg::MediaSWRContext::doAllocate(&AudioCodecContext->ch_layout, &AudioCodecContext->ch_layout, AV_SAMPLE_FMT_U8, AudioCodecContext->sample_fmt, AudioCodecContext->sample_rate, AudioCodecContext->sample_rate));
            AudioSWRContext.doInitialize();
            FFMpeg::MediaFrame AudioFrame(FFMpeg::MediaFrame::doAllocate());
            if (AudioCodecContext->frame_size <= 0) AudioCodecContext->frame_size = 2048;
            av_channel_layout_copy(&AudioFrame->ch_layout, &AudioCodecContext->ch_layout);
            AudioFrame->format = AudioCodecContext->sample_fmt;
            AudioFrame->nb_samples = AudioCodecContext->frame_size;
            AudioFrame->sample_rate = AudioCodecContext->sample_rate;
            AudioFrame.getFrameBuffer();
            FFMpeg::MediaPacket AudioPacket(FFMpeg::MediaPacket::doAllocate());
            uint32_t AudioSampleCurrent = 0;
            auto *AudioDataSample = MemoryAllocator::newArray<uint8_t>(AudioChannelLayout.getChannelCount() * AudioCodecContext->frame_size);
            for (;;) {
                if (AudioSampleCurrent < AudioDataSize) {
                    int AudioFrameSize = Objects::getMinimum(AudioCodecContext->frame_size, int(AudioDataSize - AudioSampleCurrent));
                    AudioFrame->nb_samples = AudioFrameSize;
                    AudioFrame->pts = AudioSampleCurrent;
                    for (int AudioSample = 0;AudioSample < AudioFrameSize;++AudioSample)
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
            MemoryAllocator::deleteArray(AudioDataSample);
            AudioFormatContext.doWriteTrailer();
        }

        static AudioSegment doOpen(const String &AudioPath) {
            FFMpeg::MediaFormatContext AudioFormatContext(FFMpeg::MediaFormatContext::doOpen(AudioPath));
            AudioFormatContext.doFindStreamInformation();
            int AudioStreamIndex = AudioFormatContext.doFindBestStream(AVMEDIA_TYPE_AUDIO);
            FFMpeg::MediaCodecContext AudioCodecContext(FFMpeg::MediaCodecContext::doAllocate());
            AudioCodecContext.setParameter(AudioFormatContext->streams[AudioStreamIndex]->codecpar);
            FFMpeg::MediaCodec AudioCodec(FFMpeg::MediaCodec::doFindDecoder(AudioCodecContext->codec_id));
            AudioCodecContext.doOpen(AudioCodec);
            if (AudioCodecContext->sample_rate <= 0)
                throw MediaException(String(u"AudioSegment::doOpen(const String&) AudioCodecContext->sample_rate"));
            FFMpeg::MediaSWRContext AudioSWRContext(FFMpeg::MediaSWRContext::doAllocate(&AudioCodecContext->ch_layout, &AudioCodecContext->ch_layout, AudioCodecContext->sample_fmt, AV_SAMPLE_FMT_U8, AudioCodecContext->sample_rate, AudioCodecContext->sample_rate));
            AudioSWRContext.doInitialize();
            FFMpeg::MediaFrame AudioFrame(FFMpeg::MediaFrame::doAllocate());
            FFMpeg::MediaPacket AudioPacket(FFMpeg::MediaPacket::doAllocate());
            std::vector<std::vector<uint8_t>> AudioDataOutput(AudioCodecContext->ch_layout.nb_channels);
            for (;;) {
                int AudioStatus = av_read_frame((AVFormatContext*) AudioFormatContext, (AVPacket*) AudioPacket);
                if (AudioStatus == AVERROR_EOF) break;
                else if (AudioStatus < 0) throw MediaException(String(u"AudioSegment::doOpen(const String&) av_read_frame"));
                if (AudioPacket->stream_index != AudioStreamIndex) continue;
                AudioCodecContext.doSendPacket(AudioPacket);
                while (!(AudioStatus = avcodec_receive_frame((AVCodecContext*) AudioCodecContext, (AVFrame*) AudioFrame))) {
                    auto *AudioDataBuffer = MemoryAllocator::newArray<uint8_t>(AudioCodecContext->ch_layout.nb_channels * AudioFrame->nb_samples);
                    AudioSWRContext.doConvert((const uint8_t**) AudioFrame->extended_data, (uint8_t**) &AudioDataBuffer, AudioFrame->nb_samples);
                    for (int AudioSample = 0; AudioSample < AudioFrame->nb_samples; ++AudioSample)
                        for (int AudioChannel = 0;AudioChannel < AudioCodecContext->ch_layout.nb_channels;++AudioChannel)
                            AudioDataOutput[AudioChannel].push_back(AudioDataBuffer[AudioCodecContext->ch_layout.nb_channels * AudioSample + AudioChannel]);
                    MemoryAllocator::deleteArray(AudioDataBuffer);
                }
                if (AudioStatus != AVERROR(EAGAIN))
                    throw MediaException(String(u"AudioSegment::doOpen(const String&) avcodec_receive_frame"));
            }
            return {AudioDataOutput, AudioCodecContext->ch_layout, AudioCodecContext->sample_rate};
        }

        MediaChannelLayout getChannelLayout() const noexcept {
            return AudioChannelLayout;
        }

        int getSampleRate() const noexcept {
            return AudioSampleRate;
        }

        AudioSegment setChannelLayout(const MediaChannelLayout &AudioChannelLayoutSource) const {
            if (AudioChannelLayout.getChannelMask() == AudioChannelLayoutSource.getChannelMask()) return *this;
            AVChannelLayout AudioChannelLayoutCurrent(AudioChannelLayout.toFFMpegFormat());
            AVChannelLayout AudioChannelLayoutTarget(AudioChannelLayoutSource.toFFMpegFormat());
            FFMpeg::MediaSWRContext AudioSWRContext(FFMpeg::MediaSWRContext::doAllocate(&AudioChannelLayoutCurrent, &AudioChannelLayoutTarget, AV_SAMPLE_FMT_U8P, AV_SAMPLE_FMT_U8P, AudioSampleRate, AudioSampleRate));
            AudioSWRContext.doInitialize();
            auto **AudioDataOutput = MemoryAllocator::newArray<uint8_t*>(AudioChannelLayout.getChannelCount());
            for (uint8_t AudioChannel = 0;AudioChannel < AudioChannelLayout.getChannelCount();++AudioChannel)
                AudioDataOutput[AudioChannel] = MemoryAllocator::newArray<uint8_t>(AudioDataSize);
            AudioSWRContext.doConvert((const uint8_t**) AudioData, AudioDataOutput, AudioDataSize);
            return {AudioDataOutput, AudioDataSize, AudioChannelLayoutSource, AudioSampleRate};
        }

        AudioSegment setSampleRate(int AudioSampleRateSource) const {
            if (AudioSampleRateSource < 0) throw MediaException(String(u"AudioSegment::setSampleRate(int) AudioSampleRateSource"));
            if (AudioSampleRate == AudioSampleRateSource) return *this;
            AVChannelLayout AudioChannelLayoutSource(AudioChannelLayout.toFFMpegFormat());
            FFMpeg::MediaSWRContext AudioSWRContext(FFMpeg::MediaSWRContext::doAllocate(&AudioChannelLayoutSource, &AudioChannelLayoutSource, AV_SAMPLE_FMT_U8P, AV_SAMPLE_FMT_U8P, AudioSampleRate, AudioSampleRateSource));
            AudioSWRContext.doInitialize();
            auto **AudioDataOutput = MemoryAllocator::newArray<uint8_t*>(AudioChannelLayout.getChannelCount());
            for (uint8_t AudioChannel = 0;AudioChannel < AudioChannelLayout.getChannelCount();++AudioChannel)
                AudioDataOutput[AudioChannel] = MemoryAllocator::newArray<uint8_t>(AudioDataSize);
            AudioSWRContext.doConvert((const uint8_t**) AudioData, AudioDataOutput, AudioDataSize);
            return {AudioDataOutput, AudioDataSize, AudioChannelLayout, AudioSampleRate};
        }

        OpenAL::MediaBuffer toMediaBuffer() const {
            IO::ByteBuffer AudioDataOutput(IO::ByteBuffer::doAllocate(AudioDataSize * AudioChannelLayout.getChannelCount()));
            AVChannelLayout AudioChannelLayoutSource(AudioChannelLayout.toFFMpegFormat());
            FFMpeg::MediaSWRContext AudioSWRContext(FFMpeg::MediaSWRContext::doAllocate(&AudioChannelLayoutSource, &AudioChannelLayoutSource, AV_SAMPLE_FMT_U8P, AV_SAMPLE_FMT_U8, AudioSampleRate, AudioSampleRate));
            AudioSWRContext.doInitialize();
            auto *AudioDataBuffer(AudioDataOutput.getBufferContainer());
            AudioSWRContext.doConvert((const uint8_t**) AudioData, &AudioDataBuffer, AudioDataSize);
            return {AudioChannelLayout, AudioDataOutput, (ALsizei) AudioSampleRate};
        }
    };
}
#endif

#endif
