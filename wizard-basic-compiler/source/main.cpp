#include <string>
#include <map>
#include <list>
#include <stack>
#include <set>
#include <sstream>
#include <functional>
#include <algorithm>
#include <iostream>
#include <cstdlib>
#include <fstream>

// основано на http://sourceforge.net/p/predef/wiki/OperatingSystems/
#if \
	defined(__gnu_linux__) \
	|| defined(__linux__) \
	|| defined(linux) \
	|| defined(__linux)
	#define OS_LINUX
#elif \
	defined(_WIN32) \
	|| defined(_WIN64) \
	|| defined(__WIN32__) \
	|| defined(__TOS_WIN__) \
	|| defined(__WINDOWS__)
	#define OS_WINDOWS
#else
	#error: \
		an attempt to compile for an unsupported platform; \
		supports Linux and Windows x86 and x86-64.
#endif

enum MessageType {
	MESSAGE_INFORMATION,
	MESSAGE_ERROR
};
enum OperatorAssociativity {
	ASSOCIATIVITY_LEFT,
	ASSOCIATIVITY_RIGHT
};
enum FinalState {
	STATE_PREPROCESSED_CODE,
	STATE_LIBRARY_LIST,
	STATE_PROCEDURE_LIST,
	STATE_FUNCTION_LIST,
	STATE_VARIABLE_LIST,
	STATE_BYTE_CODE,
	STATE_ASSEMBLER_CODE,
	STATE_EXECUTABLE_FILE
};
struct CommandLineArguments {
	std::string base_path;
	FinalState final_state;
	std::string input_filename;

	CommandLineArguments(void) :
		base_path("."),
		final_state(STATE_EXECUTABLE_FILE)
	{}
};
typedef std::map<size_t, std::string> CodeLines;
typedef std::list<std::string> StringList;
enum TokenizeState {
	TOKENIZE_RESET,
	TOKENIZE_NUMBER,
	TOKENIZE_IDENTIFIER
};
typedef std::map<std::string, std::string> AliasMap;
struct ByteCodeMnemonic {
	size_t line_number;
	std::string mnemonic;
	std::string operand;

	ByteCodeMnemonic(
		size_t line_number,
		std::string mnemonic,
		std::string operand = ""
	) :
		line_number(line_number),
		mnemonic(mnemonic),
		operand(operand)
	{}
};
typedef std::list<ByteCodeMnemonic> ByteCode;
// флаги будут комбинироваться,
// поэтому должны иметь значения, равные степеням двойки,
// начиная с 0 (для обозначения отсутствия значения)
enum ExpressionResultFlag {
	EXPRESSION_NOTHING,
	EXPRESSION_WERE_CONVERTED,
	EXPRESSION_IS_RESULT
};
struct CompileExpressionResult {
	ByteCode byte_code;
	unsigned char flags;

	CompileExpressionResult() :
		flags(EXPRESSION_NOTHING)
	{}
};
typedef std::map<std::string, size_t> SubprogramMap;
typedef std::stack<size_t> NumberStack;
typedef std::set<std::string> VariableSet;
struct Library {
	std::string path;
	std::string name;
};
typedef std::list<Library> LibrariesList;
struct ByteCodeModule {
	ByteCode byte_code;
	VariableSet variables;
	SubprogramMap procedures;
	SubprogramMap functions;
	LibrariesList libraries;
};
typedef std::map<std::string, float> InbuildVariableMap;
typedef std::map<std::string, std::string> InbuildStringConstantMap;
enum BuildTarget {
	TARGET_ANY,
	TARGET_LINUX,
	TARGET_WINDOWS
};

template<typename Type>
std::string ConvertToString(Type value) {
	std::ostringstream out;
	out << value;

	return out.str();
}

template<typename Type>
Type ConvertFromString(const std::string& value) {
	Type result = Type();

	std::istringstream in(value);
	in >> result;

	return result;
}

std::string StringTrim(const std::string& string) {
	std::string string_copy = string;
	string_copy.erase(
		string_copy.begin(),
		std::find_if(
			string_copy.begin(),
			string_copy.end(),
			std::not1(std::ptr_fun<int, int>(std::isspace))
		)
	);
	string_copy.erase(
		std::find_if(
			string_copy.rbegin(),
			string_copy.rend(),
			std::not1(std::ptr_fun<int, int>(std::isspace))
		).base(),
		string_copy.end()
	);

	return string_copy;
}

void ShowMessage(
	const std::string& message,
	MessageType type = MESSAGE_INFORMATION
) {
	switch (type) {
		case MESSAGE_ERROR:
			std::cerr << message << '\n';
			break;
		case MESSAGE_INFORMATION:
		default:
			std::cout << message << '\n';
			break;
	}
}

void ShowCodeLines(const CodeLines& code_lines) {
	std::string result;
	for (
		CodeLines::const_iterator i = code_lines.begin();
		i != code_lines.end();
		++i
	) {
		result += ConvertToString(i->first) + " " + i->second + "\n";
	}
	if (!result.empty()) {
		result = result.substr(0, result.length() - 1);
	}

	ShowMessage(result);
}

void ShowLibraryList(const ByteCodeModule& byte_code_module) {
	std::string result;
	for (
		LibrariesList::const_iterator i =
			byte_code_module.libraries.begin();
		i != byte_code_module.libraries.end();
		++i
	) {
		result += i->path + " " + i->name + "\n";
	}
	if (!result.empty()) {
		result = result.substr(0, result.length() - 1);
	}

	ShowMessage(result);
}

void ShowProcedureList(const ByteCodeModule& byte_code_module) {
	std::string result;
	for (
		SubprogramMap::const_iterator j =
			byte_code_module.procedures.begin();
		j != byte_code_module.procedures.end();
		++j
	) {
		result += j->first + " " + ConvertToString(j->second)+ "\n";
	}
	if (!result.empty()) {
		result = result.substr(0, result.length() - 1);
	}

	ShowMessage(result);
}

void ShowFunctionList(const ByteCodeModule& byte_code_module) {
	std::string result;
	for (
		SubprogramMap::const_iterator j =
			byte_code_module.functions.begin();
		j != byte_code_module.functions.end();
		++j
	) {
		result += j->first + " " + ConvertToString(j->second) + "\n";
	}
	if (!result.empty()) {
		result = result.substr(0, result.length() - 1);
	}

	ShowMessage(result);
}

void ShowVariableList(const ByteCodeModule& byte_code_module) {
	std::string result;
	for (
		VariableSet::const_iterator k = byte_code_module.variables.begin();
		k != byte_code_module.variables.end();
		++k
	) {
		result += *k + "\n";
	}
	if (!result.empty()) {
		result = result.substr(0, result.length() - 1);
	}

	ShowMessage(result);
}

void ShowByteCode(const ByteCodeModule& byte_code_module) {
	std::string result;
	for (
		ByteCode::const_iterator m = byte_code_module.byte_code.begin();
		m != byte_code_module.byte_code.end();
		++m
	) {
		result +=
			ConvertToString(m->line_number)
			+ " "
			+ m->mnemonic
			+ " "
			+ m->operand
			+ "\n";
	}
	if (!result.empty()) {
		result = result.substr(0, result.length() - 1);
	}

	ShowMessage(result);
}

