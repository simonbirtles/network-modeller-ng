#include "stdafx.h"
#include "DatabaseObjectFactory.h"
#include "..\Tables\Table.h"
#include "..\DatabaseCoreElements\Type.h"
#include "..\DataObjects\DatabaseObject.h"
#include "..\DatabaseCoreElements\attribute.h"
#include "..\DatabaseCoreElements\AttributeOptionList.h"


namespace NM 
{
	namespace ODB
	{
		namespace DefaultTables
		{

			/**
			*
			*
			*
			*/
			bool CreateVertexTableSchema(Table* table)
			{
				// L"NodeType"
				//::std::shared_ptr<AttributeOptionList> attrOptionListVertexType;
				//attrOptionListVertexType.reset(new AttributeOptionList(::std::wstring(L"NodeType")));

				AttributeOptionList attrOptionListVertexType(::std::wstring(L"NodeType"));
				attrOptionListVertexType.AddOption(0, L"Cloud", real_int(0));
				attrOptionListVertexType.AddOption(1, L"Router", real_int(1));
				attrOptionListVertexType.AddOption(2, L"Switch", real_int(2));

				// L"TimeUnits"
				//::std::shared_ptr<AttributeOptionList> attrOptionListTimeUnit;
				//attrOptionListTimeUnit.reset(new AttributeOptionList(::std::wstring(L"TimeUnits")));

				AttributeOptionList attrOptionListTimeUnit(::std::wstring(L"TimeUnits"));
				attrOptionListTimeUnit.AddOption(0, L"Year", real_int(0));
				attrOptionListTimeUnit.AddOption(1, L"Month", real_int(1));
				attrOptionListTimeUnit.AddOption(2, L"Day", real_int(2));
				attrOptionListTimeUnit.AddOption(3, L"Hour", real_int(3));
				attrOptionListTimeUnit.AddOption(4, L"Minute", real_int(4));
				attrOptionListTimeUnit.AddOption(5, L"Second", real_int(5));
				attrOptionListTimeUnit.AddOption(6, L"MilliSecond", real_int(6));
				attrOptionListTimeUnit.AddOption(7, L"MicroSecond", real_int(7));
				attrOptionListTimeUnit.AddOption(8, L"NanoSecond", real_int(8));
				attrOptionListTimeUnit.AddOption(9, L"PicoSecond", real_int(9));

				// L"LineTypes"
				//::std::shared_ptr<AttributeOptionList> attrOptionListLineType;
				//attrOptionListLineType.reset(new AttributeOptionList(::std::wstring(L"LineTypes")));

				AttributeOptionList attrOptionListLineType(::std::wstring(L"LineTypes"));
				attrOptionListLineType.AddOption(0, L"Solid", real_int(0));
				attrOptionListLineType.AddOption(1, L"Dash", real_int(1));
				attrOptionListLineType.AddOption(2, L"Dot", real_int(2));
				attrOptionListLineType.AddOption(3, L"DashDot", real_int(3));

				/* vertex specific attributes */
				table->InsertAttribute(Attribute(L"type", TypeT::TypeInt, ATTR_READ_ONLY | ATTR_OPTIONS_LIST | ATTR_SERIALISE | ATTR_VALUE_REQUIRED_ON_CREATE | ATTR_LOCKED_ON_SET, L"Device type i.e. Router, Switch, etc", &real_int(1), attrOptionListVertexType));					// router
				table->InsertAttribute(Attribute(L"inservice", TypeT::TypeBool, ATTR_USER_MODIFY | ATTR_SERIALISE, L"Boolean value representing this vertex active/inactive", &real_bool(true)));
				table->InsertAttribute(Attribute(L"latency_unit", TypeT::TypeInt, ATTR_USER_MODIFY | ATTR_SERIALISE | ATTR_OPTIONS_LIST, L"Unit of latency measurement e.g. sec/ms/us/..", &real_int(7), attrOptionListTimeUnit));		// ms
				table->InsertAttribute(Attribute(L"latency", TypeT::TypeDouble, ATTR_USER_MODIFY | ATTR_SERIALISE, L"RTT latency value through device", &real_double(0.0)));
				table->InsertAttribute(Attribute(L"interfaces", TypeT::TypeVector, ATTR_SERIALISE, L"Interface List"			/* just empty - no default for vector<guid> */));
				table->InsertAttribute(Attribute(L"srng", TypeT::TypeInt, ATTR_USER_MODIFY | ATTR_SERIALISE, L"Group ID of a Shared Risk Node Group", &real_int(0)));

				/* vertex graph gui attributes */
				table->InsertAttribute(Attribute(L"xpos", TypeT::TypeInt, ATTR_SERIALISE, L"X Position in graph", &real_int(10)));
				table->InsertAttribute(Attribute(L"ypos", TypeT::TypeInt, ATTR_SERIALISE, L"Y Position in graph", &real_int(10)));
				table->InsertAttribute(Attribute(L"zpos", TypeT::TypeInt, ATTR_SERIALISE, L"Z Position in graph", &real_int(0)));
				table->InsertAttribute(Attribute(L"imagewidth", TypeT::TypeInt, ATTR_USER_MODIFY | ATTR_SERIALISE, L"Width of icon", &real_int(30)));
				table->InsertAttribute(Attribute(L"imageheight", TypeT::TypeInt, ATTR_USER_MODIFY | ATTR_SERIALISE, L"Height of icon", &real_int(30)));
				table->InsertAttribute(Attribute(L"color", TypeT::TypeColor, ATTR_USER_MODIFY | ATTR_SERIALISE, L"Default image graph color", &real_color(0x00ff0000)));
				table->InsertAttribute(Attribute(L"group", TypeT::TypeUID, ATTR_SERIALISE, L"Object UID of containing group", &real_uid(INVALID_OBJECT_UID)));
				table->InsertAttribute(Attribute(L"linestyle", TypeT::TypeInt, ATTR_USER_MODIFY | ATTR_SERIALISE | ATTR_OPTIONS_LIST, L"Integer representing the line style drawn around icon", &real_int(0), attrOptionListLineType));		// solid
				table->InsertAttribute(Attribute(L"transparency", TypeT::TypeInt, ATTR_USER_MODIFY | ATTR_SERIALISE, L"Integer representing the level of transparancy of the icon", &real_int(0)));

				/* example custom attributes */
				table->InsertAttribute(Attribute(L"corecolor", TypeT::TypeString, ATTR_USER_MODIFY | ATTR_SERIALISE, L"Multi Core Colour Designation"));
				table->InsertAttribute(Attribute(L"region", TypeT::TypeString, ATTR_USER_MODIFY | ATTR_SERIALISE, L"Device Region Location"));
				table->InsertAttribute(Attribute(L"tier", TypeT::TypeString, ATTR_USER_MODIFY | ATTR_SERIALISE, L"Network Architecture - Tiering Level of Node/Vertex"));

				return true;

			}
			bool CreateInterfaceTableSchema(Table* table)
			{
				// L"InterfaceType"
				//::std::shared_ptr<AttributeOptionList> optListInterfaceTypeList;
				//optListInterfaceTypeList.reset(new AttributeOptionList(::std::wstring(L"InterfaceType")));

				AttributeOptionList optListInterfaceTypeList(::std::wstring(L"InterfaceType"));
				optListInterfaceTypeList.AddOption(0, L"Physical-L2", real_int(0));	// Main-NA ? for hosting sub ints ?
				optListInterfaceTypeList.AddOption(1, L"Physical-L3", real_int(1));
				optListInterfaceTypeList.AddOption(2, L"Logical-L2", real_int(2));
				optListInterfaceTypeList.AddOption(3, L"Logical-L3", real_int(3));

				// L"BandwidthUnits"
				//::std::shared_ptr<AttributeOptionList> optListBandwidthUnitsList;
				//optListBandwidthUnitsList.reset(new AttributeOptionList(::std::wstring(L"BandwidthUnits")));

				AttributeOptionList optListBandwidthUnitsList(::std::wstring(L"BandwidthUnits"));
				optListBandwidthUnitsList.AddOption(0, L"bps", real_int(0));
				optListBandwidthUnitsList.AddOption(1, L"Kbps", real_int(1));
				optListBandwidthUnitsList.AddOption(2, L"Mbps", real_int(2));
				optListBandwidthUnitsList.AddOption(3, L"Gbps", real_int(3));
				optListBandwidthUnitsList.AddOption(4, L"Tbps", real_int(4));


				/* interface specific attributes */
				table->InsertAttribute(Attribute(L"type", TypeT::TypeInt, ATTR_READ_ONLY | ATTR_OPTIONS_LIST | ATTR_SERIALISE | ATTR_VALUE_REQUIRED_ON_CREATE | ATTR_LOCKED_ON_SET, L"Interface Type", &real_int(1), optListInterfaceTypeList));		// L3
				table->InsertAttribute(Attribute(L"vertexUID", TypeT::TypeUID, ATTR_SERIALISE, L"Object UID of Interfaces Owning Node", &real_uid(INVALID_OBJECT_UID)));
				table->InsertAttribute(Attribute(L"edgeUID", TypeT::TypeUID, ATTR_SERIALISE, L"Object UID of Edge (Link) Connected to Interface", &real_uid(INVALID_OBJECT_UID)));
				table->InsertAttribute(Attribute(L"intfspeed", TypeT::TypeInt, ATTR_USER_MODIFY | ATTR_SERIALISE, L"Interface Speed (Port Speed)", &real_int(1000)));
				table->InsertAttribute(Attribute(L"intfspeedunit", TypeT::TypeInt, ATTR_USER_MODIFY | ATTR_SERIALISE | ATTR_OPTIONS_LIST, L"enum unit of speed measurement", &real_int(2), optListBandwidthUnitsList));	// Mbps - poss should be global !!

																																																									/* interface L3 attributes*/
				table->InsertAttribute(Attribute(L"rd", TypeT::TypeLongLong, ATTR_USER_MODIFY | ATTR_SERIALISE, L"Route Distinguisher", &real_longlong(0)));
				table->InsertAttribute(Attribute(L"ipaddress", TypeT::TypeIPv4, ATTR_USER_MODIFY | ATTR_SERIALISE, L"Interface IP Address"));
				table->InsertAttribute(Attribute(L"ipprefixlength", TypeT::TypeByte, ATTR_USER_MODIFY | ATTR_SERIALISE, L"IP Address Prefix Length", &real_byte(32)));

				/* interface graph gui attributes */
				table->InsertAttribute(Attribute(L"color", TypeT::TypeColor, ATTR_USER_MODIFY | ATTR_SERIALISE, L"Interface default graph color", &real_color(0x00ff0000)));
				/*
				* needs work for PortGroup(LAG), SubInterfaces, MainInterface(Phy), L2. L3
				*
				*/

				return true;

			}
			bool CreateEdgeTableSchema(Table* table)
			{

				// L"LinkType"
				//::std::shared_ptr<AttributeOptionList> optListLinkTypeList;
				//optListLinkTypeList.reset(new AttributeOptionList(::std::wstring(L"LinkType")));

				AttributeOptionList optListLinkTypeList(::std::wstring(L"LinkType"));
				optListLinkTypeList.AddOption(0, L"Backplane", real_int(0));
				optListLinkTypeList.AddOption(1, L"DWDM", real_int(1));
				optListLinkTypeList.AddOption(2, L"Ethernet", real_int(2));
				optListLinkTypeList.AddOption(3, L"SONET", real_int(3));
				optListLinkTypeList.AddOption(4, L"SDH", real_int(4));
				optListLinkTypeList.AddOption(5, L"ATM", real_int(5));
				optListLinkTypeList.AddOption(6, L"T1", real_int(6));
				optListLinkTypeList.AddOption(7, L"E1", real_int(7));
				optListLinkTypeList.AddOption(8, L"T3", real_int(8));
				optListLinkTypeList.AddOption(9, L"E3", real_int(9));
				optListLinkTypeList.AddOption(10, L"ISDN", real_int(10));
				optListLinkTypeList.AddOption(11, L"SMDS", real_int(11));
				optListLinkTypeList.AddOption(12, L"ADSL", real_int(12));
				optListLinkTypeList.AddOption(13, L"ADSL2", real_int(13));
				optListLinkTypeList.AddOption(14, L"ADSL2+", real_int(14));
				optListLinkTypeList.AddOption(15, L"VDSL", real_int(15));
				optListLinkTypeList.AddOption(16, L"VDSL2", real_int(16));

				//::std::shared_ptr<AttributeOptionList> attrOptionListTimeUnit;
				//attrOptionListTimeUnit.reset(new AttributeOptionList(::std::wstring(L"TimeUnits")));

				AttributeOptionList attrOptionListTimeUnit(::std::wstring(L"TimeUnits"));
				attrOptionListTimeUnit.AddOption(0, L"Year", real_int(0));
				attrOptionListTimeUnit.AddOption(1, L"Month", real_int(1));
				attrOptionListTimeUnit.AddOption(2, L"Day", real_int(2));
				attrOptionListTimeUnit.AddOption(3, L"Hour", real_int(3));
				attrOptionListTimeUnit.AddOption(4, L"Minute", real_int(4));
				attrOptionListTimeUnit.AddOption(5, L"Second", real_int(5));
				attrOptionListTimeUnit.AddOption(6, L"MilliSecond", real_int(6));
				attrOptionListTimeUnit.AddOption(7, L"MicroSecond", real_int(7));
				attrOptionListTimeUnit.AddOption(8, L"NanoSecond", real_int(8));
				attrOptionListTimeUnit.AddOption(9, L"PicoSecond", real_int(9));

				// L"LineTypes"
				//::std::shared_ptr<AttributeOptionList> attrOptionListLineType;
				//attrOptionListLineType.reset(new AttributeOptionList(::std::wstring(L"LineTypes")));

				AttributeOptionList attrOptionListLineType(::std::wstring(L"LineTypes"));
				attrOptionListLineType.AddOption(0, L"Solid", real_int(0));
				attrOptionListLineType.AddOption(1, L"Dash", real_int(1));
				attrOptionListLineType.AddOption(2, L"Dot", real_int(2));
				attrOptionListLineType.AddOption(3, L"DashDot", real_int(3));

				/* edge/link specific attributes */
				table->InsertAttribute(Attribute(L"type", TypeT::TypeInt, ATTR_USER_MODIFY | ATTR_SERIALISE | ATTR_OPTIONS_LIST, L"Link Technology i.e. Ethernet, SONET, SDH, etc", &real_int(2), optListLinkTypeList));		// ethernet
				table->InsertAttribute(Attribute(L"interfaceUID_A", TypeT::TypeUID, ATTR_SERIALISE, L"Object UID of Interface A", &real_uid(INVALID_OBJECT_UID)));
				table->InsertAttribute(Attribute(L"interfaceUID_B", TypeT::TypeUID, ATTR_SERIALISE, L"Object UID of Interface B", &real_uid(INVALID_OBJECT_UID)));
				table->InsertAttribute(Attribute(L"latency", TypeT::TypeDouble, ATTR_USER_MODIFY | ATTR_SERIALISE, L"RTT latency value", &real_double(0.0)));
				table->InsertAttribute(Attribute(L"latency_unit", TypeT::TypeInt, ATTR_USER_MODIFY | ATTR_SERIALISE | ATTR_OPTIONS_LIST, L"Unit of latency measurement e.g. sec/ms/us/..", &real_int(6), attrOptionListTimeUnit));			// poss should be global !!
				table->InsertAttribute(Attribute(L"inservice", TypeT::TypeBool, ATTR_USER_MODIFY | ATTR_SERIALISE, L"Boolean value representing this edge active/inactive", &real_bool(true)));
				table->InsertAttribute(Attribute(L"bandwidth", TypeT::TypeLongLong, ATTR_USER_MODIFY | ATTR_SERIALISE, L"Bandwidth/Capacity of link (Mbps)", &real_longlong(0)));
				table->InsertAttribute(Attribute(L"srlg", TypeT::TypeInt, ATTR_USER_MODIFY | ATTR_SERIALISE, L"Group ID of a Shared Risk Link Group", &real_int(0)));

				/* edge/link graph gui attributes */
				table->InsertAttribute(Attribute(L"color", TypeT::TypeColor, ATTR_USER_MODIFY | ATTR_SERIALISE, L"Objects default graph color", &real_color(0x00000000)));
				table->InsertAttribute(Attribute(L"group", TypeT::TypeUID, ATTR_SERIALISE, L"Object UID of containing group", &real_uid(INVALID_OBJECT_UID)));
				table->InsertAttribute(Attribute(L"linewidth", TypeT::TypeInt, ATTR_USER_MODIFY | ATTR_SERIALISE, L"Integer representing the line width", &real_int(1)));
				table->InsertAttribute(Attribute(L"linestyle", TypeT::TypeInt, ATTR_USER_MODIFY | ATTR_SERIALISE | ATTR_OPTIONS_LIST, L"Integer representing the line style drawn", &real_int(0), attrOptionListLineType));		// solid
				table->InsertAttribute(Attribute(L"transparency", TypeT::TypeInt, ATTR_USER_MODIFY | ATTR_SERIALISE, L"Integer representing the level of transparancy", &real_int(0)));
				table->InsertAttribute(Attribute(L"controlpointAxpos", TypeT::TypeInt, ATTR_SERIALISE, L"control point Ax", &real_int(-1)));	// for beizer line
				table->InsertAttribute(Attribute(L"controlpointAypos", TypeT::TypeInt, ATTR_SERIALISE, L"control point Ay", &real_int(-1)));	// for beizer line
				table->InsertAttribute(Attribute(L"controlpointBxpos", TypeT::TypeInt, ATTR_SERIALISE, L"control point Bx", &real_int(-1)));	// for beizer line
				table->InsertAttribute(Attribute(L"controlpointBypos", TypeT::TypeInt, ATTR_SERIALISE, L"control point By", &real_int(-1)));	// for beizer line

				return true;
			}
			bool CreateFlowTableSchema(Table* table)
			{
				table->InsertAttribute(Attribute(L"active", TypeT::TypeBool, ATTR_USER_MODIFY | ATTR_SERIALISE, L"Flow included in calculations", &real_bool(true)));
				table->InsertAttribute(Attribute(L"srcIP", TypeT::TypeIPv4, ATTR_USER_MODIFY | ATTR_VALUE_REQUIRED_ON_CREATE | ATTR_SERIALISE, L"Source IPv4 Address"));
				table->InsertAttribute(Attribute(L"dstIP", TypeT::TypeIPv4, ATTR_USER_MODIFY | ATTR_VALUE_REQUIRED_ON_CREATE | ATTR_SERIALISE, L"Destination IPv4 Address"));
				table->InsertAttribute(Attribute(L"trafficclass", TypeT::TypeByte, ATTR_USER_MODIFY | ATTR_SERIALISE, L"Traffic Class (DSCP)", &real_byte(0)));
				table->InsertAttribute(Attribute(L"demanduids", TypeT::TypeVector, ATTR_SERIALISE, L"Demand List"			/* just empty - no default for vector<guid> */));

				return true;
			}

