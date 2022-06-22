/* SQF Lexer


// sample code:
	SQFLexer lexer = new SQFLexer("hint 'this is a ''script''!';");
	while(true) {
		SQFToken token = lexer.Next();
		if(token == null) break;
	
		Print(token.Stringify());
	}
// outputs:
Token: Identifier, 0, "hint"
Token: String, 5, "'this is a ''script''!'"
Token: Semicolon, ?, ";" // not sure the actual index don't care to figure it out

*/



class SQFLexer {
	protected ref SQFStringStream m_Script;
	
	protected ref map<string, bool> m_Identifiers;
	protected ref map<string, bool> m_Digits;
	
	protected ref array<string> m_Keywords;
	protected ref array<string> m_Commands;
	
	
	void SQFLexer(string script) 
	{
		Init(script);
	}
	
	void Init(string script) 
	{
		m_Script = new SQFStringStream(script);
		
		InitIdentifiers();
		InitDigits();
		InitDefaults();
	}
	
	// get the next SQF token
	SQFToken Next() 
	{
		// skip white space
		while(is_space_char(m_Script.Peek()))
			m_Script.Inc();
		
		string nextChar = m_Script.Peek();
		bool startable = false;
		
		// note: the order here is important. digits take precedence over identifiers (for example);
		
		if(nextChar == "") // end of script
		{
			return new SQFToken(ESQFTokenType.END_OF_SCRIPT, m_Script.Cursor(), ""); // no more tokens!
		}
		
		// comments must come before operators so `/` isn't treated like divides
		if(is_comment_char(nextChar)) // strings take precedence over comments ( " // this is // a valid // string " )
		{
			return handleComment();
		}
		int keywrd_idx = 0;
		if(is_keyword_char(nextChar, keywrd_idx))// must match keyword
		{
			return handleKeyword(keywrd_idx);
		}
		int cmd_idx = 0;
		if(is_command_char(nextChar, cmd_idx))// must match command
		{
			return handleCommand(cmd_idx);
		}
		if(is_operator_char(nextChar)) // must match operator (full word ops like `and` are handled as commands)
		{
			return handleOperator();	
		}
		// literal handling
		if(is_digit_char(nextChar, startable) && startable) // check if is digit start character
		{
			return handleDigit();
		}
		if(is_string_char(nextChar)) // is this a `"` or `'` character
		{
			return handleString();
		}
		if(is_boolean_char(nextChar))
		{
			return handleBoolean();
		}
		// identifiers must come after boolean so `true` isn't treated like a variable!
		if(is_identifier_char(nextChar, startable) && startable) 
		{
			return handleIdentifier();
		}
		if(is_separator_char(nextChar))
		{
			return handleSeparator();
		}
				
		// no hits, character not handled by lexer (emoji perhaps?)
		m_Script.Inc(); // need to inc so we don't infinite loop
		return new SQFToken(ESQFTokenType.UNEXPECTED, m_Script.Cursor(), nextChar); // unhandled token
	}
	
	
	// check if this index starts an operator
	protected bool is_separator_char(string c)
	{
		switch(c)
		{
			case "{":
			case "}":
			case "[":
			case "]":
			case "(":
			case ")":
			case ";":
			case ":":
			case ",":
				return true;
		}
		return false;
	}
	protected SQFToken handleSeparator()
	{
		int start = m_Script.Cursor();
		string c = m_Script.Get();
		ESQFSeparatorFlags flags = 0;
		switch(c)
		{
			case "{":
				flags = ESQFSeparatorFlags.BRACE | ESQFSeparatorFlags.OPEN;
				break;
			case "}":
				flags = ESQFSeparatorFlags.BRACE | ESQFSeparatorFlags.CLOSE;
				break;
			case "[":
				flags = ESQFSeparatorFlags.BRACKET | ESQFSeparatorFlags.OPEN;
				break;
			case "]":
				flags = ESQFSeparatorFlags.BRACKET | ESQFSeparatorFlags.CLOSE;
				break;
			case "(":
				flags = ESQFSeparatorFlags.PARENTHESES | ESQFSeparatorFlags.OPEN;
				break;
			case ")":
				flags = ESQFSeparatorFlags.PARENTHESES | ESQFSeparatorFlags.CLOSE;
				break;
			case ";":
				flags = ESQFSeparatorFlags.SEMICOLON;
				break;
			case ":":
				flags = ESQFSeparatorFlags.COLON;
				break;
			case ",":
				flags = ESQFSeparatorFlags.COMMA;
				break;
			default:
				return new SQFToken(ESQFTokenType.UNEXPECTED, start, m_Script.GetText(start, m_Script.Cursor() - start));
		}
		
		return new SQFToken(ESQFTokenType.SEPARATOR, start, m_Script.GetText(start, m_Script.Cursor() - start), flags); // unimplemented
	}
	
	
	// check if this index starts an operator
	protected bool is_operator_char(string c)
	{
		switch(c)
		{
			case "+":
			case "-":
			case "/":
			case "*":
			case "^":
			case "%":
			case "&":
			case "|":
			case ">":
			case "<":
			case "=":
				return true;
		}
		return false;
	}
	protected SQFToken handleOperator()
	{
		int start = m_Script.Cursor();
		string c = m_Script.Get();
		ESQFOperatorFlags flags = 0;
		
		// implement operator stuff
		switch(c)
		{
			case "+":
				flags = ESQFOperatorFlags.PLUS;
				// hey! if we want to implement `+=` here would be a good place to add it!
				break;
			case "-":
				flags = ESQFOperatorFlags.MINUS;
				break;
			case "/":
				flags = ESQFOperatorFlags.DIVIDE;
				break;
			case "*":
				flags = ESQFOperatorFlags.MULTIPLY;
				break;
			case "^":
				flags = ESQFOperatorFlags.POWER;
				break;
			case "%":
				flags = ESQFOperatorFlags.MODULO;
				break;
			case "&":
				string next = m_Script.Peek();
				if(next != "&")
				{
					//warn user they forgot a second `&`... we'll ignore this and just assume they forgot it on accident!
					Print("malformatted '&' while lexing @ " + start.ToString() + ". Forgot a second '&'?", LogLevel.WARNING);
				}
				else 
				{
					m_Script.Inc();
				}	
				flags = ESQFOperatorFlags.AND;
			case "|":
				string next = m_Script.Peek();
				if(next != "|")
				{
					//warn user they forgot a second `&`... we'll ignore this and just assume they forgot it on accident!
					Print("malformatted '|' while lexing @ " + start.ToString() + ". Forgot a second '|'?", LogLevel.WARNING);
				}
				else 
				{
					m_Script.Inc();
				}
				flags = ESQFOperatorFlags.OR;
			
			case ">":
				string next = m_Script.Peek();
				if(next == ">")
				{
					flags = ESQFOperatorFlags.RIGHT_SHIFT;	
					m_Script.Inc();
				}
				else
				{
					flags = ESQFOperatorFlags.GREATER;	
				}
				if(next == "=")
				{
					flags |= ESQFOperatorFlags.EQUALS;
					m_Script.Inc();
				}
			case "<":
				string next = m_Script.Peek();
				flags = ESQFOperatorFlags.GREATER;	
				if(next == "=")
				{
					flags |= ESQFOperatorFlags.EQUALS;
					m_Script.Inc();
				}
			case "=":
				flags = ESQFOperatorFlags.EQUALS;
				
			default:
				return new SQFToken(ESQFTokenType.UNEXPECTED, start, m_Script.GetText(start, m_Script.Cursor() - start));
		}
		
		return new SQFToken(ESQFTokenType.OPERATOR, start, m_Script.GetText(start, m_Script.Cursor() - start), flags); // unimplemented
	}
	
	
	// check if this index starts a keyword
	protected bool is_keyword_char(string c, out int match_idx)
	{
		int start = m_Script.Cursor();
		int limit =  m_Script.Length();
		for(int i = 0; i < m_Keywords.Count(); i++)
		{
			string keyword = m_Keywords.Get(i);
			int len = keyword.Length();
			if(start + len > limit) continue;
			string text = m_Script.GetText(start, len);
			text.ToLower();
			if(text == keyword) {
				// check if following char is not a valid identifier one
				string next = m_Script.GetText(start+len,1);
				bool temp = false;
				if(!is_identifier_char(next,temp))
				{	
					match_idx = i;
					return true;
				}
			}
		}
		
		return false;
	}
	protected SQFToken handleKeyword(int index)
	{
		int start = m_Script.Cursor();
		//TODO: safety checks on index?
		string real = m_Keywords.Get(index); // real command
		m_Script.SetCursor(start + real.Length()); // move cursor beyond command
		return new SQFToken(ESQFTokenType.KEYWORD, start,  m_Script.GetText(start, m_Script.Cursor() - start)); // unimplemented
	}
	
