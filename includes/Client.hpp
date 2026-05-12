/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aibonade <aibonade@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/28 12:55:34 by aibonade          #+#    #+#             */
/*   Updated: 2026/05/12 16:29:03 by aibonade         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CLIENT_HPP
# define CLIENT_HPP
# include <string>
# include <set>
# include <algorithm>
# include "Channel.hpp"

class Client
{
private:
//data
	int					_socketFd;
	bool				_passOk;//passe a true si le client utilise PASS avec le mot de passe donne au lancement du serveur
	std::string			_nickname;
	std::string			_username;
	bool				_registered;//passe a true quand le client a passe toute la phase d'enregistrement
	std::set<Channel *>	_channels;// <const> ? et surtout strings => on utiliserait la map du serveur pour retrouver le bon serveur ? 
	std::string			_bufferIn;//besoin de plusieurs ? //public?
	std::string			_bufferOut;//public ?
//methods 
						Client();
	Client				&operator=(Client const &to_affect);


public:
//constructor/destructor
						Client(Client const &cpy);
						Client(int socketFd);
						~Client();

//setters
//besoin d'un set_fd ?
	void				set_bufferIn(std::string str);
	void				set_nickname(std::string nickname);
	void				set_username(std::string username);
	void				set_passOk(bool value);
	void				set_registered(bool value);
	void				set_bufferOut(std::string str);

//getters
	int					get_socketFd() const;
	std::string			&get_bufferIn();
	std::string			get_nickname() const;
	std::string			get_username() const;
	bool				get_passOk() const;
	bool				get_registered() const;
	// std::set<Channel *>	get_channels();//get ou print ? ou string ?
	std::string			&get_bufferOut(); 

//other methods
	bool				addChannel(Channel *chan);
	bool				removeChannel(Channel *chan);
	bool				isInChannel(Channel *chan) const;
	void				removeAllChannel();
};

#endif
