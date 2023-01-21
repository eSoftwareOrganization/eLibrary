#pragma once

#include <deque>
#include <fstream>
#include <iostream>
#include <streambuf>
#include <string>
#include <string_view>
#include <type_traits>

namespace eLibrary {
    template<typename C>
    class FastBufferedInputStream final {
    private:
        bool IOBufferStatus;
        C *IOBuffer, *IOBufferStart, *IOBufferStop;
        unsigned IOBufferSize;
        std::basic_streambuf<C> *StreamBuffer;

        void getCharacter0(C &ObjectTarget) {
            if (IOBufferStart == IOBufferStop)
                IOBufferStart = IOBuffer, IOBufferStop = IOBuffer + StreamBuffer->sgetn(IOBuffer, IOBufferSize);
            if (IOBufferStart == IOBufferStop) {
                ObjectTarget = -1;
                IOBufferStatus = false;
            } else ObjectTarget = *IOBufferStart++;
        }
    public:
        explicit FastBufferedInputStream(std::basic_streambuf<C> *StreamBufferSource, unsigned BufferSize) : IOBufferStatus(true), IOBufferSize(BufferSize), StreamBuffer(StreamBufferSource) {
            if (!StreamBufferSource) throw IOException(String(u"FastBufferedInputStream<C>::FastBufferedInputStream(std::basic_streambuf<C>*, unsigned) StreamBufferSource"));
            IOBufferStart = IOBufferStop = IOBuffer = new C[IOBufferSize];
        }

        C doPeek() noexcept {
            if (IOBufferStart == IOBufferStop) {
                IOBufferStart = IOBuffer;
                IOBufferStop = IOBuffer + StreamBuffer->sgetn(IOBuffer, IOBufferSize);
            }
            if (IOBufferStart == IOBufferStop) {
                IOBufferStatus = false;
                return -1;
            } else return *IOBufferStart;
        }

        void doPop() noexcept {
            C ObjectTarget;
            doInput(ObjectTarget);
        }

        std::basic_streambuf<C> *getStreamBuffer() const noexcept {
            return StreamBuffer;
        }

        template<typename T, typename... Ts>
        void doInput(T &ObjectTarget, Ts&... ObjectTargets) noexcept {
            doInput(ObjectTarget);
            doInput(ObjectTargets...);
        }

        FastBufferedInputStream<C> &doInput(C &ObjectTarget) noexcept {
            if (IOBufferStatus) {
                getCharacter0(ObjectTarget);
                while ((ObjectTarget >= 7 && ObjectTarget <= 13) || ObjectTarget == 32) getCharacter0(ObjectTarget);
            }
            return *this;
        }

        template<typename T> requires std::is_floating_point<T>::value
        FastBufferedInputStream<C> &doInput(T &ObjectTarget) {
            if (IOBufferStatus) {
                C CharacterTarget;
                bool ObjectSignature = false;
                for (getCharacter0(CharacterTarget);(CharacterTarget < 48 || CharacterTarget > 57) && CharacterTarget != -1; getCharacter0(CharacterTarget))
                    ObjectSignature = CharacterTarget == 45;
                for (ObjectTarget = 0;CharacterTarget > 47 && CharacterTarget < 58; getCharacter0(CharacterTarget))
                    ObjectTarget = (ObjectTarget << 1) + (ObjectTarget << 3) + (CharacterTarget & 15);
                if (CharacterTarget == 46) {
                    uintmax_t ObjectPower = 1;
                    for (getCharacter0(CharacterTarget);CharacterTarget > 47 && CharacterTarget < 58; getCharacter0(CharacterTarget))
                        ObjectTarget += T((CharacterTarget & 15) * 1.0 / (ObjectPower *= 10));
                }
                if (ObjectSignature) ObjectTarget = -ObjectTarget;
                --IOBufferStart;
            }
            return *this;
        }

