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
		return;
	if (msg.command == "CAP")
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

	if (!client->get_registered())
	{
		const std::string nick = client->get_nickname().empty() ? "*" : client->get_nickname(); 
		client->set_bufferOut(Replies::ERR_NOTREGISTERED(server.get_name(), nick));
		server.switchPollOut(client->get_socketFd());
		return;
	}

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

	if (msg.command == "WHO"	|| msg.command == "WHOIS"	||
		msg.command == "NAMES"	|| msg.command == "AWAY"	||
		msg.command == "MOTD"	|| msg.command == "LIST"	||
		msg.command == "LUSERS"	|| msg.command == "USERHOST")
		return;

}

void CommandHandler::handlePASS(Server &server, Client *client, const Message &msg)
{
	const std::string serverName = server.get_name();
	const std::string nickname = client->get_nickname().empty() ? "*" : client->get_nickname();

	if (client->get_registered())
	{
		client->set_bufferOut(Replies::ERR_ALREADYREGISTERED(serverName, nickname));
		server.switchPollOut(client->get_socketFd());
		return;
	}

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

void CommandHandler::handleNICK(Server &server, Client *client, const Message &msg)
{
	const std::string serverName = server.get_name();
	const std::string currentNickname = client->get_nickname().empty() ? "*" : client->get_nickname();

	if (msg.paramsCount() < 1)
	{
		client->set_bufferOut(Replies::ERR_NONICKNAMEGIVEN(serverName, currentNickname));
		server.switchPollOut(client->get_socketFd());
		return;
	}
	const std::string newNickname = msg.param(0);

	if (isValidClientName(newNickname) == false)
	{
		client->set_bufferOut(Replies::ERR_ERRONEUSNICKNAME(serverName, currentNickname, newNickname));
		server.switchPollOut(client->get_socketFd());
		return;
	}

	if (newNickname == currentNickname)
		return;

	if (server.get_client(newNickname) != NULL)
	{
		client->set_bufferOut(Replies::ERR_NICKNAMEINUSE(serverName, currentNickname, newNickname));
		server.switchPollOut(client->get_socketFd());
		return;
	}

	if (client->get_registered())
	{
		std::string	lst_chan = client->get_channels();
		std::string	chan;
		Channel		*chan_ptr;
		size_t		pos;

		while (!lst_chan.empty())
		{
			pos = lst_chan.find(",", 0);
				chan = lst_chan.substr(0, pos);
			if (pos != std::string::npos)
				lst_chan.erase(0, pos + 1);
			else
				lst_chan.clear();
	
			chan_ptr = server.get_channel(chan);
			if (chan_ptr)
				broadcastToChannel(server, chan_ptr, Replies::NICK_CHANGE(client->get_nickname(), client->get_username(), client->get_hostname(), newNickname), NULL);
		}

	}
	client->set_nickname(newNickname);

	registerClient(server, client);
}

void CommandHandler::handleUSER(Server &server, Client *client, const Message &msg)
{
	const std::string serverName = server.get_name();
	const std::string nickname = client->get_nickname().empty() ? "*" : client->get_nickname();

	if (client->get_registered())
	{
		client->set_bufferOut(Replies::ERR_ALREADYREGISTERED(serverName, nickname));
		server.switchPollOut(client->get_socketFd());
		return;
	}

	if (msg.paramsCount() < 4)
	{
		client->set_bufferOut(Replies::ERR_NEEDMOREPARAMS(serverName, nickname, "USER"));
		server.switchPollOut(client->get_socketFd());
		return;
	}

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

void CommandHandler::handlePING(Server &server, Client *client, const Message &msg)
{
	const std::string token = msg.param(0);

	client->set_bufferOut(Replies::PONG(token));
	server.switchPollOut(client->get_socketFd());
}

void CommandHandler::handleQUIT(Server &server, Client *client, const Message &msg)
{
	const std::string reason = msg.hasTrailing ? msg.trailing : "";
	const std::string broadcastquitMsg = "Quit: " + reason;

	if (client->get_registered())
		server.removeClientFromAllChannels(client->get_socketFd(), broadcastquitMsg);

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

	client->set_bufferOut(Replies::RPL_WELCOME(name, nickname, user, host));
	client->set_bufferOut(Replies::RPL_YOURHOST(name, nickname));
	client->set_bufferOut(Replies::RPL_CREATED(name, nickname));
	client->set_bufferOut(Replies::RPL_MYINFO(name, nickname));
	server.switchPollOut(client->get_socketFd());
	std::cout << "[INFO] Client " << nickname << "!" << user << "@" << host << std::endl;
}


bool	CommandHandler::isMemberChannel(Client *c, Channel *chan)
{
	if (!chan || !c)
		return (false);
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

bool	CommandHandler::checkLimit(Channel const *chan)
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
	if (name.empty() || name.size() < 2 || name.size() > 50)
		return (false);
	if (name[0] != '#')
		return (false);
	if (name.find_first_of(" ,:\a\r\n") != std::string::npos)
		return (false);
	return (true);
}

bool	CommandHandler::isValidClientName(std::string const &nickname)
{
	std::string allowedChar = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
	allowedChar = allowedChar + "abcdefghijklmnopqrstuvwxyz";
	allowedChar = allowedChar + "0123456789";
	allowedChar = allowedChar + "-_[]\\`^{}|";

	if (nickname.empty() || nickname.size() > 9)
		return (false);
	if (nickname.find_first_not_of(allowedChar) != std::string::npos)
		return (false);
	if (nickname.find_first_of("0123456789-") == 0)
		return (false);
	return (true);
}

std::string	CommandHandler::mkModeList(Channel *chan)
{
	std::ostringstream	modeLst;
	std::string			res;

	if (chan->get_inviteOnly())
		modeLst << "i";
	if (chan->get_topicProtected())
		modeLst << "t";
	if (chan->hasKey())
		modeLst << "k";
	if (chan->get_limit())
	{
		modeLst << "l ";
		modeLst << chan->get_limit();
	}
	res = modeLst.str();
std::cout << "[DEBUG] res = " << res << std::endl;
	return (res);
}

void CommandHandler::handleJOIN(Server &server, Client *c, const Message &msg)
{
	std::string	chan;
	std::string	key;
	std::string lst_chan;
	std::string lst_key = "";
	size_t		pos;
	Channel		*chan_ptr;
	bool		no_error = false;

	const std::string serverName = server.get_name();
	const std::string nickname = c->get_nickname();

	if (!msg.params.size() || msg.params[0].empty())
	{
		c->set_bufferOut(Replies::ERR_NEEDMOREPARAMS(serverName, nickname, "JOIN")	);
		server.switchPollOut(c->get_socketFd());
		return;
	}
	lst_chan = msg.params[0];
	if (msg.params.size() > 1)
		lst_key = msg.params[1];

	if (lst_chan == "0")
	{
		Message fakePartMessage;
		fakePartMessage.prefix = msg.prefix;
		fakePartMessage.command = "PART";
		fakePartMessage.params.push_back(c->get_channels());
		if (fakePartMessage.params[0].empty())
			return;
		if (msg.params.size() > 1)
			fakePartMessage.params.push_back(msg.params[1]);
		fakePartMessage.hasTrailing = msg.hasTrailing;
		if (msg.hasTrailing)
			fakePartMessage.trailing = msg.trailing;

		CommandHandler::handlePART(server, c, fakePartMessage);
		return;
	}

	while (!lst_chan.empty())
	{
		pos = lst_chan.find(",", 0);
			chan = lst_chan.substr(0, pos);
		if (pos != std::string::npos)
			lst_chan.erase(0, pos + 1);
		else
			lst_chan.clear();
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

		chan_ptr = server.get_channel(chan);
		if (chan_ptr && !isMemberChannel(c, chan_ptr))
		{
			if (!checkLimit(chan_ptr))
			{
				c->set_bufferOut(Replies::ERR_CHANNELISFULL(serverName, nickname, chan));
				server.switchPollOut(c->get_socketFd());
			}
			else if (chan_ptr->get_inviteOnly() && !chan_ptr->isInvited(c))
			{
				c->set_bufferOut(Replies::ERR_INVITEONLYCHAN(serverName, nickname, chan));
				server.switchPollOut(c->get_socketFd());
			}
			else if (!checkChannelKey(chan_ptr, key))
			{
				c->set_bufferOut(Replies::ERR_BADCHANNELKEY(serverName, nickname, chan));
				server.switchPollOut(c->get_socketFd());
			}
			else
				no_error = true;
		}
		else if (chan_ptr == NULL)
		{
			if (!isValidChannelName(chan))
				{
					c->set_bufferOut(Replies::ERR_NOSUCHCHANNEL(serverName, nickname, chan));
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
			chan_ptr->addMember(c);
			chan_ptr->removeInvite(c);
			c->addChannel(chan_ptr);

			broadcastToChannel(server, chan_ptr, Replies::JOIN_MSG(c->get_nickname(), c->get_username(), c->get_hostname(), chan), NULL);

			if (chan_ptr->get_topic().empty())
				c->set_bufferOut(Replies::RPL_NOTOPIC(serverName, nickname, chan));
			else
				c->set_bufferOut(Replies::RPL_TOPIC(serverName, nickname, chan, chan_ptr->get_topic()));
			server.switchPollOut(c->get_socketFd());

			namesReply(server, c, chan_ptr);
			no_error = false;
		}
	}

	return;
}

void	CommandHandler::handlePRIVMSG(Server &server, Client *c, const Message &msg)
{
	int								t = 0;
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
		c->set_bufferOut(Replies::ERR_NORECIPIENT(serverName, nickname, "PRIVMSG"));
		server.switchPollOut(c->get_socketFd());
		return;
	}
	if (msg.params.size() < 2 || msg.params[1].empty())
	{
		c->set_bufferOut(Replies::ERR_NOTEXTTOSEND(serverName, nickname));
		server.switchPollOut(c->get_socketFd());
		return;
	}

	lst_target = msg.params[0];
	while (!lst_target.empty() && pos != std::string::npos)
	{
		pos = lst_target.find(",", 0);
		target = lst_target.substr(0, pos);
		if (pos != std::string::npos)
			lst_target.erase(0, pos + 1);
		else
			lst_target.clear();
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
					c->set_bufferOut(Replies::ERR_NOSUCHCHANNEL(serverName, nickname, target));
					server.switchPollOut(c->get_socketFd());
					break;
				}
				if (!isMemberChannel(c, chan_target))
				{
					c->set_bufferOut(Replies::ERR_CANNOTSENDTOCHAN(serverName, nickname, target));
					server.switchPollOut(c->get_socketFd());
					break;
				}
				old_targets.insert(Server::lowerName(target));
				chan_members = chan_target->get_members();
				it = chan_members.begin();
				broadcastToChannel(server, chan_target, Replies::PRIVMSG_MSG(nickname, c->get_username(), c->get_hostname(), target, msg.trailing), c);
				break;
			case 2:
				user_target = checkClientExists(server, target);
				if (user_target == NULL)
				{
					c->set_bufferOut(Replies::ERR_NOSUCHNICK(serverName, nickname, target));
					server.switchPollOut(c->get_socketFd());
					break;
				}
				user_target->set_bufferOut(Replies::PRIVMSG_MSG(nickname, c->get_username(), c->get_hostname(), target, msg.trailing));
				server.switchPollOut(user_target->get_socketFd());
				old_targets.insert(Server::lowerName(target));
				break;
			case 3:
				break;
			default:
				c->set_bufferOut(Replies::ERR_NOSUCHNICK(serverName, nickname, target));
				server.switchPollOut(c->get_socketFd());
				break;
			}
		t = 0;
	}
	return;
}

