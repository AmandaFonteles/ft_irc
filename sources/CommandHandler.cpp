/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CommandHandler.cpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dnayel <dnayel@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/05 15:55:21 by dnayel            #+#    #+#             */
/*   Updated: 2026/05/23 21:54:05 by dnayel           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/CommandHandler.hpp"
#include "../includes/Server.hpp"
#include "../includes/Client.hpp"
#include "../includes/Channel.hpp"
#include "../includes/Replies.hpp"

#include <cctype>
#include <iostream>

CommandHandler::CommandHandler() {}
CommandHandler::~CommandHandler() {}

void CommandHandler::broadcastToChannel(Server &server, Channel *chan, const std::string &msg, Client *except)
{
	std::set<Client *>				members = chan->get_members();
	std::set<Client *>::iterator	it = members.begin();

	while (it != members.end())
	{
		if (*it != except)
		{
			(*it)->set_bufferOut(msg);
			server.switchPollOut((*it)->get_socketFd());
		}
		++it;
	}

}

void CommandHandler::handleCommand(Server &server, Client *client, const Message &msg)
{
	if (msg.command.empty())
		return; // Message ou comportement particulier à définir ??
// Authorized COMMANDS even without registration
	if (msg.command == "CAP") // Ignore first, modern clients send it and disconnect if ERR_
	{
		if (!msg.params.empty())
		{
			if (msg.params[0] == "LS")
				client->set_bufferOut(":ft_irc CAP * LS :\r\n");
			else if (msg.params[0] == "END")
				return;
		}
		return;
	}
	if (msg.command == "PASS")
	{
			handlePASS(server, client, msg);
			return;
	}
	if (msg.command == "NICK")
	{
		handleNICK(server, client, msg);
		return;
	}
	if (msg.command == "USER")
	{
		handleUSER(server, client, msg);
		return;
	}
	if (msg.command == "PING")
	{
		handlePING(server, client, msg);
		return;
	}
	if (msg.command == "PONG")
		return;
	if (msg.command == "QUIT")
	{
		handleQUIT(server, client, msg);
		return;
	}

// Registration check
	if (!client->get_registered()) // CHECK comment la variable est appelée dans Client.hpp
	{
		const std::string nick = client->get_nickname().empty() ? "*" : client->get_nickname(); // * instead of empty string for ERR_ replies (IRC convention)
		client->set_bufferOut(Replies::ERR_NOTREGISTERED(server.get_name(), nick));
		server.switchPollOut(client->get_socketFd());
		return;
	}
// Post registration Authorized COMMANDS
	if (msg.command == "JOIN")
	{
		handleJOIN(server, client, msg);
		return;
	}
	if (msg.command == "PRIVMSG")
	{
		handlePRIVMSG(server, client, msg);
		return;
	}
	if (msg.command == "KICK")
	{
		handleKICK(server, client, msg);
		return;
	}
	if (msg.command == "INVITE")
	{
		handleINVITE(server, client, msg);
		return;
	}
	if (msg.command == "TOPIC")
	{
		handleTOPIC(server, client, msg);
		return;
	}
	if (msg.command == "MODE")
	{
		handleMODE(server, client, msg);
		return;
	}
	if (msg.command == "PART")
	{
		handlePART(server, client, msg);
		return;
	}

// Ignored COMMANDS (not implemented) MAYBE MORE TO ADD
	if (msg.command == "WHO"	|| msg.command == "WHOIS"	||
		msg.command == "NAMES"	|| msg.command == "AWAY"	||
		msg.command == "MOTD"	|| msg.command == "LIST"	||
		msg.command == "LUSERS"	|| msg.command == "USERHOST")
		return;

}

// Handle PASS command (e.g. check password, set client password, etc.)
//	PASS
//Paramètres : <password>
//Rôle dans la registration :
//Le mot de passe doit être envoyé avant toute tentative d'enregistrement. Il est comparé au mot de passe fourni au lancement du serveur (./ircserv <port> <password>). La spec précise que si plusieurs PASS sont envoyés avant registration, seul le dernier compte.
//Vérifications dans la fonction :
//msg.paramCount() < 1 → ERR_NEEDMOREPARAMS (461)
//client.registered == true → ERR_ALREADYREGISTERED (462)
//param(0) != server.getPassword() → ERR_PASSWDMISMATCH (464) + ERROR + shouldClose = true
//⚠️ Notes importantes :
//Selon la spec : "Servers MUST send at least one of these two messages" (ERR_PASSWDMISMATCH ou ERROR). On envoie les deux pour être propre.
//En cas de mauvais mot de passe, la connexion doit être fermée (après avoir vidé le outBuffer avec le message d'erreur).
//ERROR est envoyé sans préfixe :server — c'est une commande directe, pas un numeric.
//En cas de succès : silence total — on ne répond rien. C'est la convention IRC.
//passOk = true ne déclenche PAS la registration seul : il faut aussi NICK et USER.

//ex : PASS wrongpass
void CommandHandler::handlePASS(Server &server, Client *client, const Message &msg)
{
	const std::string serverName = server.get_name();
	const std::string nickname = client->get_nickname().empty() ? "*" : client->get_nickname(); // * instead of empty string for ERR_ replies (IRC convention)

	// Already registered
	if (client->get_registered())
	{
		client->set_bufferOut(Replies::ERR_ALREADYREGISTERED(serverName, nickname));
		server.switchPollOut(client->get_socketFd());
		return;
	}

	// Not enough params
	if (msg.paramsCount() < 1)
	{
		client->set_bufferOut(Replies::ERR_NEEDMOREPARAMS(serverName, nickname, "PASS"));
		server.switchPollOut(client->get_socketFd());
		return;
	}

	const std::string password = msg.param(0);
	if (password != server.get_password())
	{
		client->set_bufferOut(Replies::ERR_PASSWDMISMATCH(serverName, nickname));
		server.switchPollOut(client->get_socketFd());
		return;
	}
	client->set_passOk(true);

	registerClient(server, client);
}

// Handle NICK command (e.g. check nickname validity, set client nickname, etc.)
//	NICK
//Paramètres : <nickname>
//Double rôle :
//Pendant la registration : définit le pseudo pour la première fois.
//Après registration : change le pseudo en cours de session.
//Règles de validité d'un nick (RFC 1459 + modern.ircdocs) :
//Longueur : 1 à 9 caractères
//Premier caractère : lettre [A-Za-z] ou l'un de []{}|\^ ` - _
//Caractères suivants : idem + chiffres [0-9]
//Explicitement interdits en première position : #, &, :, $
//Les chiffres en première position : la spec dit que les serveurs MAY disallow — on les interdit pour la conformité
//Vérifications dans la fonction :
//msg.paramCount() < 1 → ERR_NONICKNAMEGIVEN (431)
//Nick invalide (caractères interdits, mauvaise longueur) → ERR_ERRONEUSNICKNAME (432)
//Nick identique à l'actuel → ignorer silencieusement
//Nick déjà pris par un autre client → ERR_NICKNAMEINUSE (433)
//⚠️ Notes importantes :
//Le serveur confirme le changement en envoyant NICK_CHANGE à l'émetteur et à tous les membres des canaux partagés. C'est comme ça que les autres clients voient le nouveau pseudo.
//Avant registration, NICK ne génère aucune réponse en cas de succès : on attend que la registration soit complète pour envoyer les 001-004.
//La spec indique : "The NICK message may be sent from the server to clients to acknowledge their NICK command was successful" — donc si déjà registered, on envoie le NICK_CHANGE même à soi-même.

//ex : NICK newnick
void CommandHandler::handleNICK(Server &server, Client *client, const Message &msg)
{
	const std::string serverName = server.get_name();
	const std::string currentNickname = client->get_nickname().empty() ? "*" : client->get_nickname();

	// No params
	if (msg.paramsCount() < 1)
	{
		client->set_bufferOut(Replies::ERR_NONICKNAMEGIVEN(serverName, currentNickname));
		server.switchPollOut(client->get_socketFd());
		return;
	}
	const std::string newNickname = msg.param(0);
	// invalid nickname
	if (!isValidClientName(newNickname))
	{
		client->set_bufferOut(Replies::ERR_ERRONEUSNICKNAME(serverName, currentNickname, newNickname));
		server.switchPollOut(client->get_socketFd());
		return;
	}
	// Same nickname
	if (newNickname == currentNickname)
		return;
	// Already used
	if (server.get_client(newNickname) != NULL)
	{
		client->set_bufferOut(Replies::ERR_NICKNAMEINUSE(serverName, currentNickname, newNickname));
		server.switchPollOut(client->get_socketFd());
		return;
	}
	// Update client nickname + msg to shared channels if already registered
	if (client->get_registered())
	{
		const std::string nickChangeMsg = Replies::NICK_CHANGE(client->get_nickname(), client->get_username(), client->get_hostname(), newNickname);
		client->set_bufferOut(nickChangeMsg);
		server.switchPollOut(client->get_socketFd());
	}
	client->set_nickname(newNickname);

	registerClient(server, client);
}
// Handle USER command (e.g. set client username, realname, etc.)
//USER
//Paramètres : <username> 0 * :<realname>
//Rôle : Fournit l'identité complète du client. Envoyé une seule fois pendant la registration.
//Décomposition des paramètres :
//param(0) = username : identifiant Unix-like, ex: "ali"
//param(1) = 0 : mode utilisateur (ignoré par ft_irc, valeur 0 recommandée par la spec)
//param(2) = * : nom du serveur distant (ignoré, vestige IRC des années 90)
//param(3) / trailing = realname : nom complet libre, peut contenir des espaces → doit être en trailing dans la commande
//Vérifications dans la fonction :
//client.registered == true → ERR_ALREADYREGISTERED (462)
//msg.paramCount() < 4 → ERR_NEEDMOREPARAMS (461)
//username vide (param(0) == "") → ERR_NEEDMOREPARAMS (461) (spec : "If it is empty, the server SHOULD reject the command")
//⚠️ Notes importantes :
//username et realname ne peuvent être modifiés qu'en se déconnectant et reconnectant. Contrairement au nick, ils sont figés après registration.
//La spec indique que si un serveur Ident est disponible, le username fourni par Ident remplace celui de USER — pour ft_irc, on utilise toujours celui de USER.
//Le username peut être préfixé d'un ~ si pas d'Ident (convention serveur) — pour ft_irc, on stocke tel quel.
//realname peut contenir n'importe quel caractère (espaces inclus) car c'est un trailing.

//ex : USER ali 0 * :Ali Fontele (ali = username, 0 = mode, * = server, Ali Fontele = realname)
void CommandHandler::handleUSER(Server &server, Client *client, const Message &msg)
{
	const std::string serverName = server.get_name();
	const std::string nickname = client->get_nickname().empty() ? "*" : client->get_nickname();

	// Already registered
	if (client->get_registered())
	{
		client->set_bufferOut(Replies::ERR_ALREADYREGISTERED(serverName, nickname));
		server.switchPollOut(client->get_socketFd());
		return;
	}
	// Not enough params
	if (msg.paramsCount() < 4)
	{
		client->set_bufferOut(Replies::ERR_NEEDMOREPARAMS(serverName, nickname, "USER"));
		server.switchPollOut(client->get_socketFd());
		return;
	}
	// Empty username
	const std::string username = msg.param(0);
	if (username.empty())
	{
		client->set_bufferOut(Replies::ERR_NEEDMOREPARAMS(serverName, nickname, "USER"));
		server.switchPollOut(client->get_socketFd());
		return;
	}
	client->set_username(username);
	client->set_realname(msg.param(3));

	registerClient(server, client);
}

//RÉCUPÉRATION DU TOKEN :
//Cas 1 : "PING token" (paramètre normal, pas de ':')
//  msg.param(0) = "token"   msg.trailing = ""   msg.hasTrailing = false
//Cas 2 : "PING :token" (trailing)
//  msg.params = []   msg.trailing = "token"   msg.hasTrailing = true
//  msg.param(0) retourne msg.trailing car params est vide et hasTrailing=true
//msg.param(0) unifie les deux cas grâce à la logique de Message::param().
//Si les deux sont vides (PING sans argument), on répond PONG avec "".

//ex : PING token
void CommandHandler::handlePING(Server &server, Client *client, const Message &msg)
{
	const std::string token = msg.param(0);

	client->set_bufferOut(Replies::PONG(/*server.get_name()s,*/ token));
	server.switchPollOut(client->get_socketFd());
}
//"The server acknowledges this by replying with an ERROR message and closing the connection to the client."
//"Servers SHOULD prepend <reason> with the ASCII string 'Quit: ' when sending QUIT messages to other clients."