void ProcessError(const std::string& description) {
	ShowMessage("Error! " + description, MESSAGE_ERROR);
	std::exit(EXIT_FAILURE);
}

bool IsNumber(const std::string& string) {
	std::ptrdiff_t number_of_digits = std::count_if(
		string.begin(),
		string.end(),
		std::ptr_fun<int, int>(std::isdigit)
	);
	std::ptrdiff_t number_of_dots = std::count_if(
		string.begin(),
		string.end(),
		std::bind1st(std::equal_to<char>(), '.')
	);

	return
		number_of_digits != 0
		&& (number_of_dots == 0 || number_of_dots == 1)
		&& string[0] != '.'
		&& string[string.size() - 1] != '.'
		&& static_cast<std::size_t>(number_of_digits + number_of_dots)
			== string.length();
}

bool IsString(const std::string& string) {
	return
		string.size() >= 2
		&& string[0] == '"'
		&& string[string.size() - 1] == '"';
}

bool IsIdentifier(const std::string& string) {
	std::ptrdiff_t number_of_digits = std::count_if(
		string.begin(),
		string.end(),
		std::ptr_fun<int, int>(std::isdigit)
	);
	std::ptrdiff_t number_of_letters = std::count_if(
		string.begin(),
		string.end(),
		std::ptr_fun<int, int>(std::isalpha)
	);
	std::ptrdiff_t number_of_underscores = std::count_if(
		string.begin(),
		string.end(),
		std::bind1st(std::equal_to<char>(), '_')
	);

	return
		(number_of_letters != 0 || number_of_underscores != 0)
		&& !std::isdigit(string[0])
		&& static_cast<std::size_t>(
			number_of_digits + number_of_letters + number_of_underscores
		) == string.length();
}

bool IsOperator(const std::string& string) {
	return
		string == "!"
		|| string == "*"
		|| string == "/"
		|| string == "+"
		|| string == "-"
		|| string == "$"
		|| string == "<"
		|| string == ">"
		|| string == "="
		|| string == "&"
		|| string == "|";
}

OperatorAssociativity GetAssociativity(const std::string& string) {
	if (string != "!") {
		return ASSOCIATIVITY_LEFT;
	} else {
		return ASSOCIATIVITY_RIGHT;
	}
}

size_t GetPrecedence(const std::string& string) {
	if (string == "!") {
		return 6;
	} else if (string == "*" || string == "/") {
		return 5;
	} else if (string == "+" || string == "-" || string == "$") {
		return 4;
	} else if (string == "<" || string == ">") {
		return 3;
	} else if (string == "=") {
		return 2;
	} else if (string == "&") {
		return 1;
	} else {
		return 0;
	}
}

void ShowVersion(void) {
	ShowMessage(
		"Wizard BASIC compiler, v1.0\n"
		"(c) thewizardplusplus, 2014"
	);
}

void ShowShortHelp(void) {
	ShowMessage(
		"Usage:\n"
			"\twbc [option] [input_filename]\n"
		"\n"
		"Options:\n"
			"\t-v, --version - show version;\n"
			"\t-h, --help - show help;\n"
			"\t--final-state=final_state_name - set final state, "
				"where final_state_name is:\n"
				"\t\tpreprocessed-code - output preprocessed code;\n"
				"\t\tlibrary-list - output library list;\n"
				"\t\tprocedure-list - output procedure list;\n"
				"\t\tfunction-list - output function list;\n"
				"\t\tvariable-list - output variable list;\n"
				"\t\tbyte-code - output byte code;\n"
				"\t\tassembler-code - output assembler code.\n"
		"\n"
		"Arguments:\n"
			"\tinput_filename - program source code filename."
	);
}

void ShowHelp(void) {
	ShowVersion();
	ShowMessage("");
	ShowShortHelp();
}

CommandLineArguments ProcessCommandLineArguments(
	int number_of_arguments,
	char* arguments[]
) {
	CommandLineArguments command_line_arguments;
	std::string base_path = arguments[0];
	#ifdef OS_LINUX
		size_t last_separator_index = base_path.find_last_of('/');
	#elif defined(OS_WINDOWS)
		size_t last_separator_index = base_path.find_last_of('\\');
	#endif
	if (last_separator_index != std::string::npos) {
		base_path = base_path.substr(0, last_separator_index + 1);
	} else {
		#ifdef OS_LINUX
			base_path = "./";
		#elif defined(OS_WINDOWS)
			base_path = ".\\";
		#endif
	}
	command_line_arguments.base_path = base_path;

	if (number_of_arguments != 2 && number_of_arguments != 3) {
		ShowMessage(
			"Error! Invalid number of arguments. Expected 1 or 2 arguments.\n"
		);
		ShowShortHelp();

		std::exit(EXIT_FAILURE);
	} else {
		std::string argument = arguments[1];
		if (argument == "-v" || argument == "--version") {
			ShowVersion();
			std::exit(EXIT_SUCCESS);
		} else if (argument == "-h" || argument == "--help") {
			ShowHelp();
			std::exit(EXIT_SUCCESS);
		} else if (argument.substr(0, 14) == "--final-state=") {
			if (number_of_arguments != 3) {
				ShowMessage("Error! Missed input filename.\n");
				ShowShortHelp();

				std::exit(EXIT_FAILURE);
			}

			std::string final_state = argument.substr(14);
			if (final_state == "preprocessed-code") {
				command_line_arguments.final_state = STATE_PREPROCESSED_CODE;
			} else if (final_state == "library-list") {
				command_line_arguments.final_state = STATE_LIBRARY_LIST;
			} else if (final_state == "procedure-list") {
				command_line_arguments.final_state = STATE_PROCEDURE_LIST;
			} else if (final_state == "function-list") {
				command_line_arguments.final_state = STATE_FUNCTION_LIST;
			} else if (final_state == "variable-list") {
				command_line_arguments.final_state = STATE_VARIABLE_LIST;
			} else if (final_state == "byte-code") {
				command_line_arguments.final_state = STATE_BYTE_CODE;
			} else if (final_state == "assembler-code") {
				command_line_arguments.final_state = STATE_ASSEMBLER_CODE;
			} else {
				ShowMessage(
					"Error! Unknown final state \"" + final_state + "\".\n"
				);
				ShowShortHelp();

				std::exit(EXIT_FAILURE);
			}

			command_line_arguments.input_filename = arguments[2];
		} else {
			command_line_arguments.input_filename = argument;
		}
	}

	return command_line_arguments;
}

CodeLines FileRead(const std::string& filename) {
	std::ifstream in(filename.c_str());
	if (!in) {
		ProcessError("Unable to open input file \"" + filename + "\".");
	}

	CodeLines code_lines;
	size_t line_number = 0;
	while (in) {
		std::string buffer;
		std::getline(in, buffer);

		code_lines[line_number++] = buffer;
	}

	return code_lines;
}

void FileWrite(const std::string& filename, const std::string& text) {
	std::ofstream out(filename.c_str());
	if (!out) {
		ProcessError("Unable to open output file \"" + filename + "\".");
	}

	out << text;
}

