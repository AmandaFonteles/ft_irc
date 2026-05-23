/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server_C.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dnayel <dnayel@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/23 21:54:11 by afontele          #+#    #+#             */
/*   Updated: 2026/05/23 09:25:26 by dnayel           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_C_HPP
# define SERVER_C_HPP

# include <iostream>
# include <string>
# include <sstream>
# include <vector>
# include <map>
# include <poll.h>
# include <sys/socket.h>
# include <sys/types.h>
# include <cerrno>
# include <csignal>
# include <unistd.h>
# include <fcntl.h>
# include <netinet/in.h>
# include <arpa/inet.h>    /* inet_ntop() : convertit une adresse binaire IPv4 en chaîne lisible */
# include <cstring>
# include <stdexcept>

/*
 * Inclusions dans l'ordre logique de dépendance :
 *   Channel ← Client ← Server
 * La classe Server utilise des pointeurs vers Channel et Client.
 * Parser et CommandHandler sont inclus ici pour que receiveClientData()
 * puisse appeler le pipeline complet sans inclusion séparée dans Server.cpp.
 * Replies est inclus pour Replies::QUIT_MSG dans removeClientFromAllChannels().
 */
# include "Channel.hpp"
# include "Client.hpp"
# include "Parser.hpp"
# include "CommandHandler.hpp"
# include "Replies.hpp"

class Server
{
private:
    /*
     * _port : port d'écoute TCP, stocké en unsigned short pour correspondre
     * au type attendu par htons() lors de bind().
     * Les ports < 1024 sont réservés (well-known ports, RFC 1340) ;
     * le sujet impose une vérification dans le constructeur.
     */
    unsigned short              _port;

    /*
     * _password : mot de passe du serveur, const car il ne doit pas changer
     * pendant la vie du serveur. Fourni comme second argument de lancement.
     * Vérifié par handlePASS() via get_password().
     * modern.ircdocs.horse : "Servers SHOULD send ERR_PASSWDMISMATCH (464)
     * and MAY then close the connection with ERROR."
     */
    std::string const           _password;

    /*
     * _serverSocket : fd du socket d'écoute principal (le socket serveur).
     * Il est toujours en mode non-bloquant via fcntl(F_SETFL, O_NONBLOCK).
     * Poll surveille ce fd pour les événements POLLIN (nouvelle connexion).
     */
    int                         _serverSocket;

    /*
     * _pollFds : vecteur de structs pollfd passé à poll().
     * std::vector est utilisé car poll() accepte un tableau contigu, et
     * un vecteur garantit le contiguïté de ses éléments (comme un tableau C).
     * Chaque entrée contient : fd, events (masque demandé), revents (résultat).
     */
    std::vector<struct pollfd>  _pollFds;

    /*
     * _channels : map nom_normalisé → Channel*.
     * La clé est le nom du canal EN MINUSCULES via lowerName() pour garantir
     * la non-sensibilité à la casse imposée par la RFC 1459 §1.3.
     * Ex : "#Test", "#test" et "#TEST" désignent le même canal.
     */
    std::map<std::string, Channel *> _channels;

    /*
     * _clients : map fd → Client*.
     * Indexée par le file descriptor socket du client, qui est unique par
     * connexion active. La recherche par fd est O(log n) ce qui est suffisant.
     */
    std::map<int, Client *>     _clients;

    /*
     * _running : variable statique contrôlant la boucle principale ServerRun().
     * Elle est static pour être accessible par signalHandler() qui est une
     * fonction C statique (ne peut pas prendre un this* en paramètre).
     * Initialisée à true dans Server.cpp.
     */
    static bool                 _running;

    Server(); /* interdit : constructeur par défaut non utilisable */

public:
    Server(const std::string &port, const std::string &password);
    ~Server();

    /* Handlers de signaux */
    static void signalHandler(int sig);

    /* Initialisation réseau et boucle principale */
    bool    ServerInit();
    void    ServerRun();

    /* Gestion du cycle de vie des clients */
    void    acceptNewClient();
    void    receiveClientData(int clientFd);
    void    cleanClosure(int clientFd);
    void    switchPollOut(int clientFd);
    void    sendMessage(int clientFd);

    /*
     * removeClientFromAllChannels()
     * ─────────────────────────────
     * Retire un client de tous les canaux et broadcast QUIT_MSG aux membres.
     *
     * PARAMÈTRE reason (nouveau, avec valeur par défaut "")
     * ───────────────────────────────────────────────────
     * modern.ircdocs.horse / QUIT : "Servers SHOULD prepend <reason> with
     * 'Quit: ' when sending QUIT messages to other clients."
     * La raison est déjà préfixée "Quit: " par handleQUIT() avant l'appel.
     *
     * POURQUOI UN PARAMÈTRE PAR DÉFAUT ?
     * cleanClosure() (déconnexion réseau brutale) appelle cette fonction
     * sans raison connue → le paramètre vaut "" par défaut et aucun
     * QUIT_MSG n'est diffusé (comportement correct : le client est déjà
     * parti sans envoyer QUIT). handleQUIT() passe la raison explicite.
     */
    void    removeClientFromAllChannels(int clientFd,
                                        const std::string &reason = "");

    /* Gestion des canaux (implémentés dans Server_channel.cpp) */
    Channel         *get_channel(std::string const name);
    Channel         *createChannel(std::string const name);
    void             deleteChannel(Channel *chan);
    Client          *get_client(std::string const nickname);
    static std::string lowerName(std::string const name);
    void             removeClientFromChannel(Client *c, Channel *chan);

    /* Getters du serveur (implémentés dans Server_channel.cpp) */
    std::string     get_name() const;
    std::string     get_password() const;
};

#endif
