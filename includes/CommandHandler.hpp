/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CommandHandler.hpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aibonade <aibonade@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/05 15:55:06 by dnayel            #+#    #+#             */
/*   Updated: 2026/05/10 19:26:20 by aibonade         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef COMMANDHANDLER_HPP
# define COMMANDHANDLER_HPP

# include "../includes/Message.hpp"


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

	//Aileen
	//Fonctions de test (tu peux les utiliser aussi hein, c'est juste pour que tu voies ce que j'ai ajoute)
		bool	isMemberChannel(Client *c, Channel *chan);
		bool	checkChannelKey(Channel const *chan, std::string const key);
		bool	checkLimit(Channel const *chan);//true = limit channel non atteinte, false = limite atteinte
		Client	*checkClientExists(Server &server, std::string const &nickname);//verifier aussi que s'il existe il est bien register => NULL = client non existant/enregistre, sinon pointeur sur le client ? 
		bool	isValidChannelName(std::string const &name);
		bool	isValidClientName(Server const &server, std::string const &name);
	//Commandes//Le client ici du coup c'est bien celui qui a appele la commande 
		void	handleJOIN(Server &server, Client *c, const Message &msg);//ici on appelle addMember & addChannel !
		//void handlePRIVMSG(Server &server, Client *c, const Message &msg);
		//void handleKICK(Server &server, Client *c, const Message &msg);
		//void handleINVITE(Server &server, Client *c, const Message &msg);
		//void handleTOPIC(Server &server, Client *c, const Message &msg);
		//void handleMODE(Server &server, Client *c, const Message &msg);
		//void handlePART(Server &server, Client *c, const Message &msg);//Non obg mais utile pout JOIN//Penser a suppr membre & operator & invite
		//void handleNAMES(Server &server, Client *c, const Message &msg);//Non obg mais utile pour JOIN
};

#endif