CodeLines Preprocess(const CodeLines& code_lines) {
	CodeLines preprocessed_code_lines;
	for (
		CodeLines::const_iterator i = code_lines.begin();
		i != code_lines.end();
		++i
	) {
		std::string code_line = StringTrim(i->second);
		if (
			code_line.empty()
			|| code_line.substr(0, 5) == "note "
			|| code_line == "note"
		) {
			continue;
		}

		preprocessed_code_lines[i->first] = code_line;
	}

	return preprocessed_code_lines;
}

StringList Tokenize(const std::string& string, size_t line_number) {
	TokenizeState state = TOKENIZE_RESET;
	std::string accumulator;
	StringList tokens;
	for (size_t i = 0; i < string.size(); ++i) {
		char symbol = string[i];
		std::string symbol_as_string = std::string(1, symbol);
		if (std::isspace(symbol)) {
			continue;
		} else if (
			IsOperator(symbol_as_string)
			|| symbol == '('
			|| symbol == ')'
			|| symbol == ','
		) {
			if (!accumulator.empty()) {
				if (
					state == TOKENIZE_NUMBER
					&& accumulator[accumulator.size() - 1] == '.'
				) {
					ProcessError(
						"Unfinished number on line "
						+ ConvertToString(line_number)
						+ "."
					);
				}

				tokens.push_back(accumulator);
				accumulator.clear();
			}

			state = TOKENIZE_RESET;
			tokens.push_back(symbol_as_string);
		} else if (std::isdigit(symbol)) {
			if (state == TOKENIZE_RESET) {
				state = TOKENIZE_NUMBER;
			}
			accumulator += symbol;
		} else if (symbol == '.' && state == TOKENIZE_NUMBER) {
			accumulator += symbol;
		} else if (
			(std::isalpha(symbol) || symbol == '_')
			&& (state == TOKENIZE_RESET || state == TOKENIZE_IDENTIFIER)
		) {
			if (state == TOKENIZE_RESET) {
				state = TOKENIZE_IDENTIFIER;
			}
			accumulator += symbol;
		} else {
			ProcessError(
				"Unexpected symbol \""
				+ symbol_as_string
				+ "\" in expression on line "
				+ ConvertToString(line_number)
				+ "."
			);
		}
	}

	if (!accumulator.empty()) {
		if (
			state == TOKENIZE_NUMBER
			&& accumulator[accumulator.size() - 1] == '.'
		) {
			ProcessError(
				"Unfinished number on line "
				+ ConvertToString(line_number)
				+ "."
			);
		}

		tokens.push_back(accumulator);
	}

	return tokens;
}

std::string CorrectSubprogramName(const std::string& name) {
	#ifdef OS_LINUX
		return name;
	#elif defined(OS_WINDOWS)
		if (name != "c_string") {
			return "_" + name;
		} else {
			return name;
		}
	#endif
}

std::string GetSubprogramNameByAlias(const std::string& alias) {
	AliasMap alias_map;
	alias_map["!"] = CorrectSubprogramName("NumberNot");
	alias_map["*"] = CorrectSubprogramName("NumberMul");
	alias_map["/"] = CorrectSubprogramName("NumberDiv");
	alias_map["+"] = CorrectSubprogramName("NumberAdd");
	alias_map["-"] = CorrectSubprogramName("NumberSub");
	alias_map["$"] = CorrectSubprogramName("ArrayAppend");
	alias_map["<"] = CorrectSubprogramName("NumberLs");
	alias_map[">"] = CorrectSubprogramName("NumberGt");
	alias_map["="] = CorrectSubprogramName("NumberEq");
	alias_map["&"] = CorrectSubprogramName("NumberAnd");
	alias_map["|"] = CorrectSubprogramName("NumberOr");
	alias_map["ToString"] = CorrectSubprogramName("ArrayCreateFromNumber");
	alias_map["ToNumber"] = CorrectSubprogramName("ArrayConvertToNumber");

	if (alias_map.count(alias) == 1) {
		return alias_map[alias];
	} else {
		return CorrectSubprogramName(alias);
	}
}

