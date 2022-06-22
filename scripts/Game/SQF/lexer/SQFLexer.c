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
	
	
	void SQFLexer(string script) 
	{
		Init(script);
	}
	
	void Init(string script) 
	{
		m_Script = new SQFStringStream(script);
		
		InitIdentifiers();
		InitDigits();
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
		
		if(is_digit_char(nextChar, startable) && startable) // check if is digit start character
		{
			return handleDigit();
		}
		
		if(is_identifier_char(nextChar, startable) && startable) // check if is identifier start character
		{
			return handleIdentifier(); // generate an identifier token and return
		}
		
		if(is_string_char(nextChar)) // is this a `"` or `'` character
		{
			return handleString();
		} 
		
		if(is_comment_char(nextChar)) // strings take precedence over comments ( " // this is // a valid // string " )
		{
			return handleComment();
		}
		
		if(is_operator_char(nextChar)) // is this in the operator table
		{
			return handleOperator();	
		}
		
		
		
		// no hits, character not handled by lexer
		m_Script.Inc(); // need to inc so we don't infinite loop
		return new SQFToken(ESQFTokenType.UNEXPECTED, m_Script.Cursor(), nextChar); // unhandled token
	}
	
	
	
	
	protected SQFToken handleIdentifier()
	{
		// get all characters associated with the identifier and increment our cursor
		// after this routine completes cursor should be left on the first non-identifier token	
		int start = m_Script.Cursor();
		m_Script.Inc();
		bool temp = false;
		while(is_identifier_char(m_Script.Peek(), temp)) m_Script.Inc();
		return new SQFToken(ESQFTokenType.IDENTIFIER, start, m_Script.GetText(start, m_Script.Cursor() - start));
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
		return new SQFToken(ESQFTokenType.NUMBER, start, m_Script.GetText(start, m_Script.Cursor() - start));
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
			return new SQFToken(ESQFTokenType.STRING, start, m_Script.GetText(start, m_Script.Cursor() - start));
		
		// this is a special token to tell us we fucked up the string somehow
		return new SQFToken(ESQFTokenType.UNEXPECTED, start, m_Script.GetText(start, m_Script.Cursor() - start));
	}
	
	protected SQFToken handleOperator()
	{
		int start = m_Script.Cursor();
		ESQFTokenType type = ESQFTokenType.UNEXPECTED;
		string operator_char = m_Script.Get();
		
		
		switch(operator_char)
		{
			case ";":
				type = ESQFTokenType.SEMICOLON;
				break;
			case "=":
				type = ESQFTokenType.EQUALS;
				break;
			// TODO: add other operators here
		}
		
		return new SQFToken(type, start, m_Script.GetText(start, m_Script.Cursor() - start));
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
	
	protected bool is_operator_char(string c)
	{
		switch(c) {
			case ";":
				return true;
			case "=":
				return true;
			//TODO: add other operators for SQF here
			default:
				return false;
		}
		Print("compiler busted??", LogLevel.SPAM);
		return false;
	}
	// check if char represents a string quote
	protected bool is_string_char(string c)
	{
		return (c == "\"" || c == "'"); // strings can start with a " or ' in SQF but must be terminated by the same character type
	}
	
	// check if a character exists in the identifiers allowed character table
	protected bool is_identifier_char(string c,out bool can_start)
	{
		string copy = c;
		copy.ToLower(); // identifiers are case insensitive in sqf
		return m_Identifiers.Find(copy, can_start);
	}
	// initialize all characters allowed in variable, function, and method names
	protected void InitIdentifiers() 
	{
		m_Identifiers = new map<string, bool>();
		m_Identifiers.Insert("_",true);
		m_Identifiers.Insert("1",true);
		m_Identifiers.Insert("2",true);
		m_Identifiers.Insert("3",true);
		m_Identifiers.Insert("4",true);
		m_Identifiers.Insert("5",true);
		m_Identifiers.Insert("6",true);
		m_Identifiers.Insert("7",true);
		m_Identifiers.Insert("8",true);
		m_Identifiers.Insert("9",true);
		m_Identifiers.Insert("0",true);
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
	}
	
	
	// check if a character exists in the digits allowed character table
	protected bool is_digit_char(string c, out bool can_start)
	{
		string copy = c;
		copy.ToLower(); // identifiers are case insensitive in sqf
		return m_Digits.Find(copy, can_start);
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
}