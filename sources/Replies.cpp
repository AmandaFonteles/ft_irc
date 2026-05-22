/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Replies.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dnayel <dnayel@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/19 14:53:46 by dnayel            #+#    #+#             */
/*   Updated: 2026/05/22 16:23:15 by dnayel           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/Replies.hpp"

Replies::Replies() {}
Replies::~Replies() {}

/******************/
/*Helper functions*/
/******************/

std::string Replies::formatNumeric(int n)
{
	std::ostringstream oss;
	oss << std::setw(3) << std::setfill('0') << n;
	return oss.str();
}

std::string Replies::makeNumeric(const std::string &serverName, int numeric, const std::string &targetNick, const std::string &text)
{
	return (":" + serverName
		 + " " + formatNumeric(numeric)
		 + " " + targetNick
		 + " " + text
		 + "\r\n");
}

/****************************/
/*WELCOME SEQUENCE (001–004)*/
/****************************/

std::string Replies::RPL_WELCOME(const std::string &serverName, const std::string &nick, const std::string &user, const std::string &host)
{
	return (makeNumeric(serverName, 1, nick,
		":Welcome to the Internet Relay Network "
		+ nick + "!" + user + "@" + host));
}

std::string Replies::RPL_YOURHOST(const std::string &serverName, const std::string &nick)
{
	return (makeNumeric(serverName, 2, nick,
		":Your host is " + serverName + ", running version ft_irc-1.0"));
}

std::string Replies::RPL_CREATED(const std::string &serverName, const std::string &nick)
{
	return (makeNumeric(serverName, 3, nick,
		":This server was created 2026")); // date hardcodée, à changer si j'ai le temps
}

std::string Replies::RPL_MYINFO(const std::string &serverName, const std::string &nick)
{
	return (makeNumeric(serverName, 4, nick,
		serverName + " ft_irc-1.0 o itkol"));
}

/*********************/
/*REGISTRATION ERRORS*/
/*********************/

std::string Replies::ERR_NONICKNAMEGIVEN(const std::string &serverName, const std::string &nick)
{
	return (makeNumeric(serverName, 431, nick,
		":No nickname given"));
}

std::string Replies::ERR_ERRONEUSNICKNAME(const std::string &serverName, const std::string &nick, const std::string &badNick)
{
	return (makeNumeric(serverName, 432, nick,
		badNick + " :Erroneous nickname"));
}

std::string Replies::ERR_NICKNAMEINUSE(const std::string &serverName, const std::string &nick, const std::string &usedNick)
{
	return (makeNumeric(serverName, 433, nick,
		usedNick + " :Nickname is already in use"));
}

std::string Replies::ERR_NOTREGISTERED(const std::string &serverName, const std::string &nick)
{
	return (makeNumeric(serverName, 451, nick,
		":You have not registered"));
}

std::string Replies::ERR_NEEDMOREPARAMS(const std::string &serverName, const std::string &nick, const std::string &command)
{
	return (makeNumeric(serverName, 461, nick,
		command + " :Not enough parameters"));
}

std::string Replies::ERR_ALREADYREGISTERED(const std::string &serverName, const std::string &nick)
{
	return (makeNumeric(serverName, 462, nick,
		":Unauthorized command (already registered)"));
}

std::string Replies::ERR_PASSWDMISMATCH(const std::string &serverName, const std::string &nick)
{
	return (makeNumeric(serverName, 464, nick,
		":Password incorrect"));
}

/*****************/
/*CHANNEL REPLIES*/
/*****************/

std::string Replies::RPL_CHANNELMODEIS(const std::string &serverName, const std::string &nick, const std::string &channel, const std::string &modeString)
{
	return (makeNumeric(serverName, 324, nick,
		channel + " " + modeString));
}

std::string Replies::RPL_NOTOPIC(const std::string &serverName, const std::string &nick, const std::string &channel)
{
	return (makeNumeric(serverName, 331, nick,
		channel + " :No topic is set"));
}

std::string Replies::RPL_TOPIC(const std::string &serverName, const std::string &nick, const std::string &channel, const std::string &topic)
{
	return (makeNumeric(serverName, 332, nick,
		channel + " :" + topic));
}

std::string Replies::RPL_INVITING(const std::string &serverName, const std::string &nick, const std::string &targetNick, const std::string &channel)
{
	return (makeNumeric(serverName, 341, nick,
		targetNick + " " + channel));
}

std::string Replies::RPL_NAMREPLY(const std::string &serverName, const std::string &nick, const std::string &channel, const std::string &names)
{
	return (makeNumeric(serverName, 353, nick,
		"= " + channel + " :" + names));
}

std::string Replies::RPL_ENDOFNAMES(const std::string &serverName, const std::string &nick, const std::string &channel)
{
	return (makeNumeric(serverName, 366, nick,
		channel + " :End of /NAMES list"));
}

/****************/
/*CHANNEL ERRORS*/
/****************/

std::string Replies::ERR_NOSUCHNICK(const std::string &serverName, const std::string &nick, const std::string &targetNick)
{
	return (makeNumeric(serverName, 401, nick,
		targetNick + " :No such nick/channel"));
}

std::string Replies::ERR_NOSUCHCHANNEL(const std::string &serverName, const std::string &nick, const std::string &channel)
{
	return (makeNumeric(serverName, 403, nick,
		channel + " :No such channel"));
}

std::string Replies::ERR_CANNOTSENDTOCHAN(const std::string &serverName, const std::string &nick, const std::string &channel)
{
	return (makeNumeric(serverName, 404, nick,
		channel + " :Cannot send to channel"));
}

std::string Replies::ERR_NORECIPIENT(const std::string &serverName, const std::string &nick, const std::string &command)
{
	return (makeNumeric(serverName, 411, nick,
		":No recipient given (" + command + ")"));
}