	// check if this index starts a keyword
	protected bool is_command_char(string c, out int match_idx)
	{
		int start = m_Script.Cursor();
		int limit =  m_Script.Length();
		for(int i = 0; i < m_Commands.Count(); i++)
		{
			string command = m_Commands.Get(i);
			int len = command.Length();
			if(start + len > limit) continue;
			string text = m_Script.GetText(start, len);
			text.ToLower();
			if(text == command) {
				// check if following char is not a valid identifier one
				string next = m_Script.GetText(start+len,1);
				bool temp = false;
				if(!is_identifier_char(next,temp))
				{	
					match_idx = i;
					return true;
				}
			}
		}
		
		return false;
	}
	protected SQFToken handleCommand(int index)
	{
		
		int start = m_Script.Cursor();
		//TODO: safety checks on index?
		string real = m_Commands.Get(index); // real command
		m_Script.SetCursor(start + real.Length()); // move cursor beyond command
		return new SQFToken(ESQFTokenType.COMMAND, start,  m_Script.GetText(start, m_Script.Cursor() - start)); // unimplemented
	}
	
	protected bool is_boolean_char(string c)
	{
		// check if "true"
		if(m_Script.Cursor() + 4 >= m_Script.Length()) return false;
		
		string text = m_Script.GetText(m_Script.Cursor(),4);
		text.ToLower();
		if(text == "true") return true;
		
		// check if "false"
		if(m_Script.Cursor() + 5 >= m_Script.Length()) return false;
		
		text = m_Script.GetText(m_Script.Cursor(),5);
		text.ToLower();
		if(text == "false") return true;
		
		// not true or false
		return false;
	}
	protected SQFToken handleBoolean()
	{
		ESQFLiteralFlags flags = 0;
		int start = m_Script.Cursor();
		string text = m_Script.GetText(start,4);
		text.ToLower();
		if(text == "true") 
		{
			m_Script.SetCursor(start + 4); 
			flags = ESQFLiteralFlags.TRUE;
		}
		else
		{
			text = m_Script.GetText(start,5);
			text.ToLower();
			if(text == "false")
			{
				m_Script.SetCursor(start + 5); 
				flags = ESQFLiteralFlags.FALSE;
			}
			else
			{
				m_Script.Inc(); // inc cause this was unexpected and we don't want to loop
				return new SQFToken(ESQFTokenType.UNEXPECTED, start, m_Script.GetText(start, m_Script.Cursor() - start));
			}
		}
		
		return new SQFToken(ESQFTokenType.LITERAL, start, m_Script.GetText(start, m_Script.Cursor() - start), flags);
	}
	