			bool CreateDemandTableSchema(Table* table)
			{
				// L"TimeUnits"
				//::std::shared_ptr<AttributeOptionList> attrOptionListTimeUnit;
				//attrOptionListTimeUnit.reset(new AttributeOptionList(::std::wstring(L"TimeUnits")));

				AttributeOptionList attrOptionListTimeUnit(::std::wstring(L"TimeUnits"));
				attrOptionListTimeUnit.AddOption(0, L"Year", real_int(0));
				attrOptionListTimeUnit.AddOption(1, L"Month", real_int(1));
				attrOptionListTimeUnit.AddOption(2, L"Day", real_int(2));
				attrOptionListTimeUnit.AddOption(3, L"Hour", real_int(3));
				attrOptionListTimeUnit.AddOption(4, L"Minute", real_int(4));
				attrOptionListTimeUnit.AddOption(5, L"Second", real_int(5));
				attrOptionListTimeUnit.AddOption(6, L"MilliSecond", real_int(6));
				attrOptionListTimeUnit.AddOption(7, L"MicroSecond", real_int(7));
				attrOptionListTimeUnit.AddOption(8, L"NanoSecond", real_int(8));
				attrOptionListTimeUnit.AddOption(9, L"PicoSecond", real_int(9));

				table->InsertAttribute(Attribute(L"flowuid", TypeT::TypeUID, ATTR_VALUE_REQUIRED_ON_CREATE | ATTR_SERIALISE, L"Flow UID this Demand is related to", &real_uid(INVALID_OBJECT_UID)));
				table->InsertAttribute(Attribute(L"Mbpsavg", TypeT::TypeInt, ATTR_USER_MODIFY | ATTR_SERIALISE, L"Mbps Average", &real_int(0)));
				table->InsertAttribute(Attribute(L"Mbpspeak", TypeT::TypeInt, ATTR_USER_MODIFY | ATTR_SERIALISE, L"Mbps Peak", &real_int(0)));
				table->InsertAttribute(Attribute(L"freq", TypeT::TypeInt, ATTR_USER_MODIFY | ATTR_SERIALISE | ATTR_OPTIONS_LIST, L"Frequency of this demand (i.e. hour, day, week)", &real_int(2), attrOptionListTimeUnit)); // day


				table->InsertAttribute(Attribute(L"flowuid", TypeT::TypeUID, ATTR_SERIALISE | ATTR_VALUE_REQUIRED_ON_CREATE | ATTR_LOCKED_ON_SET, L"Flow UID this Demand is related to", &real_uid(INVALID_OBJECT_UID)));
				table->InsertAttribute(Attribute(L"Mbpsavg", TypeT::TypeInt, ATTR_USER_MODIFY | ATTR_SERIALISE, L"Mbps Average", &real_int(0)));
				table->InsertAttribute(Attribute(L"Mbpspeak", TypeT::TypeInt, ATTR_USER_MODIFY | ATTR_SERIALISE, L"Mbps Peak", &real_int(0)));
				table->InsertAttribute(Attribute(L"freq", TypeT::TypeInt, ATTR_USER_MODIFY | ATTR_SERIALISE | ATTR_OPTIONS_LIST, L"Frequency of this demand (i.e. hour, day, week)", &real_int(2), attrOptionListTimeUnit)); // day
				tm datetime;
				::std::memset(&datetime, 0, sizeof(datetime));
				datetime.tm_mday = 1;
				datetime.tm_isdst = -1;
				datetime.tm_year = 116;		// year 2016
				table->InsertAttribute(Attribute(L"starttime", TypeT::TypeTime, ATTR_USER_MODIFY | ATTR_SERIALISE, L"Time this demand starts", &real_time(datetime)));
				datetime.tm_hour = 23;
				datetime.tm_min = 59;
				datetime.tm_sec = 59;
				table->InsertAttribute(Attribute(L"endtime", TypeT::TypeTime, ATTR_USER_MODIFY | ATTR_SERIALISE, L"Time this demand ends", &real_time(datetime)));

				table->InsertAttribute(Attribute(L"maxlatency", TypeT::TypeDouble, ATTR_USER_MODIFY | ATTR_SERIALISE, L"Maximum latency this demand will tolerate (ms)", &real_double(300)));
				table->InsertAttribute(Attribute(L"maxjitter", TypeT::TypeDouble, ATTR_USER_MODIFY | ATTR_SERIALISE, L"Maximum jitter this demand will tolerate (ms)", &real_double(0)));
				table->InsertAttribute(Attribute(L"minavailability", TypeT::TypeDouble, ATTR_USER_MODIFY | ATTR_SERIALISE, L"Minimum availability this demand requires (%)", &real_double(1)));
				table->InsertAttribute(Attribute(L"ecmpallowed", TypeT::TypeBool, ATTR_USER_MODIFY | ATTR_SERIALISE, L"Can ECMP flows be used on this demand (for fixed paths only)", &real_bool(false)));
				table->InsertAttribute(Attribute(L"active", TypeT::TypeBool, ATTR_USER_MODIFY | ATTR_SERIALISE, L"Demand enabled", &real_bool(true)));
				table->InsertAttribute(Attribute(L"pathuids", TypeT::TypeVector, ATTR_USER_MODIFY | ATTR_SERIALISE, L"Path List"			/* just empty - no default for vector<guid> */));

				return true;

			}
			
