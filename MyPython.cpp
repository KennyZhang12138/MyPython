// Lexer code adapted from source code at https://www.dreamincode.net/forums/topic/153718-fundamentals-of-parsing/
/*
	The following code parses C/C++ source and prints out a list of tokens.
	The code is intended to be used as an example of parsing, and nothing
	else. Having said that, it must be pointed out that whilst this parser
	does correctly identify and process C/C++ tokens, there are tokens
	parsed that would not exist when a true C++ compiler tokenised it's
	input. Preprocessor tokenization is implemented in phase III of the 
        compiler front-end and such tokens as #, ## would not be seen (i.e. they
        would have been processed in phase III). Furthermore, tokens enclosed in
        single quotes would be converted directly into their integer equivalents.
        Also, unicode characters and unicode sequences are not processed by this
	example.

	I have endeavoured to put a few checks in the parser to ensure that 
	double quoted literals and block comments are terminated before the end
	of file, but this code assumes that the source provided is actually 
	compilable!!

	The code has been compiled and tested under the Microsoft VC++ version
	10, B2 Release and no guarantee can be made as to it's compatibility with
	either other versions of VC++, or any other C++ compiler.

	If you have any problems, with this code please do not hesitate to ask.
*/

#include <iostream>
#include <fstream>
#include <string>
#include <list>
#include <cctype>
#include <stack>

using namespace std;

// All tokens must derive from this token type
class base_token
{
	public:
		typedef enum {t_invalid_token=0, t_symbol,
				t_integer, t_literal,
				t_constant, t_punctuation,
				t_whitespace, t_eol, t_indent, t_dedent, t_eof
	        } type_of_token;
	private:
		type_of_token token_type;
	public:
		base_token(type_of_token token) : token_type(token) { };
		virtual int parse_token(fstream& stream, int input_char) = 0;
		virtual void print_token() = 0;
};

// A token that may contain a symbol
class symbol_token : public base_token
{
	private:
		string symbol;
	public:
		symbol_token() : base_token(t_symbol) { };
		int parse_token(fstream& stream, int input_char);
		void print_token();
};

// A token that represents an integer
class integer_token : public base_token
{
	private:
		string integer_string;
	public:
		integer_token() : base_token(t_integer) { };
		int parse_token(fstream& stream, int input_char);
		void print_token();
};

// A token that represents a literal
class literal_token : public base_token
{
	private:
		string literal_string;
	public:
		literal_token() : base_token(t_literal) { };
		int parse_token(fstream& stream, int input_char);
		void print_token();
};

// A token that represents a constant literal (e.g. 'A')
class constant_token : public base_token
{
	private:
		string constant_string;
	public:
		constant_token() : base_token(t_constant) { };
		int parse_token(fstream& stream, int input_char);
		void print_token();
};

// A token that represents a punctuation or separator
class punctuation_token : public base_token
{
	private:
		string punctuation_string;
	public:
		punctuation_token() : base_token(t_punctuation) { };
		int parse_token(fstream& stream, int input_char);
		void print_token();
};

// A token that represents whitespace
class whitespace_token : public base_token
{
	public:
		whitespace_token() : base_token(t_whitespace) { };
		int parse_token(fstream& stream, int input_char);
		void print_token();
};

// A token that represents an eol
class eol_token : public base_token
{
	public:
		eol_token() : base_token(t_eol) { };
		int parse_token(fstream& stream, int input_char);
		void print_token();
};

// A token that represents an indent
class indent_token : public base_token
{
    private:
        int indent_level;
	public:
		indent_token(int current_indent) : base_token(t_indent) {
            indent_level = current_indent;
        };
		int parse_token(fstream& stream, int input_char);
		void print_token();
};

// A token that represents an dedent
class dedent_token : public base_token
{
    private:
        int dedent_level;
	public:
		dedent_token(int current_indent) : base_token(t_dedent) {
            dedent_level = current_indent;
        };
		int parse_token(fstream& stream, int input_char);
		void print_token();
};

// A token that represents an eof
class eof_token : public base_token
{
	public:
		eof_token() : base_token(t_eof) { };
		int parse_token(fstream& stream, int input_char);
		void print_token();
};

// A token that represents an illegal character
class invalid_token : public base_token
{
	private:
		int invalid_character;
	public:
		invalid_token() : base_token(t_invalid_token) { };
		int parse_token(fstream& stream, int input_char);
		void print_token();
};

// The C++ token parser
class token_parser
{
	private:
		fstream& source_stream;
		list<base_token *> token_list;
	public:
		token_parser(fstream& stream) : source_stream(stream) { };
		bool parse_tokens();
		void print_tokens();
};

