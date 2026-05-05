/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CommandHandler.hpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dnayel <dnayel@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/05 15:55:06 by dnayel            #+#    #+#             */
/*   Updated: 2026/05/05 17:09:36 by dnayel           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef COMMANDHANDLER_HPP
#define COMMANDHANDLER_HPP

#include "../includes/Message.hpp"


class Server;
class Client;

class CommandHandler
{
	public:
		CommandHandler();
		~CommandHandler();

		void handleCommand(Server &server, Client &client, const Message &msg);

	private:
		void handlePASS(Server &server, Client &client, const Message &msg);
		void handleNICK(Server &server, Client &client, const Message &msg);
		void handleUSER(Server &server, Client &client, const Message &msg);

		void registerClient(Server &server, Client &client);
}

#endif