//ex : QUIT :Goodbye everyone
void CommandHandler::handleQUIT(Server &server, Client *client, const Message &msg)
{
	const std::string reason = msg.hasTrailing ? msg.trailing : "";
	const std::string broadcastquitMsg = "Quit: " + reason;

	if (client->get_registered())
		server.removeClientFromAllChannels(client->get_socketFd(), broadcastquitMsg); // broadcast à ajouter

	const std::string errorReason = reason.empty() ? "Goodbye" : reason;
	client->set_bufferOut(Replies::ERROR_MSG(client->get_hostname() + " (" + errorReason + ")"));
	server.switchPollOut(client->get_socketFd());
	client->set_shouldClose(true);
}

void CommandHandler::registerClient(Server &server, Client *client)
{
	if (client->get_registered())
		return;
	if (!client->get_passOk() || client->get_nickname().empty() || client->get_username().empty())
		return;
	client->set_registered(true);

	const std ::string name = server.get_name();
	const std::string nickname = client->get_nickname();
	const std::string user = client->get_username();
	const std::string host = client->get_hostname();

	// Send welcome messages 001-004
	client->set_bufferOut(Replies::RPL_WELCOME(name, nickname, user, host));
	client->set_bufferOut(Replies::RPL_YOURHOST(name, nickname));
	client->set_bufferOut(Replies::RPL_CREATED(name, nickname));
	client->set_bufferOut(Replies::RPL_MYINFO(name, nickname));
	server.switchPollOut(client->get_socketFd());
	std::cout << "[INFO] Client " << nickname << "!" << user << "@" << host << std::endl;
}



