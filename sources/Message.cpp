/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Message.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dnayel <dnayel@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/02 17:09:09 by dnayel            #+#    #+#             */
/*   Updated: 2026/05/19 12:42:13 by dnayel           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/Message.hpp"

Message::Message() : hasTrailing(false)
{
}

std::size_t	Message::paramsCount() const
{
	return params.size() + (hasTrailing ? 1 : 0); // ternary operator : if hasTrailing is true, add 1 to count the trailing as a parameter
}

std::string	Message::param(std::size_t index) const
{
	if (index < params.size())
		return params[index];
	else if (hasTrailing && index == params.size())
		return trailing;
	else
		return (""); // return empty string if index is out of bounds
}
