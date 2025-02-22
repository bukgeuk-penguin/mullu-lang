﻿#include "run.hpp"

int vartable[65535] = { 0, };
unsigned int labeltable[65535] = { 0, };

int calc(token::tokenlist& tokenlist, unsigned int& idx) {
	bool isvalue = true;
	token::token_type sign = token::token_type::unknown;
	int value = 0;
	for (int i = idx; i < tokenlist.size(); i++) {
		if (isvalue) {
			int temp = 0;
			switch (tokenlist[i].type) {
			case token::token_type::var_value: {
				temp = vartable[tokenlist[i].str.length() - 1];
				break;
			}
			case token::token_type::positive_one:
				temp = 1;
				break;
			case token::token_type::positive_ten:
				temp = 10;
				break;
			case token::token_type::negative_one:
				temp = -1;
				break;
			case token::token_type::negative_ten:
				temp = -10;
				break;
			case token::token_type::input_number:
				cin >> temp;
				break;
			case token::token_type::input_character:
				temp = getwchar();
				while (temp == L'\n')
					temp = getwchar();
				break;
			default:
				error::throwerror(tokenlist[i].info, L"", 0);
			}

			if (i == idx) value += temp;
			else if (tokenlist[i - 1].type == token::token_type::add) value += temp;
			else if (tokenlist[i - 1].type == token::token_type::sub) value -= temp;
			else if (tokenlist[i - 1].type == token::token_type::mul) value *= temp;
			else if (tokenlist[i - 1].type == token::token_type::div) value /= temp;
		} else {
			switch (tokenlist[i].type) {
			case token::token_type::add:
			case token::token_type::sub:
			case token::token_type::mul:
			case token::token_type::div:
				break;
			default:
				idx = i - 1;
				return value;
			}
		}

		isvalue = !isvalue;
	}

	idx = tokenlist.size() - 1;
	return value;
}

void run::runFile(token::tokenlist tokenlist) {
	short comment = 0; // 0 = none, 1 = line, 2 = block

	for (unsigned int i = 0; i < tokenlist.size(); i++) {
		switch (tokenlist[i].type) {
		case token::token_type::line_comment:
			if (comment == 0) comment = 1;
			break;
		case token::token_type::line_feed:
			if (comment == 1) comment = 0;
			break;
		case token::token_type::block_comment_start:
			if (comment == 0) comment = 2;
			break;
		case token::token_type::block_comment_end:
			if (comment == 2) comment = 0;
			break;
		}

		if (comment != 0) continue;

		switch (tokenlist[i].type) {
		case token::token_type::assign: {
			int temp = tokenlist[i].str.length();
			++i;
			int value = calc(tokenlist, i);
			vartable[temp - 3] = value;
			break;
		}
		case token::token_type::jump_equal:
		case token::token_type::jump_less: 
		case token::token_type::jump_greater: {
			if (i + 2 >= tokenlist.size()) {
				error::throwerror(tokenlist[i].info, L"", 0);
				break;
			}
			auto temp = tokenlist[i].type;
			++i;
			int value = calc(tokenlist, i);
			++i;
			if (tokenlist[i].type == token::token_type::label) {
				auto togo = labeltable[tokenlist[i].str.length() - 4];
				if (togo < 0) {
					error::throwerror(tokenlist[i].info, L"", 0);
				}

				bool condition = false;
				if (temp == token::token_type::jump_equal) condition = (value == 0);
				else if (temp == token::token_type::jump_less) condition = (value < 0);
				else if (temp == token::token_type::jump_greater) condition = (value > 0);

				if (condition) {
					i = togo;
				}
			} else {
				error::throwerror(tokenlist[i].info, L"", 0);
			}
			break;
		}
		case token::token_type::define_label: {
			labeltable[tokenlist[i].str.length() - 4] = i;
			break;
		}
		case token::token_type::output_number: {
			++i;
			int value = calc(tokenlist, i);
			cout << value;
			break;
		}
		case token::token_type::output_character: {
			++i;
			int value = calc(tokenlist, i);
			putwchar(value);
			break;
		}
		case token::token_type::line_comment:
		case token::token_type::line_feed:
		case token::token_type::block_comment_start:
		case token::token_type::block_comment_end:
			break;
		default:
			error::throwerror(tokenlist[i].info, L"", 0);
		}
	}
}

void run::runRepl() {
	error::repl::toggleRepl();
	wcout << L"mollu-lang-cpp " << run::version << L"\n";
	wcout << L"Type exit or exit() to exit\n";

	short comment = 0; // 0 = none, 1 = line, 2 = block
	token::tokenlist tokenlist;

	unsigned int i = 0;

	wstring cmd;
	while (true) {
		wcout << L">>> ";
		getline(wcin, cmd);
		if (cmd == L"exit" || cmd == L"exit()") break;
		auto tl = token::tokenize(cmd);
		tokenlist.insert(tokenlist.end(), tl.begin(), tl.end());

		for (; i < tokenlist.size(); i++) {
			switch (tokenlist[i].type) {
			case token::token_type::line_comment:
				if (comment == 0) comment = 1;
				break;
			case token::token_type::line_feed:
				if (comment == 1) comment = 0;
				break;
			case token::token_type::block_comment_start:
				if (comment == 0) comment = 2;
				break;
			case token::token_type::block_comment_end:
				if (comment == 2) comment = 0;
				break;
			}

			if (comment != 0) continue;

			switch (tokenlist[i].type) {
			case token::token_type::assign: {
				int temp = tokenlist[i].str.length();
				++i;
				int value = calc(tokenlist, i);
				vartable[temp - 3] = value;
				break;
			}
			case token::token_type::jump_equal:
			case token::token_type::jump_less:
			case token::token_type::jump_greater: {
				if (i + 2 >= tokenlist.size()) {
					error::throwerror(tokenlist[i].info, L"", 0);
					break;
				}
				auto temp = tokenlist[i].type;
				++i;		
				int value = calc(tokenlist, i);
				++i;
				if (tokenlist[i].type == token::token_type::label) {
					auto togo = labeltable[tokenlist[i].str.length() - 4];
					if (togo < 0) {
						error::throwerror(tokenlist[i].info, L"", 0);
					}

					bool condition = false;
					if (temp == token::token_type::jump_equal) condition = (value == 0);
					else if (temp == token::token_type::jump_less) condition = (value < 0);
					else if (temp == token::token_type::jump_greater) condition = (value > 0);

					if (condition) {
						i = togo;
					}
				} else {
					error::throwerror(tokenlist[i].info, L"", 0);
				}
				break;
			}
			case token::token_type::define_label: {
				labeltable[tokenlist[i].str.length() - 4] = i;
				break;
			}
			case token::token_type::output_number: {
				++i;
				int value = calc(tokenlist, i);
				cout << value << endl;
				break;
			}
			case token::token_type::output_character: {
				++i;
				int value = calc(tokenlist, i);
				putwchar(value);
				cout << endl;
				break;
			}
			case token::token_type::line_comment:
			case token::token_type::line_feed:
			case token::token_type::block_comment_start:
			case token::token_type::block_comment_end:
				break;
			default:
				error::throwerror(tokenlist[i].info, L"", 0);
			}
		}
	}
	error::repl::toggleRepl();
}