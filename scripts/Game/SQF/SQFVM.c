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
	
	// same as `LoadFile` script function
	static string LoadScript(ResourceName res)
	{
		Resource holder = BaseContainerTools.LoadContainer(res);
		if(!holder.IsValid()) 
		{
			Print("failed to load script from: " + res, LogLevel.ERROR);
			return "";
		}
		BaseContainer container = holder.GetResource().ToBaseContainer();
		if(!container)
		{
			Print("failed to load container from: " + res, LogLevel.ERROR);
			return "";
		}
		SQF_ScriptConfig sqf_container = SQF_ScriptConfig.Cast(BaseContainerTools.CreateInstanceFromContainer(container));
		if(!sqf_container)
		{
			Print("failed to read script from: " + res, LogLevel.ERROR);
			return "";
		}
		
		return sqf_container.GetScript();
	}
	
	
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
		// what is going to call this? should we register in GetGame().GetCallQueue	and call every 1ms ?
	}
}