CompileExpressionResult CompileExpression(
	const StringList& tokens,
	const SubprogramMap& procedures,
	SubprogramMap& functions,
	size_t line_number
) {
	CompileExpressionResult result;
	StringList stack;
	NumberStack argument_counter_stack;
	bool push_operation_were = false;
	bool conversion_is_last = false;
	size_t procedures_counter = 0;

	for (
		StringList::const_iterator i = tokens.begin();
		i != tokens.end();
		++i
	) {
		std::string token = *i;
		if (IsNumber(token)) {
			result.byte_code.push_back(
				ByteCodeMnemonic(line_number, "push_n", token)
			);
			result.flags |= EXPRESSION_IS_RESULT;

			push_operation_were = true;
		} else if (IsIdentifier(token)) {
			std::string subprogram_name = GetSubprogramNameByAlias(token);
			if (
				!IsOperator(token)
				&& (procedures.count(subprogram_name) == 1
				|| functions.count(subprogram_name) == 1)
			) {
				stack.push_back(token);
				argument_counter_stack.push(0);
			} else {
				result.byte_code.push_back(
					ByteCodeMnemonic(line_number, "push_v", token)
				);
				result.flags |= EXPRESSION_IS_RESULT;

				push_operation_were = true;
			}
		} else if (token == ",") {
			while (true) {
				if (stack.empty()) {
					ProcessError(
						"Missed subprogram argument separator "
							"or opening parenthesis on line "
						+ ConvertToString(line_number)
						+ "."
					);
				}

				std::string token_from_stack = stack.back();
				if (token_from_stack != "(") {
					std::string subprogram_name = GetSubprogramNameByAlias(
						token_from_stack
					);
					result.byte_code.push_back(
						ByteCodeMnemonic(line_number, "call", subprogram_name)
					);
					stack.pop_back();

					if (
						!IsOperator(token_from_stack)
						&& procedures.count(subprogram_name) == 1
					) {
						result.flags &= ~EXPRESSION_IS_RESULT;
						procedures_counter++;
					} else {
						result.flags |= EXPRESSION_IS_RESULT;
						push_operation_were = true;
					}

					conversion_is_last = false;
				} else {
					break;
				}
			}

			if (!argument_counter_stack.empty()) {
				if (push_operation_were) {
					argument_counter_stack.top()++;
					push_operation_were = false;
				}
			} else {
				ProcessError(
					"Subprogram argument separator without subprogram on line "
					+ ConvertToString(line_number)
					+ "."
				);
			}
		} else if (IsOperator(token)) {
			while (true) {
				if (stack.empty()) {
					break;
				}

				std::string token_from_stack = stack.back();
				if (!IsOperator(token_from_stack)) {
					break;
				}

				if (
					(GetAssociativity(token) == ASSOCIATIVITY_LEFT
					&& GetPrecedence(token) <= GetPrecedence(token_from_stack))
					|| (GetAssociativity(token) == ASSOCIATIVITY_RIGHT
					&& GetPrecedence(token) < GetPrecedence(token_from_stack))
				) {
					std::string subprogram_name = GetSubprogramNameByAlias(
						token_from_stack
					);
					result.byte_code.push_back(
						ByteCodeMnemonic(line_number, "call", subprogram_name)
					);
					stack.pop_back();

					result.flags |= EXPRESSION_IS_RESULT;
					push_operation_were = true;
					conversion_is_last = false;
				} else {
					break;
				}
			}

			stack.push_back(token);
		} else if (token == "(") {
			stack.push_back(token);
		} else if (token == ")") {
			while (true) {
				if (stack.empty()) {
					ProcessError(
						"Missed opening bracket on line "
						+ ConvertToString(line_number)
						+ "."
					);
				}

				std::string token_from_stack = stack.back();
				stack.pop_back();
				if (token_from_stack != "(") {
					std::string subprogram_name = GetSubprogramNameByAlias(
						token_from_stack
					);
					result.byte_code.push_back(
						ByteCodeMnemonic(line_number, "call", subprogram_name)
					);

					if (
						!IsOperator(token_from_stack)
						&& procedures.count(subprogram_name) == 1
					) {
						result.flags &= ~EXPRESSION_IS_RESULT;
						procedures_counter++;
					} else {
						result.flags |= EXPRESSION_IS_RESULT;
						push_operation_were = true;
					}

					conversion_is_last = false;
				} else {
					break;
				}
			}

			if (!argument_counter_stack.empty() && push_operation_were) {
				argument_counter_stack.top()++;
				push_operation_were = false;
			}

			if (!stack.empty()) {
				std::string token_from_stack = stack.back();
				std::string subprogram_name = GetSubprogramNameByAlias(
					token_from_stack
				);
				if (
					!IsOperator(token_from_stack)
					&& (procedures.count(subprogram_name) == 1
					|| functions.count(subprogram_name) == 1)
				) {
					size_t expected_argument_number = 0;
					if (procedures.count(subprogram_name) == 1) {
						expected_argument_number = procedures.find(
							subprogram_name
						)->second;
					} else if (functions.count(subprogram_name) == 1) {
						expected_argument_number = functions.find(
							subprogram_name
						)->second;
					}

					size_t real_argument_number = argument_counter_stack.top();
					if (real_argument_number != expected_argument_number) {
						ProcessError(
							"Invalid number of arguments for subprogram \""
							+ token_from_stack
							+ "\" (expected "
							+ ConvertToString(expected_argument_number)
							+ " argument"
							+ (expected_argument_number != 1 ? "s": "")
							+ ", but gots "
							+ ConvertToString(real_argument_number)
							+ ") on line "
							+ ConvertToString(line_number)
							+ "."
						);
					}

					if (subprogram_name != "c_string") {
						result.byte_code.push_back(
							ByteCodeMnemonic(
								line_number,
								"call",
								subprogram_name
							)
						);

						if (procedures.count(subprogram_name) == 1) {
							result.flags &= ~EXPRESSION_IS_RESULT;
							procedures_counter++;
						} else {
							result.flags |= EXPRESSION_IS_RESULT;
							push_operation_were = true;
						}

						conversion_is_last = false;
					} else {
						result.byte_code.push_back(
							ByteCodeMnemonic(line_number, "to_str")
						);
						result.flags |=
							EXPRESSION_WERE_CONVERTED
							| EXPRESSION_IS_RESULT;

						push_operation_were = true;
						conversion_is_last = true;
					}

					stack.pop_back();
					argument_counter_stack.pop();
				}
			}
		} else {
			ProcessError(
				"Unknown expression token \""
				+ token
				+ "\" on line "
				+ ConvertToString(line_number)
				+ "."
			);
		}
	}

	while (!stack.empty()) {
		std::string token_from_stack = stack.back();
		if (token_from_stack == "(") {
			ProcessError(
				"Missed closing bracket on line "
				+ ConvertToString(line_number)
				+ "."
			);
		}

		std::string subprogram_name = GetSubprogramNameByAlias(
			token_from_stack
		);
		result.byte_code.push_back(
			ByteCodeMnemonic(line_number, "call", subprogram_name)
		);
		stack.pop_back();

		if (procedures.count(subprogram_name) == 1) {
			result.flags &= ~EXPRESSION_IS_RESULT;
			procedures_counter++;
		} else {
			result.flags |= EXPRESSION_IS_RESULT;
		}

		conversion_is_last = false;
	}

	if (conversion_is_last) {
		ProcessError(
			"Illegal conversion on line "
			+ ConvertToString(line_number)
			+ "."
		);
	}
	if (
		procedures_counter > 1
		|| (procedures_counter == 1 && result.flags & EXPRESSION_IS_RESULT)
	) {
		ProcessError(
			"Using result of procedure on line "
			+ ConvertToString(line_number)
			+ "."
		);
	}

	return result;
}

