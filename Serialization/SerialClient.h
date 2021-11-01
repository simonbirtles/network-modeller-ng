#pragma once
#include <vector>
#include <tuple>
#include "..\TinyXML-2\tinyxml2.h"

namespace NM {
	namespace Serial{

/**
*
*	Class SerialObserver
*
*   Purpose: Used by clients to recieve notifications on Serialization tasks, such as being notified to;
*	1. Save Database
	2. Database Opened, load new data
*
*/
class SerialClient
{
public:
	SerialClient(void){};
	virtual ~SerialClient(void){};


	virtual bool SaveXMLData(tinyxml2::XMLDocument *destDoc, tinyxml2::XMLNode *currentNode) = 0;
	virtual bool LoadXMLData(tinyxml2::XMLDocument *destDoc, tinyxml2::XMLNode *currentNode) = 0;

};

	} //ns
}; // namespace