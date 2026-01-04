#pragma once

#include <vector>
#include <cstddef>
#include <sys/types.h>

#include "listener.h"

namespace networking {

class TcpClientSocket {
public:

    // constructor 
    TcpClientSocket(int sockfd);

    // move constructor and assignment operator
    TcpClientSocket(TcpClientSocket&& other);
    TcpClientSocket& operator=(TcpClientSocket&& other);

    bool isOpen() const { return sockfd_ != -1; }

    // send and receive data
    std::vector<std::byte> recv(ssize_t BufferSize = 4096) const;
    bool send(const std::byte* data, size_t size) const;

private:

    friend class TcpListener;

    int sockfd_{ -1 };

};

}