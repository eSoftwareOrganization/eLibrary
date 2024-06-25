#pragma once

#ifndef eLibraryHeaderMultimediaAudioSegment
#define eLibraryHeaderMultimediaAudioSegment

#if eLibraryFeature(Multimedia)

#include <Multimedia/MediaUtility.hpp>

namespace eLibrary::Multimedia {
    class AudioSegment final : public Object {
    private:
        MediaChannelLayout AudioChannelLayout;
        Array<Array<uint8_t>> AudioData;
        uintmax_t AudioDataSize = 0;
        int AudioSampleRate = 0;
        mutable MemoryAllocator<uint8_t> AudioAllocator;

        AudioSegment(const ::std::vector<std::vector<uint8_t>> &AudioDataSource, const MediaChannelLayout &AudioChannelLayoutSource, int AudioSampleRateSource) : AudioChannelLayout(AudioChannelLayoutSource), AudioDataSize(AudioDataSource[0].size()), AudioSampleRate(AudioSampleRateSource) {
            AudioData = Array<Array<uint8_t>>(AudioDataSource.size());
            for (uint8_t AudioChannel = 0;AudioChannel < AudioChannelLayout.getChannelCount();++AudioChannel)
                AudioData.getElement(AudioChannel) = Array<uint8_t>(AudioDataSource[AudioChannel].begin(), AudioDataSource[AudioChannel].end());
        }

        AudioSegment(const Array<Array<uint8_t>> &AudioDataSource, const MediaChannelLayout &AudioChannelLayoutSource, int AudioSampleRateSource) : AudioChannelLayout(AudioChannelLayoutSource), AudioDataSize(AudioDataSource.getElement(0).getElementSize()), AudioSampleRate(AudioSampleRateSource) {
            AudioData = Array<Array<uint8_t>>(AudioDataSource.getElementSize());
            for (uint8_t AudioChannel = 0;AudioChannel < AudioChannelLayout.getChannelCount();++AudioChannel)
                AudioData.getElement(AudioChannel) = AudioDataSource.getElement(AudioChannel);
        }

        AudioSegment(uint8_t **AudioDataSource, uintmax_t AudioDataSourceSize, const MediaChannelLayout &AudioChannelLayoutSource, int AudioSampleRateSource) : AudioChannelLayout(AudioChannelLayoutSource), AudioDataSize(AudioDataSourceSize), AudioSampleRate(AudioSampleRateSource) {
            AudioData = Array<Array<uint8_t>>(AudioChannelLayoutSource.getChannelCount());
            for (uint8_t AudioChannel = 0;AudioChannel < AudioChannelLayout.getChannelCount();++AudioChannel)
                AudioData.getElement(AudioChannel) = Array<uint8_t>(AudioDataSource[AudioChannel], AudioDataSource[AudioChannel] + AudioDataSize);
            for (uint8_t AudioChannel = 0;AudioChannel < AudioChannelLayout.getChannelCount();++AudioChannel)
                AudioAllocator.doDeallocate(AudioDataSource[AudioChannel], AudioDataSourceSize);
            MemoryAllocator<uint8_t*>::deleteArray(AudioDataSource);
        }
    public:
        doEnableCopyAssignConstruct(AudioSegment)
        doEnableMoveAssignConstruct(AudioSegment)

        void doAssign(const AudioSegment &AudioSource) noexcept {
            if (Objects::getAddress(AudioSource) == this) return;
            AudioData = Array<Array<uint8_t>>((AudioChannelLayout = AudioSource.AudioChannelLayout).getChannelCount());
            AudioDataSize = AudioSource.AudioDataSize;
            AudioSampleRate = AudioSource.AudioSampleRate;
            for (uint8_t AudioChannel = 0;AudioChannel < AudioChannelLayout.getChannelCount();++AudioChannel) {
                AudioData.getElement(AudioChannel) = Array<uint8_t>(AudioDataSize);
                Collections::doCopy(AudioSource.AudioData.getElement(AudioChannel).begin(), AudioDataSize, AudioData.getElement(AudioChannel).begin());
            }
        }

        void doAssign(AudioSegment &&AudioSource) noexcept {
            if (Objects::getAddress(AudioSource) == this) return;
            AudioChannelLayout = AudioSource.AudioChannelLayout;
            AudioData = AudioSource.AudioData;
            AudioDataSize = AudioSource.AudioDataSize;
            AudioSampleRate = AudioSource.AudioSampleRate;
            AudioSource.AudioDataSize = 0;
            AudioSource.AudioSampleRate = 0;
        }