// parse the rest of a symbol
int symbol_token::parse_token(fstream& stream, int input_char) {
	symbol = input_char;
	while (true) {
		input_char = stream.get();
		if (isalpha(input_char) || isdigit(input_char) || input_char == '_') {
			symbol += input_char;
			continue;
		}
		return input_char;
	}
}

// print the token to cout
void symbol_token::print_token() {
	cout << "TOKEN[\"symbol\" , \"" << symbol << "\"]" << endl;
}

// parse the rest of an integer
int integer_token::parse_token(fstream& stream, int input_char) {
	integer_string = input_char;
	if (input_char == '0')
	{
		input_char = stream.peek();
		if (input_char == 'X' || input_char == 'x') {
			integer_string += input_char;
			input_char = stream.get();
			while (true) {
				input_char = stream.get();
				if (isxdigit(input_char)) {
					integer_string += input_char;
					continue;
				}
				return input_char;
			}
		}
	}
	while (true) {
		input_char = stream.get();
		if (isdigit(input_char)) {
			integer_string += input_char;
			continue;
		}
		return input_char;
	}
}

// print the token to cout
void integer_token::print_token() {
	cout << "TOKEN[\"integer\" , " << integer_string << "]" << endl;
}

// parse the rest of a literal
int literal_token::parse_token(fstream& stream, int input_char) {
	literal_string.clear();
	while (true) {
		input_char = stream.get();
		if (input_char == '\\') {
			input_char = stream.peek();
			if (input_char == '\"' || input_char == '\\') {
				literal_string += '\\';
				input_char = stream.get();
				literal_string += input_char;
				continue;
			}
			if (input_char == 0x0A) {
				cout << "error: EOL encountered before closing literal quotes" << endl;
				exit(0);
			}
			if (input_char == -1) {
				cout << "error: EOF encountered before closing literal quotes" << endl;
				exit(0);
			}
			literal_string += input_char;
			continue;
		}
		if (input_char != '\"' && input_char != -1) {
			literal_string += input_char;
			continue;
		}
		if (input_char == -1) {
			cout << "error: EOF encountered before closing literal quotes" << endl;
			exit(0);
		}
		input_char = stream.get();
		return input_char;
	}
}

// print the token to cout
void literal_token::print_token() {
	cout << "TOKEN[\"literal\" , \"" << literal_string << "\"]" << endl;
}

// parse the rest of a literal
int constant_token::parse_token(fstream& stream, int input_char) {
	constant_string.clear();
	while (true) {
		input_char = stream.get();
		if (input_char == '\\') {
			input_char = stream.peek();
			if (input_char == '\'' || input_char == '\\') {
				constant_string += '\\';
				input_char = stream.get();
				constant_string += input_char;
				continue;
			}
			constant_string += input_char;
			continue;
		}
		if (input_char != '\'') {
			constant_string += input_char;
			continue;
		}
		input_char = stream.get();
		return input_char;
	}
}

// print the token to cout
void constant_token::print_token() {
	cout << "TOKEN[\"constant literal\" , \"" << constant_string << "\"]" << endl;
}

