/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aibonade <aibonade@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/28 14:54:05 by aibonade          #+#    #+#             */
/*   Updated: 2026/05/08 16:33:29 by aibonade         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

# include "../includes/Channel.hpp"

/********CONSTRUCTORS & DESTRUCTOR********/
Channel::Channel():_name("default"), _topic(""), _key(""), _limit(0), _inviteOnly(false), _topicProtected(false)
{
	return;
}

Channel::Channel(std::string name):_name(name), _topic(""), _key(""), _limit(0), _inviteOnly(false), _topicProtected(false)
{
	return;
}

Channel::~Channel()	{ return; }

Channel::Channel(Channel const &cpy):_name(cpy._name), _topic(cpy._topic), _members(cpy._members), _operators(cpy._operators), _invited(cpy._invited), _key(cpy._key), _limit(cpy._limit), _inviteOnly(cpy._inviteOnly), _topicProtected(cpy._inviteOnly)
{
	return;
}

/********SETTERS********/
//Client const *c = le client qui demande a faire l'operation, comme ca on checke s'il peut
void	Channel::set_topic(std::string newTopic, Client const *c)
{
	if ((_topicProtected && isOperator(c)) || !_topicProtected)
		_topic = newTopic;
	return;//on doit faire un message ? Si oui je le mettrais bien dans la fonction qui appellera celle-ci avec un bool sur celle là
}

void	Channel::set_key(std::string key, Client const *c)
{
	if (isOperator(c))//checker si c'est protege et comment + s'il faut une surcharge pour le cas ou on n'a pas d'appel client
		_key = key;
	return;//on doit faire un message ? Si oui je le mettrais bien dans la fonction qui appellera celle-ci avec un bool sur celle là
}

void	Channel::set_limit(unsigned int limit, Client const *c)
{
	//Si déjà des participants la limit est set donc on ne peut pas JOIN par dessus mais ça ne change rien pour les gens qui sont déjà là
	if (isOperator(c))//checker si c'est vraiment protege + s'il faut une surcharge pour le cas ou on n'a pas d'appel client
		_limit = limit;
	return;
}

void	Channel::set_inviteOnly(bool value, Client const *c)
{
	if (isOperator(c))//checker si c'est protege + s'il faut une surcharge pour le cas ou on n'a pas d'appel client
		_inviteOnly = value;
	return;
}

void	Channel::set_topicProtected(bool value, Client const *c)
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
// std::set<Client const *>	Channel::get_operators() const;//set ? string ? print ? //TO DO

size_t	Channel::get_limit() const
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
/*Channel	&Channel::operator=(Channel const &to_affect)//copier la data depuis la source vers cet objet sauf les membres const
{
	if (this != &to_affect)
	{
		//pour moi on ne copie pas le nom
		_topic = to_affect._topic;
		// _members = to_affect._members;//On copie ? idem pour operators ?
		//idem on copie la clé ou on garde la meme ? ce serait plus logique de garder celle qu'on a deja mais a voir si on a vraiment besoin de cet operateur et dans quel contexte
		if (to_affect._limit >= _members.size())
			_limit = to_affect._limit;
		_inviteOnly = to_affect._inviteOnly;
		_topic = to_affect._topic;
	}
	return (*this);
}*/

bool	Channel::addMember(Client *newMember)
{
	std::pair<std::set<Client *>::iterator, bool>	ret;
	// if (this->isMember(newMember))
	// 	return (false);
	ret = _members.insert(newMember);//Si _members contient deja newMember, insert ne l'insere pas une seconde fois mais renvoie l'iterateur de sa position dans le set et indique qu'il n'a pas fait d'insertion en mettant le deuxieme element de la paire a "false", autrement c'est true et on recupere l'iterateur du nouvel element
	return (ret.second);
}

bool	Channel::removeMember(Client *member)//On peut aussi faire plus simplement cette fonction avec .erase(member), mais ca me paraissait plus sur comme ca, on maitrise mieux ce qu'il se passe je trouve... 
{
	std::set<Client *>::iterator	it;

	it = _members.find(member);
	if (it == _members.end())
		return (false);
	_members.erase(it);
	return (true);
}

bool	Channel::addOperator(Client const *newOperator)
{
	std::pair<std::set<Client const *>::iterator, bool>	ret;

	ret = _operators.insert(newOperator);
	return (ret.second);
}

bool	Channel::removeOperator(Client const *op)
{
	std::set<Client const *>::iterator	it;

	it = _operators.find(op);
	if (it == _operators.end())
		return (false);
	_operators.erase(it);
	return (true);
}

bool	Channel::addInvite(Client const *newMember)
{
	std::pair<std::set<Client const *>::iterator, bool>	ret;

	ret = _invited.insert(newMember);
	return (ret.second);
}

bool	Channel::removeInvite(Client const *member)
{
	std::set<Client const *>::iterator	it;

	it = _invited.find(member);
	if (it == _invited.end())
		return (false);
	_invited.erase(it);
	return (true);
}
bool	Channel::hasKey() const
{
	if (_key.empty() == true)
		return (false);
	return (true);
}

bool	Channel::isFull() const
{
	if (_limit)
	{
		if (_members.size() >=_limit)
			return (true);
	}
	return (false);
}

size_t	Channel::nbMembers() const
{
	return (_members.size());
}

bool	Channel::isInvited(Client const *c) const
{
	if (_invited.find(c) == _invited.end())
		return (false);
	return (true);
}

bool	Channel::isMember(Client *c) const
{
	if (_members.find(c) == _members.end())
		return (false);
	return (true);
}

bool	Channel::isOperator(Client const *c) const
{
	if (_operators.find(c) == _operators.end())
		return (false);
	return (true);
}

bool	Channel::isKey(std::string const key) const
{
	if (key == _key)//A voir si ya des changements a faire niveau secu
		return (true);
	return (false);
}
