/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: afontele <afontele@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/23 21:54:23 by afontele          #+#    #+#             */
/*   Updated: 2026/05/01 19:50:13 by afontele         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/Server.hpp"

// ? Check if the port is between 1024 and 65535 - range of available ports
Server::Server(const std::string &port, const std::string &password)
	: _port(static_cast<short>(atoi(port.c_str()))), _password(password), _serverSocket(-1) {}

Server::Server(Server const &other) {}

Server	&Server::operator=(Server const &other) {}

Server::~Server() {
	close(_serverSocket);
	//loop to delete vector of Clients
}

//Network Setup
// ! The std::cerr msgs aren't definitive, I might change to throw runtime error
void	Server::ServerInit() {
	// 1. Create the socket with socket() - Comes with "default settings"
	// - AF_INET = set IPv4;
	// - SOCK_STREAM = Provides  sequenced,  reliable,  two-way,  connection-based byte streams.
	// - IPPROTO_TCP = set TCP as the transport protocol.
	// ! Tried using the OR and SOCK_NONBLOCK, but it's not C++98 compliant (apparently)
	_serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP); //If error errno is set
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
	//wHEN THE PROJECT SAYS WE CAN'T USE FCNTL IS JUST FOR SEND AND RECEIAVING MSG? OR HERE AS WELL
	if (fcntl(_serverSocket, F_SETFL, O_NONBLOCK) < 0) {
		close(_serverSocket);
		std::cerr << "Error: fcntl failed." << std::endl;
		return ;
	}
	
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
	// - Struct pollfd is defined in poll.h
	// - POLLIN alerts when data is ready to recv() on the socket.
	struct pollfd	serverpfd;
	serverpfd.fd = _serverSocket;
	serverpfd.events = POLLIN;
	serverpfd.revents = 0;
	_pollFds.push_back(serverpfd);

	std::cout << "[DEBUG]ServerInit - port: " << _port << std::endl;
}

//Event loop
// ! Check how to handle errors after
void	Server::ServerRun() {
	while (true) {
		// 1. Call poll() on the _pollFds vector
		// - this infinite loop + poll() is used to "put the CPU to sleep" til there's data to read;
		// - A vector of pollfds struct is passed to poll(), since a vector stores all its elements in one continuous block of memory, exactly like a C-array
		// - Change -1 to POLL_TIMEOUT!!!
		int eventTrack = poll(&_pollFds[0], _pollFds.size(), -1);
		if (eventTrack < 0) {
			if (errno == EINTR) {
				std::cout << "[DEBUG]Interrupted by signal, shouldn't crash the server" << std::endl;
				continue ;
			}
			std::cerr << "Error: Poll" << std::endl;
			break ;
		}
		
        // 2. Loop through _pollFds to find which fd triggered an event
		// - Since poll() returns how many fds flagged and not which ones, this loop is needed
		for (size_t i = 0; i < _pollFds.size(); i++) {
			// - If _pollFds[i].revents = 0, nothing happened on this socket.
			// - revents is a bitmap(each bit works as a checkbox), we use bitwise AND to check that the POLLIN box is checked
			// - The bitwise operation is necessary because the same revents can store POLLIN and other flags, and we want to treat every socket that has POLLIN in it.
			if (_pollFds[i].revents & POLLIN) {
				// 3. If it's the _serverSocket -> call acceptNewClient()
				if (_pollFds[i].fd == _serverSocket)
					//acceptNewClient();
				// 4. If it's a client fd -> call handleClientData(fd)
				else
					//receiveClientData();
			}
		}        
	}
}