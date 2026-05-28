/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: afontele <afontele@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/28 14:54:05 by aibonade          #+#    #+#             */
/*   Updated: 2026/05/28 22:58:30 by afontele         ###   ########.fr       */
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

Channel::Channel(Channel const &cpy):_name(cpy._name), _topic(cpy._topic), _members(cpy._members), _operators(cpy._operators), _invited(cpy._invited), _key(cpy._key), _limit(cpy._limit), _inviteOnly(cpy._inviteOnly), _topicProtected(cpy._topicProtected)
{
	return;
}

/********SETTERS********/

void	Channel::set_topic(std::string newTopic, Client const *c)
{
	if ((_topicProtected && isOperator(c)) || !_topicProtected)
		_topic = newTopic;
	return;
}

void	Channel::set_key(std::string key, Client const *c)
{
	if (isOperator(c))
		_key = key;
	return;
}

void	Channel::set_limit(unsigned int limit, Client const *c)
{
	if (isOperator(c))
		_limit = limit;
	return;
}

void	Channel::set_inviteOnly(bool value, Client const *c)
{
	if (isOperator(c))
		_inviteOnly = value;
	return;
}

void	Channel::set_topicProtected(bool value, Client const *c)
{
	if (isOperator(c))
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
	return (this->_topic);
}

std::set<Client *>	Channel::get_members() const
{
	std::set<Client *> cpy(this->_members);
	return (cpy);
}

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

bool	Channel::addMember(Client *newMember)
{
	std::pair<std::set<Client *>::iterator, bool>	ret;
	ret = _members.insert(newMember);
	return (ret.second);
}

bool	Channel::removeMember(Client *member)
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
	if (key == _key)
		return (true);
	return (false);
}

bool	Channel::isValidKey(std::string newkey) const
{
	if (newkey.empty())
		return (false);
	size_t key_len = newkey.size();
	if (key_len > 23)
		return (false);
	for (size_t i = 0; i < key_len ; i++)
	{
		if (!isprint(newkey[i]))
			return (false);
	}
	if (newkey.find_first_of(" ,") != std::string::npos)
		return (false);
	return (true);
}
