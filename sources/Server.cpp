/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: afontele <afontele@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/23 21:54:23 by afontele          #+#    #+#             */
/*   Updated: 2026/04/30 17:54:28 by afontele         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/Server.hpp"

//
Server::Server(const std::string &port, const std::string &password)
	: _port(atoi(port.c_str())), _password(password), _serverSocket(-1) {}

Server::Server(Server const &other) {}

Server	&Server::operator=(Server const &other) {}

Server::~Server() {
	close(_serverSocket);
	//loop to delete vector of Clients
}

//Network Setup
//OBS: The std::cerr msgs aren't definitive, I might change to throw runtime error
void	Server::ServerInit() {
	// 1. Create the socket with socket() - Comes with "default settings"
	// - AF_INET = set IPv4;
	// - SOCK_STREAM = Provides  sequenced,  reliable,  two-way,  connection-based byte streams.
	// - IPPROTO_TCP = set TCP as the transport protocol.
	// ? I don't know if using SOCK_NONBLOCK will resolve the non block problem completelly (only for Linux 2.6+) - chercher non bloquant sur discord
	_serverSocket = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, IPPROTO_TCP); //If error errno is set
	if (_serverSocket < 0) { //handle error (cerr, exception...)
		std::cerr << "Error: Fail to create socket." << std::endl;
		return ;
	}
		
    // 2. Change sock "settings" allowing port reuse with setsockopt() - preventing the "Address already in use" error.
	// - Port reuse is necessary because even if you stop to use a port (stop to run the program) the OS will wait a couple min to allow you to use this port again (run the program again)
	// - If you try to run ./ircserv again immediately, the bind() function will fail and yell at you: "Address already in use".
	int	enable = true;
	if (setsockopt(_serverSocket, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(enable)) < 0) { //it set's errno if error
		close(_serverSocket);
		std::cerr << "Error: Failed to set socket to allow port reuse." << std::endl;
		return ;
	}
	
    // 3. Make the socket non-blocking with fcntl() //chercher non bloquant sur discord
	//Won't use since we already have SOCK_NONBLCK ?
    // 4. Bind the socket to _port with bind()
	// - Struct specific for IP (will be casted to a generic struct sockaddr to fit bind())
	struct sockaddr_in	serverAddr;
	// - Clear garbage data from serverAddr
	std::memset(&serverAddr, 0, sizeof(serverAddr));
	// - Init structure:
	serverAddr.sin_family = AF_INET;
	// INADDR_ANY tells the OS to listen on all available network interfaces (0.0.0.0)
	// htonl() and htons() converts the IP to Network Byte Order (Host TO Network)
	serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	serverAddr.sin_port = htons(_port);
	// - Bind bounds the IP address and the port to the socket.
	if (bind(_serverSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0) {
		close(_serverSocket);
		std::cerr << "Error: Failed to bind socket." << std::endl;
		return ;
	}
	
    // 5. Start listening with listen()
	if (listen(_serverSocket, SOMAXCONN) < 0) {
		close(_serverSocket);
		std::cerr << "Error: Failed to listen for connections" << std::endl;
		return ;
	}
	
    // 6. Add _serverSocket to _pollFds with POLLIN event
	std::cout << "[DEBUG]ServerInit - port: " << _port << std::endl;
}

//Event loop
void	Server::ServerRun() {
	while (true) {
		// 1. Call poll() on the _pollFds vector
        // 2. Loop through _pollFds to find which fd triggered an event
        // 3. If it's the _serverSocket -> call acceptNewClient()
        // 4. If it's a client fd -> call handleClientData(fd)
	}
}