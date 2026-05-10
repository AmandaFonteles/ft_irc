/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CommandHandler.cpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aibonade <aibonade@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/05 15:55:21 by dnayel            #+#    #+#             */
/*   Updated: 2026/05/10 19:23:59 by aibonade         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/CommandHandler.hpp"
#include "../includes/Server.hpp"
#include "../includes/Client.hpp"
#include "../includes/Channel.hpp"

#include <cctype>
#include <iostream>

void CommandHandler::handleCommand(Server &server, Client &client, const Message &msg)
{
	if (msg.command.empty())
		return; // Message ou comportement particulier à définir ??
// Authorized COMMANDS even without registration
	if (msg.command == "PASS")
		handlePASS(server, client, msg);
	else if (msg.command == "NICK")
		handleNICK(server, client, msg);
	else if (msg.command == "USER")
		handleUSER(server, client, msg);

// Unauthorized COMMANDS without registration
	if (!client.get_registered()) // CHECK comment la variable est appelée dans Client.hpp
	{
		// Send error message to client (e.g. "451 :You have not registered")
		return;
	}
// Ignored COMMANDS (not implemented) !!! MAYBE MORE TO ADD
	if (msg.command == "CAP"   || msg.command == "WHO"	||
		msg.command == "WHOIS" || msg.command == "AWAY" ||
		msg.command == "NAMES" || msg.command == "LIST"	||
		msg.command == "MOTD"  || msg.command == "LUSERS" )
		return;

// Comportement si commande invalide ou inconnue ??

}

void CommandHandler::handlePASS(Server &server, Client &client, const Message &msg)
{
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
}

void CommandHandler::handleNICK(Server &server, Client &client, const Message &msg)
{
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
}

void CommandHandler::handleUSER(Server &server, Client &client, const Message &msg)
{
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
}


//Aileen part :p
bool	CommandHandler::isMemberChannel(Client *c, Channel *chan)
{
	if (chan->isMember(c) && c->isInChannel(chan))
		return (true);
	return (false);
}

bool	CommandHandler::checkChannelKey(Channel const *chan, std::string const key)
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

Client	*CommandHandler::checkClientExists(Server &server, std::string const &nickname)//Checker les pb avec le Server, peut-être enlever la reference et passer par une copie du serveur, mais faut que les adresses Clients restent les memes
{
	Client	*c = server.get_client(nickname);

	if (c == NULL || c->get_registered() == false)
		return (NULL);
	return (c);
}

bool	CommandHandler::isValidChannelName(std::string const &name)//(commence par # on ne gere pas les autres vu que notre serveur est uniquement local) + /!\insensible a la casse & n'existe pas deja !
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

bool	CommandHandler::isValidClientName(Server const &server, std::string const &nickname)//checker que le client n'existe pas deja cote serveur est a faire avant de creer un nouveau client mais c'est pas dans cette fonction :)
{
	std::string allowedChar = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
	allowedChar = allowedChar + "abcdefghijklmnopqrstuvwxyz";
	allowedChar = allowedChar + "0123456789";
	allowedChar = allowedChar + "-_[]\\`^{}|";//C'est juste pour que ce soit plus lisible qu'une seule grosse ligne

std::cout << "[DEBUG] allowChar string = \"" << allowedChar << "\""<< std::endl;
	if (nickname.empty() || nickname.size() > 9)//chaine non vide => min 1 max 9 (rfc2812)
		return (false);
	if (nickname.find_first_not_of(allowedChar) != std::string::npos)
		return (false);
	if (nickname.find_first_of("0123456789-" == 0))//nickname[0] != 0123456789-#: (# et : ne sont de toutes façons pas autorises)
		return (false);
	return (true);
}

void CommandHandler::handleJOIN(Server &server, Client *c, const Message &msg)
{
	//std::vector<std::string>	params;
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

	//checker que j'ai au moins 1 param non vide
	if (!msg.params.size() || msg.params[0].empty())
		;//ERR_NEEDMOREPARAMS(461)
	lst_chan = msg.params[0];
	if (msg.params.size() > 1)
		lst_key = msg.params[1];
	//si msg->param[0] = "0" 
	if (lst_chan == "0")//JOIN 0 == PART chan1,chan2...
	{
		;//=> On appelle Part pour chaque channel
		return;
	}

	//boucler jusqu'a ce que msg.params[0] (chan) soit vide
	while (!lst_chan.empty())
	{
	//	- recuperer le chan
		pos = lst_chan.find(",", 0);
		chan = lst_chan.substr(0, pos);
		lst_chan.erase(0, pos + 1);
	//	- recuperer la key si ya sinon mettre a ""
		if (!lst_key.empty())
		{
			pos = lst_key.find(",", 0);
			key = lst_key.substr(0, pos);
			lst_key.erase(0, pos + 1);
		}
		else
			key = "";

	//	- Channel exist
		chan_ptr = server.get_channel(chan);
		if (chan_ptr && !isMemberChannel(c, chan_ptr))//mettre ce qu'il y a dedans dans un bloc qui retourne true si reussi en mode "no_error = checksJoinIfChannelExists(c, chan_ptr, key);" A voir avec les messages d'erreur ? 
		{
			if (checkLimit(chan_ptr))
				;//ERR_CHANNELISFULL (471)
			else if (chan_ptr->get_inviteOnly() && !chan_ptr->isInvited(c))
				;//ERR_INVITEONLYCHAN(473)
			else if (checkChannelKey(chan_ptr, key))
				;//ERR_BADCHANNELKEY (475)
			else
				no_error = true;
		}
		else if (chan_ptr == NULL)//idem avec "no_error = checksJoinIfChannelDoesNotExists(c, chan_ptr, key);" ? A voir avec les messages d'erreur ?
		{
			if (!isValidChannelName(chan))
				;//ERR_NOSUCHCHANNEL (403)
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
			c->addChannel(chan_ptr);
	//	- Envoyer message a tous les membres (meme c) ":pouet!user@localhost JOIN #Tagada\r\n" avec pouet le nouveau membre et #Tagada le channel
	//	- Envoyer messages a c :
	//		- Le topic du serveur RPL_TOPIC (332)
	//		- RPL_NAMREPLY (353) => NAME commande
	//		- RPL_ENDOFNAMES (366) => idem
			no_error = false;
		}
	}

	//Questions : 
	//Si + de clefs que de channels => clefs supplementaires ignorees
	//Comment gérer les messages d'erreur non bloquants => bool no_error
	return;
}