////////////////////////////////////////////////////////////////////////////////////////////////////

//Aileen part :p
bool	CommandHandler::isMemberChannel(Client *c, Channel *chan)///!\si incoherence d'etat entre chan et client ca peut renvoyer une erreur !
{
	if (!chan || !c)
		return (false);
	if (chan->isMember(c) && c->isInChannel(chan))
		return (true);
	return (false);
}

bool	CommandHandler::checkChannelKey(Channel const *chan, std::string const key)//si chan n'a pas de clef, renvoie true
{
	if (chan->hasKey())
		return (chan->isKey(key));
	return (true);
}

bool	CommandHandler::checkLimit(Channel const *chan)//true = limit channel non atteinte, false = limite atteinte
{
	if (chan->get_limit() && (chan->get_limit() <= chan->nbMembers()))
		return (false);
	return (true);
}

Client	*CommandHandler::checkClientExists(Server &server, std::string const &nickname)
{
	Client	*c = server.get_client(nickname);

	if (c == NULL || c->get_registered() == false)
		return (NULL);
	return (c);
}

bool	CommandHandler::isValidChannelName(std::string const &name)
{
	//chaine non vide => min 2 max 50 (dont le #)
	if (name.empty() || name.size() < 2 || name.size() > 50)
		return (false);
	if (name[0] != '#')//=channel local on ne gere pas les autres types de channels
		return (false);
	if (name.find_first_of(" ,:\a\r\n") != std::string::npos)
		return (false);
	return (true);
}

bool	CommandHandler::isValidClientName(std::string const &nickname)//checker que le client n'existe pas deja cote serveur est a faire avant de creer un nouveau client mais c'est pas dans cette fonction :)
{
	std::string allowedChar = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
	allowedChar = allowedChar + "abcdefghijklmnopqrstuvwxyz";
	allowedChar = allowedChar + "0123456789";
	allowedChar = allowedChar + "-_[]\\`^{}|";//C'est juste pour que ce soit plus lisible qu'une seule grosse ligne

	if (nickname.empty() || nickname.size() > 9)//chaine non vide => min 1 max 9 (rfc2812)
		return (false);
	if (nickname.find_first_not_of(allowedChar) != std::string::npos)
		return (false);
	if (nickname.find_first_of("0123456789-") == 0)//nickname[0] != 0123456789-#: (# et : ne sont de toutes façons pas autorises)
		return (false);
	return (true);
}

