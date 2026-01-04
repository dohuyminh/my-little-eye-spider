#include <iostream>
#include <cstdio>
#include <cstring>
#include <csignal>
#include <format>

#include "memory/arena.h"
#include "networking/listener.h"
#include "networking/client_socket.h"

bool runServer = false;
memory::Arena arena(24);

void ctrl_c_handle(int sig) {
	runServer = false;
	exit(0);

}

void handle_client(networking::TcpListener& socket, networking::TcpClientSocket& client_fd) {

	// read message from client
	std::vector<std::byte> buffer = client_fd.recv();
	std::string recvMessage(reinterpret_cast<const char*>(buffer.data()), buffer.size());

	std::cout << "Attempting to allocate: " << recvMessage << '\n';

	// turn into double; add to arena
	double n = std::stod(recvMessage);

	try {
		double* alloc = arena.allocate<double>(n);
		std::string msg = std::format("Successfully allocated {} into arena\nRemaining arena capacity: {}\n", *alloc, arena.capacity() - arena.occupiedMemory());
		client_fd.send(reinterpret_cast<const std::byte*>(msg.data()), msg.size());
	} catch (std::bad_alloc const&) {
		std::string msg = "Failed to allocate to arena\n";
		client_fd.send(reinterpret_cast<const std::byte*>(msg.data()), msg.size());
	}

	// repeat back the message to client
	// std::string sendMessage = "Echo: " + recvMessage;
	// client_fd.send(reinterpret_cast<const std::byte*>(sendMessage.data()), sendMessage.size());

	// close client connection
	socket.close(client_fd);

}

int main(int argc, char const *argv[])
{

	networking::TcpListener server_socket("127.0.0.1", 8080);
	// server_socket.setNonBlocking(true);
	server_socket.listen();

	std::signal(SIGINT, ctrl_c_handle);

	runServer = true;
	while (runServer) {
		
		std::cout << "Waiting for connections...\n";
		try {
			networking::TcpClientSocket client_fd = server_socket.accept();
			std::cout << "Client connected" << "\n";

			handle_client(server_socket, client_fd);

		} catch (const std::runtime_error& e) {
			// Non-blocking accept might throw if no connections are present
		}

	}

	// cleanup and close server socket at destruction

	return 0;
}
