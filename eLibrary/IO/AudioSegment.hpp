#pragma once

#include <Core/Exception.hpp>

namespace eLibrary {
    struct AudioHeaderWAV final {
        uint32_t HeaderRIFF;
        uint32_t HeaderSizeTotal;
        uint32_t HeaderWAVE;
        uint32_t HeaderFMT;
        uint32_t HeaderSizeHeader;
        uint16_t HeaderCompression;
        uint16_t HeaderChannelCount;
        uint32_t HeaderSampleRate;
        uint32_t HeaderByteRate;
        uint16_t HeaderBlockAlignment;
        uint16_t HeaderBitsSample;
        uint32_t HeaderDATA;
        uint32_t HeaderSizeData;
    };

    class AudioSegment final : public Object {
    public:
        static auto doOpenWAV(const String &FilePath) {
            std::basic_ifstream<unsigned char> FileStream(FilePath.toU8String(), std::ios::binary | std::ios::in);
            if (!FileStream) throw IOException(String(u"AudioSegment::doOpenWAV(const String&) FileStream"));
            AudioHeaderWAV FileHeader;
            FileStream.read((unsigned char*) &FileHeader, sizeof(AudioHeaderWAV));
            if (FileHeader.HeaderRIFF != 0x46464952 || FileHeader.HeaderWAVE != 0x45564157 || FileHeader.HeaderFMT != 0x20746D66 || FileHeader.HeaderDATA != 0x61746164)
                throw Exception(String(u"AudioSegment::doOpenWAV(const String&) FileHeader"));
            auto *FileData = new unsigned char[FileHeader.HeaderSizeData];
            FileStream.read(FileData, FileHeader.HeaderSizeData);
            FileStream.close();
            return std::make_pair(FileHeader, FileData);
        }
    };
}
