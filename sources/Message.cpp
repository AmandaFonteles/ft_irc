/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Message.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: afontele <afontele@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/02 17:09:09 by dnayel            #+#    #+#             */
/*   Updated: 2026/05/28 23:02:43 by afontele         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/Message.hpp"

Message::Message() : hasTrailing(false)
{
}

std::size_t	Message::paramsCount() const
{
	return params.size() + (hasTrailing ? 1 : 0);
}

std::string	Message::param(std::size_t index) const
{
	if (index < params.size())
		return params[index];
	else if (hasTrailing && index == params.size())
		return trailing;
	else
		return ("");
}
