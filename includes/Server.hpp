/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: afontele <afontele@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/23 21:54:11 by afontele          #+#    #+#             */
/*   Updated: 2026/05/09 12:16:11 by afontele         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_HPP
# define SERVER_HPP

# include <iostream>
# include <string>
# include <sstream> // necessary for stringstream
# include <vector> //necessary for poll() - we pass a struct pollfd as poll argument
# include <map>
# include <poll.h>
# include <sys/socket.h> //for AF_INET and bind
# include <sys/types.h>
# include <cerrno>
# include <unistd.h> //for close
# include <fcntl.h>
# include <netinet/in.h> //for struct sockaddr_in, IPPROTO_TCP
# include <cstring> //for memset
# include <stdexcept> // necessary for handling error inside the Constructor
# include "Channel.hpp"
# include "Client.hpp"
# include "CommandHandler.hpp"

//we'll need signal and errno

class	Server {
private:
	unsigned short		_port; //should I use unsigned short (for endian convertion - htons() and ntohs)
	/*Basically, you’ll want to convert the numbers to Network Byte Order before they go out on the wire, and convert them to Host Byte Order as they come in off the wire.*/
	std::string const	_password;
	int					_serverSocket; //fd
	std::vector<struct pollfd>	_pollFds; //vector of pollfd structs necessary for poll()
	std::map<std::string, Channel *> _channels; //map of pointers
	std::map<int, Client *>	_clients;
	
	Server();
public:	
	Server(const std::string &port, const std::string &password);
	// Server(Server const &other);
	// Server	&operator=(Server const &other);
	~Server();

	//Server methods
	bool	ServerInit();
	void	ServerRun();

	//Client methods
	void	acceptNewClient();
	void	receiveClientData(int clientFd);
	void	cleanClosure(int clientFd);
	void	switchPollOut(int clientFd);
	void	sendMessage(int clientFd);

	//Channel methods
	Channel	*get_channel(std::string const name);//retourne NULL si pas trouve
	Channel	*createChannel(std::string const name);//retourne NULL si erreur ?
	void	deleteChannel(Channel *chan);
	Client	*get_client(std::string const nickname);//retourne NULL si pas trouve
};

#endif