        template<typename T> requires std::is_signed<T>::value
        FastBufferedInputStream<C> &doInput(T &ObjectTarget) {
            if (IOBufferStatus) {
                C CharacterTarget;
                bool ObjectSignature = false;
                for (getCharacter0(CharacterTarget);(CharacterTarget < 48 || CharacterTarget > 57) && CharacterTarget != -1; getCharacter0(CharacterTarget))
                    ObjectSignature = CharacterTarget == 45;
                for (ObjectTarget = 0;CharacterTarget >= 48 && CharacterTarget <= 57; getCharacter0(CharacterTarget))
                    ObjectTarget = (ObjectTarget << 1) + (ObjectTarget << 3) + (CharacterTarget & 15);
                if (ObjectSignature) ObjectTarget = ~ObjectTarget + 1;
                --IOBufferStart;
            }
            return *this;
        }

        template<typename T> requires std::is_unsigned<T>::value
        FastBufferedInputStream<C> &doInput(T &ObjectTarget) {
            if (IOBufferStatus) {
                C CharacterTarget;
                getCharacter0(CharacterTarget);
                while (CharacterTarget < 48 || CharacterTarget > 57) getCharacter0(CharacterTarget);
                for (ObjectTarget = 0;CharacterTarget > 47 && CharacterTarget < 58; getCharacter0(CharacterTarget))
                    ObjectTarget = (ObjectTarget << 1) + (ObjectTarget << 3) + (CharacterTarget & 15);
                --IOBufferStart;
            }
            return *this;
        }

        FastBufferedInputStream<C> &doInput(std::basic_string<C> &ObjectTarget) {
            if (IOBufferStatus) {
                C CharacterTarget;
                getCharacter0(CharacterTarget);
                while ((CharacterTarget >= 7 && CharacterTarget <= 13) || CharacterTarget == 32) getCharacter0(CharacterTarget);
                for (ObjectTarget.clear();(CharacterTarget < 7 || CharacterTarget > 13) && CharacterTarget != 32; getCharacter0(CharacterTarget))
                    ObjectTarget.push_back(CharacterTarget);
                --IOBufferStart;
            }
            return *this;
        }

        void setStreamBuffer(std::basic_streambuf<C> *StreamBufferSource) {
            if (!StreamBufferSource) throw IOException(String(u"FastBufferedInputStream<C>::setStreamBuffer(std::basic_streambuf<C>*) StreamBufferSource"));
            StreamBuffer = StreamBufferSource;
        }
    };

    template<typename C>
    class FastBufferedOutputStream final {
    private:
        std::basic_string<C> IOBuffer;
        std::basic_streambuf<C> *StreamBuffer;
    public:
        FastBufferedOutputStream(std::basic_streambuf<C> *Buffer, unsigned BufferSize) : StreamBuffer(Buffer) {
            if (!Buffer) throw IOException(String(u"FastBufferedOutputStream::FastBufferedOutputStream(std::basic_streambuf<C>*, unsigned) Buffer"));
            if (BufferSize) IOBuffer.reserve(BufferSize);
        }

        ~FastBufferedOutputStream() noexcept {
            doFlush();
        }

        void doFlush() noexcept {
            StreamBuffer->sputn(IOBuffer.data(), (std::streamsize) IOBuffer.size());
            IOBuffer.clear();
        }

        std::basic_string_view<C> getIOBuffer() const noexcept {
            return IOBuffer;
        }

        std::basic_streambuf<C> *getStreamBuffer() const noexcept {
            return StreamBuffer;
        }

        template<typename T, typename... Ts>
        void doOutput(const T &ObjectTarget, const Ts&... ObjectTargets) noexcept {
            doOutput(ObjectTarget);
            doOutput(ObjectTargets...);
        }

        template<typename T> requires std::is_signed<T>::value
        FastBufferedOutputStream<C> &doOutput(T ObjectTarget) {
            std::deque<C> ObjectDeque;
            while (ObjectTarget) {
                ObjectDeque.push_back(ObjectTarget % 10 ^ 48);
                ObjectTarget /= 10;
            }
            if (ObjectDeque.empty()) ObjectDeque.push_back('0');
            while (!ObjectDeque.empty()) {
                IOBuffer.push_back(ObjectDeque.back());
                ObjectDeque.pop_back();
            }
            return *this;
        }

