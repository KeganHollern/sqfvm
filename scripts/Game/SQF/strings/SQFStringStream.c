

class SQFStringStream {
	protected int m_Cursor;
	protected string m_Buffer;
	
	void SQFStringStream(string input) 
	{
		m_Buffer = input;
		m_Cursor = 0;
	}	
	
	// get next character in stream. increment cursor
	string Get() 
	{
		if(!HasNext()) return "";
		string c = m_Buffer.Get(m_Cursor);
		Inc();
		return c;
	}
	// get next character in stream. no increment
	string Peek() 
	{
		if(!HasNext()) return "";
		string c = m_Buffer.Get(m_Cursor);
		return c;
	}
	// increment cursor
	void Inc() 
	{
		m_Cursor++;
	}
	// decrement cursor
	void Dec() 
	{
		m_Cursor--;
	}
	// get cursor index
	int Cursor() 
	{
		return m_Cursor;
	}
	// true if stream has more content
	bool HasNext() 
	{
		return m_Cursor < m_Buffer.Length();
	}
	// set cursor position
	void SetCursor(int idx) 
	{
		m_Cursor = idx;
	}
	// get stream length
	int Length() 
	{
		return m_Buffer.Length();
	}
	
	// get substring from stream
	string GetText(int start, int length) 
	{
		if(start + length > m_Buffer.Length())
		{
			Print("invalid length. beyond stream length. " + start.ToString() + "-" + length.ToString(), LogLevel.WARNING);
			length = m_Buffer.Length() - start;
		}
		return m_Buffer.Substring(start, length);
	}
}