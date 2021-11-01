#include "stdafx.h"
#include "JsonSerialize.h"
#include <cstdlib>
#include <clocale>

namespace NM {
	namespace Serial{

/*
Class JsonSerialize
Usage:

	1. Instansiate Object
	2. Create New Object(s)
	3. Insert string:value pairs
	4. End Object(s)



	JsonSerialize j;

	j.StartObject(::std::string("Vertex"));
	j.StartObject(::std::string("THW-P1"));
	j.InsertStringValue("shortname", ::std::wstring(L"THW-P1"));
	{...}
	j.EndCurrentObject();	// THW-P1
	j.EndCurrentObject();	// Vertex
	

	j.StartObject(::std::string("Edge"));
	j.StartObject(::std::string("THW-FR5(RED)"));
	j.InsertStringValue("v1_Out_Cost", 100);
	{...}
	j.EndCurrentObject();	// THW-FR5(RED)
	j.EndCurrentObject();	// Edge

	{...}
	string str;
	j.GetRawJson(str);
	or
	j.GetFormattedJson(str);


*/

JsonSerialize::JsonSerialize(void)
{
	ClearJson();
}

JsonSerialize::~JsonSerialize(void)
{
}

void JsonSerialize::ClearJson()
{
	_rawjson.clear();
	_formattedjson.clear();
	_numberoftabs = 0;
	_opencurly = 0;
	_closedcurly = 0;
}
void JsonSerialize::WCtMB(const ::std::wstring &ws, ::std::string &mbs)
{
	char chartemp[max_string_length];
	memset(chartemp, 0x0, max_string_length);
	size_t i;

	setlocale(LC_ALL, ".ACP");
	int ret = wcstombs_s(&i, chartemp, max_string_length, ws.c_str(), max_string_length-1);
	if (ret == max_string_length) 
	{
		chartemp[max_string_length-1] = NULL;
	}

	mbs.clear();
	mbs.append(chartemp);

	return;
}

void JsonSerialize::StartObject(const ::std::wstring &strname)	
{
	::std::string svalue;
	WCtMB(strname, svalue);			// local helper function to convert ::std::wstring to ::std::string

	StartObject(svalue);

	return;
}

void JsonSerialize::StartObject(const ::std::string &strname)		
{
	if(strname.size() > max_string_length)
	{
		return;
	}


	// new json
	if(_rawjson.size() == 0)
	{
		// start object
		// add "string": (in string:value pair)
		_rawjson.append(1, QUOTE);
		_rawjson.append(strname);
		_rawjson.append(1, QUOTE);
		_rawjson.append(1, COLON);
		
		_rawjson.append(1, OPEN_CURLY);
		_opencurly++;
		return;
	}
	
	// else we already started so, what was last char
	::std::string::iterator it;
	it = _rawjson.end();
	it--;

	switch(*it)
	{
	case COLON:
		// start of object in object
		// add "string": (in string:value pair)
		_rawjson.append(1, QUOTE);
		_rawjson.append(strname);
		_rawjson.append(1, QUOTE);
		_rawjson.append(1, COLON);
		_rawjson.append(1, OPEN_CURLY);
		_opencurly++;
		break;

	case COMMA:
		// add "string": (in string:value pair)
		_rawjson.append(1, QUOTE);
		_rawjson.append(strname);
		_rawjson.append(1, QUOTE);
		_rawjson.append(1, COLON);
		_rawjson.append(1, OPEN_CURLY);
		_opencurly++;
		break;

	case CLOSE_CURLY:
		_rawjson.append(1, COMMA);
		// add "string": (in string:value pair)
		_rawjson.append(1, QUOTE);
		_rawjson.append(strname);
		_rawjson.append(1, QUOTE);
		_rawjson.append(1, COLON);
		_rawjson.append(1, OPEN_CURLY);
		_opencurly++;
		break;

	case OPEN_CURLY:
		// add "string": (in string:value pair)
		_rawjson.append(1, QUOTE);
		_rawjson.append(strname);
		_rawjson.append(1, QUOTE);
		_rawjson.append(1, COLON);
		_rawjson.append(1, OPEN_CURLY);
		_opencurly++;
		break;

	default:
		return;
		break;
	}


	return;
}

void JsonSerialize::EndCurrentObject()			
{
	::std::string::const_iterator it;
	it = _rawjson.end();
	it--;
	// if we have an open object to close (prevents closing more than we have open and passing a bad object which 
	// screws up the whole json this will be added to
	if( (_opencurly - _closedcurly)  > 0 )
	{
		if(*it == COMMA)		// we auto add COMMA to end of new complete string:value, so as we are closing, replace with }
		{
			_rawjson.pop_back();
		}

		_rawjson.append(1, CLOSE_CURLY);
		_closedcurly++;
		// auto add the COMMA, will be deleted by next EndObject if required
		_rawjson.append(1, COMMA);
	}

	if((_opencurly - _closedcurly) == 0)
	{
		_rawjson.pop_back();
	}

	return;
}
void JsonSerialize::InsertStringValue(const ::std::string &strname, const ::std::string &value)
{
	// check to make sure last is { or , 
	::std::string::iterator it;
	it = _rawjson.end();
	it--;

	if( (*it != COMMA) && (*it != OPEN_CURLY) )
	{
		return;
	}	

	// add "string" (in string:value pair)
	_rawjson.append(1, QUOTE);
	_rawjson.append(strname);
	_rawjson.append(1, QUOTE);
	// add : (in string:value pair)
	_rawjson.append(1, COLON);
	// add value (in string:value pair)
	_rawjson.append(1, QUOTE);
	_rawjson.append(value);
	_rawjson.append(1, QUOTE);
	// auto add the COMMA, will be deleted by EndObject if required
	_rawjson.append(1, COMMA);

	return;
}

void JsonSerialize::InsertStringValue(const ::std::string &strname, const ::std::wstring &value)
{
	// check to make sure last is { or , 
	::std::string::iterator it;
	it = _rawjson.end();
	it--;

	if( (*it != COMMA) && (*it != OPEN_CURLY) )
	{
		return;
	}
	
	::std::string svalue;
	WCtMB(value, svalue);			// local helper function to convert ::std::wstring to ::std::string

	InsertStringValue(strname, svalue);

	return;
}

void JsonSerialize::InsertStringValue(const ::std::string &strname, const int &value)
{
	// check to make sure last is { or , 
	::std::string::iterator it;
	it = _rawjson.end();
	it--;

	if( (*it != COMMA) && (*it != OPEN_CURLY) )
	{
		return;
	}
	
	// add "string" (in string:value pair)
	_rawjson.append(1, QUOTE);
	_rawjson.append(strname);
	_rawjson.append(1, QUOTE);
	// add : (in string:value pair)
	_rawjson.append(1, COLON);
	// add value (in string:value pair)
	_rawjson.append(::std::to_string(value));
	// auto add the COMMA, will be deleted by EndObject if required
	_rawjson.append(1, COMMA);

}
void JsonSerialize::InsertStringValue(const ::std::string &strname, const double &value)	
{
	// check to make sure last is { or , 
	::std::string::iterator it;
	it = _rawjson.end();
	it--;

	if( (*it != COMMA) && (*it != OPEN_CURLY) )
	{
		return;
	}
	
	// add "string" (in string:value pair)
	_rawjson.append(1, QUOTE);
	_rawjson.append(strname);
	_rawjson.append(1, QUOTE);
	// add : (in string:value pair)
	_rawjson.append(1, COLON);
	// add value (in string:value pair)
	_rawjson.append(::std::to_string(value));
	// auto add the COMMA, will be deleted by EndObject if required
	_rawjson.append(1, COMMA);

}
void JsonSerialize::InsertStringValue(const ::std::string &strname, const long long &value)
{
	// check to make sure last is { or , 
	::std::string::iterator it;
	it = _rawjson.end();
	it--;

	if( (*it != COMMA) && (*it != OPEN_CURLY) )
	{
		return;
	}
	
	// add "string" (in string:value pair)
	_rawjson.append(1, QUOTE);
	_rawjson.append(strname);
	_rawjson.append(1, QUOTE);
	// add : (in string:value pair)
	_rawjson.append(1, COLON);
	// add value (in string:value pair)
	_rawjson.append(::std::to_string(value));
	// auto add the COMMA, will be deleted by EndObject if required
	_rawjson.append(1, COMMA);

}
void JsonSerialize::InsertStringValue(const ::std::string &strname, const bool &value)		
{
	// check to make sure last is { or , 
	::std::string::iterator it;
	it = _rawjson.end();
	it--;

	if( (*it != COMMA) && (*it != OPEN_CURLY) )
	{
		return;
	}	

	// add "string" (in string:value pair)
	_rawjson.append(1, QUOTE);
	_rawjson.append(strname);
	_rawjson.append(1, QUOTE);
	// add : (in string:value pair)
	_rawjson.append(1, COLON);
	// add value (in string:value pair)
	if(value)
	{
		_rawjson.append("true");
	}
	else
	{
		_rawjson.append("false");
	}
	// auto add the COMMA, will be deleted by EndObject if required
	_rawjson.append(1, COMMA);
}
// used for NULL value in string:value
void JsonSerialize::InsertStringValue(const ::std::string &strname)	
{
	// check to make sure last is { or , 
	::std::string::iterator it;
	it = _rawjson.end();
	it--;

	if( (*it != COMMA) && (*it != OPEN_CURLY) )
	{
		return;
	}	

	// add "string" (in string:value pair)
	_rawjson.append(1, QUOTE);
	_rawjson.append(strname);
	_rawjson.append(1, QUOTE);
	// add : (in string:value pair)
	_rawjson.append(1, COLON);
	// add value (in string:value pair)
	_rawjson.append("null");
	// auto add the COMMA, will be deleted by EndObject if required
	_rawjson.append(1, COMMA);

}


void JsonSerialize::GetFormattedJson(::std::string &json)
{
	// check _opencurly == _closedcurly, if not return empty str
	if(_opencurly != _closedcurly)
	{
		return;
	}

	::std::string newJson;
	bool instr = false;
	char cchar = NULL;
	int indent = 0;

	for (size_t i=0; i<_rawjson.size(); ++i) 
	{ 
		cchar = _rawjson.at(i);
		switch (cchar)
		{
		case COMMA: 
            if (!instr) 
			{ 
				newJson.append(1, COMMA);
				//newJson.append(1, CR);
				newJson.append(1, LF);
				newJson.append(indent, TAB);
            } 
			else 
			{ 
				newJson.append(1, cchar); 
            } 
            break; 

        case COLON: 
            if (!instr) 
			{ 
				newJson.append(1, COLON); 
            } 
			else 
			{ 
				newJson.append(1, cchar); 
            } 
            break; 

		case OPEN_SQUARE: 
		case OPEN_CURLY: 
            if (!instr) { 
				newJson.append(1, cchar);
				//newJson.append(1, CR);
				newJson.append(1, LF);
				newJson.append(indent + 1, TAB);
                indent++; 
            } 
			else 
			{ 
				newJson.append(1, cchar);
            }
            break; 

		case CLOSE_SQUARE: 
		case CLOSE_CURLY: 
            if (!instr) 
			{ 
                indent--; 
				//newJson.append(1, CR);
				newJson.append(1, LF);
				newJson.append(indent + 1, TAB);
				newJson.append(1, cchar);
            }
			else 
			{ 
				newJson.append(1, cchar); 
            } 
            break; 
				
        case QUOTE: 
			if(i > 0)
			{
				if(_rawjson.at(i-1) != BACKSLASH) 
				{
					instr = !instr; 
				}
			}
			else //must have a quote as first char in this json (not standard but this class produces objects without outer {})
			{
				instr = !instr; 
			}
            newJson.append(1, cchar);
            break;


		case TAB:
        case SPACE:
        case CR:
		case LF:  
            if (instr) 
			{
				newJson.append(1, cchar);
            }
            break;

        default: 
			newJson.append(1, cchar); 
            break;                    
		}
	} 
	
	//copy
	json = newJson;

	return;
}
void JsonSerialize::GetRawJson(::std::string &json)
{
	// check _opencurly == _closedcurly, if not return empty str
	if(_opencurly == _closedcurly)
	{
		json = _rawjson;		// copy
	}
	else
	{
		json.clear();
	}

	return;
}

void JsonSerialize::InsertJsonObject(JsonSerialize *js)
{
	::std::string json;

	js->GetRawJson(json);
	
	_rawjson.append(json);

	return;
}

// ns
}
};
