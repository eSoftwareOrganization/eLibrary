#pragma once

#include <IO/Stream.hpp>
#include <Network/Exception.hpp>

#include <algorithm>
#include <any>
#include <array>

#ifdef _WIN32
#include <ws2tcpip.h>

typedef SOCKET SocketHandleType;
#else
#include <netinet/in.h>
#include <netinet/tcp.h>

typedef int SocketHandleType;
#endif

using namespace eLibrary::Core;

namespace eLibrary::Network {
    class NetworkAddress final : public Object {
    public:
        enum class NetworkAddressProtocol {
            ProtocolIPv4, ProtocolIPv6, ProtocolUnknown
        };
    protected:
        NetworkAddressProtocol AddressProtocol;
        byte *AddressFieldList;
    public:
        explicit NetworkAddress(const std::array<byte, 4> &AddressSource) noexcept : AddressProtocol(NetworkAddressProtocol::ProtocolIPv4) {
            AddressFieldList = new byte[4];
            std::copy(AddressSource.begin(), AddressSource.end(), AddressFieldList);
        }

        explicit NetworkAddress(const std::array<byte, 16> &AddressSource) noexcept : AddressProtocol(NetworkAddressProtocol::ProtocolIPv6) {
            AddressFieldList = new byte[16];
            std::copy(AddressSource.begin(), AddressSource.end(), AddressFieldList);
        }

        NetworkAddress(const std::initializer_list<byte> &AddressSource) noexcept {
            if (AddressSource.size() == 4) {
                AddressFieldList = new byte[4];
                std::copy(AddressSource.begin(), AddressSource.end(), AddressFieldList);
                AddressProtocol = NetworkAddressProtocol::ProtocolIPv4;
            } else if (AddressSource.size() == 16) {
                AddressFieldList = new byte[16];
                std::copy(AddressSource.begin(), AddressSource.end(), AddressFieldList);
                AddressProtocol = NetworkAddressProtocol::ProtocolIPv6;
            } else {
                AddressFieldList = nullptr;
                AddressProtocol = NetworkAddressProtocol::ProtocolUnknown;
            }
        }

        explicit NetworkAddress(const in_addr &AddressSource) noexcept : AddressProtocol(NetworkAddressProtocol::ProtocolIPv4) {
            AddressFieldList = new byte[4];
#ifdef _WIN32
            AddressFieldList[0] = AddressSource.S_un.S_un_b.s_b1;
            AddressFieldList[1] = AddressSource.S_un.S_un_b.s_b2;
            AddressFieldList[2] = AddressSource.S_un.S_un_b.s_b3;
            AddressFieldList[3] = AddressSource.S_un.S_un_b.s_b4;
#else
            AddressFieldList[3] = AddressSource.s_addr >> 24 & 0xFF;
            AddressFieldList[2] = AddressSource.s_addr >> 16 & 0xFF;
            AddressFieldList[1] = AddressSource.s_addr >> 8 & 0xFF;
            AddressFieldList[0] = AddressSource.s_addr & 0xFF;
#endif
        }

        explicit NetworkAddress(const in6_addr &AddressSource) noexcept : AddressProtocol(NetworkAddressProtocol::ProtocolIPv6) {
            AddressFieldList = new byte[16];
            memcpy(AddressFieldList, AddressSource.s6_addr, sizeof(byte) * 16);
        }

        NetworkAddress(const NetworkAddress &AddressSource) noexcept : AddressProtocol(AddressSource.AddressProtocol) {
            if (AddressProtocol == NetworkAddressProtocol::ProtocolIPv4) {
                AddressFieldList = new byte[4];
                memcpy(AddressFieldList, AddressSource.AddressFieldList, sizeof(byte) * 4);
            } else if (AddressProtocol == NetworkAddressProtocol::ProtocolIPv6) {
                AddressFieldList = new byte[16];
                memcpy(AddressFieldList, AddressSource.AddressFieldList, sizeof(byte) * 16);
            } else {
                AddressFieldList = nullptr;
                AddressProtocol = NetworkAddressProtocol::ProtocolUnknown;
            }
        }