			bool CreatePathTableSchema(Table* table)
			{
				table->InsertAttribute(Attribute(L"nodeauid", TypeT::TypeUID, ATTR_SERIALISE | ATTR_VALUE_REQUIRED_ON_CREATE, L"UID of Node A (Ingress)", &real_uid(INVALID_OBJECT_UID)));
				table->InsertAttribute(Attribute(L"nodebuid", TypeT::TypeUID, ATTR_SERIALISE | ATTR_VALUE_REQUIRED_ON_CREATE, L"UID of Node Z (Egress)", &real_uid(INVALID_OBJECT_UID)));

				table->InsertAttribute(Attribute(L"dynamic", TypeT::TypeBool, ATTR_USER_MODIFY | ATTR_SERIALISE, L"Dynamic Path if True, Static Path if False", &real_bool(true)));
				table->InsertAttribute(Attribute(L"attribute", TypeT::TypeString, ATTR_USER_MODIFY | ATTR_SERIALISE, L"Attribute Used For Link Cost In Calculations"));
				table->InsertAttribute(Attribute(L"excludelist", TypeT::TypeBool, ATTR_USER_MODIFY | ATTR_SERIALISE, L"Edge List is Exclude(True) or Include(False)", &real_bool(true)));
				table->InsertAttribute(Attribute(L"edgelist", TypeT::TypeVector, ATTR_SERIALISE | ATTR_VALUE_REQUIRED_ON_CREATE, L"List of Edges to Exclude/Include - See excludelist attribute"			/* just empty - no default for vector<guid> */));

				table->InsertAttribute(Attribute(L"active", TypeT::TypeBool, ATTR_USER_MODIFY | ATTR_SERIALISE, L"Is Path Included in Calculations", &real_bool(true)));
				table->InsertAttribute(Attribute(L"comments", TypeT::TypeString, ATTR_USER_MODIFY | ATTR_SERIALISE, L"Path Comments"));
				table->InsertAttribute(Attribute(L"edges", TypeT::TypeVector, ATTR_SERIALISE | ATTR_VALUE_REQUIRED_ON_CREATE, L"Edge List"			/* just empty - no default for vector<guid> */));

				return true;
			}

