#pragma once

#include <IO/Exception.hpp>

#include <algorithm>
#include <any>

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
            AddressFieldList[0] = AddressSource.s_addr & 0xFF000000 >> 24;
            AddressFieldList[1] = AddressSource.s_addr & 0x00FF0000 >> 16;
            AddressFieldList[2] = AddressSource.s_addr & 0x0000FF00 >> 8;
            AddressFieldList[3] = AddressSource.s_addr & 0x000000FF;
#endif
        }

        explicit NetworkAddress(const in6_addr &AddressSource) noexcept : AddressProtocol(NetworkAddressProtocol::ProtocolIPv6) {
            AddressFieldList = new byte[16];
            AddressFieldList[0] = AddressSource.s6_addr[0];
            AddressFieldList[1] = AddressSource.s6_addr[1];
            AddressFieldList[2] = AddressSource.s6_addr[2];
            AddressFieldList[3] = AddressSource.s6_addr[3];
            AddressFieldList[4] = AddressSource.s6_addr[4];
            AddressFieldList[5] = AddressSource.s6_addr[5];
            AddressFieldList[6] = AddressSource.s6_addr[6];
            AddressFieldList[7] = AddressSource.s6_addr[7];
            AddressFieldList[8] = AddressSource.s6_addr[8];
            AddressFieldList[9] = AddressSource.s6_addr[9];
            AddressFieldList[10] = AddressSource.s6_addr[10];
            AddressFieldList[11] = AddressSource.s6_addr[11];
            AddressFieldList[12] = AddressSource.s6_addr[12];
            AddressFieldList[13] = AddressSource.s6_addr[13];
            AddressFieldList[14] = AddressSource.s6_addr[14];
            AddressFieldList[15] = AddressSource.s6_addr[15];
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
#ifdef _WIN32
                AddressResult.S_un.S_un_b.s_b1 = AddressFieldList[0];
                AddressResult.S_un.S_un_b.s_b2 = AddressFieldList[1];
                AddressResult.S_un.S_un_b.s_b3 = AddressFieldList[2];
                AddressResult.S_un.S_un_b.s_b4 = AddressFieldList[3];
#else
                AddressResult.s_addr = (AddressFieldList[0] << 24) | (AddressFieldList[1] << 16) | (AddressFieldList[2] << 8) | AddressFieldList[3];
#endif
                return AddressResult;
            }
            if (AddressProtocol == NetworkAddressProtocol::ProtocolIPv6) {
                in6_addr AddressResult;
#ifdef _WIN32
                memcpy(AddressResult.u.Byte, AddressFieldList, sizeof(byte) * 16);
#else
                memcpy(AddressResult.s6_addr, AddressFieldList, sizeof(byte) * 16);
#endif
                return AddressResult;
            }
            throw Exception(String(u"NetworkAddress::toAddressIn() AddressProtocol"));
        }

        String toString() const noexcept override {
            std::stringstream CharacterStream;
            if (AddressProtocol == NetworkAddressProtocol::ProtocolIPv4) {
                CharacterStream << std::to_string(AddressFieldList[0]) << '.' << std::to_string(AddressFieldList[1]) << '.';
                CharacterStream << std::to_string(AddressFieldList[2]) << '.' << std::to_string(AddressFieldList[3]);
            }
            if (AddressProtocol == NetworkAddressProtocol::ProtocolIPv6) {
                CharacterStream << std::hex;
                for (int AddressPartition = 0; AddressPartition < 8; ++AddressPartition) {
                    CharacterStream << ((AddressFieldList[AddressPartition << 1] << 8) | AddressFieldList[AddressPartition << 1 | 1]);
                    if (AddressPartition < 7) CharacterStream << ':';
                }
            }
            if (AddressProtocol == NetworkAddressProtocol::ProtocolUnknown) CharacterStream << "<Unknown>";
            return CharacterStream.str();
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
                throw Exception(String(u"DatagramSocket::DatagramSocket(const NetworkSocketAddress&) AddressSource"));
            SocketSource = socket(AddressSource.getSocketAddress().getAddressProtocol() == NetworkAddress::NetworkAddressProtocol::ProtocolIPv4 ? AF_INET : AF_INET6, SOCK_DGRAM, 0);
            if (SocketSource == (SocketHandleType) -1) throw Exception(String(u"DatagramSocket::DatagramSocket(const NetworkSocketAddress&) socket"));
        }

        ~DatagramSocket() noexcept {
            if (!isClosed()) doClose();
        }

        void doClose() {
            if (isClosed()) throw Exception(String(u"DatagramSocket::doClose() isClosed"));
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
                    throw Exception(String(u"DatagramSocket::doReceive(char*, int) recvfrom"));
            } else {
                sockaddr_in6 SocketAddressIn;
                SocketAddressIn.sin6_addr = std::any_cast<in6_addr>(SocketAddress.getSocketAddress().toAddressIn());
                SocketAddressIn.sin6_family = AF_INET6;
                SocketAddressIn.sin6_port = htons(SocketAddress.getSocketPort());
                if ((SocketSize = recvfrom(SocketSource, SocketBuffer, SocketBufferSize, 0, (sockaddr*) &SocketAddressIn, nullptr)) < 0)
                    throw Exception(String(u"DatagramSocket::doReceive(char*, int) recvfrom"));
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
                    throw Exception(String(u"DatagramSocket::doSend(char*, int) sendto"));
            } else {
                sockaddr_in6 SocketAddressIn;
                SocketAddressIn.sin6_addr = std::any_cast<in6_addr>(SocketAddress.getSocketAddress().toAddressIn());
                SocketAddressIn.sin6_family = AF_INET6;
                SocketAddressIn.sin6_port = htons(SocketAddress.getSocketPort());
                if ((SocketSize = sendto(SocketSource, SocketBuffer, SocketBufferSize, 0, (sockaddr*) &SocketAddressIn, sizeof(sockaddr_in6))) < 0)
                    throw Exception(String(u"DatagramSocket::doSend(char*, int) sendto"));
            }
            return SocketSize;
        }

        bool isClosed() const noexcept {
            return SocketSource == (SocketHandleType) -1;
        }

        DatagramSocket &operator=(const DatagramSocket&) noexcept = delete;

        void setBroadcast(bool OptionValue) const {
            if (setsockopt(SocketSource, SOL_SOCKET, SO_BROADCAST, (char*) &OptionValue, sizeof(bool)) != 0)
                throw Exception(String(u"DatagramSocket::setBroadcast(bool) setsockopt"));
        }
    };

    class StreamSocket final : public Object {
    private:
        bool SocketConnected = false;
        SocketHandleType SocketSource;
        NetworkSocketAddress SocketAddress;

        StreamSocket(SocketHandleType SocketSourceSource, const NetworkSocketAddress &AddressSource) noexcept : SocketSource(SocketSourceSource), SocketAddress(AddressSource) {}
    public:
        explicit StreamSocket(const NetworkSocketAddress &AddressSource) : SocketAddress(AddressSource) {
            if (AddressSource.getSocketAddress().getAddressProtocol() == NetworkAddress::NetworkAddressProtocol::ProtocolUnknown)
                throw Exception(String(u"StreamSocket::StreamSocket(const NetworkSocketAddress&) AddressSource"));
            SocketSource = socket(AddressSource.getSocketAddress().getAddressProtocol() == NetworkAddress::NetworkAddressProtocol::ProtocolIPv4 ? AF_INET : AF_INET6, SOCK_STREAM, 0);
            if (SocketSource == (SocketHandleType) -1) throw Exception(String(u"StreamSocket::StreamSocket(const NetworkSocketAddress&) socket"));
        }

        ~StreamSocket() noexcept {
            if (!isClosed()) doClose();
        }

        void doClose() {
            if (isClosed()) throw Exception(String(u"StreamSocket::doClose() isClosed"));
#ifdef _WIN32
            closesocket(SocketSource);
#else
            close(SocketSource);
#endif
            SocketSource = -1;
        }

        void doConnect() {
            if (isConnected()) throw Exception(String(u"StreamSocket::doConnect() isConnected"));
            if (SocketAddress.getSocketAddress().getAddressProtocol() == NetworkAddress::NetworkAddressProtocol::ProtocolIPv4) {
                sockaddr_in SocketAddress4 = {AF_INET, htons(SocketAddress.getSocketPort()), std::any_cast<in_addr>(SocketAddress.getSocketAddress().toAddressIn())};
                if (connect(SocketSource, (sockaddr*) &SocketAddress4, sizeof(sockaddr_in)) != 0)
                    throw Exception(String(u"StreamSocket::doConnect() connect"));
            } else {
                sockaddr_in6 SocketAddress6 = {AF_INET6, htons(SocketAddress.getSocketPort()), 0, std::any_cast<in6_addr>(SocketAddress.getSocketAddress().toAddressIn())};
                if (connect(SocketSource, (sockaddr*) &SocketAddress6, sizeof(sockaddr_in6)) != 0)
                    throw Exception(String(u"StreamSocket::doConnect() connect"));
            }
            SocketConnected = true;
        }

        static StreamSocket doDeposit(SocketHandleType SocketSource, const NetworkSocketAddress &AddressSource) {
            if (SocketSource == (SocketHandleType) -1) throw Exception(String(u"StreamSocket::doDeposit(SocketHandleType, const NetworkSocketAddress&) SocketSource"));
            return {SocketSource, AddressSource};
        }

        int doReceive(char *SocketBuffer, int SocketBufferSize) const {
            if (!isConnected()) throw Exception(String(u"StreamSocket::doReceive(char*, int) isConnected"));
            int SocketSizeReceived;
            if ((SocketSizeReceived = recv(SocketSource, SocketBuffer, SocketBufferSize, 0)) < 0)
                throw Exception(String(u"StreamSocket::doReceive(char*, int) recv"));
            return SocketSizeReceived;
        }

        int doSend(char *SocketBuffer, int SocketBufferSize) const {
            if (!isConnected()) throw Exception(String(u"StreamSocket::doSend(char*, int) isConnected"));
            int SocketSizeSent;
            if ((SocketSizeSent = send(SocketSource, SocketBuffer, SocketBufferSize, 0)) < 0)
                throw Exception(String(u"StreamSocket::doSend(char*, int) send"));
            return SocketSizeSent;
        }

        bool isClosed() const noexcept {
            return SocketSource == (SocketHandleType) -1;
        }

        bool isConnected() const noexcept {
            return SocketConnected;
        }

        NetworkSocketAddress getRemoteSocketAddress() const noexcept {
            return SocketAddress;
        }

        StreamSocket &operator=(const StreamSocket&) noexcept = delete;

        void setAddressReuse(bool OptionValue) const {
            if (setsockopt(SocketSource, SOL_SOCKET, SO_REUSEADDR, (char*) &OptionValue, sizeof(bool)))
                throw Exception(String(u"StreamSocket::setAddressReuse(bool) setsockopt"));
        }

        void setKeepAlive(bool OptionValue) const {
            if (setsockopt(SocketSource, SOL_SOCKET, SO_KEEPALIVE, (char*) &OptionValue, sizeof(bool)) != 0)
                throw Exception(String(u"StreamSocket::setKeepAlive(bool) setsockopt"));
        }

        void setReceiveTimeout(int OptionValue) const {
            if (setsockopt(SocketSource, SOL_SOCKET, SO_RCVTIMEO, (char*) &OptionValue, sizeof(int)) != 0)
                throw Exception(String(u"StreamSocket::setReceiveTimeout(int) setsockopt"));
        }

        void setSendTimeout(int OptionValue) const {
            if (setsockopt(SocketSource, SOL_SOCKET, SO_SNDTIMEO, (char*) &OptionValue, sizeof(int)) != 0)
                throw Exception(String(u"StreamSocket::setSendTimeout(int) setsockopt"));
        }

        void setTCPNoDelay(bool OptionValue) const {
            if (setsockopt(SocketSource, IPPROTO_TCP, TCP_NODELAY, (char*) &OptionValue, sizeof(bool)) != 0)
                throw Exception(String(u"StreamSocket::setSendTimeout(bool) setsockopt"));
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
                throw Exception(String(u"StreamSocketServer::StreamSocketServer(const NetworkSocketAddress&) AddressSource"));
            SocketSource = socket(SocketAddress.getSocketAddress().getAddressProtocol() == NetworkAddress::NetworkAddressProtocol::ProtocolIPv4 ? AF_INET : AF_INET6, SOCK_STREAM, 0);
            if (SocketSource == (SocketHandleType) -1) throw Exception(String(u"StreamSocket::StreamSocket(const NetworkSocketAddress&) socket"));
        }

        ~StreamSocketServer() noexcept {
            if (!isClosed()) doClose();
        }

        StreamSocket doAccept() const {
            if (isClosed()) throw Exception(String(u"StreamSocketServer::doAccept() isClosed"));
            if (!isBound()) throw Exception(String(u"StreamSocketServer::doAccept() isBound"));
            SocketHandleType SocketTarget = accept(SocketSource, nullptr, nullptr);
            sockaddr SocketTargetAddress;
            getpeername(SocketTarget, &SocketTargetAddress, nullptr);
            if (SocketTargetAddress.sa_family == AF_INET) return StreamSocket::doDeposit(SocketTarget, NetworkSocketAddress(NetworkAddress(((sockaddr_in*) &SocketTargetAddress)->sin_addr), ntohs(((sockaddr_in*) &SocketTargetAddress)->sin_port)));
            else return StreamSocket::doDeposit(SocketTarget, NetworkSocketAddress(NetworkAddress(((sockaddr_in6*) &SocketTargetAddress)->sin6_addr), ntohs(((sockaddr_in6*) &SocketTargetAddress)->sin6_port)));
        }

        void doBind() {
            if (isClosed()) throw Exception(String(u"StreamSocketServer::doBind() isClosed"));
            if (isBound()) throw Exception(String(u"StreamSocketServer::doBind() isBound"));
            if (SocketAddress.getSocketAddress().getAddressProtocol() == NetworkAddress::NetworkAddressProtocol::ProtocolIPv4) {
                sockaddr_in SocketAddress4 = {AF_INET, htons(SocketAddress.getSocketPort()), std::any_cast<in_addr>(SocketAddress.getSocketAddress().toAddressIn())};
                if (bind(SocketSource, (sockaddr*) &SocketAddress4, sizeof(sockaddr_in)))
                    throw Exception(String(u"StreamSocketServer::doBind() bind"));
            } else {
                sockaddr_in6 SocketAddress6 = {AF_INET6, htons(SocketAddress.getSocketPort()), 0, std::any_cast<in6_addr>(SocketAddress.getSocketAddress().toAddressIn())};
                if (bind(SocketSource, (sockaddr*) &SocketAddress6, sizeof(sockaddr_in6)))
                    throw Exception(String(u"StreamSocketServer::doBind() bind"));
            }
            SocketBound = true;
        }

        void doClose() {
            if (isClosed()) throw Exception(String(u"StreamSocketServer::doClose() isClosed"));
#ifdef _WIN32
            closesocket(SocketSource);
#else
            close(SocketSource);
#endif
            SocketSource = -1;
        }

        void doListen(int SocketBacklog = 50) const {
            if (isClosed()) throw Exception(String(u"StreamSocketServer::doListen(int=50) isClosed()"));
            if (!isBound()) throw Exception(String(u"StreamSocketServer::doListen(int=50) isBound()"));
            if (listen(SocketSource, SocketBacklog))
                throw Exception(String(u"StreamSocketServer::doListen(int=50) listen"));
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
                throw Exception(String(u"StreamSocketServer::setAddressReuse(bool) setsockopt"));
        }
    };
}