        ~NetworkAddress() noexcept {
            if (AddressFieldList) {
                delete[] AddressFieldList;
                AddressFieldList = nullptr;
            }
        }

        NetworkAddressProtocol getAddressProtocol() const noexcept {
            return AddressProtocol;
        }

        bool isAnyLocalAddress() const noexcept {
            if (AddressProtocol == NetworkAddressProtocol::ProtocolIPv4)
                return AddressFieldList[0] == 0 && AddressFieldList[1] == 0 && AddressFieldList[2] == 0 && AddressFieldList[3] == 0;
            if (AddressProtocol == NetworkAddressProtocol::ProtocolIPv6)
                return AddressFieldList[0] == 0 && AddressFieldList[1] == 0 && AddressFieldList[2] == 0 &&
                    AddressFieldList[3] == 0 && AddressFieldList[4] == 0 && AddressFieldList[5] == 0 &&
                    AddressFieldList[6] == 0 && AddressFieldList[7] == 0 && AddressFieldList[8] == 0 &&
                    AddressFieldList[9] == 0 && AddressFieldList[10] == 0 && AddressFieldList[11] == 0 &&
                    AddressFieldList[12] == 0 && AddressFieldList[13] == 0 && AddressFieldList[14] == 0 && AddressFieldList[15] == 0;
            return false;
        }

        bool isLinkLocalAddress() const noexcept {
            if (AddressProtocol == NetworkAddressProtocol::ProtocolIPv4)
                return AddressFieldList[0] == 169 && AddressFieldList[1] == 254;
            if (AddressProtocol == NetworkAddressProtocol::ProtocolIPv6)
                return AddressFieldList[0] == 0xFE && AddressFieldList[1] == 0x80;
            return false;
        }

        bool isLoopbackAddress() const noexcept {
            if (AddressProtocol == NetworkAddressProtocol::ProtocolIPv4)
                return AddressFieldList[0] == 127;
            if (AddressProtocol == NetworkAddressProtocol::ProtocolIPv6)
                return AddressFieldList[0] == 0 && AddressFieldList[1] == 0 && AddressFieldList[2] == 0 &&
                       AddressFieldList[3] == 0 && AddressFieldList[4] == 0 && AddressFieldList[5] == 0 &&
                       AddressFieldList[6] == 0 && AddressFieldList[7] == 0 && AddressFieldList[8] == 0 &&
                       AddressFieldList[9] == 0 && AddressFieldList[10] == 0 && AddressFieldList[11] == 0 &&
                       AddressFieldList[12] == 0 && AddressFieldList[13] == 0 && AddressFieldList[14] == 0 && AddressFieldList[15] == 0x01;
            return false;
        }

        bool isMulticastAddress() const noexcept {
            if (AddressProtocol == NetworkAddressProtocol::ProtocolIPv4)
                return AddressFieldList[0] >= 224 && AddressFieldList[0] <= 239;
            if (AddressProtocol == NetworkAddressProtocol::ProtocolIPv6)
                return AddressFieldList[0] == 0xFF;
            return false;
        }

        bool isSiteLocalAddress() const noexcept {
            if (AddressProtocol == NetworkAddressProtocol::ProtocolIPv4)
                return (AddressFieldList[0] == 192 && AddressFieldList[1] == 168) || AddressFieldList[0] == 10 || (AddressFieldList[0] == 172 && (AddressFieldList[1] & 0xF0) == 16);
            if (AddressProtocol == NetworkAddressProtocol::ProtocolIPv6)
                return AddressFieldList[0] == 0xFE && AddressFieldList[1] == 0xC0;
            return false;
        }

        NetworkAddress &operator=(const NetworkAddress&) = delete;