void	CommandHandler::handleKICK(Server &server, Client *c, const Message &msg)
{
	std::string	reason = "has been kicked from channel";
	Channel		*chan;
	size_t		pos = 0;
	Client		*user_target;
	std::string	lst_members;
	std::string	target;
	std::string	chan_name;

	const std::string serverName = server.get_name();
	const std::string nickname = c->get_nickname();

	if (msg.params.empty() || msg.params.size() < 2)
	{
		c->set_bufferOut(Replies::ERR_NEEDMOREPARAMS(serverName, nickname, "KICK"));
		server.switchPollOut(c->get_socketFd());
		return;
	}

	chan_name = msg.params[0];
	lst_members = msg.params[1];
	if (msg.params.size() > 2)
		reason = msg.params[2];
	chan = server.get_channel(chan_name);
	if (!chan)
	{
		c->set_bufferOut(Replies::ERR_NOSUCHCHANNEL(serverName, nickname, chan_name));
		server.switchPollOut(c->get_socketFd());
		return;
	}
	while (!lst_members.empty() && pos != std::string::npos)
	{
		pos = lst_members.find(",", 0);
		target = lst_members.substr(0, pos);
		if (pos != std::string::npos)
			lst_members.erase(0, pos + 1);
		else
			lst_members.clear();
		user_target = checkClientExists(server, target);
		if (!isMemberChannel(c, chan))
		{
			c->set_bufferOut(Replies::ERR_NOTONCHANNEL(serverName, nickname, chan_name));
			server.switchPollOut(c->get_socketFd());
		}
		else if (!chan->isOperator(c))
		{
			c->set_bufferOut(Replies::ERR_CHANOPRIVSNEEDED(serverName, nickname, chan_name));
			server.switchPollOut(c->get_socketFd());
		}
		else if (!user_target || !user_target->get_registered() || !isMemberChannel(user_target, chan))
		{
			c->set_bufferOut(Replies::ERR_USERNOTINCHANNEL(serverName, nickname, target, chan_name));
			server.switchPollOut(c->get_socketFd());
		}
		else
		{
			broadcastToChannel(server, chan, Replies::KICK_MSG(nickname, c->get_username(), c->get_hostname(), chan_name, target, reason), NULL);
			server.removeClientFromChannel(user_target, chan);
		}
	}
	return;
}

