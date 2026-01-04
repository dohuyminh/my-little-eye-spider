#pragma once

#include <string_view>
#include <cstdint>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unordered_set>
#include <stdexcept>

namespace networking {

class TcpClientSocket;

class TcpListener {
public:

    // Constructor: combines socket() and bind
    TcpListener(std::string_view address, uint16_t port);
    void setNonBlocking(bool nonBlocking) const;
    void listen(int backlog = SOMAXCONN) const;

    // accept a new client connection, returns client socket fd
    TcpClientSocket accept();

    void close(TcpClientSocket& client_socket) noexcept;

    // Move constructor and assignment operator
    TcpListener(TcpListener&& other);
    TcpListener& operator=(TcpListener&& other);

    // Destructor
    ~TcpListener();

    // copy constructor and assignment operator are deleted to prevent copying
    TcpListener(TcpListener const&) = delete;
    TcpListener& operator=(TcpListener const&) = delete;

private:

    friend class TcpClientSocket;

    int sockfd_;

    // store client's socket file descriptor after accept
    std::unordered_set<int> client_sockets_;

    sockaddr_storage client_addr_{};
};

}