        std::any toAddressIn() const {
            if (AddressProtocol == NetworkAddressProtocol::ProtocolIPv4) {
                in_addr AddressResult;
                AddressResult.s_addr = htonl((AddressFieldList[0] << 24) | (AddressFieldList[1] << 16) | (AddressFieldList[2] << 8) | AddressFieldList[3]);
                return AddressResult;
            }
            if (AddressProtocol == NetworkAddressProtocol::ProtocolIPv6) {
                in6_addr AddressResult;
                memcpy(AddressResult.s6_addr, AddressFieldList, sizeof(byte) * 16);
                return AddressResult;
            }
            throw NetworkException(String(u"NetworkAddress::toAddressIn() AddressProtocol"));
        }

        String toString() const noexcept override {
            std::stringstream CharacterStream;
            if (AddressProtocol == NetworkAddressProtocol::ProtocolIPv4)
                CharacterStream << std::to_string(AddressFieldList[0]) << '.' << std::to_string(AddressFieldList[1]) << '.' << std::to_string(AddressFieldList[2]) << '.' << std::to_string(AddressFieldList[3]);
            if (AddressProtocol == NetworkAddressProtocol::ProtocolIPv6) {
                CharacterStream << std::hex;
                for (int AddressPartition = 0; AddressPartition < 8; ++AddressPartition) {
                    CharacterStream << ((AddressFieldList[AddressPartition << 1] << 8) | AddressFieldList[AddressPartition << 1 | 1]);
                    if (AddressPartition < 7) CharacterStream << ':';
                }
            }
            if (AddressProtocol == NetworkAddressProtocol::ProtocolUnknown) CharacterStream << "<Unknown>";
            return {CharacterStream.str()};
        }
    };

    class NetworkSocketAddress final : public Object {
    private:
        NetworkAddress SocketAddress;
        unsigned short SocketPort;
    public:
        NetworkSocketAddress(const NetworkAddress &AddressSource, unsigned short AddressPort) noexcept : SocketAddress(AddressSource), SocketPort(AddressPort) {}

        NetworkAddress getSocketAddress() const noexcept {
            return SocketAddress;
        }

        unsigned short getSocketPort() const noexcept {
            return SocketPort;
        }

        String toString() const noexcept override {
            StringStream CharacterStream;
            CharacterStream.addString({u"NetworkSocketAddress[SocketAddress="});
            CharacterStream.addString(SocketAddress.toString());
            CharacterStream.addString({u", SocketPort="});
            CharacterStream.addString(std::to_string(SocketPort));
            CharacterStream.addCharacter(u']');
            return CharacterStream.toString();
        }
    };

    class DatagramSocket final : public Object {
    private:
        SocketHandleType SocketSource;
        NetworkSocketAddress SocketAddress;
    public:
        DatagramSocket(const NetworkSocketAddress &AddressSource) : SocketAddress(AddressSource) {
            if (AddressSource.getSocketAddress().getAddressProtocol() == NetworkAddress::NetworkAddressProtocol::ProtocolUnknown)
                throw NetworkException(String(u"DatagramSocket::DatagramSocket(const NetworkSocketAddress&) AddressSource"));
            SocketSource = socket(AddressSource.getSocketAddress().getAddressProtocol() == NetworkAddress::NetworkAddressProtocol::ProtocolIPv4 ? AF_INET : AF_INET6, SOCK_DGRAM, 0);
            if (SocketSource == (SocketHandleType) -1) throw NetworkException(String(u"DatagramSocket::DatagramSocket(const NetworkSocketAddress&) socket"));
        }

        ~DatagramSocket() noexcept {
            if (!isClosed()) doClose();
        }

        void doClose() {
            if (isClosed()) throw NetworkException(String(u"DatagramSocket::doClose() isClosed"));
#ifdef _WIN32
            closesocket(SocketSource);
#else
            close(SocketSource);
#endif
            SocketSource = -1;
        }

