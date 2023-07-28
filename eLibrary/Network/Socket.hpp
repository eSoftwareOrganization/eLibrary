#pragma once

#if eLibraryFeature(Network)

#include <Core/Number.hpp>
#include <IO/Stream.hpp>
#include <Network/NetworkUtility.hpp>

#include <algorithm>
#include <any>
#include <array>

namespace eLibrary::Network {
    class NetworkAddress final : public Object {
    public:
        enum class NetworkAddressProtocol {
            ProtocolIPv4, ProtocolIPv6, ProtocolUnknown
        };
    protected:
        NetworkAddressProtocol AddressProtocol;
        uint8_t *AddressFieldList;
    public:
        explicit NetworkAddress(const std::array<uint8_t, 4> &AddressSource) noexcept : AddressProtocol(NetworkAddressProtocol::ProtocolIPv4) {
            AddressFieldList = new uint8_t[4];
            std::copy(AddressSource.begin(), AddressSource.end(), AddressFieldList);
        }

        explicit NetworkAddress(const std::array<uint8_t, 16> &AddressSource) noexcept : AddressProtocol(NetworkAddressProtocol::ProtocolIPv6) {
            AddressFieldList = new uint8_t[16];
            std::copy(AddressSource.begin(), AddressSource.end(), AddressFieldList);
        }

        NetworkAddress(const std::initializer_list<uint8_t> &AddressSource) noexcept {
            if (AddressSource.size() == 4) {
                AddressFieldList = new uint8_t[4];
                std::copy(AddressSource.begin(), AddressSource.end(), AddressFieldList);
                AddressProtocol = NetworkAddressProtocol::ProtocolIPv4;
            } else if (AddressSource.size() == 16) {
                AddressFieldList = new uint8_t[16];
                std::copy(AddressSource.begin(), AddressSource.end(), AddressFieldList);
                AddressProtocol = NetworkAddressProtocol::ProtocolIPv6;
            } else {
                AddressFieldList = nullptr;
                AddressProtocol = NetworkAddressProtocol::ProtocolUnknown;
            }
        }

        explicit NetworkAddress(const in_addr &AddressSource) noexcept : AddressProtocol(NetworkAddressProtocol::ProtocolIPv4) {
            AddressFieldList = new uint8_t[4];
            AddressFieldList[0] = AddressSource.s_addr & 0xFF;
            AddressFieldList[1] = AddressSource.s_addr >> 8 & 0xFF;
            AddressFieldList[2] = AddressSource.s_addr >> 16 & 0xFF;
            AddressFieldList[3] = AddressSource.s_addr >> 24 & 0xFF;
        }

        explicit NetworkAddress(const in6_addr &AddressSource) noexcept : AddressProtocol(NetworkAddressProtocol::ProtocolIPv6) {
            AddressFieldList = new uint8_t[16];
            memcpy(AddressFieldList, AddressSource.s6_addr, sizeof(uint8_t) * 16);
        }

        NetworkAddress(const NetworkAddress &AddressSource) noexcept {
            doAssign(AddressSource);
        }

        ~NetworkAddress() noexcept {
            if (AddressFieldList) {
                delete[] AddressFieldList;
                AddressFieldList = nullptr;
            }
        }

