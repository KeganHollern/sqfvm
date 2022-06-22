

// ordered by "check" order
enum ESQFTokenType {
	KEYWORD, 	// keywords : if while do return and
	
	COMMAND, 	// scripting commands : player moveInDriver allPlayers 
	
	IDENTIFIER, // user defined variables : _var VAR
	
	LITERAL,	// literals : "string" 1.10 true
	
	OPERATOR, 	// operators : = + - < <=
	SEPARATOR,	// separaters : ; ( ) { } [ ]
	
	COMMENT,	// comment: // /**/
	
	END_OF_SCRIPT, // end of file
	UNEXPECTED, // unhandled
};
enum ESQFIdentifierFlags {
	LOCAL = 1,
	GLOBAL = 2,
};
enum ESQFLiteralFlags {
	STRING = 1,
	NUMBER = 2,
	TRUE = 4,
	FALSE = 8,
};
enum ESQFSeparatorFlags {
	OPEN = 1,
	CLOSE = 2,
	PARENTHESES = 4, 
	BRACKET = 8,
	BRACE = 16,
	SEMICOLON = 32, 
	COLON = 64, 
	COMMA = 128, 
	// OPEN_PARENTHESES = OPEN | PARENTHESES
};
enum ESQFOperatorFlags {
	EQUALS = 1, 		// =
	PLUS = 2, 			// +
	MINUS = 4, 			// -
	LESS = 8, 			// <
	GREATER = 16, 		// >
	RIGHT_SHIFT = 32, 	// >>
	NOT = 64, 			// !
	MULTIPLY = 128, 	// *
	DIVIDE = 256, 		// / 
	MODULO = 512, 		// %
	POWER = 1024, 		// ^
	AND = 2048, 		// &
	OR = 4096, 			// |
	// LESS_EQUAL = LESS | EQUALS,
	// GREATER_EQUAL = GREATER | EQUALS
	// NOT_EQUALS = NOT | EQUALS,
}

class SQFToken {
	protected ESQFTokenType m_Type;
	protected string m_Content;
	protected int m_Start;
	protected int m_Flags;
	
	void SQFToken(ESQFTokenType type, int start, string content, int flags = 0) {
		this.m_Type = type;
		this.m_Start = start;
		this.m_Content = content;
		this.m_Flags = flags;
	}
	
	string Stringify()
	{
		string copy = m_Content;
		// jesus fuck i hate enforce
		string find = "\"";
		string replacer = "\\" + "\"";
		copy.Replace(find, replacer);
		return ("Token: ( " + typename.EnumToString(ESQFTokenType, m_Type) + " @ " + m_Flags.ToString() + " | " + m_Start.ToString() + " | \"" + copy + "\" )");
	}
	
	ESQFTokenType TokenType() {
		return m_Type;
	}
	int Flags() {
		return m_Flags;
	}
}