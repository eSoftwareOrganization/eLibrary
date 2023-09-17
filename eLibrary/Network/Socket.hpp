#pragma once

#if eLibraryFeature(Network)

#include <IO/Stream.hpp>
#include <Network/NetworkUtility.hpp>

namespace eLibrary::Network {
    class NetworkSocketAddress final : public Object {
    private:
        NetworkAddress SocketAddress;
        unsigned short SocketPort;
    public:
        NetworkSocketAddress(NetworkAddress AddressSource, unsigned short AddressPort) noexcept : SocketAddress(Objects::doMove(AddressSource)), SocketPort(AddressPort) {}

        NetworkAddress getSocketAddress() const noexcept {
            return SocketAddress;
        }

        unsigned short getSocketPort() const noexcept {
            return SocketPort;
        }

        auto toAddressIn4() const {
            if (SocketAddress.getAddressProtocol() == NetworkAddress::NetworkAddressProtocol::ProtocolIPv4) {
                sockaddr_in AddressResult = {AF_INET, htons(SocketPort), SocketAddress.toAddressIn4()};
                return AddressResult;
            }
            throw NetworkException(String(u"NetworkSocketAddress::toAddressIn4() SocketAddress::getAddressProtocol"));
        }

        auto toAddressIn6() const {
            if (SocketAddress.getAddressProtocol() == NetworkAddress::NetworkAddressProtocol::ProtocolIPv6) {
                sockaddr_in6 AddressResult = {AF_INET6, htons(SocketPort), 0, SocketAddress.toAddressIn6()};
                return AddressResult;
            }
            throw NetworkException(String(u"NetworkSocketAddress::toAddressIn6() SocketAddress::getAddressProtocol"));
        }

        String toString() const noexcept override {
            StringStream CharacterStream;
            CharacterStream.addString({u"NetworkSocketAddress[SocketAddress="});
            CharacterStream.addString(SocketAddress.toString());
            CharacterStream.addString({u", SocketPort="});
            CharacterStream.addString(String::valueOf(SocketPort));
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
        explicit DatagramSocket(const NetworkSocketAddress &AddressSource) : SocketAddress(AddressSource) {
            if (AddressSource.getSocketAddress().getAddressProtocol() == NetworkAddress::NetworkAddressProtocol::ProtocolUnknown)
                throw NetworkException(String(u"DatagramSocket::DatagramSocket(const NetworkSocketAddress&) AddressSource"));
            SocketDescriptor.doAssign(::socket(AddressSource.getSocketAddress().getAddressFamily(), SOCK_DGRAM, 0));
            if (!SocketDescriptor.isAvailable()) throw NetworkException(String(u"DatagramSocket::DatagramSocket(const NetworkSocketAddress&) isAvailable"));
        }

        void doClose() {
            SocketDescriptor.doClose();
        }

        void doReceive(IO::ByteBuffer &SocketBuffer) {
            int SocketSize;
            if (SocketAddress.getSocketAddress().getAddressProtocol() == NetworkAddress::NetworkAddressProtocol::ProtocolIPv4) {
                auto SocketAddressIn = SocketAddress.toAddressIn4();
                if ((SocketSize = ::recvfrom((int) SocketDescriptor, (char*) SocketBuffer.getBufferContainer() + SocketBuffer.getBufferPosition(), SocketBuffer.getRemaining(), 0, (sockaddr*) &SocketAddressIn, nullptr)) < 0)
                    throw NetworkException(String(u"DatagramSocket::doReceive(ByteBuffer&) recvfrom"));
            } else {
                auto SocketAddressIn = SocketAddress.toAddressIn6();
                if ((SocketSize = ::recvfrom((int) SocketDescriptor, (char*) SocketBuffer.getBufferContainer() + SocketBuffer.getBufferPosition(), SocketBuffer.getRemaining(), 0, (sockaddr*) &SocketAddressIn, nullptr)) < 0)
                    throw NetworkException(String(u"DatagramSocket::doReceive(ByteBuffer&) recvfrom"));
            }
            SocketBuffer.setBufferPosition(SocketBuffer.getBufferPosition() + SocketSize);
        }

        void doSend(IO::ByteBuffer &SocketBuffer) {
            if (SocketAddress.getSocketAddress().getAddressProtocol() == NetworkAddress::NetworkAddressProtocol::ProtocolIPv4) {
                auto SocketAddressIn = SocketAddress.toAddressIn4();
                if (::sendto((int) SocketDescriptor, (char*) SocketBuffer.getBufferContainer() + SocketBuffer.getBufferPosition(), SocketBuffer.getRemaining(), 0, (sockaddr*) &SocketAddressIn, sizeof(sockaddr_in)) < 0)
                    throw NetworkException(String(u"DatagramSocket::doSend(ByteBuffer&) sendto"));
            } else {
                auto SocketAddressIn = SocketAddress.toAddressIn6();
                if (::sendto((int) SocketDescriptor, (char*) SocketBuffer.getBufferContainer() + SocketBuffer.getBufferPosition(), SocketBuffer.getRemaining(), 0, (sockaddr*) &SocketAddressIn, sizeof(sockaddr_in6)) < 0)
                    throw NetworkException(String(u"DatagramSocket::doSend(ByteBuffer&) sendto"));
            }
            SocketBuffer.setBufferPosition(SocketBuffer.getBufferLimit());
        }

        bool isAvailable() const noexcept {
            return SocketDescriptor.isAvailable();
        }

        void setSocketOption(NetworkSocketOption OptionType, int OptionValue) {
            if (::setsockopt((int) SocketDescriptor, SOL_SOCKET, (int) OptionType, (char*) &OptionValue, sizeof(int)))
                throw NetworkException(String(u"DatagramSocket::setSocketOption(NetworkSocketOption, int) setsockopt"));
        }

        void setUDPNoChecksum(bool OptionValue) {
            if (::setsockopt((int) SocketDescriptor, IPPROTO_UDP, UDP_NOCHECKSUM, (char*) &OptionValue, sizeof(bool)))
                throw NetworkException(String(u"DatagramSocket::setUDPNoChecksum(bool) setsockopt"));
        }
    };

