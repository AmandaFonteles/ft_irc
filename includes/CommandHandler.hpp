/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CommandHandler.hpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dnayel <dnayel@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/05 15:55:06 by dnayel            #+#    #+#             */
/*   Updated: 2026/05/19 10:56:52 by dnayel           ###   ########.fr       */
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

		void handleCommand(Server &server, Client *client, const Message &msg);

	private:
	// Nayel : Registration Commands
		void handlePASS(Server &server, Client *client, const Message &msg);
		void handleNICK(Server &server, Client *client, const Message &msg);
		void handleUSER(Server &server, Client *client, const Message &msg);
		void handlePING(Server &server, Client *client, const Message &msg);
		void handlePONG(Server &server, Client *client, const Message &msg);
		void handleQUIT(Server &server, Client *client, const Message &msg);

		void registerClient(Server &server, Client *client);

	//Aileen
	//Fonctions de test (tu peux les utiliser aussi hein, c'est juste pour que tu voies ce que j'ai ajoute)
		bool	isMemberChannel(Client *c, Channel *chan);//check dans client et chan si membre du chan
		bool	checkChannelKey(Channel const *chan, std::string const key);//si chan n'a pas de clef, renvoie true sinon renvoie le resultat de isKey (comparaison avec la clef du chan)
		bool	checkLimit(Channel const *chan);//true = limit channel non atteinte ou pas de limit, false = limite atteinte
		Client	*checkClientExists(Server &server, std::string const &nickname);//verifier aussi que s'il existe il est bien register => NULL = client non existant/enregistre, sinon pointeur sur le client ?
		bool	isValidChannelName(std::string const &name);
		bool	isValidClientName(std::string const &name);
		void	namesReply(Server &server, Client *c, Channel *chan);//A voir s'il faut le message du JOIN aussi pour Nayel
	//Commandes//Le client ici du coup c'est bien celui qui a appele la commande
		void	handleJOIN(Server &server, Client *c, const Message &msg);
		void	handlePRIVMSG(Server &server, Client *c, const Message &msg);
		void	handleKICK(Server &server, Client *c, const Message &msg);
		void	handleINVITE(Server &server, Client *c, const Message &msg);
		void	handleTOPIC(Server &server, Client *c, const Message &msg);
		void	handleMODE(Server &server, Client *c, const Message &msg);
		void	handlePART(Server &server, Client *c, const Message &msg);
		//void	handleNAMES(Server &server, Client *c, const Message &msg);//Non obg mais utile pour JOIN => j'ai plutot fait un helper, on peut passer par lui si finalement on decide de coder la commande NAMES
};

#endif
