/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server_channel.cpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: afontele <afontele@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/09 10:40:24 by aibonade          #+#    #+#             */
/*   Updated: 2026/05/28 14:52:38 by afontele         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/Server.hpp"

Channel	*Server::get_channel(std::string const name)
{
	std::map<std::string, Channel *>::iterator it;

	it = _channels.find(lowerName(name));// On checke que de toutes facons name est au bon format avec lowerName
	if (it == _channels.end())
		return (NULL);
	return (it->second);
}

Channel	*Server::createChannel(std::string const name)
{
	if (get_channel(name))//il existe deja ?
		return (NULL);

	//je cree le channel avec new
	//je le mets dans _channels avec name en minuscule
	_channels[lowerName(name)] = new Channel(name);
	//je renvoie le pointeur
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
std::cout << "[DEBUG] nickname (" << nickname << ") normalized = " << nickname_lower << std::endl;
	while (it != _clients.end())
	{
std::cout << "[DEBUG] 1" << std::endl;
		if (Server::lowerName(it->second->get_nickname()) == nickname_lower)
			return (it->second);
std::cout << "[DEBUG] 2" << std::endl;
		it++;
	}
std::cout << "[DEBUG] pouet" << std::endl;
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
std::cout << "[DEBUG] name (" << name << ") normalized = " << str << std::endl;//On l'enlevera en temps voulu, laisse la collee au bord :) J'aimerais checker le # notamment
	return (str);
}

//////// NAYEL GETTERS ////
std::string	Server::get_name() const
{
	return ("ft_irc");
}

std::string	Server::get_password() const
{
	return (_password);
}
