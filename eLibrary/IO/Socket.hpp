#pragma once

#include <io.h>
#include <utility>

#ifdef _WIN32
#include <ws2tcpip.h>
#else
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#endif

namespace eLibrary {
    class SocketInetAddress final : public Object {
    private:
        addrinfo *SocketAddressInformation;
        String SocketIP;
        unsigned short SocketPort;
    public:
        SocketInetAddress(const String &IP, unsigned short Port) : SocketIP(IP), SocketPort(Port) {
            addrinfo SocketAddressInformationHint = {0};
            if (getaddrinfo(SocketIP.toU8String().c_str(), String::valueOf(SocketPort).toU8String().c_str(), &SocketAddressInformationHint, &SocketAddressInformation) != 0)
                throw Exception(String(u"SocketInetAddress::SocketInetAddress(const String&, unsigned short) getaddrinfo"));
        }

        addrinfo *getAddressInformation() const noexcept {
            return SocketAddressInformation;
        }
    };

    class Socket final : public Object {
    private:
        SOCKET SocketSource;
        SocketInetAddress SocketAddress;
    public:
        explicit Socket(const SocketInetAddress &Address) : SocketAddress(Address) {
            addrinfo *SocketAddressInformation = SocketAddress.getAddressInformation();
            if ((SocketSource = socket(SocketAddressInformation->ai_family, SocketAddressInformation->ai_socktype, SocketAddressInformation->ai_protocol)) == INVALID_SOCKET)
                throw Exception(String(u"Socket::Socket(const SocketInetAddress&) socket"));
        }

        Socket(SOCKET Source, sockaddr_in *Address) : SocketSource(Source), SocketAddress(String(inet_ntoa(Address->sin_addr)), ntohs(Address->sin_port)) {
            if (Source == INVALID_SOCKET) throw Exception(String(u"Socket::Socket(SOCKET, sockaddr_in*) Source"));
        }

        ~Socket() noexcept {
            if (SocketSource != INVALID_SOCKET) {
                closesocket(SocketSource);
                SocketSource = INVALID_SOCKET;
            }
        }

        void doClose() noexcept {
            if (SocketSource != INVALID_SOCKET) {
                closesocket(SocketSource);
                SocketSource = INVALID_SOCKET;
            }
        }

        void doConnect() const {
            if (connect(SocketSource, SocketAddress.getAddressInformation()->ai_addr, (socklen_t) SocketAddress.getAddressInformation()->ai_addrlen) != 0)
                throw Exception(String(u"Socket::doConnect() connect"));
        }

        void setKeepAlive(bool OptionValue) const {
            if (setsockopt(SocketSource, SOL_SOCKET, SO_KEEPALIVE, (char*) &OptionValue, sizeof(bool)) != 0)
                throw Exception(String(u"Socket::setKeepAlive(bool) setsockopt"));
        }

        void setReceiveTimeout(int OptionValue) const {
            if (setsockopt(SocketSource, SOL_SOCKET, SO_RCVTIMEO, (char*) &OptionValue, sizeof(int)) != 0)
                throw Exception(String(u"Socket::setReceiveTimeout(int) setsockopt"));
        }

        void setSendTimeout(int OptionValue) const {
            if (setsockopt(SocketSource, SOL_SOCKET, SO_SNDTIMEO, (char*) &OptionValue, sizeof(int)) != 0)
                throw Exception(String(u"Socket::setSendTimeout(int) setsockopt"));
        }

        void setTCPNoDelay(bool OptionValue) const {
            if (setsockopt(SocketSource, IPPROTO_TCP, TCP_NODELAY, (char*) &OptionValue, sizeof(bool)) != 0)
                throw Exception(String(u"Socket::setSendTimeout(bool) setsockopt"));
        }
    };

    class SocketServer final : public Object {
    public:
    private:
        SOCKET SocketSource;
        SocketInetAddress SocketAddress;
    public:
        explicit SocketServer(const SocketInetAddress &Address) : SocketAddress(Address) {
            addrinfo *SocketAddressInformation = SocketAddress.getAddressInformation();
            if ((SocketSource = socket(SocketAddressInformation->ai_family, SocketAddressInformation->ai_socktype, SocketAddressInformation->ai_protocol)) == INVALID_SOCKET)
                throw Exception(String(u"SocketServer::SocketServer(const SocketInetAddress&) socket"));
        }

        ~SocketServer() noexcept {
            if (SocketSource != INVALID_SOCKET) {
                closesocket(SocketSource);
                SocketSource = INVALID_SOCKET;
            }
        }

        Socket doAccept() const noexcept {
            sockaddr SocketAddressSource = {0};
            return Socket(accept(SocketSource, &SocketAddressSource, nullptr), (sockaddr_in*) &SocketAddressSource);
        }

        void doBind() const {
            if (bind(SocketSource, SocketAddress.getAddressInformation()->ai_addr, (socklen_t) SocketAddress.getAddressInformation()->ai_addrlen) < 0)
                throw Exception(String(u"SocketServer::doBind() bind"));
        }

        void doClose() noexcept {
            if (SocketSource != INVALID_SOCKET) {
                closesocket(SocketSource);
                SocketSource = INVALID_SOCKET;
            }
        }

        void doListen(int SocketMaximum) const {
            if (listen(SocketSource, SocketMaximum) < 0)
                throw Exception(String(u"SocketServer::doListen(int) listen"));
        }

        void setConnectionReuse(bool OptionValue) const {
            if (setsockopt(SocketSource, SOL_SOCKET, SO_SNDTIMEO, (char*) &OptionValue, sizeof(bool)) != 0)
                throw Exception(String(u"SocketServer::setConnectionReuse(int) setsockopt"));
        }
    };

    class URLConnection : public Object {
    private:
        bool ConnectionInput;
        bool ConnectionOutput;
        unsigned ConnectionTimeoutConnect;
        unsigned ConnectionTimeoutRead;
    public:
        virtual void doConnect() = 0;

        unsigned getConnectTimeout() const noexcept {
            return ConnectionTimeoutConnect;
        }

        virtual String getHeaderField(const String &FieldKey) const noexcept = 0;

        unsigned getReadTimeout() const noexcept {
            return ConnectionTimeoutRead;
        }

        virtual String getRequestProperty(const String &PropertyKey) const noexcept = 0;

        void setConnectTimeout(unsigned TimeoutSource) noexcept {
            ConnectionTimeoutConnect = TimeoutSource;
        }

        void setReadTimeout(unsigned TimeoutSource) noexcept {
            ConnectionTimeoutRead = TimeoutSource;
        }

        virtual String setRequestProperty(const String &PropertyKey, const String &PropertyValue) = 0;
    };
}
