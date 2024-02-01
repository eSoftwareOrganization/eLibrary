#pragma once

#ifndef eLibraryHeaderNetworkNetworkUtility
#define eLibraryHeaderNetworkNetworkUtility

#if eLibraryFeature(Network)

#include <IO/IOUtility.hpp>
#include <Network/Exception.hpp>

#if eLibrarySystem(Windows)
#include <ws2tcpip.h>
#else
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <unistd.h>
#endif

#if eLibrarySystem(Windows)
typedef SOCKET SocketHandleType;
#else
typedef int SocketHandleType;
#endif

namespace eLibrary::Network {
    class NetworkAddress final : public Object {
    public:
        enum class NetworkAddressProtocol {
            ProtocolIPv4, ProtocolIPv6, ProtocolUnknown
        };
    protected:
        NetworkAddressProtocol AddressProtocol = NetworkAddressProtocol::ProtocolUnknown;
        Array<uint8_t> AddressFieldList;
    public:
        NetworkAddress(const Array<uint8_t> &AddressSource) {
            AddressFieldList.doAssign(AddressSource);
            if (AddressFieldList.getElementSize() == 4)
                AddressProtocol = NetworkAddressProtocol::ProtocolIPv4;
            else if (AddressFieldList.getElementSize() == 16)
                AddressProtocol = NetworkAddressProtocol::ProtocolIPv6;
            else AddressProtocol = NetworkAddressProtocol::ProtocolUnknown;
        }

        explicit NetworkAddress(const in_addr &AddressSource) : AddressProtocol(NetworkAddressProtocol::ProtocolIPv4) {
            AddressFieldList.doAssign({uint8_t(AddressSource.s_addr & 0xFF), uint8_t(AddressSource.s_addr >> 8 & 0xFF), uint8_t(AddressSource.s_addr >> 16 & 0xFF), uint8_t(AddressSource.s_addr >> 24 & 0xFF)});
        }

        explicit NetworkAddress(const in6_addr &AddressSource) : AddressProtocol(NetworkAddressProtocol::ProtocolIPv6) {
            AddressFieldList.doAssign(Array<uint8_t>(16));
            Collections::doCopy(AddressSource.s6_addr, 16, AddressFieldList.begin());
        }

        auto getAddressFamily() const noexcept {
            switch (AddressProtocol) {
                case NetworkAddressProtocol::ProtocolIPv4:
                    return AF_INET;
                case NetworkAddressProtocol::ProtocolIPv6:
                    return AF_INET6;
                default:
                    return AF_UNSPEC;
            }
        }

        NetworkAddressProtocol getAddressProtocol() const noexcept {
            return AddressProtocol;
        }

        bool isAnyLocalAddress() const noexcept {
            uint8_t AddressFieldResult = 0;
            if (AddressProtocol != NetworkAddressProtocol::ProtocolUnknown)
                for (uint8_t AddressField : AddressFieldList) AddressFieldResult |= AddressField;
            return AddressFieldResult == 0;
        }

        bool isLinkLocalAddress() const noexcept {
            if (AddressProtocol == NetworkAddressProtocol::ProtocolIPv4)
                return AddressFieldList.getElement(0) == 169 && AddressFieldList.getElement(1) == 254;
            if (AddressProtocol == NetworkAddressProtocol::ProtocolIPv6)
                return AddressFieldList.getElement(0) == 0xFE && AddressFieldList.getElement(1) == 0x80;
            return false;
        }

        bool isLoopbackAddress() const noexcept {
            if (AddressProtocol == NetworkAddressProtocol::ProtocolIPv4)
                return AddressFieldList.getElement(0) == 127 && AddressFieldList.getElement(1) == 0 && AddressFieldList.getElement(2) == 0 && AddressFieldList.getElement(3) == 1;
            if (AddressProtocol == NetworkAddressProtocol::ProtocolIPv6)
                return AddressFieldList.getElement(0) == 0 && AddressFieldList.getElement(1) == 0 && AddressFieldList.getElement(2) == 0 &&
                       AddressFieldList.getElement(3) == 0 && AddressFieldList.getElement(4) == 0 && AddressFieldList.getElement(5) == 0 &&
                       AddressFieldList.getElement(6) == 0 && AddressFieldList.getElement(7) == 0 && AddressFieldList.getElement(8) == 0 &&
                       AddressFieldList.getElement(9) == 0 && AddressFieldList.getElement(10) == 0 && AddressFieldList.getElement(11) == 0 &&
                       AddressFieldList.getElement(12) == 0 && AddressFieldList.getElement(13) == 0 && AddressFieldList.getElement(14) == 0 && AddressFieldList.getElement(15) == 0x01;
            return false;
        }

