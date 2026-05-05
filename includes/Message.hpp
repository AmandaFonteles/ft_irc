/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Message.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dnayel <dnayel@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/02 17:09:04 by dnayel            #+#    #+#             */
/*   Updated: 2026/05/05 16:23:18 by dnayel           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef MESSAGE_HPP
#define MESSAGE_HPP

# include <string>
# include <vector>

// IRC msg = [":" <prefix> <SPACE> ] <command> <params> <crlf>
struct Message
{
	std::string					prefix;
	std::string					command;
	std::vector<std::string>	params;
	std::string					trailing;
	bool						hasTrailing;

	Message(); // we can actually use a constructor in a struct, it's just that all members are public by default
				// allowing to initialize hasTrailing to false and avoid uninitialized memory issues
	std::size_t					paramsCount(); // const ??

	//// Super propal, pourrait être vachement utile pour alleger le code
	//
	//    /* ------------------------------------------------------------------
    // * param(index) → std::string
    // * ------------------------------------------------------------------
    // * ACCÈS UNIFIÉ à tous les paramètres par index, trailing inclus.
    // *
    // * PROBLÈME RÉSOLU :
    // * Sans cette méthode, chaque handler devrait écrire :
    // *   std::string username;
    // *   if (msg.params.size() >= 1) username = msg.params[0];
    // *   std::string realname;
    // *   if (msg.hasTrailing) realname = msg.trailing;
    // *   else if (msg.params.size() >= 4) realname = msg.params[3];
    // *
    // * AVEC param(), on écrit simplement :
    // *   std::string username  = msg.param(0);
    // *   std::string realname  = msg.param(3);
    // *
    // * RÈGLE D'INDEXATION :
    // *   param(0) ... param(params.size()-1) → params[index]
    // *   param(params.size())                → trailing (si hasTrailing)
    // *   param(n > params.size())            → "" (chaîne vide, pas d'exception)
    // *
    // * SÉCURITÉ : retourne "" pour les index hors bornes au lieu de lancer
    // * une exception std::out_of_range ou de causer un segfault.
    // * Cela rend le code appelant plus simple (pas besoin de try/catch).
    // * ------------------------------------------------------------------ */
    //std::string param(std::size_t index) const;
};


#endif
