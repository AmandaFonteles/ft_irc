/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Replies.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dnayel <dnayel@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/19 14:53:34 by dnayel            #+#    #+#             */
/*   Updated: 2026/05/23 18:10:44 by dnayel           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef REPLIES_HPP
# define REPLIES_HPP

# include <string>
# include <sstream>  /* ostringstream for formatNumeric */
# include <iomanip>   /* setw, setfill */


class Replies
{
	public :
		Replies();
		~Replies();

	/* Welcome Sequence */
	static std::string RPL_WELCOME(const std::string &serverName, const std::string &nick, const std::string &user, const std::string &host); //001
	static std::string RPL_YOURHOST(const std::string &serverName, const std::string &nick); //002
	static std::string RPL_CREATED(const std::string &serverName, const std::string &nick); //003
	static std::string RPL_MYINFO(const std::string &serverName, const std::string &nick); //004

	/* Registration Errors */

	static std::string ERR_NONICKNAMEGIVEN(const std::string &serverName, const std::string &nick); //431
	static std::string ERR_ERRONEUSNICKNAME(const std::string &serverName, const std::string &nick, const std::string &badNick); //432
	static std::string ERR_NICKNAMEINUSE(const std::string &serverName, const std::string &nick, const std::string &usedNick); //433
	static std::string ERR_NOTREGISTERED(const std::string &serverName, const std::string &nick); //451
	static std::string ERR_NEEDMOREPARAMS(const std::string &serverName, const std::string &nick, const std::string &command); //461
	static std::string ERR_ALREADYREGISTERED(const std::string &serverName, const std::string &nick); //462
	static std::string ERR_PASSWDMISMATCH(const std::string &serverName, const std::string &nick); //464

	/* Channel Replies */

	static std::string RPL_CHANNELMODEIS(const std::string &serverName, const std::string &nick, const std::string &channel, const std::string &modeString); //324
	static std::string RPL_NOTOPIC(const std::string &serverName, const std::string &nick, const std::string &channel); //331
	static std::string RPL_TOPIC(const std::string &serverName, const std::string &nick, const std::string &channel, const std::string &topic); //332
	static std::string RPL_INVITING(const std::string &serverName, const std::string &nick, const std::string &invitedNick, const std::string &channel); //341
	static std::string RPL_NAMREPLY(const std::string &serverName, const std::string &nick, const std::string &channel, const std::string &users); //353
	static std::string RPL_ENDOFNAMES(const std::string &serverName, const std::string &nick, const std::string &channel); //366

	/* Channel Errors */

	static std::string ERR_NOSUCHNICK(const std::string &serverName, const std::string &nick, const std::string &targetNick); //401
	static std::string ERR_NOSUCHCHANNEL(const std::string &serverName, const std::string &nick, const std::string &channel); //403
	static std::string ERR_CANNOTSENDTOCHAN(const std::string &serverName, const std::string &nick, const std::string &channel); //404
	static std::string ERR_NORECIPIENT(const std::string &serverName, const std::string &nick, const std::string &command); //411
	static std::string ERR_NOTEXTTOSEND(const std::string &serverName, const std::string &nick); //412
	static std::string ERR_USERNOTINCHANNEL(const std::string &serverName, const std::string &nick, const std::string &targetNick, const std::string &channel); //441
	static std::string ERR_NOTONCHANNEL(const std::string &serverName, const std::string &nick, const std::string &channel); //442
	static std::string ERR_USERONCHANNEL(const std::string &serverName, const std::string &nick, const std::string &targetNick, const std::string &channel); //443

	static std::string ERR_KEYSET (const std::string &serverName, const std::string &nick, const std::string &channel); //467
	static std::string ERR_CHANNELISFULL(const std::string &serverName, const std::string &nick, const std::string &channel); //471
	static std::string ERR_UNKNOWNMODE(const std::string &serverName, const std::string &nick, char modeChar); //472
	static std::string ERR_INVITEONLYCHAN(const std::string &serverName, const std::string &nick, const std::string &channel); //473
	static std::string ERR_BADCHANNELKEY(const std::string &serverName, const std::string &nick, const std::string &channel); //475
	static std::string ERR_CHANOPRIVSNEEDED(const std::string &serverName, const std::string &nick, const std::string &channel); //482
	static std::string ERR_INVALIDMODEPARAM(const std::string &serverName, const std::string &nick, const std::string &channel, char modeChar, const std::string &param); //696

	/* IRC msgs without numeric codes */

	static std::string PONG(/*const std::string &serverName,*/ const std::string &token);
	static std::string NICK_CHANGE(const std::string &oldNick, const std::string &user, const std::string &host, const std::string &newNick);

	static std::string JOIN_MSG(const std::string &nick, const std::string &user, const std::string &host, const std::string &channel);
	static std::string PART_MSG(const std::string &nick, const std::string &user, const std::string &host, const std::string &channel, const std::string &reason);
	static std::string KICK_MSG(const std::string &opNick, const std::string &opUser, const std::string &opHost, const std::string &channel, const std::string &targetNick, const std::string &reason);
	static std::string INVITE_MSG(const std::string &nick, const std::string &user, const std::string &host, const std::string &targetNick, const std::string &channel);
	static std::string TOPIC_MSG(const std::string &nick, const std::string &user, const std::string &host, const std::string &channel, const std::string &topic);
	static std::string MODE_MSG(const std::string &nick, const std::string &user, const std::string &host, const std::string &channel, const std::string &modeStr);
	static std::string PRIVMSG_MSG(const std::string &fromNick, const std::string &fromUser, const std::string &fromHost, const std::string &target, const std::string &text);
	static std::string QUIT_MSG(const std::string &nick, const std::string &user, const std::string &host, const std::string &reason);
	static std::string ERROR_MSG(const std::string &reason);


	private :

	static std::string makeNumeric(const std::string &serverName, int numeric, const std::string &targetNick, const std::string &text);
	static std::string formatNumeric(int n);
};

#endif
