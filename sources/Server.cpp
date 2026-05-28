/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: afontele <afontele@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/23 21:54:23 by afontele          #+#    #+#             */
/*   Updated: 2026/05/28 23:08:27 by afontele         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/Server.hpp"

//Set static boolean to true outside the constructor
bool	Server::_running = true;

Server::Server(const std::string &port, const std::string &password) : _password(password), _serverSocket(-1) {
	std::stringstream	extractInt(port);
	int					portNb = 0;

	extractInt >> portNb;

	if (extractInt.fail() || !extractInt.eof())
		throw std::invalid_argument("Invalid port format.");
	if (portNb < 1024 || portNb > 65535)
		throw std::invalid_argument("Invalid port number.");

	_port = static_cast<unsigned short>(portNb);
}

Server::~Server() {
	close(_serverSocket);

	for (std::map<int,Client *>::iterator it = _clients.begin(); it != _clients.end(); it++) {
		close(it->first);
		delete it->second;
	}
	for (std::map<std::string, Channel *>::iterator it = _channels.begin(); it != _channels.end(); it++)
		delete it->second;

	_clients.clear();
	_channels.clear();

	std::cout << "[INFO]Server shutdown." << std::endl;
}

//Signal Handler
void	Server::signalHandler(int sig) {
	(void)sig;

	_running = false;
}

//Network Setup
bool	Server::ServerInit() {
	_serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP); 
	if (_serverSocket < 0) {
		std::cerr << "Error: Fail to create socket." << std::endl;
		return (false);
	}

	int	enable = true;
	if (setsockopt(_serverSocket, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(enable)) < 0) {
		close(_serverSocket);
		std::cerr << "Error: Failed to set socket to allow port reuse." << std::endl;
		return (false);
	}

	struct sockaddr_in	serverAddr;
	std::memset(&serverAddr, 0, sizeof(serverAddr));

	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	serverAddr.sin_port = htons(_port);
	
	if (bind(_serverSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0) {
		close(_serverSocket);
		std::cerr << "Error: Failed to bind socket." << std::endl;
		return (false);
	}

	if (listen(_serverSocket, SOMAXCONN) < 0) {
		close(_serverSocket);
		std::cerr << "Error: Failed to listen for connections" << std::endl;
		return (false);
	}

	struct pollfd	serverpfd;
	serverpfd.fd = _serverSocket;
	serverpfd.events = POLLIN;
	serverpfd.revents = 0;
	_pollFds.push_back(serverpfd);

	return (true);
}

//Event loop
void	Server::ServerRun() {
	while (_running) {
		int eventTrack = poll(&_pollFds[0], _pollFds.size(), 5000);
		if (eventTrack < 0) {
			if (errno == EINTR) {
				std::cout << "[INFO]Interrupted by signal, shouldn't crash the server" << std::endl;
				continue ;
			}
			std::cerr << "Error: Poll" << std::endl;
			break ;
		}
		if (eventTrack == 0)
    		continue ;

		for (int i = static_cast<int>(_pollFds.size() - 1); i >= 0; i--) {
			int		curFd = _pollFds[i].fd;
			short	curRev = _pollFds[i].revents;
			
			if (curRev & POLLIN) {
				if (curFd == _serverSocket)
					acceptNewClient();
				else
					receiveClientData(curFd);
			}
			if ((curRev & POLLOUT) && _clients.find(curFd) != _clients.end()) {
				sendMessage(curFd);
			}
		}
	}
}

//Acceptance of new Clients
void	Server::acceptNewClient() {
	struct sockaddr_in	clientAddr;
	socklen_t			clientAddrLen = sizeof(clientAddr);

	int	clientSocket = accept(_serverSocket, (struct sockaddr *)&clientAddr, &clientAddrLen);
	if (clientSocket < 0) {
		std::cerr << "Error: Client was't accepted" << std::endl;
		return ;
	}

	struct pollfd	clientpfd;
	Client *newClient = new Client(clientSocket);
	if (!newClient) {
		std::cerr << "Error: New failed." << std::endl;
		return ;
	}

	char hostBuf[INET_ADDRSTRLEN];
	std::memset(hostBuf, 0, sizeof(hostBuf));
	if (inet_ntop(AF_INET, &clientAddr.sin_addr, hostBuf, sizeof(hostBuf)))
		newClient->set_hostname(std::string(hostBuf));

	_clients[clientSocket] = newClient;

	clientpfd.fd = clientSocket;
	clientpfd.events = POLLIN;
	clientpfd.revents = 0;
	_pollFds.push_back(clientpfd);

	std::cout << "[INFO] New Client connected with FD: " << clientSocket << std::endl;
}

//Receive data from clients
void	Server::receiveClientData(int clientFd) {
	char	buff[1024];

	std::memset(buff, 0, sizeof(buff));

	ssize_t	bytesRead = recv(clientFd, buff, sizeof(buff) - 1, MSG_DONTWAIT);

	if (bytesRead <= 0) {
		if (bytesRead < 0)
			std::cerr << "Error: recv()" << std::endl;
		cleanClosure(clientFd);
		return ;
	}
	
	if (_clients.find(clientFd) == _clients.end()) {
		std::cerr << "Received data from unknown client." << std::endl;
		return ;
	}

	std::string	msg = buff;
	_clients[clientFd]->set_bufferIn(msg);

	std::vector<std::string> lines =  Parser::extractLines(_clients[clientFd]->get_bufferIn());
	CommandHandler	cmdHandler;
	for (std::size_t i = 0; i < lines.size(); i++)
	{
		if (lines[i].empty())
			continue ;

		Message msg = Parser::parseLine(lines[i]);

		if (!msg.command.empty())
			cmdHandler.handleCommand(*this, _clients[clientFd], msg);

		if (_clients[clientFd]->get_shouldClose()) {
			cleanClosure(clientFd);
			break ;
		}
	}
}

//Disconnect a client
void	Server::cleanClosure(int clientFd) {
	if (_clients.find(clientFd) == _clients.end())
		return ;

	removeClientFromAllChannels(clientFd, "Connection closed");

	close(clientFd);
	delete _clients[clientFd];
	_clients.erase(clientFd);

	for (size_t i = 0; i < _pollFds.size(); i++) {
		if (_pollFds[i].fd == clientFd) {
			_pollFds.erase(_pollFds.begin() + i); //use vector method and pass the iterator of the position
			break ;
		}
	}
	std::cout << "Client FD " << clientFd << " disconnected." << std::endl;
}

void	Server::switchPollOut(int clientFd) {
	for (size_t i = 0; i < _pollFds.size(); i++) {
		if (_pollFds[i].fd == clientFd) {
			_pollFds[i].events = POLLIN | POLLOUT;
			break ;
		}
	}
}

void	Server::sendMessage(int clientFd) {
	if (_clients.find(clientFd) == _clients.end()) {
		return ;
	}
	std::string	&msg = _clients[clientFd]->get_bufferOut();

	if (msg.empty())
		return ;

	ssize_t	bytesSent = send(clientFd, msg.c_str(), msg.length(), MSG_DONTWAIT);

	if (bytesSent < 0) {
		std::cerr << "Error: Server failed on sending message to client FD: "<< clientFd << std::endl;
		cleanClosure(clientFd);
		return ;
	}

	if (bytesSent < static_cast<ssize_t>(msg.length())) {
		msg.erase(0, bytesSent);
	}
	else {
		msg.clear();
		
		for (size_t i = 0; i < _pollFds.size(); i++) {
			if (_pollFds[i].fd == clientFd) {
				_pollFds[i].events = POLLIN;
				break ;
			}
		}
	}
}

//Delete client from all channels
void	Server::removeClientFromAllChannels(int clientFd, const std::string &reason) {
	if (_clients.find(clientFd) == _clients.end()) {
		return ;
	}
	
	std::string quitMsg;
	if (_clients[clientFd]->get_registered())
	{
		quitMsg = Replies::QUIT_MSG(
			_clients[clientFd]->get_nickname(),
			_clients[clientFd]->get_username(),
			_clients[clientFd]->get_hostname(),
			reason);
	}

	std::set<Client *> receiveBroadcast;
	for (std::map<std::string, Channel *>::iterator ite = _channels.begin(); ite != _channels.end(); ite++) {
		Channel *chan = ite->second;
		if (chan->isMember(_clients[clientFd])) {
			std::set<Client *> chanMembers = chan->get_members();
			for (std::set<Client *>::iterator mb = chanMembers.begin(); mb!= chanMembers.end(); mb++) {
				if (*mb != _clients[clientFd])
					receiveBroadcast.insert(*mb);
			}
		}
	}
	
	if (!quitMsg.empty()) {
		for (std::set<Client *>::iterator ito = receiveBroadcast.begin(); ito != receiveBroadcast.end(); ito++) {
			(*ito)->set_bufferOut(quitMsg);
			switchPollOut((*ito)->get_socketFd());
		}
	}
	
	std::map<std::string, Channel *>::iterator it = _channels.begin();
	while (it != _channels.end()) {
		it->second->removeMember(_clients[clientFd]);
		it->second->removeOperator(_clients[clientFd]);
		it->second->removeInvite(_clients[clientFd]);

		// 3. Check if channel is empty
		if (it->second->nbMembers() == 0) {
			std::cout << "[INFO] Channel " << it->first << "is empty. Deleting it" << std::endl;
			delete it->second;
			_channels.erase(it++);
		}
		else
			it++;
	}
}
