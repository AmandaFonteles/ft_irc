/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: afontele <afontele@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/23 21:54:23 by afontele          #+#    #+#             */
/*   Updated: 2026/05/12 10:14:48 by afontele         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/Server.hpp"

// !!! FOR ERROR: use errno on cerr messages?
Server::Server(const std::string &port, const std::string &password) : _password(password), _serverSocket(-1) {
	std::stringstream	extractInt(port);
	int					portNb = 0;

	// - Extract the stirng from the stream into the int
	extractInt >> portNb;
	
	// 1. Check for int extraction errors or leftover chars
	if (extractInt.fail() || !extractInt.eof())
		throw std::invalid_argument("Invalid port format");

	// 2. Check available port range
	if (portNb < 1024 || portNb > 65535)
		throw std::invalid_argument("Invalid port number.");

	// 3. Unsigned short cast
	_port = static_cast<unsigned short>(portNb);
}

// Server::Server(Server const &other) {}

// Server	&Server::operator=(Server const &other) {}

Server::~Server() {
	// 1. close server socket
	close(_serverSocket);
	
	// 2. loops to delete _clients and _channels
	for (std::map<int,Client *>::iterator it = _clients.begin(); it != _clients.end(); it++) {
		close(it->first);
		delete it->second;
	}
	for (std::map<std::string, Channel *>::iterator it = _channels.begin(); it != _channels.end(); it++)
		delete it->second;
		
	// 3. Empty maps _clients and _channels
	_clients.clear();
	_channels.clear();
	
	std::cout << "[DEBUG] Server shutdown." << std::endl;
}

//Network Setup
bool	Server::ServerInit() {
	// 1. Create the socket with socket() - Comes with "default settings"
	// - AF_INET = set IPv4;
	// - SOCK_STREAM = Provides  sequenced,  reliable,  two-way,  connection-based byte streams.
	// - IPPROTO_TCP = set TCP as the transport protocol.
	// ! Tried using the OR and SOCK_NONBLOCK, but it's not C++98 compliant (apparently)
	_serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP); //If error errno is set
	if (_serverSocket < 0) { //handle error (cerr, exception...)
		std::cerr << "Error: Fail to create socket." << std::endl;
		return (false);
	}
		
    // 2. Change sock "settings" allowing port reuse with setsockopt() - preventing the "Address already in use" error.
	// - Port reuse is necessary because even if you stop to use a port (stop to run the program) the OS will wait a couple min to allow you to use this port again (run the program again)
	// - If you try to run ./ircserv again immediately, the bind() function will fail and yell at you: "Address already in use".
	int	enable = true;
	if (setsockopt(_serverSocket, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(enable)) < 0) { //it set's errno if error
		close(_serverSocket);
		std::cerr << "Error: Failed to set socket to allow port reuse." << std::endl;
		return (false);
	}
	
	
    // 3. Make the socket non-blocking with fcntl() //chercher non bloquant sur discord
	// ? wHEN THE PROJECT SAYS WE CAN'T USE FCNTL IS JUST FOR SEND AND RECEIAVING MSG? OR HERE AS WELL
	// ? change to select and won't need fcntl ????
	if (fcntl(_serverSocket, F_SETFL, O_NONBLOCK) < 0) {
		close(_serverSocket);
		std::cerr << "Error: fcntl failed." << std::endl;
		return (false);
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
		return (false);
	}
	
    // 5. Start listening with listen()
	if (listen(_serverSocket, SOMAXCONN) < 0) {
		close(_serverSocket);
		std::cerr << "Error: Failed to listen for connections" << std::endl;
		return (false);
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
	return (true);
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
		for (size_t i = _pollFds.size() - 1; i >= 0; i--) {
			// - If _pollFds[i].revents = 0, nothing happened on this socket.
			// - revents is a bitmap(each bit works as a checkbox), we use bitwise AND to check that the POLLIN box is checked
			// - The bitwise operation is necessary because the same revents can store POLLIN and other flags, and we want to treat every socket that has POLLIN in it.
			if (_pollFds[i].revents & POLLIN) {
				// 3. If it's the _serverSocket, a client is waiting to join -> call acceptNewClient()
				if (_pollFds[i].fd == _serverSocket)
					acceptNewClient();
				// 4. If it's a client fd -> call handleClientData(fd)
				else
					receiveClientData(_pollFds[i].fd);
			}
			// 5. Check for POLLOUT (to send data to clients)
			if (_pollFds[i].revents & POLLOUT) {
				sendMessage(_pollFds[i].fd);
			}
		}        
	}
}

//Acceptance of new Clients
void	Server::acceptNewClient() {
	struct sockaddr_in	clientAddr;
	socklen_t			clientAddrLen = sizeof(clientAddr);

	// 1. Accept the connection
	// - We pass the clientAddr struct so the OS can fill in the client's IP and Port
	// - We need to cast sockaddr_in to sockaddr, since accept take it as argument
	int	clientSocket = accept(_serverSocket, (struct sockaddr *)&clientAddr, &clientAddrLen);
	// - Error checking (set's errno)
	if (clientSocket < 0) {
		std::cerr << "Error: Client was't accepted" << std::endl;
		return ;
	}

	// 2. Make the NEW client socket non-blocking //use socket() with flag
	if (fcntl(clientSocket, F_SETFL, O_NONBLOCK) < 0) {
		std::cerr << "Error: fcntl failed." << std::endl;
		return ;
	}
	
	// 3. Add the new client to our poll() vector
	// - Add the new client to client map also
	struct pollfd	clientpfd;
	Client *newClient = new Client(clientSocket);
	if (!newClient) {
		std::cerr << "Error: New failed." << std::endl;
		return ;
	}
	
	_clients[clientSocket] = newClient;
	clientpfd.fd = clientSocket;
	clientpfd.events = POLLIN;
	clientpfd.revents = 0;
	_pollFds.push_back(clientpfd);

	std::cout << "[DEBUG] New Client connected with FD: " << clientSocket << std::endl;
}

//Receive data from clients
void	Server::receiveClientData(int clientFd) {
	// - Create a buffer to save temporarely the data sent from client
	char	buff[1024];
	
	// - Clear the buffer from garbage data
	std::memset(buff, 0, sizeof(buff));

	// 1. Read data from the client
	// ? recv(clientFd, &buff, ...)
	ssize_t	bytesRead = recv(clientFd, buff, sizeof(buff) - 1, 0);

	// - Error checking
	if (bytesRead < 0) {
		std::cerr << "Error: Server failed on receiving message from client FD: "<< clientFd << std::endl;
		cleanClosure(clientFd);
	}
	
	// 2. Clean clousure
	else if (bytesRead == 0) {
		//remove client from vector AND map
		cleanClosure(clientFd);
	}

	// 3. Include received data to _bufferIn
	else {
		std::string	msg = buff;
		std::cout << "[DEBUG] Message received: " << msg << std::endl;
		_clients[clientFd]->set_bufferIn(msg);

		// - TEST POLLOUT (Enlever apres?)
		std::string reply = "[DEBUG]Server heard: " + msg;
		_clients[clientFd]->set_bufferOut(reply);
		switchPollOut(clientFd);
	}
}

//Disconnect a client
void	Server::cleanClosure(int clientFd) {
	// 1. Safety check
	if (_clients.find(clientFd) == _clients.end())
		return ;
		
	// 2. Remove client from _channels
	removeClientFromAllChannels(clientFd);

	// 3. Remove channels from client (? do we need that? We will delete the client after anyway)
	_clients[clientFd]->removeAllChannel();
	
	// 4. Close the socket
	close(clientFd);
	
	// 5. Delete from map and erase its key
	delete _clients[clientFd];
	_clients.erase(clientFd);
	
	//6. Remove from _pollFds
	for (size_t i = 0; i < _pollFds.size(); i++) {
		if (_pollFds[i].fd == clientFd) {
			_pollFds.erase(_pollFds.begin() + i); //use vector method and pass the iterator of the position
			break ;
		}
	}
	std::cout << "Client FD " << clientFd << " disconnected." << std::endl;
}

// - This function exist to tell the server we have a message to send to a client.
// - To send a message to a client, we need to save that msg in _bufferOut and call switchPollOut(clientFd)
void	Server::switchPollOut(int clientFd) {
	// 1. Switch events to POLLIN | POLLLOUT (server want to send data to client)
	for (size_t i = 0; i < _pollFds.size(); i++) {
		if (_pollFds[i].fd == clientFd) {
			_pollFds[i].events = POLLIN | POLLOUT;
			break ;
		}
	}
}

void	Server::sendMessage(int clientFd) {
	std::string	&msg = _clients[clientFd]->get_bufferOut();
	
	// ? Do we hve something to handle if bufferOut is empty? It shouldn't happen
	if (msg.empty())
		return ;

	// 1. Use send() to send data to client
	ssize_t	bytesSent = send(clientFd, msg.c_str(), msg.length(), 0);
	
	// - Error checking
	if (bytesSent < 0) {
		std::cerr << "Error: Server failed on sending message to client FD: "<< clientFd << std::endl;
		cleanClosure(clientFd);
	}
	
	// 2. Handle incomplete messages
	else if (bytesSent < static_cast<ssize_t>(msg.length())) {
		std::cout << "[DEBUG] Partial send. Sent " << bytesSent << " out of " << msg.length() << " bytes." << std::endl;
		// - We erase all the bytes sent to client and don't change events to POLLIN.
		// - Like that poll loop will call send message again till bytesSent == msg.length()
		msg.erase(0, bytesSent);
	}
	else {
		// 2. Clean _bufferOut
		// ? Ask Nayel how to handle the _buffers
		msg.clear(); //msg.erase(0, bytesSent);
		
		// 3. Switch event back to POLLIN only
		for (size_t i = 0; i < _pollFds.size(); i++) {
			if (_pollFds[i].fd == clientFd) {
				_pollFds[i].events = POLLIN;
				break ;
			}
		}
	}
}

//Delete client from all channels
// ? Check if we need to send messages about that to other clients
// ? Metre sur Server_channel
void	Server::removeClientFromAllChannels(int clientFd) {
	// 1. Loop through Channel map to remove the client from it
	// - Increment the iterator when calling erase. Erase destroy it, so if we use it after calling erase, the program will try to acess it that no longer exists
	std::map<std::string, Channel *>::iterator it = _channels.begin();
	while (it != _channels.end()) {
		it->second->removeMember(_clients[clientFd]);
		it->second->removeOperator(_clients[clientFd]);

		// 3. Check if channel is empty
		if (it->second->nbMembers() == 0) {
			std::cout << "[DEBUG] Channel " << it->first << "is empty. Deleting it" << std::endl;
			delete it->second;
			_channels.erase(it++);
		}
		else
			it++;
	}
}