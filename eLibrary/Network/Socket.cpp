#include <Network/Socket.hpp>

namespace eLibrary::Network {
    SocketInputStream StreamSocket::getInputStream() const {
        if (!isConnected()) throw NetworkException(String(u"StreamSocket::getInputStream() isConnected"));
        return SocketInputStream::getInstance(*this);
    }

    SocketOutputStream StreamSocket::getOutputStream() const {
        if (!isConnected()) throw NetworkException(String(u"StreamSocket::getOutputStream() isConnected"));
        return SocketOutputStream::getInstance(*this);
    }
}