ByteCodeModule Compile(
	const CodeLines& code_lines,
	const InbuildVariableMap& inbuild_variables,
	const InbuildStringConstantMap& inbuild_string_constants
) {
	ByteCodeModule byte_code_module;
	for (
		InbuildVariableMap::const_iterator i = inbuild_variables.begin();
		i != inbuild_variables.end();
		++i
	) {
		byte_code_module.variables.insert(i->first);
	}

	for (
		InbuildStringConstantMap::const_iterator j = inbuild_string_constants.
			begin();
		j != inbuild_string_constants.end();
		++j
	) {
		byte_code_module.variables.insert(j->first);
	}
	byte_code_module.variables.insert("APP_PATH");

	byte_code_module.functions[CorrectSubprogramName("c_string")] = 1;
	byte_code_module.functions[CorrectSubprogramName("ArrayAppend")] = 2;
	byte_code_module.functions[CorrectSubprogramName("ArrayConvertToNumber")] =
		1;
	byte_code_module.functions[CorrectSubprogramName("ArrayCreate")] = 1;
	byte_code_module.functions[CorrectSubprogramName("ArrayCreateFromNumber")] =
		1;
	byte_code_module.functions[CorrectSubprogramName("ArrayCreateFromString")] =
		1;
	byte_code_module.functions[CorrectSubprogramName("ArrayGetSize")] = 1;
	byte_code_module.functions[CorrectSubprogramName("ArrayItemGet")] = 2;
	byte_code_module.functions[CorrectSubprogramName("FileOpen")] = 2;
	byte_code_module.functions[CorrectSubprogramName("FileRead")] = 2;
	byte_code_module.functions[CorrectSubprogramName("FileReadAll")] = 1;
	byte_code_module.functions[CorrectSubprogramName("GetOs")] = 0;
	byte_code_module.functions[CorrectSubprogramName("MathsAngle")] = 2;
	byte_code_module.functions[CorrectSubprogramName("MathsArccos")] = 1;
	byte_code_module.functions[CorrectSubprogramName("MathsArcsin")] = 1;
	byte_code_module.functions[CorrectSubprogramName("MathsArctg")] = 1;
	byte_code_module.functions[CorrectSubprogramName("MathsCos")] = 1;
	byte_code_module.functions[CorrectSubprogramName("MathsExp")] = 1;
	byte_code_module.functions[CorrectSubprogramName("MathsIntegral")] = 1;
	byte_code_module.functions[CorrectSubprogramName("MathsLg")] = 1;
	byte_code_module.functions[CorrectSubprogramName("MathsLn")] = 1;
	byte_code_module.functions[CorrectSubprogramName("MathsModulus")] = 1;
	byte_code_module.functions[CorrectSubprogramName("MathsPower")] = 2;
	byte_code_module.functions[CorrectSubprogramName("MathsRandom")] = 2;
	byte_code_module.functions[CorrectSubprogramName("MathsSin")] = 1;
	byte_code_module.functions[CorrectSubprogramName("MathsSquareRoot")] = 1;
	byte_code_module.functions[CorrectSubprogramName("MathsTg")] = 1;
	byte_code_module.functions[CorrectSubprogramName("NumberAdd")] = 2;
	byte_code_module.functions[CorrectSubprogramName("NumberAnd")] = 2;
	byte_code_module.functions[CorrectSubprogramName("NumberDiv")] = 2;
	byte_code_module.functions[CorrectSubprogramName("NumberEq")] = 2;
	byte_code_module.functions[CorrectSubprogramName("NumberGt")] = 2;
	byte_code_module.functions[CorrectSubprogramName("NumberLs")] = 2;
	byte_code_module.functions[CorrectSubprogramName("NumberMul")] = 2;
	byte_code_module.functions[CorrectSubprogramName("NumberNot")] = 1;
	byte_code_module.functions[CorrectSubprogramName("NumberOr")] = 2;
	byte_code_module.functions[CorrectSubprogramName("NumberSub")] = 2;
	byte_code_module.functions[
		CorrectSubprogramName("TimerGetElapsedTimeInUs")
	] = 0;

	byte_code_module.procedures[
		CorrectSubprogramName("ArrayClearMemoryAfterConvertsToStrings")
	] = 0;
	byte_code_module.procedures[CorrectSubprogramName("ArrayDelete")] = 1;
	byte_code_module.procedures[CorrectSubprogramName("ArrayDeleteAll")] = 0;
	byte_code_module.procedures[CorrectSubprogramName("ArrayItemAppend")] = 2;
	byte_code_module.procedures[CorrectSubprogramName("ArrayItemInsert")] = 3;
	byte_code_module.procedures[CorrectSubprogramName("ArrayItemRemove")] = 2;
	byte_code_module.procedures[CorrectSubprogramName("ArrayItemSet")] = 3;
	byte_code_module.procedures[CorrectSubprogramName("Exit")] = 0;
	byte_code_module.procedures[CorrectSubprogramName("FileClose")] = 1;
	byte_code_module.procedures[CorrectSubprogramName("FileCloseAll")] = 0;
	byte_code_module.procedures[CorrectSubprogramName("FileRemove")] = 1;
	byte_code_module.procedures[CorrectSubprogramName("FileWrite")] = 2;
	byte_code_module.procedures[CorrectSubprogramName("Show")] = 1;
	byte_code_module.procedures[CorrectSubprogramName("TimerStart")] = 0;

	size_t condition_counter = 0;
	NumberStack condition_id_stack;
	for (
		CodeLines::const_iterator k = code_lines.begin();
		k != code_lines.end();
		++k
	) {
		size_t line_number = k->first;
		std::string code_line = k->second;
		bool new_condition = false;

		if (code_line.substr(0, 4) == "use ") {
			std::string path = StringTrim(code_line.substr(4));
			if (!IsString(path) || path == "\"\"") {
				ProcessError(
					"Invalid format of path on line "
					+ ConvertToString(line_number)
					+ "."
				);
			}
			path = path.substr(1, path.size() - 2);

			BuildTarget build_target = TARGET_ANY;
			if (path.substr(0, 6) == "linux:") {
				build_target = TARGET_LINUX;
				path = path.substr(6);
			} else if (path.substr(0, 8) == "windows:") {
				build_target = TARGET_WINDOWS;
				path = path.substr(8);
			}

			#ifdef OS_LINUX
				bool is_os_matched =
					build_target == TARGET_ANY
					|| build_target == TARGET_LINUX;
			#elif defined(OS_WINDOWS)
				bool is_os_matched =
					build_target == TARGET_ANY
					|| build_target == TARGET_WINDOWS;
			#endif
			if (is_os_matched) {
				Library library;
				#ifdef OS_LINUX
					size_t last_separator_index = path.find_last_of('/');
				#elif defined(OS_WINDOWS)
					size_t last_separator_index = path.find_last_of('\\');
				#endif
				if (last_separator_index != std::string::npos) {
					library.path = path.substr(0, ++last_separator_index);
				} else {
					last_separator_index = 0;
				}

				library.name = path.substr(last_separator_index);
				if (library.name.empty()) {
					ProcessError(
						"Invalid format of path on line "
						+ ConvertToString(line_number)
						+ "."
					);
				}

				size_t library_name_length = library.name.length();
				size_t suffix_begin_index = library_name_length - 2;
				if (
					library_name_length > 5
					&& library.name.substr(0, 3) == "lib"
					&& library.name.substr(suffix_begin_index) == ".a"
				) {
					library.name = library.name.substr(
						3,
						suffix_begin_index - 3
					);
				}

				byte_code_module.libraries.push_back(library);
			}
		} else if (code_line.substr(0, 10) == "procedure ") {
			size_t separator_index = code_line.find('/');
			if (separator_index == std::string::npos) {
				ProcessError(
					"Separator not found on line "
					+ ConvertToString(line_number)
					+ "."
				);
			}

			std::string identifier = StringTrim(
				code_line.substr(10, separator_index - 10)
			);
			if (!IsIdentifier(identifier)) {
				ProcessError(
					"Invalid identifier \""
					+ identifier
					+ "\" on line "
					+ ConvertToString(line_number)
					+ "."
				);
			}

			std::string arguments_number = StringTrim(
				code_line.substr(separator_index + 1)
			);
			byte_code_module.procedures[CorrectSubprogramName(identifier)] =
				ConvertFromString<size_t>(arguments_number);
		} else if (code_line.substr(0, 9) == "function ") {
			size_t separator_index = code_line.find('/');
			if (separator_index == std::string::npos) {
				ProcessError(
					"Separator not found on line "
					+ ConvertToString(line_number)
					+ "."
				);
			}

			std::string identifier = StringTrim(
				code_line.substr(9, separator_index - 9)
			);
			if (!IsIdentifier(identifier)) {
				ProcessError(
					"Invalid identifier \""
					+ identifier
					+ "\" on line "
					+ ConvertToString(line_number)
					+ "."
				);
			}

			std::string arguments_number = StringTrim(
				code_line.substr(separator_index + 1)
			);
			byte_code_module.functions[CorrectSubprogramName(identifier)] =
				ConvertFromString<size_t>(arguments_number);
		} else if (code_line.substr(0, 7) == "string ") {
			size_t separator_index = code_line.find('=');
			if (separator_index == std::string::npos) {
				ProcessError(
					"Separator not found on line "
					+ ConvertToString(line_number)
					+ "."
				);
			}

			std::string string = StringTrim(
				code_line.substr(separator_index + 1)
			);
			if (!IsString(string)) {
				ProcessError(
					"Invalid format of string on line "
					+ ConvertToString(line_number)
					+ "."
				);
			}
			byte_code_module.byte_code.push_back(
				ByteCodeMnemonic(line_number, "push_s", string)
			);

			std::string identifier = StringTrim(
				code_line.substr(7, separator_index - 7)
			);
			if (!IsIdentifier(identifier)) {
				ProcessError(
					"Invalid identifier \""
					+ identifier
					+ "\" on line "
					+ ConvertToString(line_number)
					+ "."
				);
			}
			byte_code_module.variables.insert(identifier);
			byte_code_module.byte_code.push_back(
				ByteCodeMnemonic(line_number, "pop", identifier)
			);
		} else if (code_line.substr(0, 4) == "let ") {
			size_t separator_index = code_line.find('=');
			if (separator_index == std::string::npos) {
				ProcessError(
					"Separator not found on line "
					+ ConvertToString(line_number)
					+ "."
				);
			}

			std::string expression = StringTrim(
				code_line.substr(separator_index + 1)
			);
			StringList tokens = Tokenize(expression, line_number);
			CompileExpressionResult compile_expression_result =
				CompileExpression(
					tokens,
					byte_code_module.procedures,
					byte_code_module.functions,
					line_number
				);
			if (!(compile_expression_result.flags & EXPRESSION_IS_RESULT)) {
				ProcessError(
					"Using result of procedure on line "
					+ ConvertToString(line_number)
					+ "."
				);
			}
			byte_code_module.byte_code.insert(
				byte_code_module.byte_code.end(),
				compile_expression_result.byte_code.begin(),
				compile_expression_result.byte_code.end()
			);
			if (compile_expression_result.flags & EXPRESSION_WERE_CONVERTED) {
				byte_code_module.byte_code.push_back(
					ByteCodeMnemonic(
						line_number,
						"call",
						GetSubprogramNameByAlias(
							"ArrayClearMemoryAfterConvertsToStrings"
						)
					)
				);
			}

			std::string identifier = StringTrim(
				code_line.substr(4, separator_index - 4)
			);
			if (!IsIdentifier(identifier)) {
				ProcessError(
					"Invalid identifier \""
					+ identifier
					+ "\" on line "
					+ ConvertToString(line_number)
					+ "."
				);
			}
			byte_code_module.variables.insert(identifier);
			byte_code_module.byte_code.push_back(
				ByteCodeMnemonic(line_number, "pop", identifier)
			);
		} else if (code_line.substr(0, 6) == "label ") {
			std::string identifier = StringTrim(code_line.substr(6));
			if (!IsIdentifier(identifier)) {
				ProcessError(
					"Invalid identifier \""
					+ identifier
					+ "\" on line "
					+ ConvertToString(line_number)
					+ "."
				);
			}
			byte_code_module.byte_code.push_back(
				ByteCodeMnemonic(line_number, "lbl", identifier)
			);
		} else if (code_line.substr(0, 6) == "go to ") {
			std::string identifier = StringTrim(code_line.substr(6));
			if (!IsIdentifier(identifier)) {
				ProcessError(
					"Invalid identifier \""
					+ identifier
					+ "\" on line "
					+ ConvertToString(line_number)
					+ "."
				);
			}
			byte_code_module.byte_code.push_back(
				ByteCodeMnemonic(line_number, "jmp", identifier)
			);
		} else if (code_line.substr(0, 3) == "if ") {
			size_t separator_index = code_line.find("then");
			if (separator_index == std::string::npos) {
				ProcessError(
					"Separator not found on line "
					+ ConvertToString(line_number)
					+ "."
				);
			}

			std::string expression = StringTrim(
				code_line.substr(3, separator_index - 3)
			);
			StringList tokens = Tokenize(expression, line_number);
			CompileExpressionResult compile_expression_result =
				CompileExpression(
					tokens,
					byte_code_module.procedures,
					byte_code_module.functions,
					line_number
				);
			if (!(compile_expression_result.flags & EXPRESSION_IS_RESULT)) {
				ProcessError(
					"Using result of procedure on line "
					+ ConvertToString(line_number)
					+ "."
				);
			}
			byte_code_module.byte_code.insert(
				byte_code_module.byte_code.end(),
				compile_expression_result.byte_code.begin(),
				compile_expression_result.byte_code.end()
			);
			if (compile_expression_result.flags & EXPRESSION_WERE_CONVERTED) {
				byte_code_module.byte_code.push_back(
					ByteCodeMnemonic(
						line_number,
						"call",
						GetSubprogramNameByAlias(
							"ArrayClearMemoryAfterConvertsToStrings"
						)
					)
				);
			}

			size_t current_condition_counter = condition_counter++;
			condition_id_stack.push(current_condition_counter);
			byte_code_module.byte_code.push_back(
				ByteCodeMnemonic(
					line_number,
					"je",
					"end_of_condition"
						+ ConvertToString(current_condition_counter)
				)
			);

			new_condition = true;
		} else {
			StringList tokens = Tokenize(code_line, line_number);
			CompileExpressionResult compile_expression_result =
				CompileExpression(
					tokens,
					byte_code_module.procedures,
					byte_code_module.functions,
					line_number
				);
			byte_code_module.byte_code.insert(
				byte_code_module.byte_code.end(),
				compile_expression_result.byte_code.begin(),
				compile_expression_result.byte_code.end()
			);
			if (compile_expression_result.flags & EXPRESSION_WERE_CONVERTED) {
				byte_code_module.byte_code.push_back(
					ByteCodeMnemonic(
						line_number,
						"call",
						GetSubprogramNameByAlias(
							"ArrayClearMemoryAfterConvertsToStrings"
						)
					)
				);
			}

			if (compile_expression_result.flags & EXPRESSION_IS_RESULT) {
				byte_code_module.byte_code.push_back(
					ByteCodeMnemonic(line_number, "clr_stck")
				);
			}
		}

		if (!new_condition) {
			while (!condition_id_stack.empty()) {
				long last_condition_id = condition_id_stack.top();
				condition_id_stack.pop();

				byte_code_module.byte_code.push_back(
					ByteCodeMnemonic(
						line_number,
						"lbl",
						"end_of_condition" + ConvertToString(last_condition_id)
					)
				);
			}
		}
	}

	return byte_code_module;
}

