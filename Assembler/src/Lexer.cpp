#include "Lexer.h"

bool Lexer::isSpace(char c)
{
	return c == '\n' ||
		c == '\r' ||
		c == '\t' ||
		c == '\v' ||
		c == ' ' ||
		c == '\f';
}

bool Lexer::isSpecial(char c)
{
	return c == '[' ||
		c == ']' || 
		c == ',' || 
		c == ':';
}

bool Lexer::isGroup(char c)
{
	beg_char = c;
	switch (c)
	{
	case '"':
		end_char = '"';
		return true;
	case '(':
		end_char = ')';
		return true;
	case ')':
		return true;
	default:
		return false;
	}
}

std::vector<std::string> Lexer::lex(const std::string& s)
{
	std::vector<std::string> out;
	char lexeme[256];
	size_t i = 0, j = 0;
	State state = State::START;
	int done = 0;
	size_t len = s.length();
	int balance = 0;

#define addchar() { lexeme[j] = s[i]; i++; j++; }
#define addlex() { lexeme[j] = 0; out.push_back(lexeme); j = 0; }

	while (i < len)
	{
		switch (state)
		{
		case State::START:
		{
			if (isSpace(s[i]))
			{
				state = State::SKIP;
			}
			else if (isGroup(s[i]))
			{
				if (s[i] == '"')
				{
					addchar();
				}
				state = State::READBLOCK;
			}
			else if (s[i] == '/' && s[i + 1] == '/')
			{
				state = State::COMMENT;
				i += 2;
			}
			else
			{
				state = State::READCHAR;
			}
			break;
		}
		case State::READCHAR:
		{
			if (isSpace(s[i]))
			{
				state = State::DUMP;
			}
			else if (s[i] == '\\')
			{
				i += 2;
			}
			else if (isGroup(s[i]))
			{
				if (s[i] == '"')
				{
					addchar();
				}
				state = State::READBLOCK;
			}
			else if (isSpecial(s[i]))
			{
				if (j == 0)
				{
					addchar();
				}
				state = State::DUMP;
			}
			else if (s[i] == '/' && s[i + 1] == '/')
			{
				state = State::COMMENT;
				i += 2;
			}
			else
			{
				addchar();
			}
			break;
		}
		case State::READBLOCK:
		{
			if (s[i] == beg_char && s[i] != '"')
			{
				balance++;
				addchar();
			}
			else if (s[i] == end_char)
			{
				balance--;
				addchar();
				if (balance <= 0)
				{
					state = State::DUMP;
				}
			}
			else if (end_char == '"' && s[i] == '\\')
			{
				// TODO: actually record chars
				i += 2;
			}
			else
			{
				addchar();
			}
			break;
		}
		case State::SKIP:
		{
			if (isSpace(s[i]))
			{
				i++;
			}
			else
			{
				state = State::READCHAR;
			}
			break;
		}
		case State::DUMP:
		{
			if (j > 0) addlex();
			state = State::START;
			break;
		}
		case State::COMMENT:
		{
			if (s[i] != '\n')
			{
				i++;
			}
			else
			{
				state = State::READCHAR;
			}
			break;
		}
		case State::END:
			i = len;
			break;
		}
	}
	
	if (j > 0) addlex();

	return out;
}