void	CommandHandler::handleINVITE(Server &server, Client *c, const Message &msg)
{
	Channel		*chan;
	Client		*invited_guy;

	const std::string serverName = server.get_name();
	const std::string nickname = c->get_nickname();

	if (msg.params.empty() || msg.params.size() < 2)
	{
		c->set_bufferOut(Replies::ERR_NEEDMOREPARAMS(serverName, nickname, "INVITE"));
		server.switchPollOut(c->get_socketFd());
		return;
	}

	const std::string targetNick = msg.params[0];
	const std::string chanName = msg.params[1];

	invited_guy = checkClientExists(server, msg.params[0]);
	chan = server.get_channel(msg.params[1]);

	if (chan == NULL)
	{
		c->set_bufferOut(Replies::ERR_NOSUCHCHANNEL(serverName, nickname, chanName));
		server.switchPollOut(c->get_socketFd());
	}
	else if (!isMemberChannel(c, chan))
	{
		c->set_bufferOut(Replies::ERR_NOTONCHANNEL(serverName, nickname, chanName));
		server.switchPollOut(c->get_socketFd());
	}
	else if (chan->get_inviteOnly() && !chan->isOperator(c))
	{
		c->set_bufferOut(Replies::ERR_CHANOPRIVSNEEDED(serverName, nickname, chanName));
		server.switchPollOut(c->get_socketFd());
	}
	else if (invited_guy == NULL || !invited_guy->get_registered())
	{
		c->set_bufferOut(Replies::ERR_NOSUCHNICK(serverName, nickname, targetNick));
		server.switchPollOut(c->get_socketFd());
	}
	else if (isMemberChannel(invited_guy, chan))
	{
		c->set_bufferOut(Replies::ERR_USERONCHANNEL(serverName, nickname, targetNick, chanName));
		server.switchPollOut(c->get_socketFd());
	}
	else
	{
		chan->addInvite(invited_guy);
		c->set_bufferOut(Replies::RPL_INVITING(serverName, nickname, targetNick, chanName));
		server.switchPollOut(c->get_socketFd());
		invited_guy->set_bufferOut(Replies::INVITE_MSG(nickname, c->get_username(), c->get_hostname(), targetNick, chanName));
		server.switchPollOut(invited_guy->get_socketFd());
	}
	return;
}

