/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aibonade <aibonade@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/28 14:54:05 by aibonade          #+#    #+#             */
/*   Updated: 2026/04/28 21:28:58 by aibonade         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

# include "../includes/Channel.hpp"

/********CONSTRUCTORS & DESTRUCTOR********/
Channel::Channel():_name("default"), _topic("default"), _key(""), _limit(0), _inviteOnly(false), _topicProtected(false)
{
	return;
}

Channel::~Channel()
{
	return;
}

Channel::Channel(Channel const &cpy):_name(cpy._name), _topic(cpy._topic), _key(cpy._key), _limit(cpy._limit), _inviteOnly(cpy._inviteOnly), _topicProtected(cpy._inviteOnly), _members(cpy._members), _operators(cpy._operators)//on copie aussi les membres et operateurs ?
{
	return;//voir question en comm
}

Channel::Channel(std::string name):_name(name), _topic("no topic set"), _key(""), _limit(0), _inviteOnly(false), _topicProtected(false)
{
	return;//a eventuellement mettre a jour avec la commande de creation d'un channel ?
}

/********SETTERS********/
//Client const &c = le client qui demande a faire l'operation, comme ca on checke s'il peut
void	Channel::set_topic(std::string newTopic, Client const &c)
{
	if ((_topicProtected && isOperator(c)) || !_topicProtected)
		_topic = newTopic;
	return;//on doit faire un message ? Si oui je le mettrais bien dans la fonction qui appellera celle-ci avec un bool sur celle là
}

void	Channel::set_key(std::string key, Client const &c)
{
	if (isOperator(c))//checker si c'est protege et comment + s'il faut une surcharge pour le cas ou on n'a pas d'appel client
		_key = key;
	return;//on doit faire un message ? Si oui je le mettrais bien dans la fonction qui appellera celle-ci avec un bool sur celle là
}

void	Channel::set_limit(unsigned int limit, Client const &c)
{
	//voir ce qu'il se passe si on a deja des participants quand on modifie cette var, comment ça se passe
	if (isOperator(c))//checker si c'est vraiment protege + s'il faut une surcharge pour le cas ou on n'a pas d'appel client
		_limit = limit;
	return;
}

void	Channel::set_inviteOnly(bool value, Client const &c)
{
	if (isOperator(c))//checker si c'est protege + s'il faut une surcharge pour le cas ou on n'a pas d'appel client
		_inviteOnly = value;
	return;
}

void	Channel::set_topicProtected(bool value, Client const &c)
{
	if (isOperator(c))//checker si c'est protege + s'il faut une surcharge pour le cas ou on n'a pas d'appel client
		_topicProtected = value;
	return;
}

/********GETTERS********/
std::string	Channel::get_name() const
{
	return (this->_name);
}

std::string	Channel::get_topic() const
{
	return (this->_name);
}

// std::set<Client &>	Channel::get_members() const;//set ? string ? print ? //TO DO
// std::set<Client const &>	Channel::get_operators() const;//set ? string ? print ? //TO DO

unsigned int	Channel::get_limit() const
{
	return (this->_limit);
}

bool	Channel::get_inviteOnly() const
{
	return (this->_inviteOnly);
}

bool	Channel::get_topicProtected() const
{
	return (this->_topicProtected);
}

/********OTHER METHODS********/
Channel	&Channel::operator=(Channel const &to_affect)
{
	if (this != &to_affect)
	{
		//pour moi on ne copie pas le nom
		_topic = to_affect._topic;
		// _members = to_affect._members;//On copie ? idem pour operators ?
		//idem on copie la clé ou on garde la meme ? ce serait plus logique de garder celle qu'on a deja mais a voir si on a vraiment besoin de cet operateur et dans quel contexte
		if (to_affect._limit >= _members.size())
			_limit = to_affect._limit;//Et sinon ? on la met au nb de participants ?
		_inviteOnly = to_affect._inviteOnly;
		_topic = to_affect._topic;
	}
	return (*this);
}

// bool	Channel::addMember(Client const &newMember);//void + Exception ? //TO DO
// bool	Channel::removeMember(Client const &member);//void + Exception ? //TO DO
// bool	Channel::addOperator(Client const &newOperator);//void + Exception ? //TO DO
// bool	Channel::removeOperator(Client const &op);//void + Exception ? //TO DO

bool	Channel::isMember(Client &c)//on peut tenter de passer par un recast pour ajouter des consts corrects... mais flemme
{
	if (_members.find(c) == _members.end())//alors je ne sais pas pk il rale...
		return (false);
	return (true);
}

bool	Channel::isOperator(Client const &c) const
{
	if (_operators.find(c) == _operators.end())
		return (false);
	return (true);
}

bool	Channel::isKey(std::string key) const
{
	if (key == _key)//A voir si ya des changements a faire niveau secu
		return (true);
	return (false);
}