        bool isMulticastAddress() const noexcept {
            if (AddressProtocol == NetworkAddressProtocol::ProtocolIPv4)
                return AddressFieldList.getElement(0) >= 224 && AddressFieldList.getElement(0) <= 239;
            if (AddressProtocol == NetworkAddressProtocol::ProtocolIPv6)
                return AddressFieldList.getElement(0) == 0xFF;
            return false;
        }

        bool isSiteLocalAddress() const noexcept {
            if (AddressProtocol == NetworkAddressProtocol::ProtocolIPv4)
                return (AddressFieldList.getElement(0) == 192 && AddressFieldList.getElement(1) == 168) || AddressFieldList.getElement(0) == 10 || (AddressFieldList.getElement(0) == 172 && (AddressFieldList.getElement(1) & 0xF0) == 16);
            if (AddressProtocol == NetworkAddressProtocol::ProtocolIPv6)
                return AddressFieldList.getElement(0) == 0xFE && AddressFieldList.getElement(1) == 0xC0;
            return false;
        }

        auto toAddressIn4() const {
            if (AddressProtocol == NetworkAddressProtocol::ProtocolIPv4) {
                in_addr AddressResult{};
                AddressResult.s_addr = htonl((AddressFieldList.getElement(0) << 24) | (AddressFieldList.getElement(1) << 16) | (AddressFieldList.getElement(2) << 8) | AddressFieldList.getElement(3));
                return AddressResult;
            }
            throw NetworkException(String(u"NetworkAddress::toAddressIn4() AddressProtocol"));
        }

        auto toAddressIn6() const {
            if (AddressProtocol == NetworkAddressProtocol::ProtocolIPv6) {
                in6_addr AddressResult{};
                Collections::doCopy(AddressFieldList.begin(), 16, AddressResult.s6_addr);
                return AddressResult;
            }
            throw NetworkException(String(u"NetworkAddress::toAddressIn6() AddressProtocol"));
        }

        String toString() const noexcept override {
            StringStream CharacterStream;
            if (AddressProtocol == NetworkAddressProtocol::ProtocolIPv4) {
                CharacterStream.addString(String::valueOf(AddressFieldList.getElement(0)));
                CharacterStream.addCharacter(u'.');
                CharacterStream.addString(String::valueOf(AddressFieldList.getElement(1)));
                CharacterStream.addCharacter(u'.');
                CharacterStream.addString(String::valueOf(AddressFieldList.getElement(2)));
                CharacterStream.addCharacter(u'.');
                CharacterStream.addString(String::valueOf(AddressFieldList.getElement(3)));
            }
            if (AddressProtocol == NetworkAddressProtocol::ProtocolIPv6) {
                for (int AddressPartition = 0; AddressPartition < 8; ++AddressPartition) {
                    CharacterStream.addString(Integer((AddressFieldList.getElement(AddressPartition << 1) << 8) | AddressFieldList.getElement(AddressPartition << 1 | 1)).toString(16));
                    if (AddressPartition < 7) CharacterStream.addCharacter(u':');
                }
            }
            if (AddressProtocol == NetworkAddressProtocol::ProtocolUnknown) CharacterStream.addString({u"<Unknown>"});
            return CharacterStream.toString();
        }
    };

    class NetworkSocketDescriptor final : public IO::FileDescriptor {
    public:
        doEnableMoveAssignConstruct(NetworkSocketDescriptor)

        constexpr NetworkSocketDescriptor() noexcept = default;

        constexpr NetworkSocketDescriptor(int DescriptorSource) noexcept {
            DescriptorHandle = DescriptorSource;
        }

        void doAssign(NetworkSocketDescriptor &&DescriptorSource) noexcept {
            if (Objects::getAddress(DescriptorSource) == this) return;
            if (isAvailable()) doClose();
            DescriptorHandle = DescriptorSource.DescriptorHandle;
            DescriptorSource.DescriptorHandle = -1;
        }

        void doClose() override {
            if (!isAvailable()) throw NetworkException(String(u"NetworkSocketDescriptor::doClose() isAvailable"));
#if eLibrarySystem(Windows)
            closesocket(DescriptorHandle);
#else
            close(DescriptorHandle);
#endif
            DescriptorHandle = -1;
        }

        String toString() const noexcept override {
            return String(u"NetworkSocketDescriptor[DescriptorHandle=").doConcat(String::valueOf(DescriptorHandle)).doConcat(u']');
        }
    };

#if eLibrarySystem(Windows)
#define doDestroySocket() WSACleanup();
#define doInitializeSocket() WSADATA SocketData;WSAStartup(MAKEWORD(2, 2), &SocketData);
#else
#define doDestroySocket()
#define doInitializeSocket()
#endif
}
#endif

#endif