// parse the rest of a punctuation sequence - this consists of
// ending up with either one, two or three characters in the
// punctuation string. NB: The sequence .. is accepted as a 
// punctuation token, but must be rejected by the compiler at
// some later stage.
int punctuation_token::parse_token(fstream& stream, int input_char) {
	punctuation_string = input_char;
	switch (input_char) {
	case '!': // Looking for either ! or !=
		input_char = stream.peek();
		if (input_char == '=') {
			input_char = stream.get();
			punctuation_string += input_char; // != token
		}
		break;
	case '#': // Looking for either # or ##
		input_char = stream.peek();
		if (input_char == '#') {
			input_char = stream.get();
			punctuation_string += input_char; // ## token
		}
		break;
	case '%': // Looking for either % or %=
		input_char = stream.peek();
		if (input_char == '=') {			
			input_char = stream.get();
			punctuation_string += input_char; // %= token
		}
		break;
	case '&': // Looking for either &, && or &=
		input_char = stream.peek();
		if (input_char == '&' || input_char == '=') {
			input_char = stream.get();
			punctuation_string += input_char; // && token
		}
		break;
	case '*': // Looking for either * or *=
		input_char = stream.peek();
		if (input_char == '=') {
			input_char = stream.get();
			punctuation_string += input_char; // *= token
		}
		break;
	case '+': // Looking for either +, ++, or +=
		input_char = stream.peek();
		if (input_char == '+' || input_char == '=') {
			input_char = stream.get();
			punctuation_string += input_char; // ++ or += token
		}
		break;
	case '-': // Looking for either -, --, -=, ->, ->*
		input_char = stream.peek();
		if (input_char == '-' || input_char == '=') {
			input_char = stream.get();
			punctuation_string += input_char; // -- or -= token
		}
		if (input_char == '>') {
			input_char = stream.get();
			punctuation_string += input_char; // -> token
			input_char = stream.peek();
			if (input_char == '*') {
				input_char = stream.get();
				punctuation_string += input_char; // ->* token
			}
		}
		break;
	case '.': // Looking for either ., .. or ...
		input_char = stream.peek();
		if (input_char == '.') {
			input_char = stream.get();
			punctuation_string += input_char; // .. token (illegal!)
			input_char = stream.peek();
			if (input_char == '.') {
				input_char = stream.get();
				punctuation_string += input_char; // ... token
			}
		}
		break;
	case '/': // Looking for either / or /=
		input_char = stream.peek();
		if (input_char == '=') {
			input_char = stream.get();
			punctuation_string += input_char; // /= token
		}
		break;
	case ':': // Looking for either : or ::
		input_char = stream.peek();
		if (input_char == ':') {
			input_char = stream.get();
			punctuation_string += input_char; // :: token
		}
		break;
	case '<': // Looking for either < or <=, <<, or <<=
		input_char = stream.peek();
		if (input_char == '=') {
			input_char = stream.get();
			punctuation_string += input_char; // <= token
			break;
		}
		if (input_char == '<') {
			input_char = stream.get();
			punctuation_string += input_char; // << token
			input_char = stream.peek();
			if (input_char == '=') {
				input_char = stream.get();
				punctuation_string += input_char; // <<= token
			}
		}
		break;
	case '=': // Looking for either = or ==
		input_char = stream.peek();
		if (input_char == '=') {
			input_char = stream.get();
			punctuation_string += input_char; // == token
		}
		break;
	case '>': // Looking for either >, >=, >>, or >>=
		input_char = stream.peek();
		if (input_char == '=') {
			input_char = stream.get();
			punctuation_string += input_char; // >= token
			break;
		}
		if (input_char == '>') {
			input_char = stream.get();
			punctuation_string += input_char; // >> token
			input_char = stream.peek();
			if (input_char == '=') {
				input_char = stream.get();
				punctuation_string += input_char; // >>= token
			}
		}
		break;
	case '|': // Looking for either |, |=, or || 
		input_char = stream.peek();
		if (input_char == '|' || input_char == '=') {
			input_char = stream.get();
			punctuation_string += input_char; // || or |= token
		}
		break;
	}
	input_char = stream.get();
	return input_char;
}

// print the token to cout
void punctuation_token::print_token() {
	cout << "TOKEN[\"punctuation\" , \"" << punctuation_string << "\"]" << endl;
}

// parse the whitespace characters
int whitespace_token::parse_token(fstream& stream, int input_char) {
	while (true) {
		input_char = stream.get();
		if (input_char == ' ' || input_char == 0x09 || input_char == 0x0B || input_char == 0x0D) {
			continue;
		}
		return input_char;
	}
}

// print the token to cout
void whitespace_token::print_token() {
	cout << "TOKEN[\"whitespace\" , \" \"]" << endl;
}

// parse the eol character
int eol_token::parse_token(fstream& stream, int input_char) {
	while (true) {
		input_char = stream.get();
		return input_char;
	}
}

// print the token to cout
void eol_token::print_token() {
	cout << "TOKEN[\"EOL\"]" << endl;
}

// parse the indent character
int indent_token::parse_token(fstream& stream, int input_char) {
	while (true) {
		input_char = stream.get();
		return input_char;
	}
}

// print the token to cout
void indent_token::print_token() {
	cout << "TOKEN[\"INDENT\": " << indent_level << "]" << endl;
}

// parse the dedent character
int dedent_token::parse_token(fstream& stream, int input_char) {
	while (true) {
		input_char = stream.get();
		return input_char;
	}
}

// print the token to cout
void dedent_token::print_token() {
	cout << "TOKEN[\"DEDENT\": " << dedent_level << "]" << endl;
}

// parse the eof character
int eof_token::parse_token(fstream& stream, int input_char) {
	return 0;
}

// print the token to cout
void eof_token::print_token(void) {
	cout << "TOKEN[\"EOF\"]" << endl;
}

// parse the invalid character
int invalid_token::parse_token(fstream& stream, int input_char) {
	invalid_character = input_char;
	input_char = stream.get();
	return input_char;
}

// print the token to cout
void invalid_token::print_token(void) {
	cout << "TOKEN[\"INVALID\"" << invalid_character << endl;
}

