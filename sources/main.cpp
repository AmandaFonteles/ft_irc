/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: afontele <afontele@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/30 15:09:52 by afontele          #+#    #+#             */
/*   Updated: 2026/05/05 15:25:20 by afontele         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <iostream>
#include "Server.hpp"

//Amanda: Je passe av[1] et av[2] directement a mon constructor
int	main(int ac, char **av) {
	if (ac != 3) {
		std::cerr << "Usage: ./ircserv <port> <password>" << std::endl;
		return (1);
	}
	
	try
	{
		// 1. Create the Server object
		Server	ircServer(av[1], av[2]);

		if (!ircServer.ServerInit()) {
			std::cerr << "[DEBUG] Server init failed." << std::endl;
			return (1);
		}

		//run Server
		ircServer.ServerRun();
	}
	catch(const std::exception& e)
	{
		std::cerr << "Error: " << e.what() << '\n';
		return (1);
	}
	
	return (0);
}