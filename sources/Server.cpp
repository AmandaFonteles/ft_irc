/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: afontele <afontele@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/23 21:54:23 by afontele          #+#    #+#             */
/*   Updated: 2026/04/29 11:01:31 by afontele         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/Server.hpp"

Server::Server(unsigned int port, const std::string &password)
	: _port(port), _password(password), _serverSocket(-1) {}

Server::Server(Server const &other) {}

Server	&Server::operator=(Server const &other) {}

Server::~Server() {
	//close(_serverSocket);
	//loop to delete vector of Clients
}

//Network Setup
void	Server::ServerInit() {
	// 1. Create the socket with socket()
	//I don't know if I can use SOCK_NONBLOCK (only for Linux 2.6+) - chercher non bloquant sur discord
	_serverSocket = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0); //If error errno is set
	if (_serverSocket < 0) //handle error (cerr, exception...)
		std::cerr << "Error: Fail to create socket." << std::endl;
    // 2. Allow port reuse with setsockopt()
	
    // 3. Make the socket non-blocking with fcntl() //chercher non bloquant sur discord
    // 4. Bind the socket to _port with bind()
    // 5. Start listening with listen()
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