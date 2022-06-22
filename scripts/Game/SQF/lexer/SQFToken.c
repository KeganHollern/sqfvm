
// TODO: expand token types
enum ESQFTokenType {
	IDENTIFIER, // variables: _var
	STRING,		// strings: "string"
	NUMBER,		// numbers: 1.10
	SEMICOLON,	// semicolon: ;
	EQUALS,		// equals: =
	END_OF_SCRIPT, // end of file
	UNEXPECTED, // unhandled
	COMMENT,	// comment: // /**/
};


class SQFToken {
	protected ESQFTokenType m_Type;
	protected string m_Content;
	protected int m_Start;
	
	void SQFToken(ESQFTokenType type, int start, string content) {
		this.m_Type = type;
		this.m_Start = start;
		this.m_Content = content;
	}
	
	string Stringify()
	{
		string copy = m_Content;
		// jesus fuck i hate enforce
		string find = "\"";
		string replacer = "\\" + "\"";
		copy.Replace(find, replacer);
		return ("Token: " + typename.EnumToString(ESQFTokenType, m_Type) + ", " + m_Start.ToString() + ", \"" + copy + "\"");
	}
	
	ESQFTokenType TokenType() {
		return m_Type;
	}
}