			bool CreateGroupTableSchema(Table* table)
			{
				// L"LineTypes"
				//::std::shared_ptr<AttributeOptionList> attrOptionListLineType;
				//attrOptionListLineType.reset(new AttributeOptionList(::std::wstring(L"LineTypes")));

				AttributeOptionList attrOptionListLineType(::std::wstring(L"LineTypes"));
				attrOptionListLineType.AddOption(0, L"Solid", real_int(0));
				attrOptionListLineType.AddOption(1, L"Dash", real_int(1));
				attrOptionListLineType.AddOption(2, L"Dot", real_int(2));
				attrOptionListLineType.AddOption(3, L"DashDot", real_int(3));


				/* group graph gui attributes */
				table->InsertAttribute(Attribute(L"xpos", TypeT::TypeInt, ATTR_SERIALISE, L"X Position in graph", &real_int(10)));
				table->InsertAttribute(Attribute(L"ypos", TypeT::TypeInt, ATTR_SERIALISE, L"Y Position in graph", &real_int(10)));
				table->InsertAttribute(Attribute(L"zpos", TypeT::TypeInt, ATTR_SERIALISE, L"Z Position in graph", &real_int(0)));
				table->InsertAttribute(Attribute(L"imagewidth", TypeT::TypeInt, ATTR_USER_MODIFY | ATTR_SERIALISE, L"Width of icon", &real_int(30)));
				table->InsertAttribute(Attribute(L"imageheight", TypeT::TypeInt, ATTR_USER_MODIFY | ATTR_SERIALISE, L"Height of icon", &real_int(30)));
				table->InsertAttribute(Attribute(L"color", TypeT::TypeColor, ATTR_USER_MODIFY | ATTR_SERIALISE, L"Default image graph color", &real_color(0x00ff0000)));
				table->InsertAttribute(Attribute(L"group", TypeT::TypeUID, ATTR_SERIALISE, L"Object UID of containing group", &real_uid(INVALID_OBJECT_UID)));
				table->InsertAttribute(Attribute(L"linestyle", TypeT::TypeInt, ATTR_USER_MODIFY | ATTR_SERIALISE | ATTR_OPTIONS_LIST, L"Integer representing the line style drawn around icon", &real_int(0), attrOptionListLineType));	// solid
				table->InsertAttribute(Attribute(L"transparency", TypeT::TypeInt, ATTR_USER_MODIFY | ATTR_SERIALISE, L"Integer representing the level of transparancy of the icon", &real_int(0)));

				return true;
			}

