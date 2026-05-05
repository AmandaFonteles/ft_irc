/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: afontele <afontele@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/28 14:55:46 by aibonade          #+#    #+#             */
/*   Updated: 2026/05/05 18:00:35 by afontele         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

# include "../includes/Client.hpp"


/********CONSTRUCTORS & DESTRUCTOR********/
Client::Client():_socketFd(-1), _passOk(false), _registered(false)//pour moi on pourrait presque le virer, sinon on met le fd a cb ?
{
	return;
}

Client::Client(Client const &cpy):_socketFd(cpy._socketFd), _nickname(cpy._nickname), _username(cpy._username), _passOk(cpy._passOk), _registered(cpy._registered), _channels(cpy._channels), _bufferIn(cpy._bufferIn), _bufferOut(cpy._bufferOut)
{
	return;
}

Client::Client(int socketFd):_socketFd(socketFd), _passOk(false), _registered(false)
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


bool	Client::addChannel(Channel *chan)//bool/exception ?//TO DO //chan existe bien => map de server ? mais ce sera avant d'appeler cette fonction qu'on le saura ça
{
	//chan != NULL
	//Essayer d'ajouter le client dans le chan avec addClient (=> dedans on le vire aussi de Invite si il est invite et on le met dans les operators s'il est le premier a rejoindre le chan)
		//si true on met le chan dans le client
		//si false on abort le truc
}
void	Client::removeChannel(Channel *chan)//remove le client aussi dans le channel//TO DO
{
	//chan !NULL
	//Verifier que le client est bien dans le chan (ici)
	//Essayer de remove le client dans chan avec removeMember (=> dedans on le vire aussi dans operators et surtout on check si le client est bien membre du chan labas et si plus de membres on suppr le chan)
		//si true on vire le chan dans le client
		//si false on abort le truc
}

void	Client::removeAllChannel()//appelle removeChannel pour chaque Channel du client//TO DO
{
	//appeler removeChannel un par un
}

bool	Client::isInChannel(Channel *chan) const
{
	if (_channels.find(chan) != _channels.end())
		return (true);
	return (false);
}