/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: afontele <afontele@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/23 21:54:11 by afontele          #+#    #+#             */
/*   Updated: 2026/05/28 22:06:05 by afontele         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_HPP
# define SERVER_HPP

# include <iostream>
# include <string>
# include <sstream>
# include <vector>
# include <map>
# include <poll.h>
# include <sys/socket.h>
# include <sys/types.h>
# include <cerrno>
# include <csignal>
# include <unistd.h>
# include <fcntl.h>
# include <netinet/in.h> 
# include <arpa/inet.h> 
# include <cstring>
# include <stdexcept>
# include "Channel.hpp"
# include "Client.hpp"
# include "CommandHandler.hpp"
# include "Parser.hpp"
# include "Replies.hpp"

class	Server {
private:
	unsigned short		_port;
	std::string const	_password;
	int					_serverSocket;
	std::vector<struct pollfd>	_pollFds;
	std::map<std::string, Channel *> _channels;
	std::map<int, Client *>	_clients;
	static bool			_running;

	Server();
public:
	Server(const std::string &port, const std::string &password);
	~Server();

	//Signal method
	static void	signalHandler(int sig);

	//Server methods
	bool	ServerInit();
	void	ServerRun();

	//Client methods
	void	acceptNewClient();
	void	receiveClientData(int clientFd);
	void	cleanClosure(int clientFd);
	void	switchPollOut(int clientFd);
	void	sendMessage(int clientFd);
	void	removeClientFromAllChannels(int clientFd, const std::string &reason);

	//Channel methods
	Channel				*get_channel(std::string const name);
	Channel				*createChannel(std::string const name);
	void				deleteChannel(Channel *chan);
	Client				*get_client(std::string const nickname);
	static std::string	lowerName(std::string const name);
	void				removeClientFromChannel(Client *c, Channel *chan);

	// Gtter
	std::string			get_name() const;
	std::string			get_password() const;
};

#endif