// parse the input source
bool token_parser::parse_tokens() {
	base_token * token;

    int current_indent = 0;

	while (!source_stream.eof()) {
		int input_char = source_stream.get();

		// Determine what the leading character is of the sequence,
		// create an appropriate token and get the actual token
		// class to parse the rest of it (if any)
		
		while (!source_stream.eof()) {
			// The following do loop is there only because I hate seeing
			// if () ... else if () ... else if () ... code!!!
			// Hence it's a do ... while ( false ) - single shot
			do
			{
				// Remove any comments from the source
				if (input_char == '#') {
					int peek_character = source_stream.peek();
					// if (peek_character == '/') {
						// Remove the line comment
                    while (peek_character != 0x0A && !source_stream.eof()) {
                        peek_character = source_stream.get();
                    }
                    token = new(nothrow) eol_token;
                    break;
					// }
					// if (peek_character == '*') {
					// 	// Remove a block comment
					// 	while (true) {
					// 		peek_character = source_stream.get();
					// 		if (peek_character == -1) {
					// 			cout << "error: block comment not terminated before EOF" << endl;
					// 			exit(0);
					// 		}
					// 		if (peek_character == 0x0A) {
					// 			token = new(nothrow) eol_token;
					// 			// Add the token to the end of the list
					// 			token_list.push_back(token);
					// 			continue;
					// 		}
					// 		if (peek_character == '*') {
					// 			peek_character = source_stream.get();
					// 			if (peek_character == -1) {
					// 				cout << "error: block comment not terminated before EOF" << endl;
					// 				exit(0);
					// 			}
					// 			if (peek_character == '/') {
					// 				// We need to ensure that a whitespace token
					// 				// is created to ensure /* */ in the middle
					// 				// of a source line is processed correctly.
					// 				input_char = source_stream.get();
					// 				input_char = ' ';
					// 				token = new(nothrow) whitespace_token;
					// 				break;
					// 			}
					// 		}
					// 	}
					// }
				}
				if (isalpha(input_char) || input_char == '_') {
					// Start of a symbol sequence
					token = new(nothrow) symbol_token;
					break;
				}
				if (input_char == 0x0A) {
					// EOL
                    int spaces = 0;
                    int p = source_stream.peek();
                    if (isspace(p)) {
                        int g = source_stream.get();
                        while (g != 0x0A && !source_stream.eof()) {
                            p = source_stream.peek();
                            if (isspace(p)) {
                                g = source_stream.get();
                                spaces++;
                            }
                            else {
                                break;
                            }
                        }
                    }
                    if (spaces > current_indent) {
                        current_indent = spaces;
                        token = new(nothrow) indent_token(current_indent);
                    }
                    else if (spaces < current_indent) {
                        current_indent = spaces;
                        token = new(nothrow) dedent_token(current_indent);
                    }
                    else {
                        token = new(nothrow) eol_token;
                    }
					break;
				}
				if (isspace(input_char)) {
					// Start of whitespace sequence
					token = new(nothrow) whitespace_token;
					break;
				}
				if (input_char == '\"') {
					// Start of literal sequence
					token = new(nothrow) literal_token;
					break;
				}
				if (input_char == '\'') {
					// Start of constant literal sequence
					token = new(nothrow) constant_token;
					break;
				}
				if (isdigit(input_char)) {
					// Start of number sequence
					token = new(nothrow) integer_token;
					break;
				}
				if (ispunct(input_char)) {
					// Start of punctuation sequence
					token = new(nothrow) punctuation_token;
					break;
				}
			}
			while (false);
			if (token == NULL) return false;
			input_char = token->parse_token(source_stream, input_char);
			// Add the token to the end of the list
			token_list.push_back(token);
			continue;
		}
	}
	// Add the EOF token to the end of the list
	token = new(nothrow) eof_token;
	token_list.push_back(token);
	return true;
}

// Simply iterate through the list of tokens and print them to cout
// Of course, get the token object to print itself :o)
void token_parser::print_tokens() {
	list<base_token *>::iterator iterator;
	iterator = token_list.begin();
	while(iterator != token_list.end()) {
		(*iterator)->print_token();
		++iterator;
	}
}

// main program entry point
int main(int argc, char** argv) {
	// Check to see that we have at least a filename
	if (argc < 2) {
		cout << "Invalid number of arguments. Filename is required." << endl;
        return -1;
	}
	string filename = argv[argc-1];

	fstream source;

	// Open the source file
	source.open(filename.c_str(), ios_base::in);
	if (source.fail()) {
		cout << "An error occurred while opening " << filename << endl;
        return -1;
	}

	// Create the token list
	token_parser parser(source);
	parser.parse_tokens();
	parser.print_tokens();
}