        int doReceive(char *SocketBuffer, int SocketBufferSize) {
            int SocketSize;
            if (SocketAddress.getSocketAddress().getAddressProtocol() == NetworkAddress::NetworkAddressProtocol::ProtocolIPv4) {
                sockaddr_in SocketAddressIn;
                SocketAddressIn.sin_addr = std::any_cast<in_addr>(SocketAddress.getSocketAddress().toAddressIn());
                SocketAddressIn.sin_family = AF_INET;
                SocketAddressIn.sin_port = htons(SocketAddress.getSocketPort());
                if ((SocketSize = recvfrom(SocketSource, SocketBuffer, SocketBufferSize, 0, (sockaddr*) &SocketAddressIn, nullptr)) < 0)
                    throw NetworkException(String(u"DatagramSocket::doReceive(char*, int) recvfrom"));
            } else {
                sockaddr_in6 SocketAddressIn;
                SocketAddressIn.sin6_addr = std::any_cast<in6_addr>(SocketAddress.getSocketAddress().toAddressIn());
                SocketAddressIn.sin6_family = AF_INET6;
                SocketAddressIn.sin6_port = htons(SocketAddress.getSocketPort());
                if ((SocketSize = recvfrom(SocketSource, SocketBuffer, SocketBufferSize, 0, (sockaddr*) &SocketAddressIn, nullptr)) < 0)
                    throw NetworkException(String(u"DatagramSocket::doReceive(char*, int) recvfrom"));
            }
            return SocketSize;
        }

        int doSend(char *SocketBuffer, int SocketBufferSize) {
            int SocketSize;
            if (SocketAddress.getSocketAddress().getAddressProtocol() == NetworkAddress::NetworkAddressProtocol::ProtocolIPv4) {
                sockaddr_in SocketAddressIn;
                SocketAddressIn.sin_addr = std::any_cast<in_addr>(SocketAddress.getSocketAddress().toAddressIn());
                SocketAddressIn.sin_family = AF_INET;
                SocketAddressIn.sin_port = htons(SocketAddress.getSocketPort());
                if ((SocketSize = sendto(SocketSource, SocketBuffer, SocketBufferSize, 0, (sockaddr*) &SocketAddressIn, sizeof(sockaddr_in))) < 0)
                    throw NetworkException(String(u"DatagramSocket::doSend(char*, int) sendto"));
            } else {
                sockaddr_in6 SocketAddressIn;
                SocketAddressIn.sin6_addr = std::any_cast<in6_addr>(SocketAddress.getSocketAddress().toAddressIn());
                SocketAddressIn.sin6_family = AF_INET6;
                SocketAddressIn.sin6_port = htons(SocketAddress.getSocketPort());
                if ((SocketSize = sendto(SocketSource, SocketBuffer, SocketBufferSize, 0, (sockaddr*) &SocketAddressIn, sizeof(sockaddr_in6))) < 0)
                    throw NetworkException(String(u"DatagramSocket::doSend(char*, int) sendto"));
            }
            return SocketSize;
        }

        bool isClosed() const noexcept {
            return SocketSource == (SocketHandleType) -1;
        }

        DatagramSocket &operator=(const DatagramSocket&) noexcept = delete;

        void setBroadcast(bool OptionValue) const {
            if (setsockopt(SocketSource, SOL_SOCKET, SO_BROADCAST, (char*) &OptionValue, sizeof(bool)) != 0)
                throw NetworkException(String(u"DatagramSocket::setBroadcast(bool) setsockopt"));
        }
    };

    class SocketInputStream;

    class SocketOutputStream;

    class StreamSocket final : public Object {
    private:
        bool SocketConnected = false;
        SocketHandleType SocketSource;
        NetworkSocketAddress SocketAddress;

        StreamSocket(SocketHandleType SocketSourceSource, const NetworkSocketAddress &AddressSource) noexcept : SocketSource(SocketSourceSource), SocketAddress(AddressSource) {}
    public:
        explicit StreamSocket(const NetworkSocketAddress &AddressSource) : SocketAddress(AddressSource) {
            if (AddressSource.getSocketAddress().getAddressProtocol() == NetworkAddress::NetworkAddressProtocol::ProtocolUnknown)
                throw NetworkException(String(u"StreamSocket::StreamSocket(const NetworkSocketAddress&) AddressSource"));
            SocketSource = socket(AddressSource.getSocketAddress().getAddressProtocol() == NetworkAddress::NetworkAddressProtocol::ProtocolIPv4 ? AF_INET : AF_INET6, SOCK_STREAM, 0);
            if (SocketSource == (SocketHandleType) -1) throw NetworkException(String(u"StreamSocket::StreamSocket(const NetworkSocketAddress&) socket"));
        }

