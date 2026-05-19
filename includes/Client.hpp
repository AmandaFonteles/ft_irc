/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dnayel <dnayel@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/28 12:55:34 by aibonade          #+#    #+#             */
/*   Updated: 2026/05/19 15:15:36 by dnayel           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CLIENT_HPP
# define CLIENT_HPP
# include <string>
# include <set>
# include <algorithm>
# include "Channel.hpp"

class Client
{
private:
//data
	int					_socketFd;
	bool				_passOk;//passe a true si le client utilise PASS avec le mot de passe donne au lancement du serveur
	std::string			_nickname;
	std::string			_username;
	bool				_registered;//passe a true quand le client a passe toute la phase d'enregistrement
	std::set<Channel *>	_channels;// <const> ? et surtout strings => on utiliserait la map du serveur pour retrouver le bon serveur ?
	std::string			_bufferIn;//besoin de plusieurs ? //public?
	std::string			_bufferOut;//public ?

//methods
	Client();
	Client				&operator=(Client const &to_affect);

// Nayel
    /*
     * _hostname : adresse IP du client (ex: "127.0.0.1" ou "192.168.1.42").
     *
     * AJOUTÉ PAR PERSONNE B pour construire le préfixe IRC "nick!user@host"
     * utilisé dans tous les messages envoyés au nom du client.
     *
     * REMPLI PAR PERSONNE A dans acceptNewClient() :
     *   char hostBuf[INET_ADDRSTRLEN];
     *   inet_ntop(AF_INET, &clientAddr.sin_addr, hostBuf, sizeof(hostBuf));
     *   newClient->set_hostname(hostBuf);
     *
     * Valeur par défaut : "localhost" (pour les tests sans inet_ntop).
     */
	std::string			_hostname;
    /*
     * _shouldClose : signal de fermeture différée.
     *
     * AJOUTÉ PAR PERSONNE B pour gérer la séquence :
     *   1. handleQuit() ou mauvais PASS → enqueue ERROR dans _bufferOut
     *   2. _shouldClose = true
     *   3. Personne A continue à envoyer _bufferOut via send() (POLLOUT)
     *   4. Quand _bufferOut est vide ET _shouldClose = true → cleanClosure()
     *
     * POURQUOI PAS close() immédiat dans handleQuit() ?
     *   - close(fd) pendant la boucle poll() invalide les pollfd en cours
     *   - Le client doit recevoir le message ERROR avant la coupure TCP
     *   - On ne peut pas send() après close()
     *
     * PERSONNE A doit ajouter dans sendMessage() :
     *   if (_clients[fd]->get_shouldClose() && _clients[fd]->get_bufferOut().empty())
     *       cleanClosure(fd);
     */
	bool				_shouldClose;
	std::string			_realname;//TO DO : a ajouter dans la classe client ? et dans la registration ? et dans le prefixe des messages ? (nick!user@host realname) ou (nick!user@host) ? spec : "The real name may be sent in the trailing part of the USER command and MUST be stored by the server as part of the client's information. It is not used in any protocol messages, but may be returned in response to a WHOIS query." => du coup on peut s'en passer pour ft_irc, ou alors on le stocke mais on ne l'utilise pas pour les messages ?

public:
//constructor/destructor
						Client(Client const &cpy);
						Client(int socketFd);
						~Client();

//setters
//besoin d'un set_fd ?
	void				set_bufferIn(std::string str);
	void				set_nickname(std::string nickname);
	void				set_username(std::string username);
	void				set_passOk(bool value);
	void				set_registered(bool value);
	void				set_bufferOut(std::string str);

//getters
	int					get_socketFd() const;
	std::string			&get_bufferIn();
	std::string			get_nickname() const;
	std::string			get_username() const;
	bool				get_passOk() const;
	bool				get_registered() const;
	// std::set<Channel *>	get_channels();//get ou print ? ou string ?
	std::string			&get_bufferOut();

//other methods
	bool				addChannel(Channel *chan);
	bool				removeChannel(Channel *chan);
	bool				isInChannel(Channel *chan) const;
	void				removeAllChannel();

// Nayel
	void				set_hostname(std::string hostname);
	std::string			get_hostname();

	void				set_shouldClose(bool value);
	bool				get_shouldClose() const;

	void				set_realname(std::string realname);
	std::string			get_realname() const;
};

#endif
