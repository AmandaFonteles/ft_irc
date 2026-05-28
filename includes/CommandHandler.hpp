/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CommandHandler.hpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: afontele <afontele@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/05 15:55:06 by dnayel            #+#    #+#             */
/*   Updated: 2026/05/28 23:49:03 by afontele         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef COMMANDHANDLER_HPP
# define COMMANDHANDLER_HPP

# include "../includes/Message.hpp"


class Server;
class Client;
class Channel;

class CommandHandler
{
	public:
		CommandHandler();
		~CommandHandler();

		void		handleCommand(Server &server, Client *client, const Message &msg);

	private:
	// Registration Commands
		void		handlePASS(Server &server, Client *client, const Message &msg);
		void		handleNICK(Server &server, Client *client, const Message &msg);
		void		handleUSER(Server &server, Client *client, const Message &msg);
		void		handlePING(Server &server, Client *client, const Message &msg);
		void		handleQUIT(Server &server, Client *client, const Message &msg);
		void		registerClient(Server &server, Client *client);
		void		broadcastToChannel(Server &server, Channel *chan, const std::string &msg, Client *except);
		bool		isMemberChannel(Client *c, Channel *chan);
		bool		checkChannelKey(Channel const *chan, std::string const key);
		bool		checkLimit(Channel const *chan);
		Client		*checkClientExists(Server &server, std::string const &nickname);
		bool		isValidChannelName(std::string const &name);
		bool		isValidClientName(std::string const &name);
		void		namesReply(Server &server, Client *c, Channel *chan);
		std::string	mkModeList(Channel *chan);
		//Commands
		void		handleJOIN(Server &server, Client *c, const Message &msg);
		void		handlePRIVMSG(Server &server, Client *c, const Message &msg);
		void		handleKICK(Server &server, Client *c, const Message &msg);
		void		handleINVITE(Server &server, Client *c, const Message &msg);
		void		handleTOPIC(Server &server, Client *c, const Message &msg);
		void		handleMODE(Server &server, Client *c, const Message &msg);
		void		handlePART(Server &server, Client *c, const Message &msg);
};

#endif
