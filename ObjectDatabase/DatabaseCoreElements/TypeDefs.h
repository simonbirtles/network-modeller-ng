#pragma once
#include "RealValue.h"
#include <sstream>
#include <codecvt>
#include <vector>
#include <guiddef.h>
#include <cguid.h>
#include <Ws2tcpip.h>

#pragma warning( push )
#pragma warning( disable : 4251 )

namespace NM 
{
	namespace ODB 
	{		
		// Type.h 
		enum class TypeT {TypeString, TypeInt, TypeDouble, TypeBool, TypeLongLong, TypeIPv4, TypeByte, TypeColor, TypeTime, TypeShort, TypeUID, TypeVector, TypeUnknown};

		// Database Type Definitions
		typedef int						ODBInt;
		typedef double					ODBDouble;
		typedef bool					ODBBool;
		typedef unsigned long long		ODBLongLong;
		typedef unsigned char			ODBByte;
		typedef unsigned short			ODBShort;
		typedef GUID					ODBUID;
		typedef in_addr					ODBINADDR;
		typedef tm						ODBTime;
		typedef ::std::wstring			ODBWString;
		typedef unsigned long			ODBColor;

		// forward declare for vector of GUIDs
		class RealUID;
		typedef ::std::vector<RealUID>	ODBVectorUID;
		

		/**
		*
		*	Fundamental Numeric Datatype class
		*
		*/
		template <typename T>
		class  RealBasicType : public RealValue<T>
		{
		public:
			RealBasicType(T val);
			RealBasicType();
			~RealBasicType();
	
			RealBasicType<T>& operator=(const RealBasicType<T> &rhs);		// assignment op
		
			RealValue*	Clone() const;
			//using RealValue<T>::Set;										// required to bring RealValue<T>::Set forward into scope as overloads from baseclass otherwise RealValue<T>::Set gets hidden due to override here as RealBaseType<T>::Set - in this case the str version of Set.
			bool Set(::std::wstring const &wStrValue);
			::std::wstring GetString();
			T Get();

			bool operator==(BaseValue const &rhs);
			bool operator<(BaseValue const &rhs);
			bool operator>(BaseValue const &rhs);

			bool SetXML(tinyxml2::XMLNode *thisAttributeNode);
			bool GetXML(tinyxml2::XMLNode *thisAttributeNode);

		private:
			::std::wstring makeString(::std::true_type);
			::std::wstring makeString(::std::false_type);

		};

		// Derived classes for non fundamental data types

		/**
		*
		*	RealVector class
		*
		*/
		class RealVector :
			public RealValue<ODBVectorUID>
		{
		public:
			RealVector(std::initializer_list<RealUID> vals);
			RealVector(ODBVectorUID val);
			RealVector();
			~RealVector();

			//RealVector(const RealVector &rhs);				// copy ctor
			RealVector& operator=(const RealVector &rhs);			// assignment operator
			RealVector& operator=(const ODBVectorUID &rhs);			// assignment operator

			RealVector*	Clone() const;
			bool Set(::std::wstring const &wStrValue);
			::std::wstring GetString();

			bool SetXML(tinyxml2::XMLNode *thisAttributeNode);
			bool GetXML(tinyxml2::XMLNode *thisAttributeNode);
		private:
			bool operator==(BaseValue const &rhs){ return false; };
			bool operator<(BaseValue const &rhs){ return false; };
			bool operator>(BaseValue const &rhs){ return false; };

		};