std::string ConvertByteCodeToAssembler(
	const ByteCodeModule& byte_code_module,
	const InbuildVariableMap& inbuild_variables,
	const InbuildStringConstantMap& inbuild_string_constants
) {
	std::string assembler_code =
		std::string("\t.text\n")
		+ "\t.global " + CorrectSubprogramName("main") + "\n"
		+ CorrectSubprogramName("main") + ":\n"
		+ "\tmov 8(%esp), %eax\n"
		+ "\tpush (%eax)\n"
		+ "\tcall " + CorrectSubprogramName("ProcessAppPath") + "\n"
		+ "\tadd $4, %esp\n"
		+ "\tsubl $4, %esp\n"
		+ "\tfstps (%esp)\n"
		+ "\tpop APP_PATH\n"
		+ "\n";

	for (
		InbuildStringConstantMap::const_iterator i =
			inbuild_string_constants.begin();
		i != inbuild_string_constants.end();
		++i
	) {
		assembler_code +=
			"\tpush $CONSTANT_STRING"
				+ ConvertToString(
					std::distance(inbuild_string_constants.begin(), i)
				)
				+ "\n"
			+ "\tcall " + CorrectSubprogramName("ArrayCreateFromString") + "\n"
			+ "\tadd $4, %esp\n"
			+ "\tsubl $4, %esp\n"
			+ "\tfstps (%esp)\n"
			+ "\tpop " + i->first + "\n";
	}
	assembler_code += "\n";

	StringList numbers;
	StringList strings;
	for (
		ByteCode::const_iterator j = byte_code_module.byte_code.begin();
		j != byte_code_module.byte_code.end();
		++j
	) {
		ByteCodeMnemonic mnemonic = *j;
		if (mnemonic.mnemonic == "push_n") {
			if (!IsNumber(mnemonic.operand)) {
				ProcessError(
					"Invalid number \""
					+ mnemonic.operand
					+ "\" on line "
					+ ConvertToString(mnemonic.line_number)
					+ "."
				);
			}

			numbers.push_back(mnemonic.operand);
			assembler_code +=
				std::string("\tsubl $4, %esp\n")
				+ "\tfld CONSTANT_NUMBER"
					+ ConvertToString(numbers.size() - 1)
					+ "\n"
				+ "\tfstps (%esp)\n";
		} else if (mnemonic.mnemonic == "push_s") {
			if (!IsString(mnemonic.operand)) {
				ProcessError(
					"Invalid string format on line "
					+ ConvertToString(mnemonic.line_number)
					+ "."
				);
			}

			strings.push_back(mnemonic.operand);
			assembler_code +=
				"\tpush $CONSTANT_STRING"
					+ ConvertToString(
						inbuild_string_constants.size() + strings.size() - 1
					)
					+ "\n"
				+ "\tcall "
					+ CorrectSubprogramName("ArrayCreateFromString")
					+ "\n"
				+ "\tadd $4, %esp\n"
				+ "\tsubl $4, %esp\n"
				+ "\tfstps (%esp)\n";
		} else if (mnemonic.mnemonic == "push_v") {
			if (!IsIdentifier(mnemonic.operand)) {
				ProcessError(
					"Invalid identifier \""
					+ mnemonic.operand
					+ "\" on line "
					+ ConvertToString(mnemonic.line_number)
					+ "."
				);
			}

			VariableSet::const_iterator iterator =
				byte_code_module.variables.find(mnemonic.operand);
			if (iterator == byte_code_module.variables.end()) {
				ProcessError(
					"Undefined variable \""
					+ mnemonic.operand
					+ "\" on line "
					+ ConvertToString(mnemonic.line_number)
					+ "."
				);
			}

			assembler_code += "\tpush " + mnemonic.operand + "\n";
		} else if (mnemonic.mnemonic == "pop") {
			if (!IsIdentifier(mnemonic.operand)) {
				ProcessError(
					"Invalid identifier \""
					+ mnemonic.operand
					+ "\" on line "
					+ ConvertToString(mnemonic.line_number)
					+ "."
				);
			}

			VariableSet::const_iterator iterator =
				byte_code_module.variables.find(mnemonic.operand);
			if (iterator == byte_code_module.variables.end()) {
				ProcessError(
					"Undefined variable \""
					+ mnemonic.operand
					+ "\" on line "
					+ ConvertToString(mnemonic.line_number)
					+ "."
				);
			}

			assembler_code += "\tpop " + mnemonic.operand + "\n";
		} else if (mnemonic.mnemonic == "lbl") {
			if (!IsIdentifier(mnemonic.operand)) {
				ProcessError(
					"Invalid identifier \""
					+ mnemonic.operand
					+ "\" on line "
					+ ConvertToString(mnemonic.line_number)
					+ "."
				);
			}

			assembler_code += mnemonic.operand + ":\n";
		} else if (mnemonic.mnemonic == "jmp") {
			if (!IsIdentifier(mnemonic.operand)) {
				ProcessError(
					"Invalid identifier \""
					+ mnemonic.operand
					+ "\" on line "
					+ ConvertToString(mnemonic.line_number)
					+ "."
				);
			}

			assembler_code += "\tjmp " + mnemonic.operand + "\n";
		} else if (mnemonic.mnemonic == "je") {
			if (!IsIdentifier(mnemonic.operand)) {
				ProcessError(
					"Invalid identifier \""
					+ mnemonic.operand
					+ "\" on line "
					+ ConvertToString(mnemonic.line_number)
					+ "."
				);
			}

			assembler_code +=
				std::string("\tcmp $0, (%esp)\n")
				+ "\tpop %eax\n"
				+ "\tje " + mnemonic.operand + "\n";
		} else if (mnemonic.mnemonic == "call") {
			SubprogramMap::const_iterator iterator =
				byte_code_module.procedures.find(mnemonic.operand);
			bool return_result = false;

			if (iterator == byte_code_module.procedures.end()) {
				iterator = byte_code_module.functions.find(mnemonic.operand);
				if (iterator == byte_code_module.functions.end()) {
					ProcessError(
						"Undefined subprogram \""
						+ mnemonic.operand
						+ "\" on line "
						+ ConvertToString(mnemonic.line_number)
						+ "."
					);
				}

				return_result = true;
			}

			assembler_code += "\tcall " + mnemonic.operand + "\n";

			size_t number_of_arguments = 4 * iterator->second;
			if (number_of_arguments > 0) {
				assembler_code +=
					"\taddl $" + ConvertToString<size_t>(number_of_arguments)
						+ ", %esp\n";
			}

			if (return_result) {
				assembler_code +=
					std::string("\tsubl $4, %esp\n")
					+ "\tfstps (%esp)\n";
			}
		} else if (mnemonic.mnemonic == "to_str") {
			assembler_code +=
				"\tcall " + CorrectSubprogramName("ArrayConvertToString") + "\n"
				+ "\taddl $4, %esp\n"
				+ "\tpush %eax\n";
		} else if (mnemonic.mnemonic == "clr_stck") {
			assembler_code += "\tpop %eax\n";
		} else {
			ProcessError(
				"Undefined mnemonic \""
				+ mnemonic.operand
				+ "\" on line "
				+ ConvertToString(mnemonic.line_number)
				+ "."
			);
		}
	}
	assembler_code += "\n";

	assembler_code +=
		"\tcall " + CorrectSubprogramName("ArrayDeleteAll") + "\n"
		+ "\tcall " + CorrectSubprogramName("FileCloseAll") + "\n"
		+ "\tmov $0, %eax\n"
		+ "\tret\n";

	assembler_code += "\t.data\n";
	StringList::const_iterator numbers_begin = numbers.begin();
	for (
		StringList::const_iterator k = numbers_begin;
		k != numbers.end();
		++k
	) {
		assembler_code +=
			"CONSTANT_NUMBER"
				+ ConvertToString(std::distance(numbers_begin, k))
				+ ":\n"
			+ "\t.float " + *k + "\n";
	}
	for (
		VariableSet::const_iterator n = byte_code_module.variables.begin();
		n != byte_code_module.variables.end();
		++n
	) {
		std::string variable_name = *n;
		assembler_code +=
			variable_name + ":\n"
			+ "\t.float "
				+ (inbuild_variables.count(variable_name) == 0
					? "0"
					: ConvertToString(
						inbuild_variables.find(variable_name)->second
					))
				+ "\n";
	}
	assembler_code += "\n";

	for (
		InbuildStringConstantMap::const_iterator l =
			inbuild_string_constants.begin();
		l != inbuild_string_constants.end();
		++l
	) {
		assembler_code +=
			"CONSTANT_STRING"
				+ ConvertToString(
					std::distance(inbuild_string_constants.begin(), l)
				)
				+ ":\n"
			+ "\t.string " + l->second + "\n";
	}
	StringList::const_iterator strings_begin = strings.begin();
	for (
		StringList::const_iterator m = strings_begin;
		m != strings.end();
		++m
	) {
		assembler_code +=
			"CONSTANT_STRING"
				+ ConvertToString(
					inbuild_string_constants.size()
					+ std::distance(strings_begin, m)
				)
				+ ":\n"
			"\t.string " + *m + "\n";
	}

	return assembler_code;
}