        ~StreamSocket() noexcept {
            if (!isClosed()) doClose();
        }

        void doClose() {
            if (isClosed()) throw NetworkException(String(u"StreamSocket::doClose() isClosed"));
#ifdef _WIN32
            closesocket(SocketSource);
#else
            close(SocketSource);
#endif
            SocketSource = -1;
        }

        void doConnect() {
            if (isConnected()) throw NetworkException(String(u"StreamSocket::doConnect() isConnected"));
            if (SocketAddress.getSocketAddress().getAddressProtocol() == NetworkAddress::NetworkAddressProtocol::ProtocolIPv4) {
                sockaddr_in SocketAddress4 = {AF_INET, htons(SocketAddress.getSocketPort()), std::any_cast<in_addr>(SocketAddress.getSocketAddress().toAddressIn()), {}};
                if (connect(SocketSource, (sockaddr*) &SocketAddress4, sizeof(sockaddr_in)) != 0)
                    throw NetworkException(String(u"StreamSocket::doConnect() connect"));
            } else {
                sockaddr_in6 SocketAddress6 = {AF_INET6, htons(SocketAddress.getSocketPort()), 0, std::any_cast<in6_addr>(SocketAddress.getSocketAddress().toAddressIn()), {}};
                if (connect(SocketSource, (sockaddr*) &SocketAddress6, sizeof(sockaddr_in6)) != 0)
                    throw NetworkException(String(u"StreamSocket::doConnect() connect"));
            }
            SocketConnected = true;
        }

        static StreamSocket doDeposit(SocketHandleType SocketSource, const NetworkSocketAddress &AddressSource) {
            if (SocketSource == (SocketHandleType) -1) throw NetworkException(String(u"StreamSocket::doDeposit(SocketHandleType, const NetworkSocketAddress&) SocketSource"));
            return {SocketSource, AddressSource};
        }

        bool isClosed() const noexcept {
            return SocketSource == (SocketHandleType) -1;
        }

        bool isConnected() const noexcept {
            return SocketConnected;
        }

        SocketInputStream getInputStream() const;

        SocketOutputStream getOutputStream() const;

        NetworkSocketAddress getRemoteSocketAddress() const noexcept {
            return SocketAddress;
        }

        SocketHandleType getSocketHandle() const noexcept {
            return SocketSource;
        }

        StreamSocket &operator=(const StreamSocket&) noexcept = delete;

        void setAddressReuse(bool OptionValue) const {
            if (setsockopt(SocketSource, SOL_SOCKET, SO_REUSEADDR, (char*) &OptionValue, sizeof(bool)))
                throw NetworkException(String(u"StreamSocket::setAddressReuse(bool) setsockopt"));
        }

        void setKeepAlive(bool OptionValue) const {
            if (setsockopt(SocketSource, SOL_SOCKET, SO_KEEPALIVE, (char*) &OptionValue, sizeof(bool)) != 0)
                throw NetworkException(String(u"StreamSocket::setKeepAlive(bool) setsockopt"));
        }

        void setReceiveTimeout(int OptionValue) const {
            if (setsockopt(SocketSource, SOL_SOCKET, SO_RCVTIMEO, (char*) &OptionValue, sizeof(int)) != 0)
                throw NetworkException(String(u"StreamSocket::setReceiveTimeout(int) setsockopt"));
        }

        void setSendTimeout(int OptionValue) const {
            if (setsockopt(SocketSource, SOL_SOCKET, SO_SNDTIMEO, (char*) &OptionValue, sizeof(int)) != 0)
                throw NetworkException(String(u"StreamSocket::setSendTimeout(int) setsockopt"));
        }

