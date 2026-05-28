/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: afontele <afontele@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/28 12:55:34 by aibonade          #+#    #+#             */
/*   Updated: 2026/05/28 22:12:54 by afontele         ###   ########.fr       */
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
	bool				_passOk;
	std::string			_nickname;
	std::string			_username;
	bool				_registered;
	std::set<Channel *>	_channels;
	std::string			_bufferIn;
	std::string			_bufferOut;

//methods
	Client();
	Client				&operator=(Client const &to_affect);
	std::string			_hostname;
	bool				_shouldClose;
	std::string			_realname;

public:
//constructor/destructor
						Client(Client const &cpy);
						Client(int socketFd);
						~Client();

//setters
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
	std::string			get_channels() const;
	std::string			&get_bufferOut();

//other methods
	bool				addChannel(Channel *chan);
	bool				removeChannel(Channel *chan);
	bool				isInChannel(Channel *chan) const;
	void				removeAllChannel();
	void				set_hostname(std::string hostname);
	std::string			get_hostname();
	void				set_shouldClose(bool value);
	bool				get_shouldClose() const;
	void				set_realname(std::string realname);
	std::string			get_realname() const;
};

#endif
