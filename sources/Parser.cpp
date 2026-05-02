/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Parser.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dnayel <dnayel@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/29 12:28:04 by dnayel            #+#    #+#             */
/*   Updated: 2026/05/02 15:34:33 by dnayel           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/Parser.hpp"

Parser::Parser()
{
}

Parser::~Parser()
{
}
std::string Parser::toUpper(const std::string &str)
{
	std::string result = str;
	for (std::string::size_type i = 0; i < result.size(); i++)
		result[i] = std::toupper(result[i]);
	return result;
}

std::vector<std::string>	Parser::extractLines(std::string &buffer)
{
	std::vector<std::string>	lines;
	std::string::size_type		pos; // = typedef of string index also is the return type of line.size/find()

	while ((pos = buffer.find('\n')) != std::string::npos) // while \n in buffer
	{
		lines.push_back(buffer.substr(0, pos)); // push line until \n in vector
		buffer.erase(0, pos + 1); // erase line + \n from buffer
		// cas particulier et limites ? (lignes vide, \n find de buffer, etc...)
	}
	return lines;
}
// Parsing according to IRC msg format : [":" <prefix> <SPACE> ] <command> <params> <crlf>
Message Parser::parseLine(const std::string &line)
{
	Message					msg;
	std::string::size_type	pos = 0;
	std::string::size_type	len = line.size();

	if (line.empty())
		return msg; // return empty message if line is empty

	// Extracting Prefix
	if (line[0] == ':')
	{
		std::string::size_type	spacePos = line.find(' ', 1); // 1 skips the initial ':'

		if (spacePos == std::string::npos) // no space after prefix, invalid message
			return msg;
		msg.prefix = line.substr(1, spacePos - 1); // -1 to exclude the initial ':'
		pos = spacePos; // pos at the end of prefix
		while (pos < len && line[pos] == ' ')
			pos++;
		if (pos >= len) // no command after prefix, invalid message
			return msg;
	}

	// Extracting COMMAND
	{
		std::string::size_type	spacePos = line.find(' ', pos);

		if (spacePos == std::string::npos) // Command is last element, no params
		{
			msg.command = toUpper(line.substr(pos)); // Command is case-insensitive, convert to uppercase for uniformity
			return msg;
		}
		msg.command = toUpper(line.substr(pos, spacePos - pos));
		pos = spacePos; // pos at the end of command
	}

	// Extracting PARAMS and TRAILING
	while (pos < len)
	{
		std::string::size_type	spacePos = line.find(' ', pos);

		while (line[pos] == ' ')
			pos++;
		if (pos >= len) // no more params
			break;
		if (line[pos] == ':') // Trailing
		{
			msg.trailing = line.substr(pos + 1); // Trailing is everything after ':'
			msg.hasTrailing = true;
			break; // Trailing is always last, we can stop parsing
		}
		if (spacePos == std::string::npos) // Last param (EoL)
		{
			msg.params.push_back(line.substr(pos));
			break;
		}
		msg.params.push_back(line.substr(pos, spacePos - pos));
		pos = spacePos; // pos at the end of previous param

	}

	return msg;
}
