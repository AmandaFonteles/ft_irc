/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Message.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dnayel <dnayel@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/02 17:09:09 by dnayel            #+#    #+#             */
/*   Updated: 2026/05/05 16:51:33 by dnayel           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/Message.hpp"

Message::Message() : hasTrailing(false)
{
}

std::size_t	Message::paramsCount()
{
	return params.size() + (hasTrailing ? 1 : 0); // ternary operator : if hasTrailing is true, add 1 to count the trailing as a parameter
}
// Manque la gestion du \r dans le parsing, et de la validation du message