    class StreamSocket final : public Object {
    private:
        bool SocketConnected = false;
        NetworkSocketDescriptor SocketDescriptor;
        NetworkSocketAddress SocketAddress;

        StreamSocket(NetworkSocketDescriptor SocketDescriptorSource, NetworkSocketAddress AddressSource) noexcept : SocketConnected(true), SocketDescriptor(Objects::doMove(SocketDescriptorSource)), SocketAddress(Objects::doMove(AddressSource)) {}

        doDisableCopyAssignConstruct(StreamSocket)

        friend class SocketInputStream;
        friend class SocketOutputStream;
        friend class StreamSocketServer;
    public:
        explicit StreamSocket(const NetworkSocketAddress &AddressSource) : SocketAddress(AddressSource) {
            if (AddressSource.getSocketAddress().getAddressProtocol() == NetworkAddress::NetworkAddressProtocol::ProtocolUnknown)
                throw NetworkException(String(u"StreamSocket::StreamSocket(const NetworkSocketAddress&) AddressSource"));
            SocketDescriptor.doAssign(::socket(AddressSource.getSocketAddress().getAddressFamily(), SOCK_STREAM, 0));
            if (!SocketDescriptor.isAvailable()) throw NetworkException(String(u"StreamSocket::StreamSocket(const NetworkSocketAddress&) SocketDescriptor::isAvailable"));
        }

        void doClose() {
            SocketDescriptor.doClose();
        }