std::string Replies::ERR_NOTEXTTOSEND(const std::string &serverName, const std::string &nick)
{
	return (makeNumeric(serverName, 412, nick,
		":No text to send"));
}

std::string Replies::ERR_USERNOTINCHANNEL(const std::string &serverName, const std::string &nick, const std::string &targetNick, const std::string &channel)
{
	return (makeNumeric(serverName, 441, nick,
		targetNick + " " + channel + " :They aren't on that channel"));
}

std::string Replies::ERR_NOTONCHANNEL(const std::string &serverName, const std::string &nick, const std::string &channel)
{
	return (makeNumeric(serverName, 442, nick,
		channel + " :You're not on that channel"));
}

std::string Replies::ERR_USERONCHANNEL(const std::string &serverName, const std::string &nick, const std::string &targetNick, const std::string &channel)
{
	return (makeNumeric(serverName, 443, nick,
		targetNick + " " + channel + " :is already on channel"));
}

/**********************/

std::string Replies::ERR_KEYSET(const std::string &serverName, const std::string &nick, const std::string &channel)
{
	return (makeNumeric(serverName, 467, nick,
		channel + " :Channel key already set"));
}

std::string Replies::ERR_CHANNELISFULL(const std::string &serverName, const std::string &nick, const std::string &channel)
{
	return (makeNumeric(serverName, 471, nick,
		channel + " :Cannot join channel (+l)"));
}

std::string Replies::ERR_UNKNOWNMODE(const std::string &serverName, const std::string &nick, char modeChar)
{
	return (makeNumeric(serverName, 472, nick,
		std::string(1, modeChar) + " :is unknown mode char to me"));
}

std::string Replies::ERR_INVITEONLYCHAN(const std::string &serverName, const std::string &nick, const std::string &channel)
{
	return (makeNumeric(serverName, 473, nick,
		channel + " :Cannot join channel (+i)"));
}

std::string Replies::ERR_BADCHANNELKEY(const std::string &serverName, const std::string &nick, const std::string &channel)
{
	return (makeNumeric(serverName, 475, nick,
		channel + " :Cannot join channel (+k)"));
}

std::string Replies::ERR_CHANOPRIVSNEEDED(const std::string &serverName, const std::string &nick, const std::string &channel)
{
	return (makeNumeric(serverName, 482, nick,
		channel + " :You're not channel operator"));
}

std::string Replies::ERR_INVALIDMODEPARAM(const std::string &serverName, const std::string &nick, const std::string &channel, char modeChar, const std::string &param)
{
	return (makeNumeric(serverName, 696, nick,
		channel + " " + std::string(1, modeChar) + " " + param + " :Invalid MODE parameter"));
}

/******************/
/*NO NUMERIC CODES*/
/******************/

std::string Replies::PONG(const std::string &serverName, const std::string &token)
{
	return (":" + serverName + " PONG " + serverName + " :" + token + "\r\n");
}

std::string Replies::NICK_CHANGE(const std::string &oldNick, const std::string &user, const std::string &host, const std::string &newNick)
{
	return (":" + oldNick + "!" + user + "@" + host + " NICK :" + newNick + "\r\n");
}

std::string Replies::JOIN_MSG(const std::string &nick, const std::string &user, const std::string &host, const std::string &channel)
{
	return (":" + nick + "!" + user + "@" + host + " JOIN :" + channel + "\r\n");
}

std::string Replies::PART_MSG(const std::string &nick, const std::string &user, const std::string &host, const std::string &channel, const std::string &reason)
{
	std::string msg = ":" + nick + "!" + user + "@" + host + " PART " + channel;
	if (!reason.empty())
		msg += " :" + reason;
	msg += "\r\n";
	return msg;
}

std::string Replies::KICK_MSG(const std::string &opNick, const std::string &opUser, const std::string &opHost, const std::string &channel, const std::string &targetNick, const std::string &reason)
{
	std::string msg = ":" + opNick + "!" + opUser + "@" + opHost + " KICK " + channel + " " + targetNick;
	if (!reason.empty())
		msg += " :" + reason;
	else
		msg += " :" + opNick;
	msg += "\r\n";
	return msg;
}

std::string Replies::INVITE_MSG(const std::string &nick, const std::string &user, const std::string &host, const std::string &targetNick, const std::string &channel)
{
	return (":" + nick + "!" + user + "@" + host + " INVITE " + targetNick + " :" + channel + "\r\n");
}

std::string Replies::TOPIC_MSG(const std::string &nick, const std::string &user, const std::string &host, const std::string &channel, const std::string &topic)
{
	return (":" + nick + "!" + user + "@" + host + " TOPIC " + channel + " :" + topic + "\r\n");
}

std::string Replies::MODE_MSG(const std::string &nick, const std::string &user, const std::string &host, const std::string &channel, const std::string &modeStr)
{
	return (":" + nick + "!" + user + "@" + host + " MODE " + channel + " " + modeStr + "\r\n");
}

std::string Replies::PRIVMSG_MSG(const std::string &fromNick, const std::string &fromUser, const std::string &fromHost, const std::string &target, const std::string &text)
{
	return (":" + fromNick + "!" + fromUser + "@" + fromHost + " PRIVMSG " + target + " :" + text + "\r\n");
}

std::string Replies::QUIT_MSG(const std::string &nick, const std::string &user, const std::string &host, const std::string &reason)
{
	std::string msg = ":" + nick + "!" + user + "@" + host + " QUIT";
	if (!reason.empty())
		msg += " :" + reason;
	msg += "\r\n";
	return msg;
}

std::string Replies::ERROR_MSG(const std::string &reason)
{
	return ("ERROR :Closing Link: " + reason + "\r\n");
}


