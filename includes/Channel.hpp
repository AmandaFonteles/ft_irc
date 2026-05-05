/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: afontele <afontele@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/28 12:55:22 by aibonade          #+#    #+#             */
/*   Updated: 2026/05/05 18:21:39 by afontele         ###   ########.fr       */
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
	std::set<Client &>			_members;//pas sure qu'ils soient const ici.... =>quand on les remove ici faut remove le chan chez eux aussi 
	std::set<Client const &>	_operators;
	std::set<Client const &>	_invited;
	std::string					_key;//"" => no key
	unsigned int				_limit;//nb user max => 0 = no limit ?
	bool						_inviteOnly;
	bool						_topicProtected;
//methods 
								Channel();


public:
//constructor/destructor
								Channel(Channel const &cpy);
								Channel(std::string name);//comment on le cree ? de quoi on a besoin ?
								~Channel();

//setters, Client const &c = le client qui demande a faire l'operation, comme ca on checke s'il peut
	void						set_topic(std::string newTopic, Client const &c);
	void						set_key(std::string key, Client const &c);//comment on les protege au fait ?
	void						set_limit(unsigned int limit, Client const &c);
	void						set_inviteOnly(bool value, Client const &c);
	void						set_topicProtected(bool value, Client const &c);

//getters
	std::string					get_name() const;
	std::string					get_topic() const;
	std::set<Client &>			get_members() const;//set ? string ? print ?
	std::set<Client const &>	get_operators() const;//set ? string ? print ?
	unsigned int				get_limit() const;
	bool						get_inviteOnly() const;
	bool						get_topicProtected() const;

//other methods
	Channel						&operator=(Channel const &to_affect);
	bool						addMember(Client &newMember);//void + Exception ?
	bool						removeMember(Client const &member);//void + Exception ?
	bool						addOperator(Client const &newOperator);//void + Exception ?
	bool						removeOperator(Client const &op);//void + Exception ?
	bool						isMember(Client &c);
	// bool						isMember(Client const &c) const;
	bool						isOperator(Client const &c) const;
	bool						isKey(std::string key) const;
	bool						isInvited(Client const &c) const;//A ajouter sur le cpp
	bool						isfull() const;//A ajouter sur le cpp
	bool						hasKey() const;//A ajouter sur le cpp
	bool						addInvite(Client const &newmember);//void + Exception ?//A ajouter sur le cpp
	bool						removeInvite(Client const &member);//void + Exception ?//A ajouter sur le cpp
};


#endif
