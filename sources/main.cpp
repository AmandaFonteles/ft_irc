/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: afontele <afontele@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/30 15:09:52 by afontele          #+#    #+#             */
/*   Updated: 2026/05/28 22:17:30 by afontele         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <iostream>
#include <csignal>
#include "Server.hpp"

int	main(int ac, char **av) {
	// 1. Parameter check
	if (ac != 3) {
		std::cerr << "Usage: ./ircserv <port> <password>" << std::endl;
		return (1);
	}
	
	// 2. Signal setting
	signal(SIGINT, Server::signalHandler);
	signal(SIGQUIT, Server::signalHandler);
	signal(SIGTERM, Server::signalHandler);
	
	try
	{
		// 3. Create the Server object
		Server	ircServer(av[1], av[2]);

		if (!ircServer.ServerInit()) {
			std::cerr << "[DEBUG] Server init failed." << std::endl;
			return (1);
		}

		// 4. run Server
		ircServer.ServerRun();
	}
	catch(const std::exception& e)
	{
		std::cerr << "Error: " << e.what() << '\n';
		return (1);
	}
	
	return (0);
}