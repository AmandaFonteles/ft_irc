/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aibonade <aibonade@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/28 14:55:46 by aibonade          #+#    #+#             */
/*   Updated: 2026/05/08 19:06:11 by aibonade         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

# include "../includes/Client.hpp"

/********CONSTRUCTORS & DESTRUCTOR********/
Client::Client():_socketFd(-1), _passOk(false), _nickname(""), _username(""), _registered(false), _bufferIn(""), _bufferOut("")
{
	return;
}

Client::Client(Client const &cpy):_socketFd(cpy._socketFd), _passOk(cpy._passOk), _nickname(cpy._nickname), _username(cpy._username), _registered(cpy._registered), _channels(cpy._channels), _bufferIn(cpy._bufferIn), _bufferOut(cpy._bufferOut)
{
	return;
}

Client::Client(int socketFd):_socketFd(socketFd), _passOk(false), _nickname(""), _username(""), _registered(false), _bufferIn(""), _bufferOut("")
{
	return;
}

Client::~Client()
{
	return;
}

/********SETTERS********/
void	Client::set_nickname(std::string nickname)//TO DO
{
	//quels checks ? //voir dans le serveur si d'autres clients ont le meme nickname ? du coup ca peut valoir le coup de faire plutot une map<uname/nickname, Client> que <int, Client> ?
	_nickname = nickname;
	return;//exception ? Booleen ? 
}

void	Client::set_username(std::string username)
{
	// if (_username.empty() && !username.empty())
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
	_bufferOut = _bufferOut + str;//separateur a ajouter ? ou il est debase dans la str ? ou yen n'a pas ?
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

// std::set<Channel>	Client::get_channels();//get ou print ? ou string ? //TO DO
// Channel	Client::get_channel(int i);//ou nom du chanel ? //TO DO

std::string	&Client::get_bufferIn()
{
	return (_bufferIn);
}

std::string	&Client::get_bufferOut()
{
	return (_bufferOut);
}

/********OTHER METHODS********/
// Client	&Client::operator=(Client const &to_affect)//est-ce qu'on copie les buffers, passOK et channels et fd ? //TO DO
// {
// 	if (this != &to_affect)
// 	{
// 		//en fait pour moi ca n'a pas vraiment de sens de faire un client =, si ? Ya peut-etre un truc que j'ai mal compris...
// 	}
// 	return (*this);
// }

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

void	Client::removeAllChannel()//penser a d'abord appeler removeMember autant que necessaire dans le handdler !
{
	std::set<Channel *>::iterator	it = _channels.begin();

	while (it != _channels.end())
	{
		this->removeChannel(*it);
		it++;
	}
	return;
}

bool	Client::isInChannel(Channel *chan) const
{
	if (_channels.find(chan) != _channels.end())
		return (true);
	return (false);
}