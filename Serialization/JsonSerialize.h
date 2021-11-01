#pragma once

#include <string>
#include <memory>

namespace NM {
	namespace Serial{


class JsonSerialize
{

private:
	static const char OPEN_CURLY		= 0x7b;
	static const char CLOSE_CURLY		= 0x7d;
	static const char OPEN_SQUARE		= 0x5b;
	static const char CLOSE_SQUARE		= 0x5d;
	static const char QUOTE				= 0x22;
	static const char COLON				= 0x3a;
	static const char COMMA				= 0x2c;
	static const char CR				= 0x0d;
	static const char LF				= 0x0a;
	static const char TAB				= 0x09;
	static const char BACKSLASH			= 0x5c;
	static const char SPACE				= 0x20;
	static const int max_string_length	= 0xff;

	::std::string	_rawjson;													// raw json without spaces, tabs, new lines etc
	::std::string	_formattedjson;												// pretty laid out json
	int				_numberoftabs;												// how many indents are required at this level
	int				_opencurly;													// how many open objects do we have
	int				_closedcurly;												// how many complete objects do we have


	void FormatJson();															// makes json pretty
	void InsertIndent();														// indents based on number of tabs 
	::std::string RepeatTab(int number);
	
	// helper functions
	void WCtMB(const ::std::wstring &ws, ::std::string &mbs);		
	

public:
	JsonSerialize(void);
	virtual ~JsonSerialize(void);

	void ClearJson();															// clears the json currently stored
	void GetFormattedJson(::std::string &json);									// Get formatted Json
	void GetRawJson(::std::string &json);										// Get raw Json
	void StartObject(const ::std::string &strname);									// start/open new object  ' { '			
	void StartObject(const ::std::wstring &strname);									// start/open new object  ' { '			
	void EndCurrentObject();															// end/close object       ' } '  
	void InsertStringValue(const ::std::string &strname, const ::std::string &value);		// string
	void InsertStringValue(const ::std::string &strname, const ::std::wstring &value);		// string
	void InsertStringValue(const ::std::string &strname, const int &value);					// number Integer/real
	void InsertStringValue(const ::std::string &strname, const double &value);				// number rational/real
	void InsertStringValue(const ::std::string &strname, const long long &value);				// number rational/real
	void InsertStringValue(const ::std::string &strname, const bool &value);				// boolean value
	void InsertStringValue(const ::std::string &strname);								// insert with NULL value
	void InsertJsonObject(JsonSerialize *js);									// insert an existing Json object
	
					// WideCharToMultiByte
};


}
}