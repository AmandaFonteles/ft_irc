/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CommandHandler.cpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dnayel <dnayel@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/05 15:55:21 by dnayel            #+#    #+#             */
/*   Updated: 2026/05/05 16:57:42 by dnayel           ###   ########.fr       */
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
	if (cmd == "PASS")
		handlePASS(server, client, msg);
	else if (cmd == "NICK")
		handleNICK(server, client, msg);
	else if (cmd == "USER")
		handleUSER(server, client, msg);

// Unauthorized COMMANDS without registration
	if (!client.isRegistered) // CHECK comment la variable est appelée dans Client.hpp
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
