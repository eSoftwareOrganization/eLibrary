#pragma once

#if eLibraryFeature(Network)

#include <IO/File.hpp>
#include <Network/Exception.hpp>

extern "C" {
#include <openssl/ssl.h>
}

#if eLibrarySystem(Windows)
#include <winsock2.h>

typedef SOCKET SocketHandleType;
#else
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <unistd.h>

typedef int SocketHandleType;
#endif

namespace eLibrary::Network {
    class NetworkSocketDescriptor final : public IO::FileDescriptor {
    private:
        doDisableCopyAssignConstruct(NetworkSocketDescriptor)
    public:
        constexpr NetworkSocketDescriptor() noexcept : IO::FileDescriptor() {}

        constexpr NetworkSocketDescriptor(int DescriptorSource) noexcept {
            DescriptorHandle = DescriptorSource;
        }

        constexpr NetworkSocketDescriptor(NetworkSocketDescriptor &&DescriptorSource) noexcept {
            DescriptorHandle = DescriptorSource.DescriptorHandle;
            DescriptorSource.DescriptorHandle = -1;
        }

        ~NetworkSocketDescriptor() noexcept {
            if (isAvailable()) doClose();
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
    };

    namespace OpenSSL {
        class NetworkSSLMethod final : public Object {
        private:
            const SSL_METHOD *MethodObject;

            constexpr NetworkSSLMethod(const SSL_METHOD *MethodSource) noexcept: MethodObject(MethodSource) {}

            doDisableCopyAssignConstruct(NetworkSSLMethod)
        public:
            static NetworkSSLMethod getMethodTLS() noexcept {
                return {TLS_method()};
            }

            static NetworkSSLMethod getMethodTLSClient() noexcept {
                return {TLS_client_method()};
            }

            static NetworkSSLMethod getMethodTLSServer() noexcept {
                return {TLS_server_method()};
            }

            explicit operator const SSL_METHOD*() const noexcept {
                return MethodObject;
            }
        };

        class NetworkSSLContext final : public Object {
        private:
            SSL_CTX *ContextObject;

            doDisableCopyAssignConstruct(NetworkSSLContext)
        public:
            explicit NetworkSSLContext(const NetworkSSLMethod &ContextMethod) {
                ContextObject = SSL_CTX_new((const SSL_METHOD*) ContextMethod);
                if (!ContextObject)
                    throw NetworkException(String(u"NetworkSSLContext::NetworkSSLContext(const NetworkSSLMethod&) SSL_CTX_new"));
            }

            ~NetworkSSLContext() noexcept {
                if (ContextObject) {
                    ::SSL_CTX_free(ContextObject);
                    ContextObject = nullptr;
                }
            }

            explicit operator SSL_CTX*() const noexcept {
                return ContextObject;
            }
        };

        class NetworkSSLDescriptor final : public Object {
        private:
            SSL *DescriptorObject;

            doDisableCopyAssignConstruct(NetworkSSLDescriptor)
        public:
            explicit NetworkSSLDescriptor(const NetworkSSLContext &DescriptorContext) {
                DescriptorObject = SSL_new((SSL_CTX *) DescriptorContext);
                if (!DescriptorObject)
                    throw NetworkException(String(u"NetworkSSLDescriptor::NetworkSSLDescriptor(const NetworkSSLContext&) SSL_new"));
            }

            ~NetworkSSLDescriptor() noexcept {
                if (DescriptorObject) {
                    SSL_free(DescriptorObject);
                    DescriptorObject = nullptr;
                }
            }

            void doConnect() const {
                if (SSL_connect(DescriptorObject) == -1)
                    throw NetworkException(String(u"NetworkSSLDescriptor::doConnect() SSL_connect"));
            }

            int doRead(void *DescriptorBuffer, int DescriptorBufferSize) noexcept {
                return SSL_read(DescriptorObject, DescriptorBuffer, DescriptorBufferSize);
            }

            void doShutdown() noexcept {
                SSL_shutdown(DescriptorObject);
            }

            void doWrite(void *DescriptorBuffer, int DescriptorBufferSize) noexcept {
                SSL_write(DescriptorObject, DescriptorBuffer, DescriptorBufferSize);
            }

            void setFileDescriptor(const NetworkSocketDescriptor &DescriptorSource) noexcept {
                SSL_set_fd(DescriptorObject, (int) DescriptorSource);
            }

            explicit operator SSL *() const noexcept {
                return DescriptorObject;
            }
        };

        class NetworkSSLInitializer final : public Object {
        public:
            constexpr NetworkSSLInitializer() noexcept = delete;

            static void doDestroy() noexcept {}

            static void doInitialize() noexcept {
                SSL_library_init();
                OpenSSL_add_all_algorithms();
            }
        };
    }
}
#endif