void	CommandHandler::handleTOPIC(Server &server, Client *c, const Message &msg)
{
	Channel		*chan;
	std::string	topic;

	const std::string serverName = server.get_name();
	const std::string nickname = c->get_nickname();

	if (msg.params.empty() || msg.params[0].empty())
	{
		c->set_bufferOut(Replies::ERR_NEEDMOREPARAMS(serverName, nickname, "TOPIC"));
		server.switchPollOut(c->get_socketFd());
		return;
	}

	const std::string chanName = msg.params[0];
	chan = server.get_channel(msg.params[0]);

	if (chan == NULL)
	{
		c->set_bufferOut(Replies::ERR_NOSUCHCHANNEL(serverName, nickname, chanName));
		server.switchPollOut(c->get_socketFd());
	}
	else if (!isMemberChannel(c, chan))
	{
		c->set_bufferOut(Replies::ERR_NOTONCHANNEL(serverName, nickname, chanName));
		server.switchPollOut(c->get_socketFd());
	}
	else if (msg.params.size() == 1 && !msg.hasTrailing)
	{
		topic = chan->get_topic();
		if (topic.empty())
			c->set_bufferOut(Replies::RPL_NOTOPIC(serverName, nickname, chanName));
		else
			c->set_bufferOut(Replies::RPL_TOPIC(serverName, nickname, chanName, topic));
		server.switchPollOut(c->get_socketFd());
	}
	else if (chan->get_topicProtected() && !chan->isOperator(c))
	{
		c->set_bufferOut(Replies::ERR_CHANOPRIVSNEEDED(serverName, nickname, chanName));
		server.switchPollOut(c->get_socketFd());
	}
	else
	{
		chan->set_topic(msg.params[1], c);
		broadcastToChannel(server, chan, Replies::TOPIC_MSG(nickname, c->get_username(), c->get_hostname(), chanName, msg.trailing), NULL);
	}
	return;
}

