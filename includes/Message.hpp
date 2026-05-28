/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Message.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: afontele <afontele@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/02 17:09:04 by dnayel            #+#    #+#             */
/*   Updated: 2026/05/28 22:08:30 by afontele         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef MESSAGE_HPP
# define MESSAGE_HPP

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

	Message();
	std::size_t					paramsCount() const;
	std::string					param(std::size_t index) const;
};


#endif