        void setTCPNoDelay(bool OptionValue) const {
            if (setsockopt(SocketSource, IPPROTO_TCP, TCP_NODELAY, (char*) &OptionValue, sizeof(bool)) != 0)
                throw NetworkException(String(u"StreamSocket::setSendTimeout(bool) setsockopt"));
        }

        String toString() const noexcept override {
            StringStream CharacterStream;
            CharacterStream.addString({u"StreamSocket[SocketAddress="});
            CharacterStream.addString(SocketAddress.toString());
            CharacterStream.addCharacter(u']');
            return CharacterStream.toString();
        }
    };

    class StreamSocketServer final : public Object {
    private:
        bool SocketBound = false;
        SocketHandleType SocketSource;
        NetworkSocketAddress SocketAddress;
    public:
        explicit StreamSocketServer(const NetworkSocketAddress &AddressSource) : SocketAddress(AddressSource) {
            if (AddressSource.getSocketAddress().getAddressProtocol() == NetworkAddress::NetworkAddressProtocol::ProtocolUnknown)
                throw NetworkException(String(u"StreamSocketServer::StreamSocketServer(const NetworkSocketAddress&) AddressSource"));
            SocketSource = socket(SocketAddress.getSocketAddress().getAddressProtocol() == NetworkAddress::NetworkAddressProtocol::ProtocolIPv4 ? AF_INET : AF_INET6, SOCK_STREAM, 0);
            if (SocketSource == (SocketHandleType) -1) throw NetworkException(String(u"StreamSocket::StreamSocket(const NetworkSocketAddress&) socket"));
        }

        ~StreamSocketServer() noexcept {
            if (!isClosed()) doClose();
        }

        StreamSocket doAccept() const {
            if (isClosed()) throw NetworkException(String(u"StreamSocketServer::doAccept() isClosed"));
            if (!isBound()) throw NetworkException(String(u"StreamSocketServer::doAccept() isBound"));
            SocketHandleType SocketTarget = accept(SocketSource, nullptr, nullptr);
            sockaddr SocketTargetAddress;
            getpeername(SocketTarget, &SocketTargetAddress, nullptr);
            if (SocketTargetAddress.sa_family == AF_INET) return StreamSocket::doDeposit(SocketTarget, NetworkSocketAddress(NetworkAddress(((sockaddr_in*) &SocketTargetAddress)->sin_addr), ntohs(((sockaddr_in*) &SocketTargetAddress)->sin_port)));
            else return StreamSocket::doDeposit(SocketTarget, NetworkSocketAddress(NetworkAddress(((sockaddr_in6*) &SocketTargetAddress)->sin6_addr), ntohs(((sockaddr_in6*) &SocketTargetAddress)->sin6_port)));
        }

        void doBind() {
            if (isClosed()) throw NetworkException(String(u"StreamSocketServer::doBind() isClosed"));
            if (isBound()) throw NetworkException(String(u"StreamSocketServer::doBind() isBound"));
            if (SocketAddress.getSocketAddress().getAddressProtocol() == NetworkAddress::NetworkAddressProtocol::ProtocolIPv4) {
                sockaddr_in SocketAddress4 = {AF_INET, htons(SocketAddress.getSocketPort()), std::any_cast<in_addr>(SocketAddress.getSocketAddress().toAddressIn()), {}};
                if (bind(SocketSource, (sockaddr*) &SocketAddress4, sizeof(sockaddr_in)))
                    throw NetworkException(String(u"StreamSocketServer::doBind() bind"));
            } else {
                sockaddr_in6 SocketAddress6 = {AF_INET6, htons(SocketAddress.getSocketPort()), 0, std::any_cast<in6_addr>(SocketAddress.getSocketAddress().toAddressIn()), {}};
                if (bind(SocketSource, (sockaddr*) &SocketAddress6, sizeof(sockaddr_in6)))
                    throw NetworkException(String(u"StreamSocketServer::doBind() bind"));
            }
            SocketBound = true;
        }

        void doClose() {
            if (isClosed()) throw NetworkException(String(u"StreamSocketServer::doClose() isClosed"));
#ifdef _WIN32
            closesocket(SocketSource);
#else
            close(SocketSource);
#endif
            SocketSource = -1;
        }

