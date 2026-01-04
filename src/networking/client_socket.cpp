#include "client_socket.h"

#include <stdexcept>
#include <sys/socket.h>

namespace networking {

TcpClientSocket::TcpClientSocket(int sockfd) : sockfd_(sockfd) {}

TcpClientSocket::TcpClientSocket(TcpClientSocket&& other) : sockfd_(other.sockfd_) {
    other.sockfd_ = -1;
}

TcpClientSocket& TcpClientSocket::operator=(TcpClientSocket&& other) {
    if (this != &other) {
        sockfd_ = other.sockfd_;
        other.sockfd_ = -1;
    }
    return *this;
}

std::vector<std::byte> TcpClientSocket::recv(ssize_t BufferSize) const {
    std::vector<std::byte> buffer(BufferSize);
    ssize_t bytes_received = ::recv(sockfd_, &buffer[0], BufferSize, 0);
    if (bytes_received == -1) {
        throw std::runtime_error("recv error");
    }
    return buffer;
}

bool TcpClientSocket::send(const std::byte* data, size_t size) const {
    ssize_t bytes_sent = ::send(sockfd_, data, size, 0);
    return bytes_sent != -1 && static_cast<size_t>(bytes_sent) == size;
}

TcpClientSocket TcpListener::accept() {
    socklen_t addr_len = sizeof(client_addr_);
    int client_fd = ::accept(sockfd_, (struct sockaddr *)&client_addr_, &addr_len);
    if (client_fd != -1) {
        client_sockets_.insert(client_fd);
    }
    return TcpClientSocket(client_fd);
}

void TcpListener::close(TcpClientSocket& client_socket) noexcept {
    int client_fd = client_socket.sockfd_;
    if (client_fd != -1 && client_sockets_.count(client_fd)) {
        ::close(client_fd);
        client_sockets_.erase(client_fd);
    }
    client_socket.sockfd_ = -1;
}

}