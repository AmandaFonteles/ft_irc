/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: afontele <afontele@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/23 21:54:11 by afontele          #+#    #+#             */
/*   Updated: 2026/04/30 15:38:21 by afontele         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_HPP
#define SERVER_HPP

#include <iostream>
#include <string>
#include <vector> //will I use vector??
#include <poll.h> //is there a c++ poll.h?
#include <sys/socket.h> //for AF_INET and bind
#include <sys/types.h>
#include <cerrno>
#include <unistd.h> //for close
#include <fcntl.h>
#include <netinet/in.h> //for struct sockaddr_in, IPPROTO_TCP
#include <cstring> //for memset
//we'll need signal and errno

class	Server {
private:
	unsigned int		_port; //should I use unsigned short (for endian convertion - htons() and ntohs)
	/*Basically, you’ll want to convert the numbers to Network Byte Order before they go out on the wire, and convert them to Host Byte Order as they come in off the wire.*/
	std::string const	_password; //do I need that?
	int					_serverSocket; //fd
	std::vector</*ClientClass?*/>	_clients; //client sockets //maybe use maps, like Chatgpt's planning
	Server();
public:	
	Server(const std::string &port, const std::string &password);
	Server(Server const &other);
	Server	&operator=(Server const &other);
	~Server();

	void	ServerInit();
	void	ServerRun();
};

#endif