//ex : JOIN #chan1,#chan2 key1,key2
void CommandHandler::handleJOIN(Server &server, Client *c, const Message &msg)
{
//variables :
//	- std::string	key_tmp;
//	- std::string	chan_tmp;
	std::string	chan;
	std::string	key;
	std::string lst_chan;
	std::string lst_key = "";
	size_t		pos;
	Channel		*chan_ptr;
	bool		no_error = false;

	const std::string serverName = server.get_name();
	const std::string nickname = c->get_nickname();

//checker que j'ai au moins 1 param non vide
	if (!msg.params.size() || msg.params[0].empty())
	{
		c->set_bufferOut(Replies::ERR_NEEDMOREPARAMS(serverName, nickname, "JOIN")	);//ERR_NEEDMOREPARAMS(461)
		server.switchPollOut(c->get_socketFd());
		return;
	}
	lst_chan = msg.params[0];
	if (msg.params.size() > 1)
		lst_key = msg.params[1];
//si msg->param[0] = "0"
	if (lst_chan == "0")//JOIN 0 == PART chan1,chan2...
	{
		//;// => on cree un message part avec prefix = ???(celui du msg actuel ?), command = "PART", params = c->get_channels() (donc sous forme de string), trailing ???, has trailing ????
		//;//=> On appelle Part avec le nouveau message
		return;
	}

//boucler jusqu'a ce que msg.params[0] (chan) soit vide
	while (!lst_chan.empty())//pos != std::string::npos
	{
//	- recuperer le chan
		pos = lst_chan.find(",", 0);
			chan = lst_chan.substr(0, pos);
		if (pos != std::string::npos)
			lst_chan.erase(0, pos + 1);
		else
			lst_chan.clear();
//	- recuperer la key si ya sinon mettre a ""
		if (!lst_key.empty())
		{
			pos = lst_key.find(",", 0);
			key = lst_key.substr(0, pos);
			if (pos != std::string::npos)
				lst_key.erase(0, pos + 1);
			else
				lst_key.clear();
		}
		else
			key = "";

//	- Channel exist
		chan_ptr = server.get_channel(chan);
		if (chan_ptr && !isMemberChannel(c, chan_ptr))//mettre ce qu'il y a dedans dans un bloc qui retourne true si reussi en mode "no_error = checksJoinIfChannelExists(c, chan_ptr, key);" A voir avec les messages d'erreur ?
		{
			if (!checkLimit(chan_ptr))
			{
				c->set_bufferOut(Replies::ERR_CHANNELISFULL(serverName, nickname, chan));//ERR_CHANNELISFULL (471)
				server.switchPollOut(c->get_socketFd());
			}
			else if (chan_ptr->get_inviteOnly() && !chan_ptr->isInvited(c))
			{
				c->set_bufferOut(Replies::ERR_INVITEONLYCHAN(serverName, nickname, chan));//ERR_INVITEONLYCHAN(473)
				server.switchPollOut(c->get_socketFd());
			}
			else if (!checkChannelKey(chan_ptr, key))
			{
				c->set_bufferOut(Replies::ERR_BADCHANNELKEY(serverName, nickname, chan));//ERR_BADCHANNELKEY (475)
				server.switchPollOut(c->get_socketFd());
			}
			else
				no_error = true;
		}
		else if (chan_ptr == NULL)//idem avec "no_error = checksJoinIfChannelDoesNotExists(c, chan_ptr, key);" ? A voir avec les messages d'erreur ?
		{
			if (!isValidChannelName(chan))
				{
					c->set_bufferOut(Replies::ERR_NOSUCHCHANNEL(serverName, nickname, chan));//ERR_NOSUCHCHANNEL (403)
					server.switchPollOut(c->get_socketFd());
				}
			else
			{
				chan_ptr = server.createChannel(chan);
				chan_ptr->addOperator(c);
				no_error = true;
			}
		}
		if (no_error)
		{
//	- Si tout s'est bien passe jusqu'ici c devient membre
			chan_ptr->addMember(c);
			chan_ptr->removeInvite(c);
			c->addChannel(chan_ptr);

	//	- Envoyer message a tous les membres (meme c) ":pouet!user@localhost JOIN #Tagada\r\n" avec pouet le nouveau membre et #Tagada le channel
			broadcastToChannel(server, chan_ptr, Replies::JOIN_MSG(c->get_nickname(), c->get_username(), c->get_hostname(), chan), NULL);

	//	- Envoyer messages a c :
	//		- si topic du serveur != "" => RPL_TOPIC (332) (en gros => ":server 332 :On aime les fraises\r\n")
			if (chan_ptr->get_topic().empty())
				c->set_bufferOut(Replies::RPL_NOTOPIC(serverName, nickname, chan));//RPL_NOTOPIC (331)
			else
				c->set_bufferOut(Replies::RPL_TOPIC(serverName, nickname, chan, chan_ptr->get_topic()));//RPL_TOPIC (332)
			server.switchPollOut(c->get_socketFd());

			namesReply(server, c, chan_ptr);//ici on a RPL_NAMREPLY (353) & RPL_ENDOFNAMES (366) => idem, on a juste besoin de les renvoyer en numeric comme pour TOPIC
			no_error = false;
		}
	}

//Questions :
//Si + de clefs que de channels => clefs supplementaires ignorees
//Comment gérer les messages d'erreur non bloquants => bool no_error
	return;
}
//ex : PRIVMSG #chan1,#chan2 user1,user2 :Hello everyone
void	CommandHandler::handlePRIVMSG(Server &server, Client *c, const Message &msg)
{
	//Le check du client non null à faire avant non ?
	int								t = 0;//  0 = erreur, 1 = chan, 2 = client, 3 = deja vu
	std::string						lst_target;
	std::string						target;
	std::set<std::string>			old_targets;
	size_t							pos = 0;
	Channel							*chan_target;
	std::set<Client *>				chan_members;
	std::set<Client *>::iterator	it;
	Client							*user_target;

	const std::string serverName = server.get_name();
	const std::string nickname = c->get_nickname();

	if	(msg.params.empty() || msg.params[0].empty())
	{
		c->set_bufferOut(Replies::ERR_NORECIPIENT(serverName, nickname, "PRIVMSG"));//ERR_NORECIPIENT(411)
		server.switchPollOut(c->get_socketFd());
		return;
	}
	if (msg.params.size() < 2 || msg.params[1].empty())
	{
		c->set_bufferOut(Replies::ERR_NOTEXTTOSEND(serverName, nickname));//ERR_NOTEXTTOSEND(412)
		server.switchPollOut(c->get_socketFd());
		return;
	}

//checker les param (au moins 2) et si 2, !param[1].empty()
	lst_target = msg.params[0];
	while (!lst_target.empty() && pos != std::string::npos)//gerder la condition pos ?
	{
//	- recuperer la target
		pos = lst_target.find(",", 0);
		target = lst_target.substr(0, pos);
		if (pos != std::string::npos)
			lst_target.erase(0, pos + 1);
		else
			lst_target.clear();;
//Definir la target avec t
		if (old_targets.find(Server::lowerName(target)) != old_targets.end())
			t = 3;
		else if (isValidChannelName(target))
			t = 1;
		else if (isValidClientName(target))
			t = 2;
		switch (t)
		{
			case 1:
				chan_target = server.get_channel(target);
				if (chan_target == NULL)
				{
					c->set_bufferOut(Replies::ERR_NOSUCHCHANNEL(serverName, nickname, target));//ERR_NOSUCHNICK (401)/ERR_NOSUCHCHANNEL(403) => perso je prefere 403
					server.switchPollOut(c->get_socketFd());
					break;
				}
				if (!isMemberChannel(c, chan_target))
				{
					c->set_bufferOut(Replies::ERR_CANNOTSENDTOCHAN(serverName, nickname, target));//ERR_CANNOTSENDTOCHAN (404)
					server.switchPollOut(c->get_socketFd());
					break;
				}
				old_targets.insert(Server::lowerName(target));
				chan_members = chan_target->get_members();
				it = chan_members.begin();
				//while (it != chan_members.end())
				//{
				//	user_target = *it;
				//	if (old_targets.find(Server::lowerName(user_target->get_nickname())) == old_targets.end() && user_target != c)
				//	{
				//		//;//envoyer le param[1] a la target
				//		// broadcast ??
				//		old_targets.insert(Server::lowerName(user_target->get_nickname()));
				//	}
				//	it++;
				//}
				broadcastToChannel(server, chan_target, Replies::PRIVMSG_MSG(nickname, c->get_username(), c->get_hostname(), target, msg.trailing), c);
				break;
			case 2:
				user_target = checkClientExists(server, target);
				if (user_target == NULL)
				{
					c->set_bufferOut(Replies::ERR_NOSUCHNICK(serverName, nickname, target));//ERR_NOSUCHNICK (401)
					server.switchPollOut(c->get_socketFd());
					break;
				}
				user_target->set_bufferOut(Replies::PRIVMSG_MSG(nickname, c->get_username(), c->get_hostname(), target, msg.trailing));//envoyer le param[1] a la target
				server.switchPollOut(user_target->get_socketFd());
				old_targets.insert(Server::lowerName(target));
				break;
			case 3:
				break;
			default:
				c->set_bufferOut(Replies::ERR_NOSUCHNICK(serverName, nickname, target));//ERR_NOSUCHNICK(401)
				server.switchPollOut(c->get_socketFd());
				break;
			}
		t = 0;
	}
	return;
}
//ex : KICK #chan1,#chan2 user1,user2 :You are not welcome here
void	CommandHandler::handleKICK(Server &server, Client *c, const Message &msg)//historiquement on pouvait avoir #chan1,#chan2 user1,user2 mais ce n'est plus tres usite ajd, du coup je ne l'ai pas implemente mais a voir si vous preferez que je le fasse aussi au cas ou
{
	std::string	reason = "has been kicked from channel";//si pas de raison precisee => mettre un message par defaut (au debut ? en mode std::string reason = "has been kicked from channel") //Faut mettre les deux points devant ? (":has been kicked from channel")
	Channel		*chan;
	size_t		pos = 0;
	Client		*user_target;
	std::string	lst_members;
	std::string	target;
	std::string	chan_name;

	const std::string serverName = server.get_name();
	const std::string nickname = c->get_nickname();

//Params necessaires (#chan list_user & raison(opt))
	if (msg.params.empty() || msg.params.size() < 2)
	{
		c->set_bufferOut(Replies::ERR_NEEDMOREPARAMS(serverName, nickname, "KICK"));//ERR_NEEDMOREPARAMS(461)
		server.switchPollOut(c->get_socketFd());
		return;
	}

	chan_name = msg.params[0];
	lst_members = msg.params[1];
	if (msg.params.size() > 2)
		reason = msg.params[2];
//channel existant
	chan = server.get_channel(chan_name);
	if (!chan)
	{
		c->set_bufferOut(Replies::ERR_NOSUCHCHANNEL(serverName, nickname, chan_name));//ERR_NOSUCHCHANNEL (403)
		server.switchPollOut(c->get_socketFd());
		return;
	}
	while (!lst_members.empty() && pos != std::string::npos)//virer condition pos ?
	{
		pos = lst_members.find(",", 0);
		target = lst_members.substr(0, pos);
		if (pos != std::string::npos)
			lst_members.erase(0, pos + 1);
		else
			lst_members.clear();
		user_target = checkClientExists(server, target);
		if (!isMemberChannel(c, chan))//checker avec la normalisation du client name
		{
			c->set_bufferOut(Replies::ERR_NOTONCHANNEL(serverName, nickname, chan_name));//ERR_NOTONCHANNEL (442)
			server.switchPollOut(c->get_socketFd());
			//break ?
		}
		else if (!chan->isOperator(c))
		{
			c->set_bufferOut(Replies::ERR_CHANOPRIVSNEEDED(serverName, nickname, chan_name));//ERR_CHANOPRIVSNEEDED (482)
			server.switchPollOut(c->get_socketFd());
			//break ? et if au lieu de else if ?
		}
		else if (!user_target || !user_target->get_registered() || !isMemberChannel(user_target, chan))
		{
			c->set_bufferOut(Replies::ERR_USERNOTINCHANNEL(serverName, nickname, target, chan_name));//ERR_USERNOTINCHANNEL (441)
			server.switchPollOut(c->get_socketFd());
			//continue ??
		}
		else
		{
			broadcastToChannel(server, chan, Replies::KICK_MSG(nickname, c->get_username(), c->get_hostname(), chan_name, target, reason), NULL);
			server.removeClientFromChannel(user_target, chan);
		}
	}
	return;
}