			bool CreateLayerTableSchema(Table* table)
			{
				return true;
			}



			///**
			//* AddFlowProtocol
			//*
			//* Check for existing protocol, delete if different, add new protocol attributes
			//* Will remove a protocol and not add new if None is specified for IPProtocol
			//*
			//*/
			//void DatabaseObjectFactory::AddFlowProtocol(IPProtocol Protocol, DatabaseObject* flow)
			//{
			//	assert(false);
			//	//if( flow == nullptr )
			//	//{
			//	//	return;
			//	//}

			//	//// delete existing protocol if one exists, prob should not exist as dont think we will expose this function to clients, 
			//	//// we expect that of the protocol needs to be changed, the client should delete existing and create new one.
			//	//if( flow->FindAttribute(L"protocol") != INVALID_ATTRIBUTE)
			//	//{
			//	//	Value v( *( flow->GetValue(L"protocol")) );	
			//	//	IPProtocol prot = static_cast<IPProtocol>(v.Get<ODBInt>());
			//	//	switch(prot)
			//	//	{
			//	//		case IPProtocol::TCP:
			//	//		case IPProtocol::UDP:
			//	//			flow->DeleteAttribute(L"srcPort");
			//	//			flow->DeleteAttribute(L"dstPort");
			//	//			break;

