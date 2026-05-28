/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: afontele <afontele@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/28 14:55:46 by aibonade          #+#    #+#             */
/*   Updated: 2026/05/28 22:59:30 by afontele         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

# include "../includes/Client.hpp"

/********CONSTRUCTORS & DESTRUCTOR********/
Client::Client():_socketFd(-1), _passOk(false), _nickname(""), _username(""), _registered(false), _bufferIn(""), _bufferOut(""), _hostname("localhost"), _shouldClose(false)
{
	return;
}

Client::Client(Client const &cpy):_socketFd(cpy._socketFd), _passOk(cpy._passOk), _nickname(cpy._nickname), _username(cpy._username), _registered(cpy._registered), _channels(cpy._channels), _bufferIn(cpy._bufferIn), _bufferOut(cpy._bufferOut), _hostname(cpy._hostname), _shouldClose(cpy._shouldClose)
{
	return;
}

Client::Client(int socketFd):_socketFd(socketFd), _passOk(false), _nickname(""), _username(""), _registered(false), _bufferIn(""), _bufferOut(""), _hostname("localhost"), _shouldClose(false)
{
	return;
}

Client::~Client()
{
	return;
}

/********SETTERS********/
void	Client::set_nickname(std::string nickname)
{
	_nickname = nickname;
	return;
}

void	Client::set_username(std::string username)
{
	if (_username.empty() && !username.empty())
		_username = username;
	return;
}

void	Client::set_passOk(bool value)
{
	_passOk = value;
	return;
}

void	Client::set_registered(bool value)
{
	_registered = value;
}

void	Client::set_bufferIn(std::string str)
{
	_bufferIn = _bufferIn + str;
	return;
}

void	Client::set_bufferOut(std::string str)
{
	_bufferOut = _bufferOut + str;
	return;
}

/********GETTERS********/
int	Client::get_socketFd() const
{
	return (_socketFd);
}

std::string	Client::get_nickname() const
{
	return (_nickname);
}

std::string	Client::get_username() const
{
	return (_username);
}

bool	Client::get_passOk() const
{
	return (_passOk);
}

bool	Client::get_registered() const
{
	return (_registered);
}

std::string	&Client::get_bufferIn()
{
	return (_bufferIn);
}

std::string	&Client::get_bufferOut()
{
	return (_bufferOut);
}

std::string			Client::get_channels() const
{
	std::string 					s = "";

	if (!_channels.empty())
	{
		std::set<Channel *>::iterator	it = _channels.begin();
		std::set<Channel *>::iterator	ite = _channels.end();
		
		s += (*it)->get_name();
		it++;
		while (it != ite)
		{
			s += "," + (*it)->get_name();
			it++;
		}
	}
	return s;
}

bool	Client::addChannel(Channel *chan)
{
	std::pair<std::set<Channel *>::iterator, bool>	ret;

	if (chan->isMember(this))
	{
		ret = this->_channels.insert(chan);
		return (ret.second);
	}
	return (false);
}

bool	Client::removeChannel(Channel *chan)
{
	std::set<Channel *>::iterator	it;

	if (chan->isMember(this))
		return (false);
	it = _channels.find(chan);
	if (it == _channels.end())
		return (false);
	_channels.erase(it);
	return (true);
}

void	Client::removeAllChannel()
{
	std::set<Channel *>::iterator	it = _channels.begin();
	Channel	*chan;

	while (it != _channels.end())
	{
		chan = *it;
		this->removeChannel(chan); //++);
		it++; //erase
	}
	return;
}

bool	Client::isInChannel(Channel *chan) const
{
	if (_channels.find(chan) != _channels.end())
		return (true);
	return (false);
}

void	Client::set_hostname(std::string hostname)
{
	_hostname = hostname;
}
std::string	Client::get_hostname()
{
	return (_hostname);
}

void	Client::set_shouldClose(bool value)
{
	_shouldClose = value;
}
bool	Client::get_shouldClose() const
{
	return (_shouldClose);
}

void	Client::set_realname(std::string realname)
{
	_realname = realname;
}
std::string	Client::get_realname() const
{
	return (_realname);
}