//ex : INVITE user1 #chan1
void	CommandHandler::handleINVITE(Server &server, Client *c, const Message &msg)//nickname chan//ya une incoherence dans les infos de modern.ircdocs et la RFC sur l'existance des chan et client du coup j'ai tranche en demandant a ce que les deux existent bien
{
	Channel		*chan;
	Client		*invited_guy;

	const std::string serverName = server.get_name();
	const std::string nickname = c->get_nickname();

	if (msg.params.empty() || msg.params.size() < 2)
	{
		c->set_bufferOut(Replies::ERR_NEEDMOREPARAMS(serverName, nickname, "INVITE"));//ERR_NEEDMOREPARAMS(461)
		server.switchPollOut(c->get_socketFd());
		return;
	}

	const std::string targetNick = msg.params[0];
	const std::string chanName = msg.params[1];

	invited_guy = checkClientExists(server, msg.params[0]);
	chan = server.get_channel(msg.params[1]); // chan pas utilisable dans mes ERR_ j'utlise msg.params[1] pour récup le nom du chan, est ce que ca vaut le coup de le passer en lowercaser pour les msgs ?

	if (chan == NULL)
	{
		c->set_bufferOut(Replies::ERR_NOSUCHCHANNEL(serverName, nickname, chanName));//ERR_NOSUCHCHANNEL (403)
		server.switchPollOut(c->get_socketFd());
	}
	else if (!isMemberChannel(c, chan))
	{
		c->set_bufferOut(Replies::ERR_NOTONCHANNEL(serverName, nickname, chanName));//ERR_NOTONCHANNEL (442)
		server.switchPollOut(c->get_socketFd());
	}
	else if (chan->get_inviteOnly() && !chan->isOperator(c))
	{
		c->set_bufferOut(Replies::ERR_CHANOPRIVSNEEDED(serverName, nickname, chanName));//ERR_CHANOPRIVSNEEDED (482)
		server.switchPollOut(c->get_socketFd());
	}
	else if (invited_guy == NULL || !invited_guy->get_registered())
	{
		c->set_bufferOut(Replies::ERR_NOSUCHNICK(serverName, nickname, targetNick));//ERR_NOSUCHNICK(401)
		server.switchPollOut(c->get_socketFd());
	}
	else if (isMemberChannel(invited_guy, chan))
	{
		c->set_bufferOut(Replies::ERR_USERONCHANNEL(serverName, nickname, targetNick, chanName));//ERR_USERONCHANNEL(443)
		server.switchPollOut(c->get_socketFd());
	}
	else
	{
		chan->addInvite(invited_guy);
		c->set_bufferOut(Replies::RPL_INVITING(serverName, nickname, targetNick, chanName));//RPL_INVITING (341) a op
		server.switchPollOut(c->get_socketFd());
		invited_guy->set_bufferOut(Replies::INVITE_MSG(nickname, c->get_username(), c->get_hostname(), targetNick, chanName));//message d'invitation a invited_guy (:<c au bon format> INVITE <invited_guy> <chan>)
		server.switchPollOut(invited_guy->get_socketFd());
	}
	return;
}

