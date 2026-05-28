/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server_channel.cpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: afontele <afontele@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/09 10:40:24 by aibonade          #+#    #+#             */
/*   Updated: 2026/05/28 23:00:18 by afontele         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/Server.hpp"

Channel	*Server::get_channel(std::string const name)
{
	std::map<std::string, Channel *>::iterator it;

	it = _channels.find(lowerName(name));
	if (it == _channels.end())
		return (NULL);
	return (it->second);
}

Channel	*Server::createChannel(std::string const name)
{
	if (get_channel(name))
		return (NULL);

	_channels[lowerName(name)] = new Channel(name);
	return (_channels[lowerName(name)]);
}

void	Server::deleteChannel(Channel *chan)
{
	_channels.erase(lowerName(chan->get_name()));
	if (chan)
		delete chan;
	return;
}

void	Server::removeClientFromChannel(Client *c, Channel *chan)
{
	chan->removeOperator(c);
	chan->removeMember(c);
	chan->removeInvite(c);
	c->removeChannel(chan);
	if (chan->nbMembers() == 0)
		deleteChannel(chan);
}

Client	*Server::get_client(std::string const nickname)
{
	std::map<int, Client *>::iterator	it;
	std::string							nickname_lower = Server::lowerName(nickname);

	it = _clients.begin();
	while (it != _clients.end())
	{
		if (Server::lowerName(it->second->get_nickname()) == nickname_lower)
			return (it->second);
		it++;
	}
	return (NULL);
}

std::string	Server::lowerName(std::string const name)
{
	int			i;
	std::string	str = name;

	i = 0;
	while (str[i])
	{
		str[i] = static_cast<char>(std::tolower(static_cast<unsigned char>(str[i])));
		i++;
	}
	return (str);
}

//////// GETTERS ////
std::string	Server::get_name() const
{
	return ("ft_irc");
}

std::string	Server::get_password() const
{
	return (_password);
}