	// check if a character exists in the identifiers allowed character table
	protected bool is_identifier_char(string c,out bool can_start)
	{
		string copy = c;
		copy.ToLower(); // identifiers are case insensitive in sqf
		return m_Identifiers.Find(copy, can_start);
	}
	protected SQFToken handleIdentifier()
	{
		// get all characters associated with the identifier and increment our cursor
		// after this routine completes cursor should be left on the first non-identifier token	
		int start = m_Script.Cursor();
		m_Script.Inc();
		bool temp = false;
		while(is_identifier_char(m_Script.Peek(), temp)) m_Script.Inc();
		ESQFIdentifierFlags flags = ESQFIdentifierFlags.GLOBAL;
		if(m_Script.At(start) == "_") {
			flags = ESQFIdentifierFlags.LOCAL;
		}
		return new SQFToken(ESQFTokenType.IDENTIFIER, start, m_Script.GetText(start, m_Script.Cursor() - start), flags);
	}
	
	// check if a character exists in the digits allowed character table
	protected bool is_digit_char(string c, out bool can_start)
	{
		string copy = c;
		copy.ToLower(); // identifiers are case insensitive in sqf
		return m_Digits.Find(copy, can_start);
	}
	protected SQFToken handleDigit()
	{
		// get all characters associated with the digit
				// logical checks do not apply, "1.1.1.1" would be considered a valid digit in this case
		// parser will take care of saying "hey malformatted digit!"
		int start = m_Script.Cursor();
		m_Script.Inc(); // inc off first character
		bool temp = false;
		while(is_digit_char(m_Script.Peek(), temp)) m_Script.Inc();
		return new SQFToken(ESQFTokenType.LITERAL, start, m_Script.GetText(start, m_Script.Cursor() - start), ESQFLiteralFlags.NUMBER);
	}
	