void MakeExecutableFile(
	const std::string& assembler_code,
	const LibrariesList& libraries,
	const CommandLineArguments& command_line_arguments
) {
	#ifdef OS_LINUX
		std::string input_filename = std::string(std::tmpnam(NULL)) + ".s";
	#elif defined(OS_WINDOWS)
		std::string input_filename = std::string(std::tmpnam(NULL)) + "s";
	#endif
	FileWrite(input_filename, assembler_code);

	std::string output_filename = command_line_arguments.input_filename;
	#ifdef OS_LINUX
		size_t last_separator_index =
			command_line_arguments.input_filename.find_last_of('/');
	#elif defined(OS_WINDOWS)
		size_t last_separator_index =
			command_line_arguments.input_filename.find_last_of('\\');
	#endif
	size_t suffix_begin_index =
		command_line_arguments.input_filename.find_last_of('.');
	if (
		suffix_begin_index != std::string::npos
		&& (last_separator_index == std::string::npos
		|| suffix_begin_index > last_separator_index)
	) {
		output_filename = output_filename.substr(0, suffix_begin_index);
	}

	std::string input_file_path;
	if (last_separator_index != std::string::npos) {
		input_file_path = command_line_arguments.input_filename.substr(
			0,
			last_separator_index + 1
		);
	} else {
		#ifdef OS_LINUX
			input_file_path = "./";
		#elif defined(OS_WINDOWS)
			input_file_path = ".\\";
		#endif
	}

	std::string command =
		"g++ -O2 -m32 -o "
		+ output_filename
		+ " "
		+ input_filename;
	for (
		LibrariesList::const_iterator i = libraries.begin();
		i != libraries.end();
		++i
	) {
		Library library = *i;
		if (!library.path.empty()) {
			command += " -L" + input_file_path + library.path;
		}
		command += " -l" + library.name;
	}
	command +=
		" -L"
		+ command_line_arguments.base_path
		+ "../framework/libraries/wizard_basic_framework/ -lwbf";
	ShowMessage(command);

	int status_code = std::system(command.c_str());
	if (status_code != 0) {
		ProcessError("Assembling or linking finished with error.");
	}

	int removed = std::remove(input_filename.c_str());
	if (removed != 0) {
		ShowMessage(
			"Unable to remove temporary file \"" + input_filename + "\".",
			MESSAGE_ERROR
		);
	}
}

