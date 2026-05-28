/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Parser.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: afontele <afontele@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/29 12:27:34 by dnayel            #+#    #+#             */
/*   Updated: 2026/05/28 22:07:48 by afontele         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef PARSER_HPP
# define PARSER_HPP

# include <string>
# include <vector>
# include "../includes/Message.hpp"

class Parser
{
	public :
		Parser();
		~Parser();

		static std::vector<std::string>	extractLines(std::string &buffer);
		static Message parseLine(const std::string &line);

	private :
		static std::string toUpper(const std::string &str);

};

#endif
