#include "listener.h"

namespace networking {

TcpListener::TcpListener(std::string_view address, uint16_t port) {
    // Implementation of socket creation and binding goes here

    addrinfo hints{}, *servinfo, *p;
    hints.ai_family = AF_INET; // IPv4
    hints.ai_socktype = SOCK_STREAM; // TCP
    hints.ai_flags = AI_PASSIVE; // For wildcard IP address

    if (int s = getaddrinfo(address.empty() ? nullptr : address.data(),
                                 std::to_string(port).c_str(),
                                 &hints, &servinfo); s != 0) {
        throw std::runtime_error("getaddrinfo error: " + std::string(gai_strerror(s)));
    }   

    for (p = servinfo; p != nullptr; p = p->ai_next) {
        
        // socket
        sockfd_ = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (sockfd_ == -1) {
            continue; // try next
        }   

        int yes = 1;

        // socket options
        if (setsockopt(sockfd_, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
            ::close(sockfd_);
            continue; // try next
        }
        
        // bind
        if (bind(sockfd_, p->ai_addr, p->ai_addrlen) == -1) {
            ::close(sockfd_);
            continue; // try next
        }   

        break; // successfully bound
    }

    freeaddrinfo(servinfo);

    if (p == nullptr) {
        throw std::runtime_error("Failed to bind socket");
    }
}

void TcpListener::setNonBlocking(bool nonBlocking) const {
    int flags = fcntl(sockfd_, F_GETFL, 0);
    if (flags == -1) {
        throw std::runtime_error("fcntl F_GETFL error");
    }
    flags = nonBlocking ? (flags | O_NONBLOCK) : (flags & ~O_NONBLOCK);
    if (fcntl(sockfd_, F_SETFL, flags) == -1) {
        throw std::runtime_error("fcntl F_SETFL error");
    }   
}

void TcpListener::listen(int backlog) const {
    if (::listen(sockfd_, backlog) == -1) {
        throw std::runtime_error("listen error");
    }   
}

TcpListener::TcpListener(TcpListener&& other) 
    : sockfd_(other.sockfd_), client_sockets_(std::move(other.client_sockets_)), client_addr_(other.client_addr_) {
    other.sockfd_ = -1;
}

TcpListener& TcpListener::operator=(TcpListener&& other) {
    if (this != &other) {
        // Close existing sockets
        for (int client_fd : client_sockets_) {
            ::close(client_fd);
        }
        ::close(sockfd_);

        // Move data
        sockfd_ = other.sockfd_;
        client_sockets_ = std::move(other.client_sockets_);
        client_addr_ = other.client_addr_;

        other.sockfd_ = -1;
    }
    return *this;
}

TcpListener::~TcpListener() {
    // close all client sockets
    for (int client_fd : client_sockets_) {
        ::close(client_fd);
    }
    // close server socket
    ::close(sockfd_);
}

}