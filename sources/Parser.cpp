/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Parser.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: afontele <afontele@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/29 12:28:04 by dnayel            #+#    #+#             */
/*   Updated: 2026/05/28 23:04:42 by afontele         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/Parser.hpp"

Parser::Parser() {}
Parser::~Parser() {}

std::string Parser::toUpper(const std::string &str)
{
	std::string result = str;
	for (std::string::size_type i = 0; i < result.size(); i++)
		result[i] = std::toupper(static_cast<unsigned char>(result[i]));
	return result;
}

std::vector<std::string>	Parser::extractLines(std::string &buffer)
{
	std::vector<std::string>	lines;
	std::string::size_type		pos;

	while ((pos = buffer.find('\n')) != std::string::npos)
	{
		std::string line = buffer.substr(0, pos);

		buffer.erase(0, pos + 1);

		if (!line.empty() && line[line.size() - 1] == '\r')
			line.erase(line.size() - 1);

		if (line.size() > 510)
			line.resize(510);

		lines.push_back(line);
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
		return msg;

	if (line[0] == ':')
	{
		std::string::size_type	spacePos = line.find(' ', 1);

		if (spacePos == std::string::npos)
			return msg;
		msg.prefix = line.substr(1, spacePos - 1);
		pos = spacePos;
		while (pos < len && line[pos] == ' ')
			pos++;
		if (pos >= len)
			return msg;
	}

	// Extracting COMMAND
	{
		std::string::size_type	spacePos = line.find(' ', pos);

		if (spacePos == std::string::npos)
		{
			msg.command = toUpper(line.substr(pos));
			return msg;
		}
		msg.command = toUpper(line.substr(pos, spacePos - pos));
		pos = spacePos;
	}

	// Extracting PARAMS and TRAILING
	while (pos < len)
	{
		while (pos < len && line[pos] == ' ')
			pos++;

		std::string::size_type	spacePos = line.find(' ', pos);

		if (pos >= len)
			break;
		if (line[pos] == ':')
		{
			msg.trailing = line.substr(pos + 1);
			msg.params.push_back(line.substr(pos + 1));
			msg.hasTrailing = true;
			break;
		}
		if (spacePos == std::string::npos)
		{
			msg.params.push_back(line.substr(pos));
			break;
		}
		msg.params.push_back(line.substr(pos, spacePos - pos));
		pos = spacePos;

	}

	return msg;
}