//ex : TOPIC #chan1 :New topic for chan1
void	CommandHandler::handleTOPIC(Server &server, Client *c, const Message &msg)//chan [nouveau topic]
{
	Channel		*chan;
	std::string	topic;

	const std::string serverName = server.get_name();
	const std::string nickname = c->get_nickname();

	if (msg.params.empty() || msg.params[0].empty())
	{
		c->set_bufferOut(Replies::ERR_NEEDMOREPARAMS(serverName, nickname, "TOPIC"));//ERR_NEEDMOREPARAMS(461)
		server.switchPollOut(c->get_socketFd());
		return;
	}

	const std::string chanName = msg.params[0];
	chan = server.get_channel(msg.params[0]);

	if (chan == NULL)
	{
		c->set_bufferOut(Replies::ERR_NOSUCHCHANNEL(serverName, nickname, chanName));//ERR_NOSUCHCHANNEL (403)
		server.switchPollOut(c->get_socketFd());
	}
	else if (!isMemberChannel(c, chan))
	{
		c->set_bufferOut(Replies::ERR_NOTONCHANNEL(serverName, nickname, chanName));//ERR_NOTONCHANNEL (442)
		server.switchPollOut(c->get_socketFd());
	}
	else if (msg.params.size() == 1 && !msg.hasTrailing)
	{
		topic = chan->get_topic();
		if (topic.empty())
			c->set_bufferOut(Replies::RPL_NOTOPIC(serverName, nickname, chanName));//RPL_NOTOPIC (331)
		else
			c->set_bufferOut(Replies::RPL_TOPIC(serverName, nickname, chanName, topic));//RPL_TOPIC (332)
		server.switchPollOut(c->get_socketFd());
	}
	else if (chan->get_topicProtected() && !chan->isOperator(c))
	{
		c->set_bufferOut(Replies::ERR_CHANOPRIVSNEEDED(serverName, nickname, chanName));//ERR_CHANOPRIVSNEEDED (482)
		server.switchPollOut(c->get_socketFd());
	}
	else
	{
		chan->set_topic(msg.params[1], c);
		//Broadcast ? ;//Topic a change (meme si on a mis le meme topic ou qu'on l'a clear) => ":<c au bon format> TOPIC <chan> :<new topic set>")
		broadcastToChannel(server, chan, Replies::TOPIC_MSG(nickname, c->get_username(), c->get_hostname(), chanName, msg.trailing), NULL);
	}
	return;
}

//ex : PART #chan1,#chan2 :Goodbye everyone
void	CommandHandler::handlePART(Server &server, Client *c, const Message &msg)//Non obg mais utile pout JOIN//Penser a suppr membre & operator & invite
{
	size_t		pos;
	Channel		*chan_ptr;
	std::string lst_chan;
	std::string	chan;
	std::string	reason = c->get_nickname();//a voir s'il faut ajouter les ":" devant ici

	const std::string serverName = server.get_name();
	const std::string nickname = c->get_nickname();

	if (msg.params.empty() || msg.params[0].empty())
	{
		c->set_bufferOut(Replies::ERR_NEEDMOREPARAMS(serverName, nickname, "PART"));//ERR_NEEDMOREPARAMS(461)
		server.switchPollOut(c->get_socketFd());
		return;
	}
	lst_chan = msg.params[0];
	if (msg.params.size() > 1)
		reason = msg.params[1];
	while (!lst_chan.empty())
	{
		pos = lst_chan.find(",", 0);
		chan = lst_chan.substr(0, pos);
		if (pos != std::string::npos)
			lst_chan.erase(0, pos + 1);
		else
			lst_chan.clear();
		chan_ptr = server.get_channel(chan);
		if (chan_ptr == NULL)
		{
			c->set_bufferOut(Replies::ERR_NOSUCHCHANNEL(serverName, nickname, chan));//ERR_NOSUCHCHANNEL (403)
			server.switchPollOut(c->get_socketFd());
		}
		else if (!isMemberChannel(c, chan_ptr))
		{
			c->set_bufferOut(Replies::ERR_NOTONCHANNEL(serverName, nickname, chan));//ERR_NOTONCHANNEL (442)
			server.switchPollOut(c->get_socketFd());
		}
		else
		{
			//;//broadcast de la reponse de PART => "<c au bon format> <chan> :raison"
			broadcastToChannel(server, chan_ptr, Replies::PART_MSG(nickname, c->get_username(), c->get_hostname(), chan, reason), NULL);
			server.removeClientFromChannel(c, chan_ptr);
		}
	}
	return;
}