	// check if char represents a string quote
	protected bool is_string_char(string c)
	{
		return (c == "\"" || c == "'"); // strings can start with a " or ' in SQF but must be terminated by the same character type
	}
	protected SQFToken handleString()
	{
		int start = m_Script.Cursor();
		string open_character = m_Script.Get();
		bool safely_closed = false;
		
		while(m_Script.HasNext())
		{
			string next_character = m_Script.Get();
			if(next_character == open_character)
			{
				string following_character = m_Script.Peek();
				if(following_character != open_character)
				{
					safely_closed = true;
					break;  // not escaped, must be the end of our string
				}
				else
				{
					m_Script.Inc(); // escaped quote, not the end of our string
				}
			}
		}
		// there is a chance some wierd shit will happen where we HitZone
		// example script: `hint " this is a string`
		// in this case, no terminator for the string exists, but the token ends due to END OF SCRIPT.
		// we should probably handle this as an error
		
		
		if(safely_closed)
			return new SQFToken(ESQFTokenType.LITERAL, start, m_Script.GetText(start, m_Script.Cursor() - start), ESQFLiteralFlags.STRING);
		
		// this is a special token to tell us we fucked up the string somehow
		return new SQFToken(ESQFTokenType.UNEXPECTED, start, m_Script.GetText(start, m_Script.Cursor() - start));
	}
	
	// check for `//` or `/*` characters */
	protected bool is_comment_char(string c)
	{
		if(c != "/") return false;
		m_Script.Inc();
		string next = m_Script.Peek();
		m_Script.Dec();
		// note i had a strange bug when doing if(next == "/" || next == "*") return true;
		if(next == "/")	return true;
		if(next == "*") return true;
		
		//int ascii = next.ToAscii();
		//Print(ascii);
		//Print("*".ToAscii());
		
		return false;
	}
	protected SQFToken handleComment()
	{
		int start = m_Script.Cursor();
		m_Script.Inc(); // move to second comment character
		if(m_Script.Get() == "*") // check if second is a block and move to next character
		{
			// block comment
			while(m_Script.HasNext())
			{
				string next = m_Script.Get(); // get next character (could be second * in `/**/`
				string following = m_Script.Peek();
				if(next == "*" && following == "/")
				{
					break;
				}
			}
		}
		else
		{
			// line comment
			while(m_Script.HasNext())
			{
				string next = m_Script.Peek();
				if(next == "\r" || next == "\n") break;
				m_Script.Inc();
			}
		}
		
		string comment = m_Script.GetText(start, m_Script.Cursor() - start);
		
		return new SQFToken(ESQFTokenType.COMMENT, start, comment);
	}
	
	
	
	// check if character is a whitespace
	protected bool is_space_char(string c)
	{
		string copy = c;
		copy.ToLower();
		switch(copy)
		{
			case " ":
		    case "\t":
		    case "\r":
		    case "\n":
		    	return true;
			default:
		    	return false;
		}
		Print("compiler busted??", LogLevel.SPAM);
		return false;
	}
	
	
	
	
	
	
	
	
	
