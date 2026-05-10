/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server_channel.cpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aibonade <aibonade@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/09 10:40:24 by aibonade          #+#    #+#             */
/*   Updated: 2026/05/10 17:09:25 by aibonade         ###   ########.fr       */
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
	return (_channels[name]);
}

void	Server::deleteChannel(Channel *chan)
{
	_channels.erase(chan->get_name());
	if (chan)
		delete chan;
	return;
}

Client	*Server::get_client(std::string const nickname)
{
	std::map<int, Client *>::iterator it;

	it = _clients.begin();
	while (it != _clients.end())
	{
		if (it->second->get_nickname() == nickname)
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
		str[i] = std::toupper(str[i]);
		i++;
	}
std::cout << "[DEBUG] name (" << name << ") normalized = " << str << std::endl;//On l'enlevera en temps voulu, laisse la collee au bord :) J'aimerais checker le # notamment 
	return (str);
}