        AudioSegment doConcat(const AudioSegment &AudioSource) const noexcept {
            auto AudioData1(setChannelLayout(Objects::getMaximum(AudioChannelLayout, AudioSource.AudioChannelLayout)).setSampleRate(Objects::getMaximum(AudioSampleRate, AudioSource.AudioSampleRate)).AudioData);
            auto AudioData2(AudioSource.setChannelLayout(Objects::getMaximum(AudioChannelLayout, AudioSource.AudioChannelLayout)).setSampleRate(Objects::getMaximum(AudioSampleRate, AudioSource.AudioSampleRate)).AudioData);
            auto AudioDataOutput(Array<Array<uint8_t>>(Objects::getMaximum(AudioChannelLayout, AudioSource.AudioChannelLayout).getChannelCount()));
            for (uint8_t AudioChannel = 0;AudioChannel < AudioDataOutput.getElementSize();++AudioChannel)
                AudioDataOutput.getElement(AudioChannel) = AudioData1.getElement(AudioChannel).doConcat(AudioData2.getElement(AudioChannel));
            return {AudioDataOutput, Objects::getMaximum(AudioChannelLayout, AudioSource.AudioChannelLayout), AudioSampleRate};
        }

        void doExport(const String &AudioPath) const {
            FFMpeg::MediaFormatContext AudioFormatContext(FFMpeg::MediaFormatContext::doAllocateOutput(AudioPath));
            if (avio_open(&AudioFormatContext->pb, AudioPath.toU8String().c_str(), AVIO_FLAG_WRITE) < 0)
                doThrowChecked(MediaException, u"AudioSegment::doExport(const String&) avio_open"_S);
            FFMpeg::MediaCodec AudioCodec(FFMpeg::MediaCodec::doFindEncoder(AudioFormatContext->oformat->audio_codec));
            FFMpeg::MediaCodecContext AudioCodecContext(FFMpeg::MediaCodecContext::doAllocate(AudioCodec));
            AVChannelLayout AudioChannelLayoutSource(AudioChannelLayout.toFFMpegFormat());
            av_channel_layout_copy(&AudioCodecContext->ch_layout, &AudioChannelLayoutSource);
            AudioCodecContext->sample_fmt = AudioCodec->sample_fmts[0];
            AudioCodecContext->sample_rate = (int) AudioSampleRate;
            AVStream *AudioStreamObject;
            AudioCodecContext.doOpen(AudioCodec);
            if (!(AudioStreamObject = avformat_new_stream((AVFormatContext*) AudioFormatContext, (const AVCodec*) AudioCodec)))
                doThrowChecked(MediaException, u"AudioSegment::doExport(const String&) avformat_new_stream"_S);
            if (avcodec_parameters_from_context(AudioStreamObject->codecpar, (AVCodecContext*) AudioCodecContext))
                doThrowChecked(MediaException, u"AudioSegment::doExport(const String&) avcodec_parameters_from_context"_S);
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
            auto *AudioDataSample = AudioAllocator.doAllocate(AudioChannelLayout.getChannelCount() * AudioCodecContext->frame_size);
            for (;;) {
                if (AudioSampleCurrent < AudioDataSize) {
                    int AudioFrameSize = Objects::getMinimum(AudioCodecContext->frame_size, int(AudioDataSize - AudioSampleCurrent));
                    AudioFrame->nb_samples = AudioFrameSize;
                    AudioFrame->pts = AudioSampleCurrent;
                    for (int AudioSample = 0;AudioSample < AudioFrameSize;++AudioSample)
                        for (uint8_t AudioChannel = 0;AudioChannel < AudioChannelLayout.getChannelCount();++AudioChannel)
                            AudioDataSample[AudioChannelLayout.getChannelCount() * AudioSample + AudioChannel] = AudioData.getElement(AudioChannel).getElement(AudioSampleCurrent + AudioSample);
                    AudioSampleCurrent += AudioFrameSize;
                    auto *AudioDataPointer = reinterpret_cast<uint8_t*>(AudioDataSample);
                    AudioSWRContext.doConvert((const uint8_t**) &AudioDataPointer, AudioFrameSize, AudioFrame->extended_data, AudioFrameSize);
                } else AudioFrame.~MediaFrame();
                AudioCodecContext.doSendFrame(AudioFrame);
                int AudioStatus;
                while (!(AudioStatus = avcodec_receive_packet((AVCodecContext*) AudioCodecContext, (AVPacket*) AudioPacket)))
                    AudioFormatContext.doWriteFrame(AudioPacket);
                if (AudioStatus == AVERROR_EOF) break;
                else if (AudioStatus != AVERROR(EAGAIN))
                    doThrowChecked(MediaException, u"AudioSegment::doExport(const String&) avcodec_receive_packet"_S);
            }
            MemoryAllocator<uint8_t>::deleteArray(AudioDataSample);
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
            if (AudioCodecContext->sample_rate <= 0) [[unlikely]]
                doThrowChecked(MediaException, u"AudioSegment::doOpen(const String&) AudioCodecContext->sample_rate"_S);
            FFMpeg::MediaSWRContext AudioSWRContext(FFMpeg::MediaSWRContext::doAllocate(&AudioCodecContext->ch_layout, &AudioCodecContext->ch_layout, AudioCodecContext->sample_fmt, AV_SAMPLE_FMT_U8, AudioCodecContext->sample_rate, AudioCodecContext->sample_rate));
            AudioSWRContext.doInitialize();
            FFMpeg::MediaFrame AudioFrame(FFMpeg::MediaFrame::doAllocate());
            FFMpeg::MediaPacket AudioPacket(FFMpeg::MediaPacket::doAllocate());
            ::std::vector<::std::vector<uint8_t>> AudioDataOutput(AudioCodecContext->ch_layout.nb_channels);
            for (;;) {
                int AudioStatus = av_read_frame((AVFormatContext*) AudioFormatContext, (AVPacket*) AudioPacket);
                if (AudioStatus == AVERROR_EOF) break;
                else if (AudioStatus < 0) doThrowChecked(MediaException, u"AudioSegment::doOpen(const String&) av_read_frame"_S);
                if (AudioPacket->stream_index != AudioStreamIndex) continue;
                AudioCodecContext.doSendPacket(AudioPacket);
                while (!(AudioStatus = avcodec_receive_frame((AVCodecContext*) AudioCodecContext, (AVFrame*) AudioFrame))) {
                    auto *AudioDataBuffer = MemoryAllocator<uint8_t>::newArray(AudioCodecContext->ch_layout.nb_channels * AudioFrame->nb_samples);
                    AudioSWRContext.doConvert((const uint8_t**) AudioFrame->extended_data, AudioFrame->nb_samples, (uint8_t**) &AudioDataBuffer, AudioFrame->nb_samples);
                    for (int AudioSample = 0; AudioSample < AudioFrame->nb_samples; ++AudioSample)
                        for (int AudioChannel = 0;AudioChannel < AudioCodecContext->ch_layout.nb_channels;++AudioChannel)
                            AudioDataOutput[AudioChannel].push_back(AudioDataBuffer[AudioCodecContext->ch_layout.nb_channels * AudioSample + AudioChannel]);
                    MemoryAllocator<uint8_t>::deleteArray(AudioDataBuffer);
                }
                if (AudioStatus != AVERROR(EAGAIN))
                    doThrowChecked(MediaException, u"AudioSegment::doOpen(const String&) avcodec_receive_frame"_S);
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
            auto **AudioDataSource = MemoryAllocator<uint8_t*>::newArray(AudioChannelLayout.getChannelCount());
            for (uint8_t AudioChannel = 0;AudioChannel < AudioChannelLayout.getChannelCount();++AudioChannel) {
                AudioDataSource[AudioChannel] = AudioAllocator.doAllocate(AudioDataSize);
                Collections::doCopy(AudioData.getElement(AudioChannel).begin(), AudioDataSize, AudioDataSource[AudioChannel]);
            }
            auto **AudioDataOutput = MemoryAllocator<uint8_t*>::newArray(AudioChannelLayoutSource.getChannelCount());
            for (uint8_t AudioChannel = 0;AudioChannel < AudioChannelLayoutSource.getChannelCount();++AudioChannel)
                AudioDataOutput[AudioChannel] = AudioAllocator.doAllocate(AudioDataSize);
            AudioSWRContext.doConvert((const uint8_t**) AudioDataSource, AudioDataSize, AudioDataOutput, AudioDataSize);
            for (uint8_t AudioChannel = 0;AudioChannel < AudioChannelLayout.getChannelCount();++AudioChannel)
                AudioAllocator.doDeallocate(AudioDataSource[AudioChannel], AudioDataSize);
            MemoryAllocator<uint8_t*>::deleteArray(AudioDataSource);
            return {AudioDataOutput, AudioDataSize, AudioChannelLayoutSource, AudioSampleRate};
        }

        AudioSegment setSampleRate(int AudioSampleRateSource) const {
            if (AudioSampleRateSource < 0) doThrowChecked(MediaException, u"AudioSegment::setSampleRate(int) AudioSampleRateSource"_S);
            if (AudioSampleRate == AudioSampleRateSource) return *this;
            AVChannelLayout AudioChannelLayoutSource(AudioChannelLayout.toFFMpegFormat());
            FFMpeg::MediaSWRContext AudioSWRContext(FFMpeg::MediaSWRContext::doAllocate(&AudioChannelLayoutSource, &AudioChannelLayoutSource, AV_SAMPLE_FMT_U8P, AV_SAMPLE_FMT_U8P, AudioSampleRate, AudioSampleRateSource));
            AudioSWRContext.doInitialize();
            auto **AudioDataSource = MemoryAllocator<uint8_t*>::newArray(AudioChannelLayout.getChannelCount());
            for (uint8_t AudioChannel = 0;AudioChannel < AudioChannelLayout.getChannelCount();++AudioChannel) {
                AudioDataSource[AudioChannel] = AudioAllocator.doAllocate(AudioDataSize);
                Collections::doCopy(AudioData.getElement(AudioChannel).begin(), AudioDataSize, AudioDataSource[AudioChannel]);
            }
            auto **AudioDataOutput = MemoryAllocator<uint8_t*>::newArray(AudioChannelLayout.getChannelCount());
            int AudioDataSizeOutput = swr_get_out_samples((SwrContext*) AudioSWRContext, (int) AudioDataSize);
            for (uint8_t AudioChannel = 0;AudioChannel < AudioChannelLayout.getChannelCount();++AudioChannel)
                AudioDataOutput[AudioChannel] = AudioAllocator.doAllocate(AudioDataSizeOutput);
            AudioSWRContext.doConvert((const uint8_t**) AudioDataSource, AudioDataSize, AudioDataOutput, AudioDataSizeOutput);
            for (uint8_t AudioChannel = 0;AudioChannel < AudioChannelLayout.getChannelCount();++AudioChannel)
                AudioAllocator.doDeallocate(AudioDataSource[AudioChannel], AudioDataSize);
            MemoryAllocator<uint8_t*>::deleteArray(AudioDataSource);
            return {AudioDataOutput, (uintmax_t) AudioDataSizeOutput, AudioChannelLayout, AudioSampleRateSource};
        }

        OpenAL::MediaBuffer toMediaBuffer() const {
            Array<uint8_t> AudioDataOutput(AudioDataSize * AudioChannelLayout.getChannelCount());
            AVChannelLayout AudioChannelLayoutSource(AudioChannelLayout.toFFMpegFormat());
            FFMpeg::MediaSWRContext AudioSWRContext(FFMpeg::MediaSWRContext::doAllocate(&AudioChannelLayoutSource, &AudioChannelLayoutSource, AV_SAMPLE_FMT_U8P, AV_SAMPLE_FMT_U8, AudioSampleRate, AudioSampleRate));
            AudioSWRContext.doInitialize();
            auto *AudioDataBuffer(AudioDataOutput.getElementContainer());
            auto **AudioDataSource = MemoryAllocator<uint8_t*>::newArray(AudioChannelLayout.getChannelCount());
            for (uint8_t AudioChannel = 0;AudioChannel < AudioChannelLayout.getChannelCount();++AudioChannel) {
                AudioDataSource[AudioChannel] = AudioAllocator.doAllocate(AudioDataSize);
                Collections::doCopy(AudioData.getElement(AudioChannel).begin(), AudioDataSize, AudioDataSource[AudioChannel]);
            }
            AudioSWRContext.doConvert((const uint8_t**) AudioDataSource, AudioDataSize, &AudioDataBuffer, AudioDataSize);
            for (uint8_t AudioChannel = 0;AudioChannel < AudioChannelLayout.getChannelCount();++AudioChannel)
                AudioAllocator.doDeallocate(AudioDataSource[AudioChannel], AudioDataSize);
            MemoryAllocator<uint8_t*>::deleteArray(AudioDataSource);
            return {AudioChannelLayout, AudioDataOutput, (ALsizei) AudioSampleRate};
        }
    };
}
#endif

#endif