	// initialize all characters allowed in variable, function, and method names
	protected void InitIdentifiers() 
	{
		m_Identifiers = new map<string, bool>();
		m_Identifiers.Insert("_",true);
		m_Identifiers.Insert("a",true);
		m_Identifiers.Insert("b",true);
		m_Identifiers.Insert("c",true);
		m_Identifiers.Insert("d",true);
		m_Identifiers.Insert("e",true);
		m_Identifiers.Insert("f",true);
		m_Identifiers.Insert("g",true);
		m_Identifiers.Insert("h",true);
		m_Identifiers.Insert("i",true);
		m_Identifiers.Insert("j",true);
		m_Identifiers.Insert("k",true);
		m_Identifiers.Insert("l",true);
		m_Identifiers.Insert("m",true);
		m_Identifiers.Insert("n",true);
		m_Identifiers.Insert("o",true);
		m_Identifiers.Insert("p",true);
		m_Identifiers.Insert("q",true);
		m_Identifiers.Insert("r",true);
		m_Identifiers.Insert("s",true);
		m_Identifiers.Insert("t",true);
		m_Identifiers.Insert("u",true);
		m_Identifiers.Insert("v",true);
		m_Identifiers.Insert("w",true);
		m_Identifiers.Insert("x",true);
		m_Identifiers.Insert("y",true);
		m_Identifiers.Insert("z",true);
		// integers cannot start identifiers
		m_Identifiers.Insert("1",false);
		m_Identifiers.Insert("2",false);
		m_Identifiers.Insert("3",false);
		m_Identifiers.Insert("4",false);
		m_Identifiers.Insert("5",false);
		m_Identifiers.Insert("6",false);
		m_Identifiers.Insert("7",false);
		m_Identifiers.Insert("8",false);
		m_Identifiers.Insert("9",false);
		m_Identifiers.Insert("0",false);
	}
	
	// initialize all characters allowed in numbers (1000, 1.1, 0x1A, 1e2)
	protected void InitDigits() 
	{
		m_Digits = new map<string, bool>();
		m_Digits.Insert("1",true);
		m_Digits.Insert("2",true);
		m_Digits.Insert("3",true);
		m_Digits.Insert("4",true);
		m_Digits.Insert("5",true);
		m_Digits.Insert("6",true);
		m_Digits.Insert("7",true);
		m_Digits.Insert("8",true);
		m_Digits.Insert("9",true);
		m_Digits.Insert("0",true);
		 // special case where we can support 1e10 as a valid number
		m_Digits.Insert("e",false);
		 // special case for hex numbers 0x1A
		m_Digits.Insert("x",false);
		m_Digits.Insert("a",false);
		m_Digits.Insert("b",false);
		m_Digits.Insert("c",false);
		m_Digits.Insert("d",false);
		m_Digits.Insert("e",false);
		m_Digits.Insert("f",false);
		// special case for decimals 1.10
		m_Digits.Insert(".",false); 
	}
	
	// https://gist.github.com/commy2/016676126737a9a4389c85925b45a68e
	// https://community.bistudio.com/wiki/Category:Arma_3:_Scripting_Commands
	protected void InitDefaults()
	{
		m_Keywords = new array<string>();
		m_Commands = new array<string>();
		
		// initialize keywords
		m_Keywords.Insert("if");
		m_Keywords.Insert("then");
		m_Keywords.Insert("else");
		m_Keywords.Insert("while");
		m_Keywords.Insert("do");
		m_Keywords.Insert("waituntil");
		m_Keywords.Insert("for");
		m_Keywords.Insert("exitwith");
		m_Keywords.Insert("from");
		m_Keywords.Insert("to");
		m_Keywords.Insert("switch");
		m_Keywords.Insert("case");
		m_Keywords.Insert("default");
		m_Keywords.Insert("try");
		m_Keywords.Insert("catch");
		m_Keywords.Insert("throw");
		m_Keywords.Insert("step");
		m_Keywords.Insert("private");
		m_Keywords.Insert("foreach");
		
		//TODO: continue to add commands
		m_Commands.Insert("mod");
		m_Commands.Insert("atan2");
		m_Commands.Insert("min");	
		m_Commands.Insert("max");	
		m_Commands.Insert("max");
		m_Commands.Insert("or");
		m_Commands.Insert("and");
		
		m_Commands.Insert("spawn");
		m_Commands.Insert("call");
		m_Commands.Insert("diag_log");
		m_Commands.Insert("format");
		m_Commands.Insert("time");
	}
}