		/**
		*
		*	RealString class
		*
		*/
		class RealString :
			public RealValue<ODBWString>
		{
		public:
			RealString();
			RealString(ODBWString val);
			/*RealString(const wchar_t& val);*/
			~RealString();

			//RealString(RealString const &rhs);				// copy ctor
			RealString& operator=(RealString const &rhs);			// assignment operator
			RealString& operator=(wchar_t const *rhs);			// assignment operator

			RealValue*	Clone() const;
			bool Set(::std::wstring const &wStrValue);
			::std::wstring GetString();
			bool operator==(BaseValue const &rhs);
			bool operator<(BaseValue const &rhs);
			bool operator>(BaseValue const &rhs);
			bool SetXML(tinyxml2::XMLNode *thisAttributeNode);
			bool GetXML(tinyxml2::XMLNode *thisAttributeNode);
		private:

		};
		/**
		*
		*	RealUID class (GUID/UUID)
		*
		*/
		class  RealUID :
			public RealValue<ODBUID>
		{
		public:
			RealUID();
			RealUID(ODBUID val);
			~RealUID();			

			//RealUID(RealUID const &rhs);				// copy ctor
			RealUID& operator=(RealUID const &rhs);			// assignment operator
			RealUID& operator=(ODBUID const &rhs);			// assignment operator

			RealValue*	Clone() const;
			bool Set(::std::wstring const &wStrValue);
			::std::wstring GetString();
			bool operator==(BaseValue const &rhs);
			bool operator<(BaseValue const &rhs);
			bool operator>(BaseValue const &rhs);
			bool SetXML(tinyxml2::XMLNode *thisAttributeNode);
			bool GetXML(tinyxml2::XMLNode *thisAttributeNode);

			//bool operator<(RealValue<ODBUID> rhs) const;
		private:

	
		};
		/**
		*
		*	RealTime class (tm struct)
		*
		*/
		class RealTime :
			public RealValue<ODBTime>
		{
		public:
			RealTime(ODBTime val);
			RealTime();
			~RealTime();

			//RealTime(RealTime const &rhs);				// copy ctor
			RealTime& operator=(RealTime const &rhs);			// assignment operator

			RealValue*	Clone() const;
			bool Set(::std::wstring const &wStrValue);
			::std::wstring GetString();
			bool operator==(BaseValue const &rhs);
			bool operator<(BaseValue const &rhs);
			bool operator>(BaseValue const &rhs);
			bool SetXML(tinyxml2::XMLNode *thisAttributeNode);
			bool GetXML(tinyxml2::XMLNode *thisAttributeNode);
		private:

	
		};
		/**
		*
		*	RealInAddr class 
		*
		*/
		class RealInAddr :
			public RealValue<ODBINADDR>
		{
		public:
			//explicit RealInAddr(ODBINADDR val);
			RealInAddr();
			~RealInAddr();

			//RealInAddr(RealInAddr const &rhs);				// copy ctor
			RealInAddr(ODBINADDR const &rhs);				// copy ctor
			RealInAddr& operator=(RealInAddr const &rhs);			// assignment operator
			RealInAddr& operator=(ODBINADDR const &rhs);			// assignment operator

			RealValue*	Clone() const;
			bool Set(::std::wstring const &wStrValue);
			::std::wstring GetString();
			bool operator==(BaseValue const &rhs);
			bool operator<(BaseValue const &rhs);
			bool operator>(BaseValue const &rhs);
			bool SetXML(tinyxml2::XMLNode *thisAttributeNode);
			bool GetXML(tinyxml2::XMLNode *thisAttributeNode);
		private:

		};
		/**
		*
		*	RealByte class 
		*
		*/
		class RealByte :
			public RealValue<ODBByte>
		{
		public:
			RealByte(ODBByte val);
			RealByte();
			~RealByte();

			//RealByte(RealByte const &rhs);				// copy ctor
			RealByte& operator=(RealByte const &rhs);			// assignment operator

			RealValue*	Clone() const;
			bool Set(::std::wstring const &wStrValue);
			::std::wstring GetString();
			bool operator==(BaseValue const &rhs);
			bool operator<(BaseValue const &rhs);
			bool operator>(BaseValue const &rhs);
			bool SetXML(tinyxml2::XMLNode *thisAttributeNode);
			bool GetXML(tinyxml2::XMLNode *thisAttributeNode);
		private:

		};
		/**
		*
		*	RealColor class
		*
		*/
		class  RealColor :
			public RealValue<ODBColor>
		{
		public:
			RealColor();
			RealColor(ODBColor val);
			~RealColor();

			//RealColor(RealColor const &rhs);				// copy ctor
			RealColor& operator=(RealColor const &rhs);			// assignment operator
			RealColor& operator=(ODBColor const &rhs);			// assignment operator

			RealValue*	Clone() const;
			bool Set(::std::wstring const &wStrValue);
			::std::wstring GetString();
			bool operator==(BaseValue const &rhs);
			bool operator<(BaseValue const &rhs);
			bool operator>(BaseValue const &rhs);
			bool SetXML(tinyxml2::XMLNode *thisAttributeNode);
			bool GetXML(tinyxml2::XMLNode *thisAttributeNode);
			ODBColor GetRed();
			ODBColor GetGreen();
			ODBColor GetBlue();
		private:

		};



		
		// Value constructs
		typedef RealBasicType<ODBInt> 		real_int;
		typedef RealBasicType<ODBDouble>	real_double;
		typedef RealBasicType<ODBBool>		real_bool;
		typedef RealBasicType<ODBLongLong>	real_longlong;
		typedef RealBasicType<ODBShort>		real_short;
		typedef RealString					real_string;
		typedef RealUID						real_uid;
		typedef RealTime					real_time;
		typedef RealInAddr					real_inaddr;
		typedef RealByte					real_byte;
		typedef RealVector					real_vector;
		typedef RealColor					real_color;

		
		

// ns

	}
}

#pragma warning( pop )