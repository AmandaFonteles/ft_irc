/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aibonade <aibonade@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/28 12:55:34 by aibonade          #+#    #+#             */
/*   Updated: 2026/05/01 15:43:34 by aibonade         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CLIENT_HPP
# define CLIENT_HPP
# include <string>
# include <set>
# include "Channel.hpp"

class Client
{
private:
//data
	int					_socketFd;
	bool				_passOk;//passe a true si le client utilise PASS avec le mot de passe donne au lancement du serveur=>ou est stocke ce mdp ?
	std::string			_nickname;
	std::string			_username;//setter a proteger on ne peut le modifier qu'au debut
	bool				_registered;//passe a true quand le client a passe toute la phase d'enregistrement
	std::set<Channel &>	_channels;// <const> ? et surtout strings => on utiliserait la map du serveur pour retrouver le bon serveur ?  
	std::string			_bufferIn;//besoin de plusieurs ? //public?
	std::string			_bufferOut;//queue ? //public ?
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
	void				set_bufferIn(std::string str);//+str ?
	void				set_nickname(std::string nickname);
	void				set_username(std::string username);
	void				set_passOk(bool value);
	void				set_registred(bool value);
	void				set_bufferOut(std::string str);//+str ..ajouter un separateur ? Ou il existe deja ?

//getters
	int					get_socketFd() const;
	std::string			&get_bufferIn();
	std::string			get_nickname() const;
	std::string			get_username() const;
	bool				get_passOk() const;
	bool				get_registred() const;
	std::set<Channel>	get_channels();//get ou print ? ou string ?
	// Channel				get_channel(int i);//ou nom du chanel ?
	std::string			&get_bufferOut(); 

//other methods
	void				addChannel(Channel &chan);//bool/exception ?
	void				removeChannel(Channel const &chan);
	bool				isInChannel(Channel &chan);
};


#endif
