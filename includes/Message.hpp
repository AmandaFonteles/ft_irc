/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Message.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dnayel <dnayel@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/02 17:09:04 by dnayel            #+#    #+#             */
/*   Updated: 2026/05/19 12:41:54 by dnayel           ###   ########.fr       */
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
	std::size_t					paramsCount() const; // const ??
	std::string					param(std::size_t index) const;
};


#endif