        template<typename T> requires std::is_unsigned<T>::value
        FastBufferedOutputStream<C> &doOutput(T ObjectTarget) {
            std::deque<C> ObjectDeque;
            if (ObjectTarget < 0) {
                IOBuffer.push_back('-');
                ObjectTarget = ~ObjectTarget + 1;
            }
            while (ObjectTarget) {
                ObjectDeque.push_back(ObjectTarget % 10 ^ 48);
                ObjectTarget /= 10;
            }
            if (ObjectDeque.empty()) ObjectDeque.push_back('0');
            while (!ObjectDeque.empty()) {
                IOBuffer.push_back(ObjectDeque.back());
                ObjectDeque.pop_back();
            }
            return *this;
        }

        FastBufferedOutputStream<C> &doOutput(C ObjectTarget) noexcept {
            IOBuffer.push_back(ObjectTarget);
            return *this;
        }

        FastBufferedOutputStream<C> &doOutput(const std::basic_string<C> &ObjectTarget) noexcept {
            for (auto CharacterTarget : ObjectTarget) IOBuffer.push_back(CharacterTarget);
            return *this;
        }

        void setIOBuffer(const std::basic_string<C> &IOBufferSource) {
            IOBuffer.clear();
            IOBuffer = IOBufferSource;
        }

        void setStreamBuffer(std::basic_streambuf<C> *StreamBufferSource) {
            if (!StreamBufferSource) throw IOException(String(u"FastBufferedOutputStream<C>::setStreamBuffer(std::basic_streambuf<C>*) StreamBufferSource"));
            StreamBuffer = StreamBufferSource;
        }
    };

    FastBufferedOutputStream<char> consoleErr8() noexcept {
        static FastBufferedOutputStream<char> StreamErr8(std::cerr.rdbuf(), 0);
        return StreamErr8;
    }

    FastBufferedInputStream<char> consoleIn8() noexcept {
        static FastBufferedInputStream<char> StreamIn8(std::cin.rdbuf(), 1048576);
        return StreamIn8;
    }

    FastBufferedOutputStream<char> consoleOut8() noexcept {
        static FastBufferedOutputStream<char> StreamOut8(std::cout.rdbuf(), 0);
        return StreamOut8;
    }

    FastBufferedInputStream<char16_t> consoleIn16() noexcept {
#ifdef _WIN32
        static std::basic_ifstream<char16_t> StreamFile("CON");
#else
        static std::basic_ifstream<char16_t> StreamFile("/dev/console");
#endif
        static FastBufferedInputStream<char16_t> StreamIn16(StreamFile.rdbuf(), 1048576);
        return StreamIn16;
    }

    FastBufferedOutputStream<char16_t> consoleOut16() noexcept {
#ifdef _WIN32
        static std::basic_ofstream<char16_t> StreamFile("CON");
#else
        static std::basic_ofstream<char16_t> StreamFile("/dev/console");
#endif
        static FastBufferedOutputStream<char16_t> StreamOut16(StreamFile.rdbuf(), 0);
        return StreamOut16;
    }

    FastBufferedInputStream<char32_t> consoleIn32() noexcept {
#ifdef _WIN32
        static std::basic_ofstream<char32_t> StreamFile("CON");
#else
        static std::basic_ofstream<char32_t> StreamFile("/dev/console");
#endif
        static FastBufferedInputStream<char32_t> StreamIn32(StreamFile.rdbuf(), 1048576);
        return StreamIn32;
    }

    FastBufferedOutputStream<char32_t> consoleOut32() noexcept {
#ifdef _WIN32
        static std::basic_ofstream<char32_t> StreamFile("CON");
#else
        static std::basic_ofstream<char32_t> StreamFile("/dev/console");
#endif
        static FastBufferedOutputStream<char32_t> StreamOut32(StreamFile.rdbuf(), 0);
        return StreamOut32;
    }

    FastBufferedOutputStream<wchar_t> consoleErrW() noexcept {
        static FastBufferedOutputStream<wchar_t> StreamErrW(std::wcerr.rdbuf(), 0);
        return StreamErrW;
    }

    FastBufferedInputStream<wchar_t> consoleInW() noexcept {
        static FastBufferedInputStream<wchar_t> StreamInW(std::wcin.rdbuf(), 1048576);
        return StreamInW;
    }

    FastBufferedOutputStream<wchar_t> consoleOutW() noexcept {
        static FastBufferedOutputStream<wchar_t> StreamOutW(std::wcout.rdbuf(), 0);
        return StreamOutW;
    }
}