void	CommandHandler::handlePART(Server &server, Client *c, const Message &msg)
{
	size_t		pos;
	Channel		*chan_ptr;
	std::string lst_chan;
	std::string	chan;
	std::string	reason = c->get_nickname();

	const std::string serverName = server.get_name();
	const std::string nickname = c->get_nickname();

	if (msg.params.empty() || msg.params[0].empty())
	{
		c->set_bufferOut(Replies::ERR_NEEDMOREPARAMS(serverName, nickname, "PART"));
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
			c->set_bufferOut(Replies::ERR_NOSUCHCHANNEL(serverName, nickname, chan));
			server.switchPollOut(c->get_socketFd());
		}
		else if (!isMemberChannel(c, chan_ptr))
		{
			c->set_bufferOut(Replies::ERR_NOTONCHANNEL(serverName, nickname, chan));
			server.switchPollOut(c->get_socketFd());
		}
		else
		{
			broadcastToChannel(server, chan_ptr, Replies::PART_MSG(nickname, c->get_username(), c->get_hostname(), chan, reason), NULL);
			server.removeClientFromChannel(c, chan_ptr);
		}
	}
	return;
}

void	CommandHandler::namesReply(Server &server, Client *c, Channel *chan)
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

	c->set_bufferOut(Replies::RPL_NAMREPLY(server.get_name(), c->get_nickname(), chan->get_name(), lst_names));
	c->set_bufferOut(Replies::RPL_ENDOFNAMES(server.get_name(), c->get_nickname(), chan->get_name()));
	server.switchPollOut(c->get_socketFd());
	return;
}

