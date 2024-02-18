#pragma once

#ifndef eLibraryHeaderNetworkSocket
#define eLibraryHeaderNetworkSocket

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
                sockaddr_in AddressResult{};
                AddressResult.sin_addr = SocketAddress.toAddressIn4();
                AddressResult.sin_family = AF_INET;
                AddressResult.sin_port = htons(SocketPort);
                return AddressResult;
            }
            doThrowChecked(NetworkException(u"NetworkSocketAddress::toAddressIn4() SocketAddress::getAddressProtocol"_S));
        }

        auto toAddressIn6() const {
            if (SocketAddress.getAddressProtocol() == NetworkAddress::NetworkAddressProtocol::ProtocolIPv6) {
                sockaddr_in6 AddressResult{};
                AddressResult.sin6_addr = SocketAddress.toAddressIn6();
                AddressResult.sin6_family = AF_INET6;
                AddressResult.sin6_port = htons(SocketPort);
                return AddressResult;
            }
            doThrowChecked(NetworkException(u"NetworkSocketAddress::toAddressIn6() SocketAddress::getAddressProtocol"_S));
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

    class DatagramSocket final : public Object, public NonCopyable {
    private:
        NetworkSocketDescriptor SocketDescriptor;
        NetworkSocketAddress SocketAddress;
    public:
        explicit DatagramSocket(const NetworkSocketAddress &AddressSource) : SocketAddress(AddressSource) {
            if (AddressSource.getSocketAddress().getAddressProtocol() == NetworkAddress::NetworkAddressProtocol::ProtocolUnknown)
                doThrowChecked(NetworkException(u"DatagramSocket::DatagramSocket(const NetworkSocketAddress&) AddressSource"_S));
            SocketDescriptor.doAssign(::socket(AddressSource.getSocketAddress().getAddressFamily(), SOCK_DGRAM, 0));
            if (!SocketDescriptor.isAvailable()) doThrowChecked(NetworkException(u"DatagramSocket::DatagramSocket(const NetworkSocketAddress&) isAvailable"_S));
        }

        void doClose() {
            SocketDescriptor.doClose();
        }

        void doReceive(IO::ByteBuffer &SocketBuffer) {
            int SocketSize;
            if (SocketAddress.getSocketAddress().getAddressProtocol() == NetworkAddress::NetworkAddressProtocol::ProtocolIPv4) {
                auto SocketAddressIn = SocketAddress.toAddressIn4();
                if ((SocketSize = ::recvfrom((int) SocketDescriptor, (char*) SocketBuffer.getBufferContainer().getElementContainer() + SocketBuffer.getBufferPosition(), SocketBuffer.getRemaining(), 0, (sockaddr*) &SocketAddressIn, nullptr)) < 0)
                    doThrowChecked(NetworkException(u"DatagramSocket::doReceive(ByteBuffer&) recvfrom"_S));
            } else {
                auto SocketAddressIn = SocketAddress.toAddressIn6();
                if ((SocketSize = ::recvfrom((int) SocketDescriptor, (char*) SocketBuffer.getBufferContainer().getElementContainer() + SocketBuffer.getBufferPosition(), SocketBuffer.getRemaining(), 0, (sockaddr*) &SocketAddressIn, nullptr)) < 0)
                    doThrowChecked(NetworkException(u"DatagramSocket::doReceive(ByteBuffer&) recvfrom"_S));
            }
            SocketBuffer.setBufferPosition(SocketBuffer.getBufferPosition() + SocketSize);
        }

        void doSend(IO::ByteBuffer &SocketBuffer) {
            if (SocketAddress.getSocketAddress().getAddressProtocol() == NetworkAddress::NetworkAddressProtocol::ProtocolIPv4) {
                auto SocketAddressIn = SocketAddress.toAddressIn4();
                if (::sendto((int) SocketDescriptor, (char*) SocketBuffer.getBufferContainer().getElementContainer() + SocketBuffer.getBufferPosition(), SocketBuffer.getRemaining(), 0, (sockaddr*) &SocketAddressIn, sizeof(sockaddr_in)) < 0)
                    doThrowChecked(NetworkException(u"DatagramSocket::doSend(ByteBuffer&) sendto"_S));
            } else {
                auto SocketAddressIn = SocketAddress.toAddressIn6();
                if (::sendto((int) SocketDescriptor, (char*) SocketBuffer.getBufferContainer().getElementContainer() + SocketBuffer.getBufferPosition(), SocketBuffer.getRemaining(), 0, (sockaddr*) &SocketAddressIn, sizeof(sockaddr_in6)) < 0)
                    doThrowChecked(NetworkException(u"DatagramSocket::doSend(ByteBuffer&) sendto"_S));
            }
            SocketBuffer.setBufferPosition(SocketBuffer.getBufferLimit());
        }

        bool isAvailable() const noexcept {
            return SocketDescriptor.isAvailable();
        }

        void setSocketOption(NetworkSocketOption OptionType, int OptionValue) {
            if (::setsockopt((int) SocketDescriptor, SOL_SOCKET, (int) OptionType, (char*) &OptionValue, sizeof(int)))
                doThrowChecked(NetworkException(u"DatagramSocket::setSocketOption(NetworkSocketOption, int) setsockopt"_S));
        }

#if UDP_NOCHECKSUM
        void setUDPNoChecksum(bool OptionValue) {
            if (::setsockopt((int) SocketDescriptor, IPPROTO_UDP, UDP_NOCHECKSUM, (char*) &OptionValue, sizeof(bool)))
                doThrowChecked(NetworkException(u"DatagramSocket::setUDPNoChecksum(bool) setsockopt"_S));
        }
#endif
    };

    class StreamSocket final : public Object, public NonCopyable {
    private:
        bool SocketConnected = false;
        NetworkSocketDescriptor SocketDescriptor;
        NetworkSocketAddress SocketAddress;

        StreamSocket(NetworkSocketDescriptor SocketDescriptorSource, NetworkSocketAddress AddressSource) noexcept : SocketConnected(true), SocketDescriptor(Objects::doMove(SocketDescriptorSource)), SocketAddress(Objects::doMove(AddressSource)) {}

        friend class SocketInputStream;
        friend class SocketOutputStream;
        friend class StreamSocketServer;
    public:
        explicit StreamSocket(const NetworkSocketAddress &AddressSource) : SocketAddress(AddressSource) {
            if (AddressSource.getSocketAddress().getAddressProtocol() == NetworkAddress::NetworkAddressProtocol::ProtocolUnknown)
                doThrowChecked(NetworkException(u"StreamSocket::StreamSocket(const NetworkSocketAddress&) AddressSource"_S));
            SocketDescriptor.doAssign(::socket(AddressSource.getSocketAddress().getAddressFamily(), SOCK_STREAM, 0));
            if (!SocketDescriptor.isAvailable()) doThrowChecked(NetworkException(u"StreamSocket::StreamSocket(const NetworkSocketAddress&) SocketDescriptor::isAvailable"_S));
        }

        void doClose() {
            SocketDescriptor.doClose();
        }

        void doConnect() {
            if (isConnected()) doThrowChecked(NetworkException(u"StreamSocket::doConnect() isConnected"_S));
            if (SocketAddress.getSocketAddress().getAddressProtocol() == NetworkAddress::NetworkAddressProtocol::ProtocolIPv4) {
                auto SocketAddress4 = SocketAddress.toAddressIn4();
                if (::connect((int) SocketDescriptor, (sockaddr*) &SocketAddress4, sizeof(sockaddr_in)))
                    doThrowChecked(NetworkException(u"StreamSocket::doConnect() connect"_S));
            } else {
                auto SocketAddress6 = SocketAddress.toAddressIn6();
                if (::connect((int) SocketDescriptor, (sockaddr*) &SocketAddress6, sizeof(sockaddr_in6)))
                    doThrowChecked(NetworkException(u"StreamSocket::doConnect() connect"_S));
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
                doThrowChecked(NetworkException(u"StreamSocket::setSocketOption(NetworkSocketOption, int) setsockopt"_S));
        }

        void setTCPNoDelay(bool OptionValue) const {
            if (::setsockopt((int) SocketDescriptor, IPPROTO_TCP, TCP_NODELAY, (char*) &OptionValue, sizeof(bool)))
                doThrowChecked(NetworkException(u"StreamSocket::setTCPNoDelay(bool) setsockopt"_S));
        }

        String toString() const noexcept override {
            return u"StreamSocket[SocketAddress="_S.doConcat(SocketAddress.toString()).doConcat(u']');
        }
    };

    class StreamSocketServer final : public Object, public NonCopyable {
    private:
        bool SocketBound = false;
        NetworkSocketDescriptor SocketDescriptor;
        NetworkSocketAddress SocketAddress;
    public:
        explicit StreamSocketServer(const NetworkSocketAddress &AddressSource) : SocketAddress(AddressSource) {
            if (AddressSource.getSocketAddress().getAddressProtocol() == NetworkAddress::NetworkAddressProtocol::ProtocolUnknown)
                doThrowChecked(NetworkException(u"StreamSocketServer::StreamSocketServer(const NetworkSocketAddress&) AddressSource"_S));
            SocketDescriptor.doAssign(::socket(SocketAddress.getSocketAddress().getAddressFamily(), SOCK_STREAM, 0));
            if (!SocketDescriptor.isAvailable()) doThrowChecked(NetworkException(u"StreamSocket::StreamSocket(const NetworkSocketAddress&) SocketDescriptor::isAvailable"_S));
        }

        ~StreamSocketServer() noexcept {
            if (isAvailable()) doClose();
        }

        StreamSocket doAccept() const {
            if (!isAvailable()) doThrowChecked(NetworkException(u"StreamSocketServer::doAccept() isAvailable"_S));
            if (!isBound()) doThrowChecked(NetworkException(u"StreamSocketServer::doAccept() isBound"_S));
            int SocketTarget = ::accept((int) SocketDescriptor, nullptr, nullptr);
            sockaddr_storage SocketTargetAddress{};
            socklen_t SocketTargetAddressSize = sizeof(sockaddr_storage);
            if (::getpeername(SocketTarget, (sockaddr*) &SocketTargetAddress, &SocketTargetAddressSize))
                doThrowChecked(NetworkException(u"StreamSocketServer::doAccept() getpeername"_S));
            if (((sockaddr*) &SocketTargetAddress)->sa_family == AF_INET) return {SocketTarget, NetworkSocketAddress(NetworkAddress(((sockaddr_in*) &SocketTargetAddress)->sin_addr), ntohs(((sockaddr_in*) &SocketTargetAddress)->sin_port))};
            else if (((sockaddr*) &SocketTargetAddress)->sa_family == AF_INET6) return {SocketTarget, NetworkSocketAddress(NetworkAddress(((sockaddr_in6*) &SocketTargetAddress)->sin6_addr), ntohs(((sockaddr_in6*) &SocketTargetAddress)->sin6_port))};
            else doThrowChecked(NetworkException(u"StreamSocketServer::doAccept() SocketTargetAddress.sa_family"_S));
        }

        void doBind() {
            if (!isAvailable()) doThrowChecked(NetworkException(u"StreamSocketServer::doBind() isClosed"_S));
            if (isBound()) doThrowChecked(NetworkException(u"StreamSocketServer::doBind() isBound"_S));
            if (SocketAddress.getSocketAddress().getAddressProtocol() == NetworkAddress::NetworkAddressProtocol::ProtocolIPv4) {
                auto SocketAddress4 = SocketAddress.toAddressIn4();
                if (::bind((int) SocketDescriptor, (sockaddr*) &SocketAddress4, sizeof(sockaddr_in)))
                    doThrowChecked(NetworkException(u"StreamSocketServer::doBind() bind"_S));
            } else {
                auto SocketAddress6 = SocketAddress.toAddressIn6();
                if (::bind((int) SocketDescriptor, (sockaddr*) &SocketAddress6, sizeof(sockaddr_in6)))
                    doThrowChecked(NetworkException(u"StreamSocketServer::doBind() bind"_S));
            }
            SocketBound = true;
        }

        void doClose() {
            SocketDescriptor.doClose();
        }

        void doListen(int SocketBacklog = 0) const {
            if (!isAvailable()) doThrowChecked(NetworkException(u"StreamSocketServer::doListen(int=0) isAvailable()"_S));
            if (!isBound()) doThrowChecked(NetworkException(u"StreamSocketServer::doListen(int=0) isBound()"_S));
            if (::listen((int) SocketDescriptor, SocketBacklog))
                doThrowChecked(NetworkException(u"StreamSocketServer::doListen(int=0) listen"_S));
        }

        bool isAvailable() const noexcept {
            return SocketDescriptor.isAvailable();
        }

        bool isBound() const noexcept {
            return SocketBound;
        }

        void setSocketOption(NetworkSocketOption OptionType, int OptionValue) {
            if (::setsockopt((int) SocketDescriptor, SOL_SOCKET, (int) OptionType, (char*) &OptionValue, sizeof(int)))
                doThrowChecked(NetworkException(u"StreamSocketServer::setSocketOption(NetworkSocketOption, int) setsockopt"_S));
        }

        void setTCPNoDelay(bool OptionValue) const {
            if (::setsockopt((int) SocketDescriptor, IPPROTO_TCP, TCP_NODELAY, (char*) &OptionValue, sizeof(bool)))
                doThrowChecked(NetworkException(u"StreamSocketServer::setTCPNoDelay(bool) setsockopt"_S));
        }
    };

    class SocketInputStream final : public IO::InputStream {
    private:
        StreamSocket &SocketDescriptor;

        SocketInputStream(StreamSocket &SocketSource) noexcept : SocketDescriptor(SocketSource) {}
    public:
        int doRead() override {
            if (!isAvailable()) doThrowChecked(NetworkException(u"SocketInputStream::doRead() isAvailable"_S));
            char SocketCharacter;
            if (::recv((int) SocketDescriptor.SocketDescriptor, &SocketCharacter, 1, 0) < 0)
                doThrowChecked(NetworkException(u"SocketInputStream::doRead() recv"_S));
            return SocketCharacter;
        }

        void doRead(IO::ByteBuffer &SocketBuffer) override {
            if (!isAvailable()) doThrowChecked(NetworkException(u"SocketInputStream::doRead(ByteBuffer&) isAvailable"_S));
            int SocketSizeReceived;
            if ((SocketSizeReceived = ::recv((int) SocketDescriptor.SocketDescriptor, (char*) SocketBuffer.getBufferContainer().getElementContainer() + SocketBuffer.getBufferPosition(), SocketBuffer.getRemaining(), 0)) < 0)
                doThrowChecked(NetworkException(u"SocketInputStream::doRead(ByteBuffer&) recv"_S));
            SocketBuffer.setBufferPosition(SocketBuffer.getBufferPosition() + SocketSizeReceived);
        }

        static SocketInputStream getInstance(StreamSocket &SocketSource) {
            if (!SocketSource.isConnected()) doThrowChecked(NetworkException(u"SocketInputStream::getInstance(StreamSocket&) SocketSource.isConnected"_S));
            return {SocketSource};
        }

        bool isAvailable() const noexcept override {
            return SocketDescriptor.isAvailable();
        }
    };

    class SocketOutputStream final : public IO::OutputStream {
    private:
        StreamSocket &SocketDescriptor;

        SocketOutputStream(StreamSocket &SocketSource) noexcept : SocketDescriptor(SocketSource) {}
    public:
        void doWrite(uint8_t SocketCharacter) override {
            if (!isAvailable()) doThrowChecked(NetworkException(u"SocketOutputStream::doWrite(uint8_t) isAvailable"_S));
            if (::send((int) SocketDescriptor.SocketDescriptor, (char*) &SocketCharacter, 1, 0) < 0)
                doThrowChecked(NetworkException(u"SocketOutputStream::doWrite(uint8_t) send"_S));
        }

        void doWrite(const IO::ByteBuffer &SocketBuffer) override {
            if (!isAvailable()) doThrowChecked(NetworkException(u"SocketOutputStream::doWrite(const ByteBuffer&) isAvailable"_S));
            if (::send((int) SocketDescriptor.SocketDescriptor, (char*) SocketBuffer.getBufferContainer().getElementContainer() + SocketBuffer.getBufferPosition(), SocketBuffer.getRemaining(), 0) < 0)
                doThrowChecked(NetworkException(u"SocketOutputStream::doWrite(const ByteBuffer&) send"_S));
            SocketBuffer.setBufferPosition(SocketBuffer.getBufferLimit());
        }

        static SocketOutputStream getInstance(StreamSocket &SocketSource) {
            if (!SocketSource.isConnected()) doThrowChecked(NetworkException(u"SocketOutputStream::getInstance(StreamSocket&) SocketSource::isConnected"_S));
            return {SocketSource};
        }

        bool isAvailable() const noexcept override {
            return SocketDescriptor.isAvailable();
        }
    };
}
#endif

#endif