void	CommandHandler::namesReply(Server &server, Client *c, Channel *chan)//A voir s'il faut le message du JOIN aussi pour Nayel
{
	std::string						lst_names;
	std::set<Client *>				members = chan->get_members();
	std::set<Client *>::iterator	it = members.begin();
	Client							*m;

	while (it != members.end())
	{
		if (!lst_names.empty())
			lst_names = lst_names + " ";
		m = *it;
		if (chan->isOperator(m))
			lst_names = lst_names + "@";
		lst_names = lst_names + m->get_nickname();
		it++;
	}

	c->set_bufferOut(Replies::RPL_NAMREPLY(server.get_name(), c->get_nickname(), chan->get_name(), lst_names));//envoyer a c RPL_NAMREPLY (353) avec lst_names comme tail, le symbole du chan a priori ce sera tj = pour nous
	c->set_bufferOut(Replies::RPL_ENDOFNAMES(server.get_name(), c->get_nickname(), chan->get_name()));//envoyer a c RPL_ENDOFNAMES (366)
	server.switchPollOut(c->get_socketFd());
	return;
}

//ex : MODE #Tagada +o Pouet
void	CommandHandler::handleMODE(Server &server, Client *c, const Message &msg)//target (chan) [modestring mode_arg] => ex: #Tagada +o Pouet tagada
{///!\ ON EST CENSES POUVOIR AVOIR DES TRUCS COMME +ltkey pouet...
	Channel		*chan_ptr;
	Client		*target_user;
	std::string	modestring;

	const std::string serverName = server.get_name();
	const std::string nickname = c->get_nickname();
	const std::string chanName = msg.params[0];

	if (chanName[0] != '#')
	{
		return;//pour l'instant on ne gère que les modes de chan, pas les modes de client, du coup si la target n'est pas un chan on ignore la commande, a voir si on doit envoyer un message d'erreur ou pas
	}

	if (msg.params.empty() || msg.params[0].empty())
	{
		c->set_bufferOut(Replies::ERR_NEEDMOREPARAMS(serverName, nickname, "MODE"));//ERR_NEEDMOREPARAMS (461)
		server.switchPollOut(c->get_socketFd());
		return;
	}
	chan_ptr = server.get_channel(msg.params[0]);
	if (!chan_ptr)
	{
		c->set_bufferOut(Replies::ERR_NOSUCHCHANNEL(serverName, nickname, chanName));//ERR_NOSUCHCHANNEL (403)
		server.switchPollOut(c->get_socketFd());
		return;
	}
	if (msg.params.size() < 2) // if no modestring, just return the current modes of the channel with RPL_CHANNELMODEIS (324)
	{
		c->set_bufferOut(Replies::RPL_CHANNELMODEIS(serverName, nickname, chanName, "itkol"));//RPL_CHANNELMODEIS (324)
		server.switchPollOut(c->get_socketFd());
		return;
	}
	if (!chan_ptr->isOperator(c))
	{
		c->set_bufferOut(Replies::ERR_CHANOPRIVSNEEDED(serverName, nickname, chanName));//ERR_CHANOPRIVSNEEDED (482)
		server.switchPollOut(c->get_socketFd());
		return;
	}
	modestring = msg.params[1];
	if (modestring.size() != 2 || modestring.find_first_not_of("+-itlok") != std::string::npos
		|| (modestring[0] != '+' && modestring[0] != '-')
		|| modestring.find_first_of("itlok") == std::string::npos)
	{
		c->set_bufferOut(Replies::ERR_UNKNOWNMODE(serverName, nickname, modestring[0]));//ERR_UNKNOWNMODE (472)
		server.switchPollOut(c->get_socketFd());
		return;
	}

	std::string			type[10] = {"+i","-i","+t","-t","+l","-l","+o","-o","+k","-k"};
	int					i = 0;
	std::stringstream	extract_nb;
	size_t				nb_l = 0;

	while (i < 10)
	{
		if (type[i] == modestring)
			break;
		i++;
	}
	switch (i)
	{
	//si i (+ ou -) (invite-only) => D ?
	//	- + => chan->set_inviteOnly(true);
	//	- - => chan->set_inviteOnly(false);
		case 0://+i
			if (chan_ptr->get_inviteOnly())
				return;
			chan_ptr->set_inviteOnly(true, c);
			//broadcast ;//message broadcast du changement => j'hesite soit on le met a chaque fois comme ici, soit on ne le fait qu'une fois apres le switch case, je te laisse voir le plus pratique pour toi Nayel
			broadcastToChannel(server, chan_ptr, Replies::MODE_MSG(nickname, c->get_username(), c->get_hostname(), chanName, modestring), NULL);
			break;
		case 1://-i
			if (!chan_ptr->get_inviteOnly())
				return;
			chan_ptr->set_inviteOnly(false, c);
			//broadcast ;//message broadcast du changement
			broadcastToChannel(server, chan_ptr, Replies::MODE_MSG(nickname, c->get_username(), c->get_hostname(), chanName, modestring), NULL);
			break;
	//si t (+ ou -) (topic protected) => D ?
	//	- + => chan->set_topicProtected(true);
	//	- - => chan->set_topicProtected(false);
		case 2://+t
			if (chan_ptr->get_topicProtected())
				return;
			chan_ptr->set_topicProtected(true, c);
			//broadcast ;//message broadcast du changement
			broadcastToChannel(server, chan_ptr, Replies::MODE_MSG(nickname, c->get_username(), c->get_hostname(), chanName, modestring), NULL);
			break;
		case 3://-t
			if (!chan_ptr->get_topicProtected())
				return;
			chan_ptr->set_topicProtected(false, c);
			//broadcast ;//message broadcast du changement
			broadcastToChannel(server, chan_ptr, Replies::MODE_MSG(nickname, c->get_username(), c->get_hostname(), chanName, modestring), NULL);
			break;
	//si l (+ ou -) (limit) => +l 10 ou -l => C ? => ignore la commande si pas de param
	//	- + nb => set_limit(nb)
	//	- - => set_limit(0);
		case 4://+l
			if (msg.params.size() < 3 || msg.params[2].empty())
				return;
			extract_nb << msg.params[2];
			extract_nb >> nb_l;
			if (extract_nb.fail() || !extract_nb.eof() || nb_l == 0)
			{
				c->set_bufferOut(Replies::ERR_INVALIDMODEPARAM(serverName, nickname, chanName, 'l', msg.params[2]));//ERR_INVALIDMODEPARAM (696) param[2] == [modestring mode_arg]
				server.switchPollOut(c->get_socketFd());
				return;
			}
			if (chan_ptr->get_limit() == nb_l)
				return;
			chan_ptr->set_limit(nb_l, c);
			//broadcast ;//message broadcast du changement
				broadcastToChannel(server, chan_ptr, Replies::MODE_MSG(nickname, c->get_username(), c->get_hostname(), chanName, modestring), NULL);
			break;
		case 5://-l
			if (chan_ptr->get_limit() == 0)
				return;
			chan_ptr->set_limit(0, c);
			//broadcast ;//message broadcast du changement
			broadcastToChannel(server, chan_ptr, Replies::MODE_MSG(nickname, c->get_username(), c->get_hostname(), chanName, modestring), NULL);
			break;
	//si o (operator) +o nickname -o nickname => B ? => ignore la commande si pas de param
	//	- + member =>
	//		- check member est dans le chan => ERR_USERNOTINCHANNEL ?
	//		- ajoute member en operateur si pas deja et envoie message si change;ent a eu lieu
	//	- - member =>
	//		- check member est dans le chan => ERR_USERNOTINCHANNEL ?
	//		- retire member des operateurs si bien l4un d4entre eux et envoie message si change;ent a eu lieu
	//	- dans les deux cas continuer silencieusement (dc pas d'erreur ni de reponse juste on termine) sans ajouter ou retirer le status si est deja ou n'est deja pas operator
		case 6://+o
			if (msg.params.size() < 3 || msg.params[2].empty())
				return;
			target_user = checkClientExists(server, msg.params[2]);
			if (!target_user || !isMemberChannel(target_user, chan_ptr))
			{
				c->set_bufferOut(Replies::ERR_USERNOTINCHANNEL(serverName, nickname, msg.params[2], chanName));//ERR_USERNOTINCHANNEL (441)
				server.switchPollOut(c->get_socketFd());
				return;
			}
			if (!chan_ptr->isOperator(target_user))
			{
				chan_ptr->addOperator(target_user);
				//broadcast ;//broadcast sur chan_ptr pour dire que target_user est bien operator => "<c au bon format> MODE #chan +o user_target\r\n"
				broadcastToChannel(server, chan_ptr, Replies::MODE_MSG(nickname, c->get_username(), c->get_hostname(), chanName, modestring), NULL);
			}
			break;
		case 7://-o
			if (msg.params.size() < 3 || msg.params[2].empty())
				return;
			target_user = checkClientExists(server, msg.params[2]);
			if (!target_user || !isMemberChannel(target_user, chan_ptr))
			{
				c->set_bufferOut(Replies::ERR_USERNOTINCHANNEL(serverName, nickname, msg.params[2], chanName));//ERR_USERNOTINCHANNEL (441)
				server.switchPollOut(c->get_socketFd());
				return;
			}
			if (chan_ptr->isOperator(target_user))
			{
				chan_ptr->removeOperator(target_user);
				//broadcast ;//broadcast sur chan_ptr pour dire que target_user n'est plus operator "<c au bon format> MODE #chan -o user_target\r\n"
				broadcastToChannel(server, chan_ptr, Replies::MODE_MSG(nickname, c->get_username(), c->get_hostname(), chanName, modestring), NULL);
			}
			break;
	//k (key) => B ? ou C ? => ignore la commande si pas de param
	//	- + newkey
	//		- si key deja set => ;//ERR_KEYSET (467)
	//		- check newkey valable => ;//ERR_INVALIDMODEPARAM (696) ou ERR_INVALIDKEY (525)
	//		- chan->set_key(newkey);
	//	- -k key OU -k
	//		- chan->set_key("");
	//Servers MAY choose to hide sensitive information when sending the mode changes.
	//  key        =  1*23( %x01-05 / %x07-08 / %x0C / %x0E-1F / %x21-7F )
    //   ; any 7-bit US_ASCII character,
    //   ; except NUL, CR, LF, FF, h/v TABs, and " "
		case 8://+k
			if (msg.params.size() < 3 || msg.params[2].empty())
				return;
			if (chan_ptr->hasKey())
			{
				c->set_bufferOut(Replies::ERR_KEYSET(serverName, nickname, chanName));//ERR_KEYSET (467)
				server.switchPollOut(c->get_socketFd());
				return;
			}
			if (!chan_ptr->isValidKey(msg.params[2]))//1->23char, ascii vsibiles, pas d'espaces, \r\n\t\v\n\0 interdits, pas de ','
			{
				c->set_bufferOut(Replies::ERR_INVALIDMODEPARAM(serverName, nickname, chanName, 'k', msg.params[2]));//ERR_INVALIDMODEPARAM (696)
				server.switchPollOut(c->get_socketFd());
				return;
			}
			chan_ptr->set_key(msg.params[2], c);
			//broadcast;//broadcast un message indiquant qu'une clef a ete mise sur le channel (on la donne ou pas ?)
			broadcastToChannel(server, chan_ptr, Replies::MODE_MSG(nickname, c->get_username(), c->get_hostname(), chanName, modestring), NULL);
			break;
		case 9://-k
			if (!chan_ptr->hasKey())
				return;
			chan_ptr->set_key("", c);
			//broadcast;//broadcast un message indiquant que la clef du channel a ete effacee
			broadcastToChannel(server, chan_ptr, Replies::MODE_MSG(nickname, c->get_username(), c->get_hostname(), chanName, modestring), NULL);
			break;
		default:
			c->set_bufferOut(Replies::ERR_UNKNOWNMODE(serverName, nickname, modestring[0]));//ERR_UNKNOWNMODE (472)
			server.switchPollOut(c->get_socketFd());
			break;
	}
	return;
}
