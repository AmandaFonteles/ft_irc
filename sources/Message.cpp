#include "../includes/Message.hpp"

Message::Message() : hasTrailing(false)
{
}

std::size_t	Message::paramsCount()
{
	return params.size() + (hasTrailing ? 1 : 0); // ternary operator : if hasTrailing is true, add 1 to count the trailing as a parameter
}
