/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aibonade <aibonade@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/28 12:55:22 by aibonade          #+#    #+#             */
/*   Updated: 2026/05/12 16:42:53 by aibonade         ###   ########.fr       */
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
	std::string					_topic;//"" => no topic
	std::set<Client *>			_members;
	std::set<Client const *>	_operators;
	std::set<Client const *>	_invited;
	std::string					_key;//"" => no key
	size_t						_limit;//nb user max => 0 = no limit ?
	bool						_inviteOnly;
	bool						_topicProtected;
//methods 
								Channel();


public:
//constructor/destructor
								Channel(Channel const &cpy);
								Channel(std::string name);
								~Channel();

//setters, Client const &c = le client qui demande a faire l'operation, comme ca on checke s'il peut
	void						set_topic(std::string newTopic, Client const *c);
	void						set_key(std::string key, Client const *c);//comment on les protege au fait ?
	void						set_limit(unsigned int limit, Client const *c);
	void						set_inviteOnly(bool value, Client const *c);
	void						set_topicProtected(bool value, Client const *c);

//getters
	std::string					get_name() const;
	std::string					get_topic() const;
	std::set<Client *>			get_members() const;//set ? string ? print ?
	// std::set<Client const &>	get_operators() const;//set ? string ? print ?
	size_t						get_limit() const;
	bool						get_inviteOnly() const;
	bool						get_topicProtected() const;

//other methods
	Channel						&operator=(Channel const &to_affect);//A garder ?
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
	bool						isValidKey(std::string newkey)const;//1->23char, ascii vsibiles, pas d'espaces, \r\n\t\v\n\0 interdits, pas de ','
};


#endif
