/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aibonade <aibonade@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/28 14:55:46 by aibonade          #+#    #+#             */
/*   Updated: 2026/04/29 13:11:39 by aibonade         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

# include "../includes/Client.hpp"


/********CONSTRUCTORS & DESTRUCTOR********/
Client::Client():_socketFd(-1), _passOk(false), _registred(false)//pour moi on pourrait presque le virer, sinon on met le fd a cb ?
{
	return;
}

Client::Client(Client const &cpy):_socketFd(cpy._socketFd), _nickname(cpy._nickname), _username(cpy._username), _passOk(cpy._passOk), _registred(cpy._registred), _channels(cpy._channels), _bufferIn(cpy._bufferIn), _bufferOut(cpy._bufferOut)
{
	return;
}

Client::Client(int socketFd):_socketFd(socketFd), _passOk(false), _registred(false)
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
	if (_username.empty() && !username.empty())
		_username = username;
	return;//exception ? Booleen ?
}

void	Client::set_passOk(bool value)
{
	_passOk = value;
	return;
}

void	Client::set_registred(bool value)
{
	_registred = value;
}

void	Client::set_bufferIn(std::string str)
{
	_bufferIn = _bufferIn + str;//separateur a ajouter ? ou il est debase dans la str ? ou yen n'a pas ?
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

bool	Client::get_registred() const
{
	return (_registred);
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

// void	Client::addChannel(Channel &chan);//bool/exception ?//TO DO
// void	Client::removeChannel(Channel const &chan);//TO DO

bool	Client::isInChannel(Channel &chan)
{
	if (_channels.find(chan) != _channels.end())
		return (true);
	return (false);
}