        void doAssign(const NetworkAddress &AddressSource) noexcept {
            AddressProtocol = AddressSource.AddressProtocol;
            delete[] AddressFieldList;
            if (AddressProtocol == NetworkAddressProtocol::ProtocolIPv4) {
                AddressFieldList = new uint8_t[4];
                memcpy(AddressFieldList, AddressSource.AddressFieldList, sizeof(uint8_t) * 4);
            } else if (AddressProtocol == NetworkAddressProtocol::ProtocolIPv6) {
                AddressFieldList = new uint8_t[16];
                memcpy(AddressFieldList, AddressSource.AddressFieldList, sizeof(uint8_t) * 16);
            } else {
                AddressFieldList = nullptr;
                AddressProtocol = NetworkAddressProtocol::ProtocolUnknown;
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

        NetworkAddress &operator=(const NetworkAddress &AddressSource) noexcept {
            doAssign(AddressSource);
            return *this;
        }

        Any toAddressIn() const {
            if (AddressProtocol == NetworkAddressProtocol::ProtocolIPv4) {
                in_addr AddressResult;
                AddressResult.s_addr = htonl((AddressFieldList[0] << 24) | (AddressFieldList[1] << 16) | (AddressFieldList[2] << 8) | AddressFieldList[3]);
                return {AddressResult};
            }
            if (AddressProtocol == NetworkAddressProtocol::ProtocolIPv6) {
                in6_addr AddressResult;
                memcpy(AddressResult.s6_addr, AddressFieldList, sizeof(uint8_t) * 16);
                return {AddressResult};
            }
            throw NetworkException(String(u"NetworkAddress::toAddressIn() AddressProtocol"));
        }

        String toString() const noexcept override {
            StringStream CharacterStream;
            if (AddressProtocol == NetworkAddressProtocol::ProtocolIPv4) {
                CharacterStream.addString(std::to_string(AddressFieldList[0]));
                CharacterStream.addCharacter(u'.');
                CharacterStream.addString(std::to_string(AddressFieldList[1]));
                CharacterStream.addCharacter(u'.');
                CharacterStream.addString(std::to_string(AddressFieldList[2]));
                CharacterStream.addCharacter(u'.');
                CharacterStream.addString(std::to_string(AddressFieldList[3]));
            }
            if (AddressProtocol == NetworkAddressProtocol::ProtocolIPv6) {
                for (int AddressPartition = 0; AddressPartition < 8; ++AddressPartition) {
                    CharacterStream.addString(Integer((AddressFieldList[AddressPartition << 1] << 8) | AddressFieldList[AddressPartition << 1 | 1]).toString(16));
                    if (AddressPartition < 7) CharacterStream.addCharacter(u':');
                }
            }
            if (AddressProtocol == NetworkAddressProtocol::ProtocolUnknown) CharacterStream.addString({u"<Unknown>"});
            return CharacterStream.toString();
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

    enum class NetworkSocketOption : int {
        OptionAddressReuse = SO_REUSEADDR,
        OptionBroadcast = SO_BROADCAST,
        OptionKeepAlive = SO_KEEPALIVE,
        OptionLinger = SO_LINGER,
        OptionReceiveBufferSize = SO_RCVBUF,
        OptionReceiveTimeout = SO_RCVTIMEO,
        OptionSendBufferSize = SO_SNDBUF,
        OptionSendTimeout = SO_SNDTIMEO,
    };

    class DatagramSocket final : public Object {
    private:
        NetworkSocketDescriptor SocketDescriptor;
        NetworkSocketAddress SocketAddress;

        doDisableCopyAssignConstruct(DatagramSocket)
    public:
        DatagramSocket(const NetworkSocketAddress &AddressSource) : SocketAddress(AddressSource) {
            if (AddressSource.getSocketAddress().getAddressProtocol() == NetworkAddress::NetworkAddressProtocol::ProtocolUnknown)
                throw NetworkException(String(u"DatagramSocket::DatagramSocket(const NetworkSocketAddress&) AddressSource"));
            SocketDescriptor.doAssign(socket(AddressSource.getSocketAddress().getAddressProtocol() == NetworkAddress::NetworkAddressProtocol::ProtocolIPv4 ? AF_INET : AF_INET6, SOCK_DGRAM, 0));
            if (!SocketDescriptor.isAvailable()) throw NetworkException(String(u"DatagramSocket::DatagramSocket(const NetworkSocketAddress&) isAvailable"));
        }

        ~DatagramSocket() noexcept {
            if (isAvailable()) doClose();
        }

        void doClose() {
            SocketDescriptor.doClose();
        }

        int doReceive(char *SocketBuffer, int SocketBufferSize) {
            int SocketSize;
            if (SocketAddress.getSocketAddress().getAddressProtocol() == NetworkAddress::NetworkAddressProtocol::ProtocolIPv4) {
                sockaddr_in SocketAddressIn = {AF_INET, htons(SocketAddress.getSocketPort()), SocketAddress.getSocketAddress().toAddressIn().getValue<in_addr>(), {}};
                if ((SocketSize = recvfrom((SocketHandleType) SocketDescriptor, SocketBuffer, SocketBufferSize, 0, (sockaddr*) &SocketAddressIn, nullptr)) < 0)
                    throw NetworkException(String(u"DatagramSocket::doReceive(char*, int) recvfrom"));
            } else {
                sockaddr_in6 SocketAddressIn = {AF_INET6, htons(SocketAddress.getSocketPort()), 0, SocketAddress.getSocketAddress().toAddressIn().getValue<in6_addr>(), {}};
                if ((SocketSize = recvfrom((SocketHandleType) SocketDescriptor, SocketBuffer, SocketBufferSize, 0, (sockaddr*) &SocketAddressIn, nullptr)) < 0)
                    throw NetworkException(String(u"DatagramSocket::doReceive(char*, int) recvfrom"));
            }
            return SocketSize;
        }

        int doSend(char *SocketBuffer, int SocketBufferSize) {
            int SocketSize;
            if (SocketAddress.getSocketAddress().getAddressProtocol() == NetworkAddress::NetworkAddressProtocol::ProtocolIPv4) {
                sockaddr_in SocketAddressIn;
                SocketAddressIn.sin_addr = SocketAddress.getSocketAddress().toAddressIn().getValue<in_addr>();
                SocketAddressIn.sin_family = AF_INET;
                SocketAddressIn.sin_port = htons(SocketAddress.getSocketPort());
                if ((SocketSize = sendto((SocketHandleType) SocketDescriptor, SocketBuffer, SocketBufferSize, 0, (sockaddr*) &SocketAddressIn, sizeof(sockaddr_in))) < 0)
                    throw NetworkException(String(u"DatagramSocket::doSend(char*, int) sendto"));
            } else {
                sockaddr_in6 SocketAddressIn = {AF_INET6, htons(SocketAddress.getSocketPort()), 0, SocketAddress.getSocketAddress().toAddressIn().getValue<in6_addr>(), {}};
                if ((SocketSize = sendto((SocketHandleType) SocketDescriptor, SocketBuffer, SocketBufferSize, 0, (sockaddr*) &SocketAddressIn, sizeof(sockaddr_in6))) < 0)
                    throw NetworkException(String(u"DatagramSocket::doSend(char*, int) sendto"));
            }
            return SocketSize;
        }

        bool isAvailable() const noexcept {
            return SocketDescriptor.isAvailable();
        }

        void setSocketOption(NetworkSocketOption OptionType, int OptionValue) {
            if (::setsockopt((SocketHandleType) SocketDescriptor, SOL_SOCKET, (int) OptionType, (char*) &OptionValue, sizeof(int)))
                throw NetworkException(String(u"DatagramSocket::setSendTimeout(NetworkSocketOption, int) setsockopt"));
        }
    };

    class StreamSocket final : public Object {
    private:
        bool SocketConnected = false;
        NetworkSocketDescriptor SocketDescriptor;
        NetworkSocketAddress SocketAddress;

        StreamSocket(NetworkSocketDescriptor SocketDescriptorSource, const NetworkSocketAddress &AddressSource) noexcept : SocketDescriptor(std::move(SocketDescriptorSource)), SocketAddress(AddressSource) {}

        doDisableCopyAssignConstruct(StreamSocket)

        friend class SocketInputStream;
        friend class SocketOutputStream;
        friend class StreamSocketServer;
    public:
        explicit StreamSocket(const NetworkSocketAddress &AddressSource) : SocketAddress(AddressSource) {
            if (AddressSource.getSocketAddress().getAddressProtocol() == NetworkAddress::NetworkAddressProtocol::ProtocolUnknown)
                throw NetworkException(String(u"StreamSocket::StreamSocket(const NetworkSocketAddress&) AddressSource"));
            SocketDescriptor.doAssign(socket(AddressSource.getSocketAddress().getAddressProtocol() == NetworkAddress::NetworkAddressProtocol::ProtocolIPv4 ? AF_INET : AF_INET6, SOCK_STREAM, 0));
            if (!SocketDescriptor.isAvailable()) throw NetworkException(String(u"StreamSocket::StreamSocket(const NetworkSocketAddress&) SocketDescriptor::isAvailable"));
        }

        ~StreamSocket() noexcept {
            if (isAvailable()) doClose();
        }

        void doClose() {
            SocketDescriptor.doClose();
        }

        void doConnect() {
            if (isConnected()) throw NetworkException(String(u"StreamSocket::doConnect() isConnected"));
            if (SocketAddress.getSocketAddress().getAddressProtocol() == NetworkAddress::NetworkAddressProtocol::ProtocolIPv4) {
                sockaddr_in SocketAddress4 = {AF_INET, htons(SocketAddress.getSocketPort()), SocketAddress.getSocketAddress().toAddressIn().getValue<in_addr>(), {}};
                if (connect((SocketHandleType) SocketDescriptor, (sockaddr*) &SocketAddress4, sizeof(sockaddr_in)) != 0)
                    throw NetworkException(String(u"StreamSocket::doConnect() connect"));
            } else {
                sockaddr_in6 SocketAddress6 = {AF_INET6, htons(SocketAddress.getSocketPort()), 0, SocketAddress.getSocketAddress().toAddressIn().getValue<in6_addr>(), {}};
                if (connect((SocketHandleType) SocketDescriptor, (sockaddr*) &SocketAddress6, sizeof(sockaddr_in6)) != 0)
                    throw NetworkException(String(u"StreamSocket::doConnect() connect"));
            }
            SocketConnected = true;
        }

        bool isAvailable() const noexcept {
            return SocketDescriptor.isAvailable();
        }

        bool isConnected() const noexcept {
            return SocketConnected;
        }

        NetworkSocketAddress getRemoteSocketAddress() const noexcept {
            return SocketAddress;
        }

        void setSocketOption(NetworkSocketOption OptionType, int OptionValue) {
            if (::setsockopt((SocketHandleType) SocketDescriptor, SOL_SOCKET, (int) OptionType, (char*) &OptionValue, sizeof(int)))
                throw NetworkException(String(u"StreamSocket::setSocketOption(NetworkSocketOption, int) setsockopt"));
        }

        void setTCPNoDelay(bool OptionValue) const {
            if (::setsockopt((SocketHandleType) SocketDescriptor, IPPROTO_TCP, TCP_NODELAY, (char*) &OptionValue, sizeof(bool)) != 0)
                throw NetworkException(String(u"StreamSocket::setTCPNoDelay(bool) setsockopt"));
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
        NetworkSocketDescriptor SocketDescriptor;
        NetworkSocketAddress SocketAddress;

        doDisableCopyAssignConstruct(StreamSocketServer)
    public:
        explicit StreamSocketServer(const NetworkSocketAddress &AddressSource) : SocketAddress(AddressSource) {
            if (AddressSource.getSocketAddress().getAddressProtocol() == NetworkAddress::NetworkAddressProtocol::ProtocolUnknown)
                throw NetworkException(String(u"StreamSocketServer::StreamSocketServer(const NetworkSocketAddress&) AddressSource"));
            SocketDescriptor.doAssign(socket(SocketAddress.getSocketAddress().getAddressProtocol() == NetworkAddress::NetworkAddressProtocol::ProtocolIPv4 ? AF_INET : AF_INET6, SOCK_STREAM, 0));
            if (!SocketDescriptor.isAvailable()) throw NetworkException(String(u"StreamSocket::StreamSocket(const NetworkSocketAddress&) SocketDescriptor::isAvailable"));
        }

        ~StreamSocketServer() noexcept {
            if (isAvailable()) doClose();
        }

        StreamSocket doAccept() const {
            if (!isAvailable()) throw NetworkException(String(u"StreamSocketServer::doAccept() isAvailable"));
            if (!isBound()) throw NetworkException(String(u"StreamSocketServer::doAccept() isBound"));
            SocketHandleType SocketTarget = accept((SocketHandleType) SocketDescriptor, nullptr, nullptr);
            sockaddr SocketTargetAddress;
            getpeername(SocketTarget, &SocketTargetAddress, nullptr);
            if (SocketTargetAddress.sa_family == AF_INET) return {SocketTarget, NetworkSocketAddress(NetworkAddress(((sockaddr_in*) &SocketTargetAddress)->sin_addr), ntohs(((sockaddr_in*) &SocketTargetAddress)->sin_port))};
            else if (SocketTargetAddress.sa_family == AF_INET6) return {SocketTarget, NetworkSocketAddress(NetworkAddress(((sockaddr_in6*) &SocketTargetAddress)->sin6_addr), ntohs(((sockaddr_in6*) &SocketTargetAddress)->sin6_port))};
            else throw NetworkException(String(u"StreamSocketServer::doAccept() SocketTargetAddress.sa_family"));
        }

        void doBind() {
            if (!isAvailable()) throw NetworkException(String(u"StreamSocketServer::doBind() isClosed"));
            if (isBound()) throw NetworkException(String(u"StreamSocketServer::doBind() isBound"));
            if (SocketAddress.getSocketAddress().getAddressProtocol() == NetworkAddress::NetworkAddressProtocol::ProtocolIPv4) {
                sockaddr_in SocketAddress4 = {AF_INET, htons(SocketAddress.getSocketPort()), SocketAddress.getSocketAddress().toAddressIn().getValue<in_addr>(), {}};
                if (bind((SocketHandleType) SocketDescriptor, (sockaddr*) &SocketAddress4, sizeof(sockaddr_in)))
                    throw NetworkException(String(u"StreamSocketServer::doBind() bind"));
            } else {
                sockaddr_in6 SocketAddress6 = {AF_INET6, htons(SocketAddress.getSocketPort()), 0, SocketAddress.getSocketAddress().toAddressIn().getValue<in6_addr>(), {}};
                if (bind((SocketHandleType) SocketDescriptor, (sockaddr*) &SocketAddress6, sizeof(sockaddr_in6)))
                    throw NetworkException(String(u"StreamSocketServer::doBind() bind"));
            }
            SocketBound = true;
        }

        void doClose() {
            SocketDescriptor.doClose();
        }

        void doListen(int SocketBacklog = 0) const {
            if (!isAvailable()) throw NetworkException(String(u"StreamSocketServer::doListen(int=0) isAvailable()"));
            if (!isBound()) throw NetworkException(String(u"StreamSocketServer::doListen(int=0) isBound()"));
            if (listen((SocketHandleType) SocketDescriptor, SocketBacklog))
                throw NetworkException(String(u"StreamSocketServer::doListen(int=0) listen"));
        }

        bool isAvailable() const noexcept {
            return SocketDescriptor.isAvailable();
        }

        bool isBound() const noexcept {
            return SocketBound;
        }

        void setSocketOption(NetworkSocketOption OptionType, int OptionValue) {
            if (::setsockopt((SocketHandleType) SocketDescriptor, SOL_SOCKET, (int) OptionType, (char*) &OptionValue, sizeof(int)))
                throw NetworkException(String(u"StreamSocketServer::setSocketOption(NetworkSocketOption, int) setsockopt"));
        }
    };

    class SocketInputStream final : public IO::InputStream {
    private:
        const NetworkSocketDescriptor *SocketDescriptor;

        SocketInputStream(const StreamSocket &SocketSource) noexcept : SocketDescriptor(&SocketSource.SocketDescriptor) {}

        doDisableCopyAssignConstruct(SocketInputStream)
    public:
        void doClose() override {
            SocketDescriptor = nullptr;
        }

        int doRead() override {
            if (!isAvailable()) throw NetworkException(String(u"SocketInputStream::doRead() isAvailable"));
            char SocketCharacter;
            recv((SocketHandleType) SocketDescriptor, &SocketCharacter, 1, 0);
            return SocketCharacter;
        }

        uint32_t doRead(uint8_t *SocketBuffer, uint32_t SocketOffset, uint32_t SocketSize) override {
            if (!isAvailable()) throw NetworkException(String(u"SocketInputStream::doRead(uint8_t*, uint32_t, uint32_t) isAvailable"));
            int SocketSizeReceived;
            if ((SocketSizeReceived = recv((SocketHandleType) SocketDescriptor, (char*) SocketBuffer + SocketOffset, SocketSize, 0)) < 0)
                throw NetworkException(String(u"SocketInputStream::doRead(uint8_t*, int, int) recv"));
            return SocketSizeReceived;
        }

        static SocketInputStream getInstance(const StreamSocket &SocketSource) {
            if (!SocketSource.isConnected()) throw NetworkException(String(u"SocketInputStream::getInstance(const StreamSocket&) SocketSource.isConnected"));
            return {SocketSource};
        }

        bool isAvailable() const noexcept override {
            return SocketDescriptor && SocketDescriptor->isAvailable();
        }
    };

    class SocketOutputStream final : public IO::OutputStream {
    private:
        const NetworkSocketDescriptor *SocketDescriptor;

        SocketOutputStream(const StreamSocket &SocketSource) noexcept : SocketDescriptor(&SocketSource.SocketDescriptor) {}

        doDisableCopyAssignConstruct(SocketOutputStream);
    public:
        void doClose() noexcept override {
            SocketDescriptor = nullptr;
        }

        void doWrite(int SocketCharacter) override {
            if (!isAvailable()) throw NetworkException(String(u"SocketOutputStream::doRead() isAvailable"));
            if (send((SocketHandleType) SocketDescriptor, (char*) &SocketCharacter, 1, 0) < 0)
                throw NetworkException(String(u"SocketOutputStream::doWrite(int) send"));
        }

        void doWrite(uint8_t *SocketBuffer, uint32_t SocketOffset, uint32_t SocketSize) override {
            if (!isAvailable()) throw NetworkException(String(u"SocketOutputStream::doWrite(uint8_t*, uint32_t, uint32_t) isAvailable"));
            if (send((SocketHandleType) SocketDescriptor, (char*) SocketBuffer + SocketOffset, SocketSize, 0) < 0)
                throw NetworkException(String(u"SocketOutputStream::doWrite(uint8_t*, uint32_t, uint32_t) send"));
        }

        static SocketOutputStream getInstance(const StreamSocket &SocketSource) {
            if (!SocketSource.isConnected()) throw NetworkException(String(u"SocketOutputStream::getInstance(const StreamSocket&) SocketSource::isConnected"));
            return {SocketSource};
        }

        bool isAvailable() const noexcept override {
            return SocketDescriptor && SocketDescriptor->isAvailable();
        }
    };
}
#endif
