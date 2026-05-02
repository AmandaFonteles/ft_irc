/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Parser.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dnayel <dnayel@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/29 12:27:34 by dnayel            #+#    #+#             */
/*   Updated: 2026/05/02 15:14:54 by dnayel           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef PARSER_HPP
#define PARSER_HPP

# include <string>
# include <vector>
# include "../includes/Message.hpp"

// static methods because Parser is never instantiated, it only provides utility functions
class Parser
{
	public :
		Parser();
		~Parser();

		static std::vector<std::string>	extractLines(std::string &buffer); // until first \n
		static Message parseLine(const std::string &line); // const to ensure line is not modified by parseLine, and to allow passing string literals without error (e.g. parseLine("PING :server"))

	private :
		static std::string toUpper(const std::string &str);

};

#endif
