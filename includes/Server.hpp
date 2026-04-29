/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: afontele <afontele@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/23 21:54:11 by afontele          #+#    #+#             */
/*   Updated: 2026/04/29 12:07:19 by afontele         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_HPP
#define SERVER_HPP

#include <iostream>
#include <string>
#include <vector> //will I use vector??
#include <poll.h> //is there a c++ poll.h?
#include <sys/socket.h>
#include <sys/types.h>
#include <cerrno>
#include <unistd.h> //for close
#include <fcntl.h>
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
	Server(unsigned int port, const std::string &password);
	Server(Server const &other);
	Server	&operator=(Server const &other);
	~Server();

	void	ServerInit();
	void	ServerRun();
};

#endif