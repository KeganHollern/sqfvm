/*
 * SQFVM is a custom SQF Engine for Reforger based on Arma 3 2.10 commands
 * 
 * Author: Kegan "lystic" Hollern
 * Last Modified: June 21st 2022
 * 
 */

// Global Accessor
ref SQFVM g_GlobalScriptEngine; // ref should hold this object forever :)
SQFVM GetScriptEngine() {
	if(!g_GlobalScriptEngine) {
		g_GlobalScriptEngine = new SQFVM();
	}
	return g_GlobalScriptEngine;	
}

class SQFVM {
	void SQFVM() 
	{
		Init();
	}
	
	void Init()
	{
		
	}
	
	
	void TestLexer(string script) {
	
		SQFLexer lexer = new SQFLexer(script);
		while(true) {
			SQFToken nextToken = lexer.Next();
			if(nextToken.TokenType() == ESQFTokenType.END_OF_SCRIPT)
				break;
			
			Print(nextToken.Stringify());
		}
		Print("lexing test complete");
	}
	
	protected void tick() 
	{
		// tick the script engine
	}
}



