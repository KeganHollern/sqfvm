[BaseContainerProps(configRoot: true)]
class SQF_ScriptConfig
{
	[Attribute("", UIWidgets.EditBox, "SQF Script")]
	protected string m_sScriptCode;
	
	
	string GetScript()
	{
		return m_sScriptCode;
	}
};