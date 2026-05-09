/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server_channel.cpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aibonade <aibonade@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/09 10:40:24 by aibonade          #+#    #+#             */
/*   Updated: 2026/05/09 11:13:03 by aibonade         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/Server.hpp"


Channel	*Server::get_Channel(std::string const name)
{
	std::map<std::string, Channel *>::iterator it;

	it = _channels.find(name);
	if (it == _channels.end())
		return (NULL);
	return (it->second);
}

Channel	*Server::createChannel(std::string const name)//retourne NULL si erreur ?
{
	if (get_Channel(name))//il existe deja
		return (NULL);

	//je cree le channel avec new
	//je le mets dans _channels avec name
	_channels[name] = new Channel(name);
	//je renvoie le pointeur
	return (_channels[name]);
}

void	Server::deleteChannel(Channel *chan)
{
	if (chan)
		delete chan;
	return;
}
// Client	*Server::get_Client(std::string const nickname);//retourne NULL si pas trouve