void	CommandHandler::handleMODE(Server &server, Client *c, const Message &msg)
{
	Channel		*chan_ptr;
	Client		*target_user;
	std::string	modestring;

	const std::string serverName = server.get_name();
	const std::string nickname = c->get_nickname();
	const std::string chanName = msg.params[0];


	if (msg.params.empty() || msg.params[0].empty())
	{
		c->set_bufferOut(Replies::ERR_NEEDMOREPARAMS(serverName, nickname, "MODE"));
		server.switchPollOut(c->get_socketFd());
		return;
	}
	if (isValidClientName(msg.params[0]))
	{
		return;
	}
	chan_ptr = server.get_channel(msg.params[0]);
	if (!chan_ptr)
	{
		c->set_bufferOut(Replies::ERR_NOSUCHCHANNEL(serverName, nickname, chanName));
		server.switchPollOut(c->get_socketFd());
		return;
	}
	if (msg.params.size() < 2)
	{
		c->set_bufferOut(Replies::RPL_CHANNELMODEIS(serverName, nickname, chanName, mkModeList(chan_ptr)));
		server.switchPollOut(c->get_socketFd());
		return;
	}
	if (!chan_ptr->isOperator(c))
	{
		c->set_bufferOut(Replies::ERR_CHANOPRIVSNEEDED(serverName, nickname, chanName));
		server.switchPollOut(c->get_socketFd());
		return;
	}
	modestring = msg.params[1];
	if (modestring.size() != 2 || modestring.find_first_not_of("+-itlok") != std::string::npos
		|| (modestring[0] != '+' && modestring[0] != '-')
		|| modestring.find_first_of("itlok") == std::string::npos)
	{
		c->set_bufferOut(Replies::ERR_UNKNOWNMODE(serverName, nickname, modestring[0]));
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
		case 0:
			if (chan_ptr->get_inviteOnly())
				return;
			chan_ptr->set_inviteOnly(true, c);
			broadcastToChannel(server, chan_ptr, Replies::MODE_MSG(nickname, c->get_username(), c->get_hostname(), chanName, modestring), NULL);
			break;
		case 1:
			if (!chan_ptr->get_inviteOnly())
				return;
			chan_ptr->set_inviteOnly(false, c);
			broadcastToChannel(server, chan_ptr, Replies::MODE_MSG(nickname, c->get_username(), c->get_hostname(), chanName, modestring), NULL);
			break;
		case 2:
			if (chan_ptr->get_topicProtected())
				return;
			chan_ptr->set_topicProtected(true, c);
			broadcastToChannel(server, chan_ptr, Replies::MODE_MSG(nickname, c->get_username(), c->get_hostname(), chanName, modestring), NULL);
			break;
		case 3:
			if (!chan_ptr->get_topicProtected())
				return;
			chan_ptr->set_topicProtected(false, c);
			broadcastToChannel(server, chan_ptr, Replies::MODE_MSG(nickname, c->get_username(), c->get_hostname(), chanName, modestring), NULL);
			break;
		case 4:
			if (msg.params.size() < 3 || msg.params[2].empty())
				return;
			extract_nb << msg.params[2];
			extract_nb >> nb_l;
			if (extract_nb.fail() || !extract_nb.eof() || nb_l == 0 || msg.params[2].find("-") != std::string::npos)
			{
				c->set_bufferOut(Replies::ERR_INVALIDMODEPARAM(serverName, nickname, chanName, 'l', msg.params[2]));
				server.switchPollOut(c->get_socketFd());
				return;
			}
			if (chan_ptr->get_limit() == nb_l)
				return;
			chan_ptr->set_limit(nb_l, c);
				broadcastToChannel(server, chan_ptr, Replies::MODE_MSG(nickname, c->get_username(), c->get_hostname(), chanName, modestring), NULL);
			break;
		case 5:
			if (chan_ptr->get_limit() == 0)
				return;
			chan_ptr->set_limit(0, c);
			broadcastToChannel(server, chan_ptr, Replies::MODE_MSG(nickname, c->get_username(), c->get_hostname(), chanName, modestring), NULL);
			break;
		case 6:
			if (msg.params.size() < 3 || msg.params[2].empty())
				return;
			target_user = checkClientExists(server, msg.params[2]);
			if (!target_user || !isMemberChannel(target_user, chan_ptr))
			{
				c->set_bufferOut(Replies::ERR_USERNOTINCHANNEL(serverName, nickname, msg.params[2], chanName));
				server.switchPollOut(c->get_socketFd());
				return;
			}
			if (!chan_ptr->isOperator(target_user))
			{
				chan_ptr->addOperator(target_user);
				broadcastToChannel(server, chan_ptr, Replies::MODE_MSG(nickname, c->get_username(), c->get_hostname(), chanName, modestring), NULL);
			}
			break;
		case 7:
			if (msg.params.size() < 3 || msg.params[2].empty())
				return;
			target_user = checkClientExists(server, msg.params[2]);
			if (!target_user || !isMemberChannel(target_user, chan_ptr))
			{
				c->set_bufferOut(Replies::ERR_USERNOTINCHANNEL(serverName, nickname, msg.params[2], chanName));
				server.switchPollOut(c->get_socketFd());
				return;
			}
			if (chan_ptr->isOperator(target_user))
			{
				chan_ptr->removeOperator(target_user);
				broadcastToChannel(server, chan_ptr, Replies::MODE_MSG(nickname, c->get_username(), c->get_hostname(), chanName, modestring), NULL);
			}
			break;
		case 8:
			if (msg.params.size() < 3 || msg.params[2].empty())
				return;
			if (chan_ptr->hasKey())
			{
				c->set_bufferOut(Replies::ERR_KEYSET(serverName, nickname, chanName));
				server.switchPollOut(c->get_socketFd());
				return;
			}
			if (!chan_ptr->isValidKey(msg.params[2]))
			{
				c->set_bufferOut(Replies::ERR_INVALIDMODEPARAM(serverName, nickname, chanName, 'k', msg.params[2]));
				server.switchPollOut(c->get_socketFd());
				return;
			}
			chan_ptr->set_key(msg.params[2], c);
			broadcastToChannel(server, chan_ptr, Replies::MODE_MSG(nickname, c->get_username(), c->get_hostname(), chanName, modestring), NULL);
			break;
		case 9:
			if (!chan_ptr->hasKey())
				return;
			chan_ptr->set_key("", c);
			broadcastToChannel(server, chan_ptr, Replies::MODE_MSG(nickname, c->get_username(), c->get_hostname(), chanName, modestring), NULL);
			break;
		default:
			c->set_bufferOut(Replies::ERR_UNKNOWNMODE(serverName, nickname, modestring[0]));
			server.switchPollOut(c->get_socketFd());
			break;
	}
	return;
}