int main(int number_of_arguments, char* arguments[]) {
	CommandLineArguments command_line_arguments = ProcessCommandLineArguments(
		number_of_arguments,
		arguments
	);
	CodeLines code_lines = Preprocess(
		FileRead(command_line_arguments.input_filename)
	);
	if (code_lines.empty()) {
		ProcessError("Program is empty.");
	}
	if (command_line_arguments.final_state == STATE_PREPROCESSED_CODE) {
		ShowCodeLines(code_lines);
		std::exit(EXIT_SUCCESS);
	}

	InbuildVariableMap inbuild_variables;
	inbuild_variables["FALSE"] = 0.0f;
	inbuild_variables["TRUE"] = 1.0f;
	inbuild_variables["PI"] = 3.14159f;
	inbuild_variables["E"] = 2.71828f;
	inbuild_variables["FILE_OPEN_MODE_READ"] = 0.0f;
	inbuild_variables["FILE_OPEN_MODE_APPEND"] = 1.0f;
	inbuild_variables["FILE_OPEN_MODE_REWRITE"] = 2.0f;
	inbuild_variables["FILE_OPEN_ERROR"] = -1.0f;
	inbuild_variables["OS_LINUX"] = 1.0f;
	inbuild_variables["OS_WINDOWS"] = 0.0f;

	InbuildStringConstantMap inbuild_string_constants;
	inbuild_string_constants["NEW_LINE"] = "\"\\n\"";
	#ifdef OS_LINUX
		inbuild_string_constants["PATH_SEPARATOR"] = "\"/\"";
	#elif defined(OS_WINDOWS)
		inbuild_string_constants["PATH_SEPARATOR"] = "\"\\\\\"";
	#endif

	ByteCodeModule byte_code_module = Compile(
		code_lines,
		inbuild_variables,
		inbuild_string_constants
	);
	switch (command_line_arguments.final_state) {
		case STATE_LIBRARY_LIST:
			ShowLibraryList(byte_code_module);
			std::exit(EXIT_SUCCESS);
			break;
		case STATE_PROCEDURE_LIST:
			ShowProcedureList(byte_code_module);
			std::exit(EXIT_SUCCESS);
			break;
		case STATE_FUNCTION_LIST:
			ShowFunctionList(byte_code_module);
			std::exit(EXIT_SUCCESS);
			break;
		case STATE_VARIABLE_LIST:
			ShowVariableList(byte_code_module);
			std::exit(EXIT_SUCCESS);
			break;
		case STATE_BYTE_CODE:
			ShowByteCode(byte_code_module);
			std::exit(EXIT_SUCCESS);
			break;
		default:
			break;
	}

	std::string assembler_code = ConvertByteCodeToAssembler(
		byte_code_module,
		inbuild_variables,
		inbuild_string_constants
	);
	if (command_line_arguments.final_state == STATE_ASSEMBLER_CODE) {
		if (
			!assembler_code.empty()
			&& assembler_code[assembler_code.length() - 1] == '\n'
		) {
			assembler_code = assembler_code.substr(
				0,
				assembler_code.length() - 1
			);
		}

		ShowMessage(assembler_code);
		std::exit(EXIT_SUCCESS);
	}

	MakeExecutableFile(
		assembler_code,
		byte_code_module.libraries,
		command_line_arguments
	);
}