        void doListen(int SocketBacklog = 0) const {
            if (isClosed()) throw NetworkException(String(u"StreamSocketServer::doListen(int=0) isClosed()"));
            if (!isBound()) throw NetworkException(String(u"StreamSocketServer::doListen(int=0) isBound()"));
            if (listen(SocketSource, SocketBacklog))
                throw NetworkException(String(u"StreamSocketServer::doListen(int=0) listen"));
        }

        bool isBound() const noexcept {
            return SocketBound;
        }

        bool isClosed() const noexcept {
            return SocketSource == (SocketHandleType) -1;
        }

        StreamSocketServer &operator=(const StreamSocketServer&) noexcept = delete;

        void setAddressReuse(bool OptionValue) const {
            if (setsockopt(SocketSource, SOL_SOCKET, SO_REUSEADDR, (char*) &OptionValue, sizeof(bool)))
                throw NetworkException(String(u"StreamSocketServer::setAddressReuse(bool) setsockopt"));
        }
    };

    class SocketInputStream : public IO::InputStream {
    private:
        SocketHandleType SocketHandle;

        SocketInputStream(const StreamSocket &SocketSource) noexcept : SocketHandle(SocketSource.getSocketHandle()) {}
    public:
        void doClose() override {
            if (isAvailable()) SocketHandle = (SocketHandleType) -1;
        }

        int doRead() override {
            if (!isAvailable()) throw NetworkException(String(u"SocketInputStream::doRead() isAvailable"));
            char SocketCharacter;
            recv(SocketHandle, &SocketCharacter, 1, 0);
            return SocketCharacter;
        }

        int doRead(byte *SocketBuffer, int SocketOffset, int SocketSize) override {
            if (!isAvailable()) throw NetworkException(String(u"SocketInputStream::doRead(byte*, int, int) isAvailable"));
            int SocketSizeReceived;
            if ((SocketSizeReceived = recv(SocketHandle, (char*) SocketBuffer + SocketOffset, SocketSize, 0)) < 0)
                throw NetworkException(String(u"SocketInputStream::doRead(byte*, int, int) recv"));
            return SocketSizeReceived;
        }

        static SocketInputStream getInstance(const StreamSocket &SocketSource) {
            if (!SocketSource.isConnected()) throw NetworkException(String(u"SocketInputStream::getInstance(const StreamSocket&) SocketSource.isConnected"));
            return SocketInputStream(SocketSource);
        }

        bool isAvailable() const noexcept override {
            return SocketHandle != (SocketHandleType) -1;
        }
    };

    class SocketOutputStream : public IO::OutputStream {
    private:
        SocketHandleType SocketHandle;

        SocketOutputStream(const StreamSocket &SocketSource) noexcept : SocketHandle(SocketSource.getSocketHandle()) {}
    public:
        void doClose() override {
            if (isAvailable()) SocketHandle = (SocketHandleType) -1;
        }

        void doWrite(int SocketCharacter) override {
            if (!isAvailable()) throw NetworkException(String(u"SocketOutputStream::doRead() isAvailable"));
            send(SocketHandle, (char*) &SocketCharacter, 1, 0);
        }

        void doWrite(byte *SocketBuffer, int SocketOffset, int SocketSize) override {
            if (!isAvailable()) throw NetworkException(String(u"SocketOutputStream::doWrite(byte*, int, int) isAvailable"));
            if (send(SocketHandle, (char*) SocketBuffer + SocketOffset, SocketSize, 0) < 0)
                throw NetworkException(String(u"SocketOutputStream::doWrite(byte*, int, int) send"));
        }

        static SocketOutputStream getInstance(const StreamSocket &SocketSource) {
            if (!SocketSource.isConnected()) throw NetworkException(String(u"SocketOutputStream::getInstance(const StreamSocket&) SocketSource.isConnected"));
            return SocketOutputStream(SocketSource);
        }

        bool isAvailable() const noexcept {
            return SocketHandle != (SocketHandleType) -1;
        }
    };
}