			//	//		case IPProtocol::ICMP:
			//	//			flow->DeleteAttribute(L"type");
			//	//			flow->DeleteAttribute(L"code");
			//	//			break;
			//	//		default:
			//	//			break;
			//	//	}
			//	//	flow->DeleteAttribute(L"protocol");
			//	//}


			//	//// add new protocol attributes with defaults
			//	//switch(Protocol)
			//	//{
			//	//case IPProtocol::TCP:
			//	//	flow->InsertAttribute(Attribute(L"protocol",	TypeT::TypeInt,		false,	true, true, true,		L"Protocol",				&real_int(static_cast<int>(IPProtocol::TCP)) ));
			//	//	flow->InsertAttribute(Attribute(L"srcPort",		TypeT::TypeShort,	false,	true, true, true,		L"Source TCP Port",			&real_short(0) ));
			//	//	flow->InsertAttribute(Attribute(L"dstPort",		TypeT::TypeShort,	false,	true, true, true,		L"Destination TCP Port",	&real_short(0) ));		
			//	//	break;

			//	//case IPProtocol::UDP:
			//	//	flow->InsertAttribute(Attribute(L"protocol",	TypeT::TypeInt,		false,	true, true, true,		L"Protocol",				&real_int(static_cast<int>(IPProtocol::UDP)) ));
			//	//	flow->InsertAttribute(Attribute(L"srcPort",		TypeT::TypeShort,	false,	true, true, true,		L"Source UDP Port",			&real_short(0) ));
			//	//	flow->InsertAttribute(Attribute(L"dstPort",		TypeT::TypeShort,	false,	true, true, true,		L"Destination UDP Port",	&real_short(0) ));		
			//	//	break;

			//	//case IPProtocol::ICMP:
			//	//	flow->InsertAttribute(Attribute(L"protocol",	TypeT::TypeInt,		false,	true, true, true,		L"Protocol",				&real_int(static_cast<int>(IPProtocol::ICMP)) ));
			//	//	flow->InsertAttribute(Attribute(L"type",		TypeT::TypeShort,	false,	true, true, true,		L"ICMP Type",				&real_short(255) ));
			//	//	flow->InsertAttribute(Attribute(L"code",		TypeT::TypeShort,	false,	true, true, true,		L"ICMP Code",				&real_short(255) ));		
			//	//	break;

			//	//case IPProtocol::None:
			//	//default:
			//	//	// TODO LOG Log(Unsupported Protocol Type)
			//	//	break;
			//	//}

			//	return;
			//}


	// ns
		}
	}
}
