/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server_C_channel.cpp                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dnayel <dnayel@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/09 10:40:24 by aibonade          #+#    #+#             */
/*   Updated: 2026/05/23 09:25:39 by dnayel           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/Server_C.hpp"

/*
 * Ce fichier regroupe les méthodes du Server liées à la gestion des canaux
 * et à la recherche de clients. Il est séparé de Server.cpp pour répartir
 * les responsabilités entre les membres de l'équipe (Personne A et C).
 *
 * RÈGLE DE NORMALISATION DES NOMS DE CANAUX :
 * RFC 1459 §1.3 : les noms de canaux sont insensibles à la casse.
 * "#Test", "#test" et "#TEST" désignent le même canal.
 * La map _channels est donc indexée PAR LE NOM EN MINUSCULES via lowerName().
 * Toutes les opérations sur la map (find, insert, erase) doivent utiliser
 * lowerName() pour garantir la cohérence.
 */

/* ---------------------------------------------------------------------------
 *  get_channel()
 * ---------------------------------------------------------------------------
 * Retourne le pointeur vers le canal si trouvé, NULL sinon.
 * Normalise le nom avec lowerName() avant la recherche.
 * --------------------------------------------------------------------------- */
Channel *Server::get_channel(std::string const name)
{
    std::map<std::string, Channel *>::iterator it;

    it = _channels.find(lowerName(name));
    if (it == _channels.end())
        return NULL;
    return it->second;
}

/* ---------------------------------------------------------------------------
 *  createChannel()
 * ---------------------------------------------------------------------------
 * Crée un nouveau canal vide et l'ajoute à _channels.
 * Retourne NULL si le canal existe déjà (protection contre les doublons).
 *
 * CLÉ DE LA MAP = lowerName(name) :
 * La valeur stockée dans la map est le canal créé avec le nom ORIGINAL
 * (pour préserver la casse dans les messages, ex : "#Test" pas "#test"),
 * mais la clé de recherche est la version normalisée.
 *
 * CORRECTION DE BUG :
 * L'ancienne version avait "_channels[name]" dans le return, ce qui créait
 * une entrée avec le nom non-normalisé en clé. Si le nom contenait des
 * majuscules, deux entrées distinctes existaient dans la map pour le même
 * canal conceptuel. Désormais, on utilise toujours lowerName(name) comme clé.
 * --------------------------------------------------------------------------- */
Channel *Server::createChannel(std::string const name)
{
    if (get_channel(name))
        return NULL; /* canal déjà existant */

    _channels[lowerName(name)] = new Channel(name);
    return _channels[lowerName(name)];
}

/* ---------------------------------------------------------------------------
 *  deleteChannel()
 * ---------------------------------------------------------------------------
 * Supprime un canal de la map et libère sa mémoire.
 *
 * CORRECTION DE BUG :
 * L'ancienne version utilisait chan->get_name() (nom original, potentiellement
 * avec majuscules) comme clé d'effacement. Or la map est indexée par le nom
 * normalisé. Utiliser get_name() non normalisé pouvait donc ne rien effacer
 * (entrée introuvable dans la map) et provoquer une double-libération.
 * On utilise désormais lowerName(chan->get_name()) pour correspondre à la clé.
 * --------------------------------------------------------------------------- */
void Server::deleteChannel(Channel *chan)
{
    _channels.erase(lowerName(chan->get_name())); /* clé normalisée */
    if (chan)
        delete chan;
}

/* ---------------------------------------------------------------------------
 *  removeClientFromChannel()
 * ---------------------------------------------------------------------------
 * Retire un client d'un canal spécifique (appelé par PART et KICK).
 * Retire le client des trois ensembles : membres, opérateurs, invités.
 * Puis retire le canal de la liste du client.
 * Si le canal devient vide, il est supprimé.
 *
 * ORDRE DE CES OPÉRATIONS :
 * Chan::removeMember() est appelé AVANT c->removeChannel() parce que
 * removeChannel() dans Client vérifie si isMember(this) est faux avant
 * d'effectuer la suppression. Si removeMember() n'a pas encore été appelé,
 * removeChannel() retourne false sans rien faire.
 * --------------------------------------------------------------------------- */
void Server::removeClientFromChannel(Client *c, Channel *chan)
{
    chan->removeOperator(c);
    chan->removeMember(c);
    chan->removeInvite(c);
    c->removeChannel(chan);
    if (chan->nbMembers() == 0)
        deleteChannel(chan);
}

/* ---------------------------------------------------------------------------
 *  get_client()
 * ---------------------------------------------------------------------------
 * Recherche un client par son pseudo, de manière insensible à la casse.
 * Retourne NULL si le client n'est pas trouvé ou n'est pas enregistré.
 *
 * RFC 1459 §1.3 : les nicks sont également insensibles à la casse.
 * On normalise à la fois le nick recherché et le nick stocké.
 * --------------------------------------------------------------------------- */
Client *Server::get_client(std::string const nickname)
{
    std::string                             nickname_lower = Server::lowerName(nickname);
    std::map<int, Client *>::iterator       it             = _clients.begin();

    while (it != _clients.end())
    {
        if (Server::lowerName(it->second->get_nickname()) == nickname_lower)
            return it->second;
        ++it;
    }
    return NULL;
}

/* ---------------------------------------------------------------------------
 *  lowerName()
 * ---------------------------------------------------------------------------
 * Convertit une chaîne en minuscules caractère par caractère.
 *
 * POURQUOI std::tolower(str[i]) SANS CAST N'EST PAS SUFFISANT ?
 * std::tolower() prend un int. Si char est signé (cas courant sur Linux x86)
 * et que str[i] > 127 (ex: un octet d'accent ISO-8859), la valeur est négative.
 * Passer une valeur négative à tolower() est un comportement indéfini (UB)
 * selon C++98. Le cast static_cast<unsigned char>() force la valeur dans
 * [0, 255], ce qui est le domaine légal de tolower().
 * --------------------------------------------------------------------------- */
std::string Server::lowerName(std::string const name)
{
    std::string str = name;
    for (std::size_t i = 0; i < str.size(); i++)
        str[i] = static_cast<char>(std::tolower(static_cast<unsigned char>(str[i])));
    return str;
}

/* ---------------------------------------------------------------------------
 *  Getters du serveur
 * ---------------------------------------------------------------------------
 * get_name() : retourne le nom du serveur tel qu'il apparaît dans les messages
 * IRC : ":ft_irc 001 alice :Welcome..."
 * Il n'est pas configuré dynamiquement car ft_irc a un seul nom fixe.
 *
 * get_password() : retourne le mot de passe serveur pour la vérification dans
 * handlePASS(). _password est const, ce getter est donc const également.
 * --------------------------------------------------------------------------- */
std::string Server::get_name() const
{
    return "ft_irc";
}

std::string Server::get_password() const
{
    return _password;
}
