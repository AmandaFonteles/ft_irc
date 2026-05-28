/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: afontele <afontele@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/28 12:55:22 by aibonade          #+#    #+#             */
/*   Updated: 2026/05/28 22:14:13 by afontele         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CHANNEL_HPP
# define CHANNEL_HPP
# include <string>
# include <set>

class Client;

class Channel
{
private:
//data
	std::string					_name;
	std::string					_topic;
	std::set<Client *>			_members;
	std::set<Client const *>	_operators;
	std::set<Client const *>	_invited;
	std::string					_key;
	size_t						_limit;
	bool						_inviteOnly;
	bool						_topicProtected;
//methods 
								Channel();

public:
//constructor/destructor
								Channel(Channel const &cpy);
								Channel(std::string name);
								~Channel();

//setters
	void						set_topic(std::string newTopic, Client const *c);
	void						set_key(std::string key, Client const *c);
	void						set_limit(unsigned int limit, Client const *c);
	void						set_inviteOnly(bool value, Client const *c);
	void						set_topicProtected(bool value, Client const *c);

//getters
	std::string					get_name() const;
	std::string					get_topic() const;
	std::set<Client *>			get_members() const;
	size_t						get_limit() const;
	bool						get_inviteOnly() const;
	bool						get_topicProtected() const;

//other methods
	Channel						&operator=(Channel const &to_affect);
	bool						addMember(Client *newMember);
	bool						removeMember(Client *member);
	bool						addOperator(Client const *newOperator);
	bool						removeOperator(Client const *op);
	bool						addInvite(Client const *newMember);
	bool						removeInvite(Client const *member);
	bool						isMember(Client *c) const;
	bool						isOperator(Client const *c) const;
	bool						isKey(std::string const key) const;
	bool						isInvited(Client const *c) const;
	bool						isFull() const;
	bool						hasKey() const;
	size_t						nbMembers() const;
	bool						isValidKey(std::string newkey)const;
};


#endif