        void doConnect() {
            if (isConnected()) throw NetworkException(String(u"StreamSocket::doConnect() isConnected"));
            if (SocketAddress.getSocketAddress().getAddressProtocol() == NetworkAddress::NetworkAddressProtocol::ProtocolIPv4) {
                auto SocketAddress4 = SocketAddress.toAddressIn4();
                if (::connect((int) SocketDescriptor, (sockaddr*) &SocketAddress4, sizeof(sockaddr_in)))
                    throw NetworkException(String(u"StreamSocket::doConnect() connect"));
            } else {
                auto SocketAddress6 = SocketAddress.toAddressIn6();
                if (::connect((int) SocketDescriptor, (sockaddr*) &SocketAddress6, sizeof(sockaddr_in6)))
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
            if (::setsockopt((int) SocketDescriptor, SOL_SOCKET, (int) OptionType, (char*) &OptionValue, sizeof(int)))
                throw NetworkException(String(u"StreamSocket::setSocketOption(NetworkSocketOption, int) setsockopt"));
        }

        void setTCPNoDelay(bool OptionValue) const {
            if (::setsockopt((int) SocketDescriptor, IPPROTO_TCP, TCP_NODELAY, (char*) &OptionValue, sizeof(bool)))
                throw NetworkException(String(u"StreamSocket::setTCPNoDelay(bool) setsockopt"));
        }

        String toString() const noexcept override {
            return String(u"StreamSocket[SocketAddress=").doConcat(SocketAddress.toString()).doConcat(u']');
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
            SocketDescriptor.doAssign(::socket(SocketAddress.getSocketAddress().getAddressFamily(), SOCK_STREAM, 0));
            if (!SocketDescriptor.isAvailable()) throw NetworkException(String(u"StreamSocket::StreamSocket(const NetworkSocketAddress&) SocketDescriptor::isAvailable"));
        }

        ~StreamSocketServer() noexcept {
            if (isAvailable()) doClose();
        }

        StreamSocket doAccept() const {
            if (!isAvailable()) throw NetworkException(String(u"StreamSocketServer::doAccept() isAvailable"));
            if (!isBound()) throw NetworkException(String(u"StreamSocketServer::doAccept() isBound"));
            int SocketTarget = ::accept((int) SocketDescriptor, nullptr, nullptr);
            sockaddr_storage SocketTargetAddress{};
            int SocketTargetAddressSize = sizeof(sockaddr_storage);
            if (::getpeername(SocketTarget, (sockaddr*) &SocketTargetAddress, &SocketTargetAddressSize))
                throw NetworkException(String(u"StreamSocketServer::doAccept() getpeername"));
            if (((sockaddr*) &SocketTargetAddress)->sa_family == AF_INET) return {SocketTarget, NetworkSocketAddress(NetworkAddress(((sockaddr_in*) &SocketTargetAddress)->sin_addr), ntohs(((sockaddr_in*) &SocketTargetAddress)->sin_port))};
            else if (((sockaddr*) &SocketTargetAddress)->sa_family == AF_INET6) return {SocketTarget, NetworkSocketAddress(NetworkAddress(((sockaddr_in6*) &SocketTargetAddress)->sin6_addr), ntohs(((sockaddr_in6*) &SocketTargetAddress)->sin6_port))};
            else throw NetworkException(String(u"StreamSocketServer::doAccept() SocketTargetAddress.sa_family"));
        }

        void doBind() {
            if (!isAvailable()) throw NetworkException(String(u"StreamSocketServer::doBind() isClosed"));
            if (isBound()) throw NetworkException(String(u"StreamSocketServer::doBind() isBound"));
            if (SocketAddress.getSocketAddress().getAddressProtocol() == NetworkAddress::NetworkAddressProtocol::ProtocolIPv4) {
                auto SocketAddress4 = SocketAddress.toAddressIn4();
                if (::bind((int) SocketDescriptor, (sockaddr*) &SocketAddress4, sizeof(sockaddr_in)))
                    throw NetworkException(String(u"StreamSocketServer::doBind() bind"));
            } else {
                auto SocketAddress6 = SocketAddress.toAddressIn6();
                if (::bind((int) SocketDescriptor, (sockaddr*) &SocketAddress6, sizeof(sockaddr_in6)))
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
            if (::listen((int) SocketDescriptor, SocketBacklog))
                throw NetworkException(String(u"StreamSocketServer::doListen(int=0) listen"));
        }

        bool isAvailable() const noexcept {
            return SocketDescriptor.isAvailable();
        }

        bool isBound() const noexcept {
            return SocketBound;
        }

        void setSocketOption(NetworkSocketOption OptionType, int OptionValue) {
            if (::setsockopt((int) SocketDescriptor, SOL_SOCKET, (int) OptionType, (char*) &OptionValue, sizeof(int)))
                throw NetworkException(String(u"StreamSocketServer::setSocketOption(NetworkSocketOption, int) setsockopt"));
        }

        void setTCPNoDelay(bool OptionValue) const {
            if (::setsockopt((int) SocketDescriptor, IPPROTO_TCP, TCP_NODELAY, (char*) &OptionValue, sizeof(bool)))
                throw NetworkException(String(u"StreamSocketServer::setTCPNoDelay(bool) setsockopt"));
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
            if (::recv((int) *SocketDescriptor, &SocketCharacter, 1, 0) < 0)
                throw NetworkException(String(u"SocketInputStream::doRead() recv"));
            return SocketCharacter;
        }

        void doRead(IO::ByteBuffer &SocketBuffer) override {
            if (!isAvailable()) throw NetworkException(String(u"SocketInputStream::doRead(ByteBuffer&) isAvailable"));
            int SocketSizeReceived;
            if ((SocketSizeReceived = ::recv((int) *SocketDescriptor, (char*) SocketBuffer.getBufferContainer() + SocketBuffer.getBufferPosition(), SocketBuffer.getRemaining(), 0)) < 0)
                throw NetworkException(String(u"SocketInputStream::doRead(ByteBuffer&) recv"));
            SocketBuffer.setBufferPosition(SocketBuffer.getBufferPosition() + SocketSizeReceived);
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

        doDisableCopyAssignConstruct(SocketOutputStream)
    public:
        void doClose() noexcept override {
            SocketDescriptor = nullptr;
        }

        void doWrite(uint8_t SocketCharacter) override {
            if (!isAvailable()) throw NetworkException(String(u"SocketOutputStream::doWrite(uint8_t) isAvailable"));
            if (::send((int) *SocketDescriptor, (char*) &SocketCharacter, 1, 0) < 0)
                throw NetworkException(String(u"SocketOutputStream::doWrite(uint8_t) send"));
        }

        void doWrite(const IO::ByteBuffer &SocketBuffer) override {
            if (!isAvailable()) throw NetworkException(String(u"SocketOutputStream::doWrite(const ByteBuffer&) isAvailable"));
            if (::send((int) *SocketDescriptor, (char*) SocketBuffer.getBufferContainer() + SocketBuffer.getBufferPosition(), SocketBuffer.getRemaining(), 0) < 0)
                throw NetworkException(String(u"SocketOutputStream::doWrite(const ByteBuffer&) send"));
            SocketBuffer.setBufferPosition(SocketBuffer.getBufferLimit());
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
