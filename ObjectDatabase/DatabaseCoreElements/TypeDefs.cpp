#include "stdafx.h"
#include "TypeDefs.h"
#include "RealValue.h"
#include "Type.h"
#include "..\TinyXML-2\tinyxml2.h"

#include <time.h> 
#include <sstream>
#include <iomanip>
#include <map>
#include <locale>

extern void WCtMB(const ::std::wstring &ws, ::std::string &mbs);
extern void MBtWC(::std::wstring &ws, const ::std::string &mbs);

namespace NM 
{
	namespace ODB 
	{	

	class BaseValue;

	/// init static map in  CType
	std::map<TypeT, BaseValue*> CType::_prototypes;
	

#pragma region RealBasicTypeClass

	template class RealBasicType<ODBInt>;
	template class RealBasicType<ODBDouble>;
	template class RealBasicType<ODBBool>;
	template class RealBasicType<ODBLongLong>;
	template class RealBasicType<ODBShort>;
	
	template <class T>
	RealBasicType<T>::RealBasicType(T val) : 
		RealValue<T>(val)
	{
	}

	template <class T>
	RealBasicType<T>::RealBasicType() : 
		RealValue<T>(0)
	{
	}

	template <class T>
	RealBasicType<T>::~RealBasicType()
	{
	}


	template <class T>
	RealBasicType<T>& RealBasicType<T>::operator=(const RealBasicType<T> &rhs)		// assignment op
	{
		if (&rhs == this) return *this;
		RealValue<T>::Set(Value(rhs));
		return *this;
	}

	template <class T>
	RealValue<T>*	RealBasicType<T>::Clone() const
	{
		return new RealBasicType<T>(*this);
	}

	template <class T>
	T RealBasicType<T>::Get()
	{
		return RealValue<T>::Get();																	
	}

	template <class T>
	bool RealBasicType<T>::Set(::std::wstring const &wStrValue)
	{
		if (!::std::is_fundamental<T>())
		{
			throw std::invalid_argument("RealValue<T>::FromWString() DataType T in RealValue<T> is not a fundamental type");
		}

		::std::wistringstream iss(wStrValue);
		T parsed;
		if (iss >> parsed)
		{
			return RealValue<T>::Set(parsed);												
		}
		return false;
	}

	template <class T>
	::std::wstring RealBasicType<T>::GetString()
	{
		return makeString(::std::is_fundamental<T>());
	}

	template <class T>
	bool RealBasicType<T>::operator==(BaseValue const &rhs)
	{
		return (Get() == Value(rhs).Get<T>());															
	}

	template <class T>
	bool RealBasicType<T>::operator<(BaseValue const &rhs)
	{
		return (Get() < Value(rhs).Get<T>());																
	}

	template <class T>
	bool RealBasicType<T>::operator>(BaseValue const &rhs)
	{
		return (Get() > Value(rhs).Get<T>());															
	}

	template <class T>
	bool RealBasicType<T>::SetXML(tinyxml2::XMLNode *thisAttributeNode)
	{
		// thisAttributeNode is the <attribute name='...'> so children are the values
		for (tinyxml2::XMLNode *valueNode = thisAttributeNode->FirstChild(); valueNode; valueNode = valueNode->NextSibling())
		{
			::std::string strValue = valueNode->Value();
			if ((strValue == "value") && (!valueNode->NoChildren()))
			{
				// only one child of a value which is the value, get the value itself
				tinyxml2::XMLNode *value = valueNode->FirstChild();
				// get value string
				::std::string val = value->Value();
				// convert 
				::std::wstring wstrValue;
				MBtWC(wstrValue, val);
				// and set
				if (!Set(wstrValue))
				{
					return false;
				}
			}
		}
		return true;
	}

	template <class T>
	bool RealBasicType<T>::GetXML(tinyxml2::XMLNode *thisAttributeNode)
	{
		// insert value into passed attribute node 
		tinyxml2::XMLDocument* destDoc = thisAttributeNode->GetDocument();
		// create value xml
		tinyxml2::XMLElement *value = destDoc->NewElement("value");
		thisAttributeNode->InsertEndChild(value);
		// now have <value></value>, add child node for actual value
		::std::string strValue;
		// get value as wstring and convert to string
		WCtMB(this->GetString(), strValue);
		// set the value in this attribute node so... <value> strValue </value>
		value->SetText(strValue.c_str());
		return true;
	}

	template <class T>
	::std::wstring RealBasicType<T>::makeString(::std::true_type)
	{
		return ::std::to_wstring(Get());
	};

	template <class T>
	::std::wstring RealBasicType<T>::makeString(::std::false_type)
	{
		throw std::invalid_argument("RealValue<T>::FromWString() DataType T in RealValue<T> is not a fundamental type");
		return ::std::wstring(L"");
	}

#pragma endregion RealBasicTypeClass




#pragma region RealVectorClass
		/**
		*
		*	RealVector class
		*
		*/
		RealVector::RealVector(ODBVectorUID val):
				RealValue<ODBVectorUID>(val)
		{
		}
		RealVector::RealVector(std::initializer_list<RealUID> vals) :
			RealValue<ODBVectorUID>(vals)
		{
		}
		RealVector::RealVector() :
			RealValue<ODBVectorUID>({})
		{
		}
		/**
		* dtor
		*
		*/
		RealVector::~RealVector()
		{
		}
		/**
		* copy ctor
		*
		*/
		//RealVector::RealVector(RealVector const &rhs):				
		//	RealValue<ODBVectorUID>(rhs)
		//{
		//}
		/**
		* assignment operator (obliterate our _val and replace with rhs._val)
		*
		*/
		RealVector& RealVector::operator=(const RealVector &rhs)			
		{
			// clean ours (not sure if we need to do this, but if we had pointers.???!?!)
			//_val.clear();
			// copy rhs Vector to ours.
			//_val = rhs._val;
			RealValue::Set(Value(rhs));
			return *this;
		}

		RealVector& RealVector::operator=(const ODBVectorUID &rhs)
		{
			RealValue::Set(rhs);
			//_val = rhs;
			return *this;
		}
		/**
		*
		*
		*/
		RealVector* RealVector::Clone() const
		{ 
			return new RealVector(*this);		// call copy ctor
		}
		/**
		* RealVector::Set
		* 
		* Set Value from wstring
		* Expects NM Types as strings and be delimited by '\n'
		* Does delete existing entires, then adds new ones
		*
		*/
		bool RealVector::Set(::std::wstring const &wStrValue)
		{
			ODBVectorUID newVec;
			//_val.clear();
			// take string and split into elements using \n as delimiter into a vector of wstrings
			std::vector<std::wstring> tokens;
			std::size_t start = 0, end = 0;
			while ((end = wStrValue.find('\n', start)) != std::string::npos) 
			{
				tokens.push_back(wStrValue.substr(start, end - start));
				start = end + 1;
			}
			tokens.push_back(wStrValue.substr(start));

			std::vector<std::wstring>::iterator it = tokens.begin();
			// for each wstring elment in the vector, create a new container object and call Set(wstring) method
			while( it != tokens.end() )
			{
				// create var for the NM datatype (value type) stored in the vector
				ODBVectorUID::value_type value;
				// call the set(wstring) method on the value
				value.Set(*it);
				// add the NM datatype value to the _val vector
				newVec.push_back(value);
				//_val.push_back( value );
				// next value
				++it;
			}

			RealValue::Set(newVec);

			return true;
		}
		/**
		* RealVector::GetString()
		*
		* Convert value to WString and return as WString
		* Each value is delimited by '\n' in the string returned
		* 
		*/
		::std::wstring RealVector::GetString()
		{
			ODBVectorUID vecVal = Get();
			::std::wstring strValue;
			ODBVectorUID::iterator it = vecVal.begin();
			while( it != vecVal.end() )
			{
				strValue += (*it).GetString();
				strValue += ('\n');
				++it;
			}
			return ::std::move(strValue);
		}
		/**
		*
		*
		*/
		//bool RealVector::operator==(BaseValue const &rhs)
		//{
		//	return _val == RealValue<ODBVectorUID>::operator NM::ODB::ODBVectorUID();
		//}
		///**
		//*
		//*
		//*/
		//bool RealVector::operator<(BaseValue const &rhs)
		//{
		//	return _val < RealValue<ODBVectorUID>::operator NM::ODB::ODBVectorUID();
		//}
		///**
		//*
		//*
		//*/
		//bool RealVector::operator>(BaseValue const &rhs)
		//{
		//	return _val > RealValue<ODBVectorUID>::operator NM::ODB::ODBVectorUID();
		//}
		/**
		*
		*
		*/
		bool RealVector::SetXML(tinyxml2::XMLNode *thisAttributeNode)
		{
			::std::wstring stringValue;
			// thisAttributeNode is the <attribute name='...'> so children are the values
			for( tinyxml2::XMLNode *valueNode=thisAttributeNode->FirstChild(); valueNode; valueNode=valueNode->NextSibling() )
			{
				::std::string strValue = valueNode->Value();
				if( ( strValue == "value") && ( !valueNode->NoChildren() ) )
				{
					// only one child of a single <value>x</value> which is the value, get the value itself x
					tinyxml2::XMLNode *value = valueNode->FirstChild();
					// get value string
					::std::string val = value->Value();
					// convert 
					::std::wstring wstrValue;
					MBtWC(wstrValue, val);
					// and set
					if( stringValue.size() > 0)
						stringValue += '\n';
					
					stringValue += wstrValue;	
				}
			}		
			return this->Set(stringValue);
		}
		/**
		*
		*
		*/
		bool RealVector::GetXML(tinyxml2::XMLNode *thisAttributeNode)
		{
			// insert value into passed attribute node 
			tinyxml2::XMLDocument* destDoc = thisAttributeNode->GetDocument();

			ODBVectorUID vecVal = Get();
			ODBVectorUID::iterator it = vecVal.begin();
			while( it != vecVal.end() )
			{
				// get value[x] from the vector element which is a NM type wrapper which has GetString method
				::std::wstring wstrValue = (*it).GetString();
				// create value xml
				tinyxml2::XMLElement *value = destDoc->NewElement( "value" );
				thisAttributeNode->InsertEndChild(value);
				// now have <value></value>, add child node for actual value
				::std::string strValue;
				// get value as wstring and convert to string
				WCtMB(wstrValue, strValue);
				// set the value in this attribute node so... <value> strValue </value>
				value->SetText(strValue.c_str());
				// next element in the vector
				++it;
			}
			return true;
		}



#pragma endregion RealVectorClass

#pragma region RealStringClass
		/**
		*
		*	RealString class
		*
		*/
		RealString::RealString(ODBWString val):
				RealValue<ODBWString>(val)
		{
		}
		RealString::RealString():
			RealValue<ODBWString>(L"")
		{
		}
		//RealString::RealString(const wchar_t& val) :
		//	RealValue<ODBWString>(val)
		//{

		//}
		/**
		*
		*
		*/
		RealString::~RealString()
		{
		}
		/**
		* copy constructor - should be in clone only acording to C++ rules, polymorphic types should use clone() for copy to avoid splicing.
		* Disable copy ctor and dont allow direct copying at this level ? 
		*/		
		//RealString::RealString(const RealString& rhs):
		//	RealValue<ODBWString>(rhs._val) 
		//{
		//}
		/**
		* assignment operator
		*
		*/			
		RealString& RealString::operator=(const RealString &rhs)
		{
			RealValue::Set(Value(rhs));
			//_val = rhs._val;
			return *this;
		}

		RealString& RealString::operator=(wchar_t const *rhs)
		{
			RealValue::Set(Value(real_string(rhs)));
			//_val = rhs;
			return *this;
		}
		/**
		*
		*
		*/
		RealValue<ODBWString>*	RealString::Clone() const
		{ 
			return new RealString(*this);		// call copy ctor
		}
		/**
		*
		* Set Value from wstring
		*/
		bool RealString::Set(::std::wstring const &wStrValue)
		{
			RealValue::Set( Value(real_string(wStrValue)) );
			//_val = wStrValue;
			return true;
		}
		/**
		*
		* Convert value to WString and return as WString
		*/
		::std::wstring RealString::GetString()
		{
			return RealValue<ODBWString>::operator NM::ODB::ODBWString();
		}
		/**
		*
		*
		*/
		bool RealString::operator==(BaseValue const &rhs)
		{
			return Get() == (reinterpret_cast<RealValue<ODBWString> const &>(rhs).operator NM::ODB::ODBWString());
			//return _val == (reinterpret_cast<RealValue<ODBWString> const &>(rhs).operator NM::ODB::ODBWString() );
		}
		/**
		*
		*
		*/
		bool RealString::operator<(BaseValue const &rhs)
		{
			return Get() < (reinterpret_cast<RealValue<ODBWString> const &>(rhs).operator NM::ODB::ODBWString());
			//return _val < (reinterpret_cast<RealValue<ODBWString> const &>(rhs).operator NM::ODB::ODBWString() );
		}
		/**
		*
		*
		*/
		bool RealString::operator>(BaseValue const &rhs)
		{
			return Get() > (reinterpret_cast<RealValue<ODBWString> const &>(rhs).operator NM::ODB::ODBWString());
			//return _val > (reinterpret_cast<RealValue<ODBWString> const &>(rhs).operator NM::ODB::ODBWString() );
		}

		bool RealString::SetXML(tinyxml2::XMLNode *thisAttributeNode)
		{
			// thisAttributeNode is the <attribute name='...'> so children are the values
			for( tinyxml2::XMLNode *valueNode=thisAttributeNode->FirstChild(); valueNode; valueNode=valueNode->NextSibling() )
			{
				::std::string strValue = valueNode->Value();
				if( ( strValue == "value") && ( !valueNode->NoChildren() ) )
				{
					// only one child of a value which is the value, get the value itself
					tinyxml2::XMLNode *value = valueNode->FirstChild();
					// get value string
					::std::string val = value->Value();
					// convert 
					::std::wstring wstrValue;
					MBtWC(wstrValue, val);
					// and set
					if( !Set(wstrValue) )
					{
						return false;
					}
				}				
			}			
			return true;
		}
		/**
		*
		*
		*/
		bool RealString::GetXML(tinyxml2::XMLNode *thisAttributeNode)
		{
			// insert value into passed attribute node 
			tinyxml2::XMLDocument* destDoc = thisAttributeNode->GetDocument();
			// create value xml
			tinyxml2::XMLElement *value = destDoc->NewElement( "value" );
			thisAttributeNode->InsertEndChild(value);
			// now have <value></value>, add child node for actual value
			::std::string strValue;
			// get value as wstring and convert to string
			WCtMB(this->GetString(), strValue);
			// set the value in this attribute node so... <value> strValue </value>
			value->SetText(strValue.c_str());
			return true;
		}



#pragma endregion RealStringClass

#pragma region RealUIDClass

		/**
		*
		*	RealUID class (GUID/UUID)
		*
		*/
		RealUID::RealUID(ODBUID val):
				RealValue(val)
		{
		}
		RealUID::RealUID():
				RealValue(GUID_NULL)
		{
		}
		/**
		*
		*
		*/
		RealUID::~RealUID()
		{
		}
		/**
		* copy ctor
		*
		*/
		//RealUID::RealUID(RealUID const &rhs):				
		//	RealValue(rhs._val)
		//{
		//}
		/**
		* assignment operator
		*
		*/
		RealUID& RealUID::operator=(RealUID const &rhs)			
		{
			RealValue::Set(Value(rhs));
			//_val = rhs._val;
			return *this;
		}

		RealUID& RealUID::operator=(ODBUID const &rhs)
		{
			//_val = rhs;
			RealValue::Set(rhs);
			return *this;
		}
		/**
		*
		*
		*/
		RealValue<ODBUID>*	RealUID::Clone() const
		{ 
			return new RealUID(*this);			// call copy ctor
		}
		/**
		*
		* Set Value from wstring
		*/
		bool RealUID::Set(::std::wstring const &wStrValue)
		{
			UUID myuuid;
			HRESULT hr = CLSIDFromString(wStrValue.c_str(), &myuuid);
			if( hr == NOERROR )
			{
				//_val = myuuid; 
				RealValue::Set(myuuid);
				return true;
			}
			return false;
		}
		/**
		*
		* Convert value to WString and return as WString
		*/
		::std::wstring RealUID::GetString()
		{			
			OLECHAR* bstrGuid;
			HRESULT hr = ::StringFromCLSID(Get(), &bstrGuid);
			::std::wstring guidstr = bstrGuid;
			::CoTaskMemFree(bstrGuid);
			return guidstr;
		}
		/**
		* 
		*
		*/
		bool RealUID::operator==(BaseValue const &rhs)
		{
			ODBUID guid2 = (reinterpret_cast<RealValue<ODBUID> const &>(rhs).operator NM::ODB::ODBUID() ); 
			return (IsEqualGUID(Get(), guid2) == TRUE ? true : false) ;
		}
		/**
		*
		*
		*/
		bool RealUID::operator<(BaseValue const &rhs)
		{
			ODBUID guid1 = Get();
			ODBUID guid2 = (reinterpret_cast<RealValue<ODBUID> const &>(rhs).operator NM::ODB::ODBUID() ); 
			

			if(guid1.Data1!=guid2.Data1)
			{
				return guid1.Data1 < guid2.Data1;
			}

			if(guid1.Data2!=guid2.Data2)
			{
				return guid1.Data2 < guid2.Data2;
			}

			if(guid1.Data3!=guid2.Data3)
			{
				return guid1.Data3 < guid2.Data3;
			}

			for(int i=0;i<8;i++) 
			{
				if(guid1.Data4[i]!=guid2.Data4[i])
				{
					return guid1.Data4[i] < guid2.Data4[i];
				}
			}
			return false ;
		}
		/**
		*
		*
		*/
		bool RealUID::operator>(BaseValue const &rhs)
		{
			ODBUID guid1 = Get();
			ODBUID guid2 = (reinterpret_cast<RealValue<ODBUID> const &>(rhs).operator NM::ODB::ODBUID() ); 
			
			if(guid1.Data1!=guid2.Data1)
			{
				return guid1.Data1 > guid2.Data1;
			}
			if(guid1.Data2!=guid2.Data2)
			{
				return guid1.Data2 > guid2.Data2;
			}
			if(guid1.Data3!=guid2.Data3)
			{
				return guid1.Data3 > guid2.Data3;
			}
			for(int i=0;i<8;i++) 
			{
				if(guid1.Data4[i]!=guid2.Data4[i])
				{
					return guid1.Data4[i] > guid2.Data4[i];
				}
			}
			return false ;
		}
		/**
		*
		*
		*/
		bool RealUID::SetXML(tinyxml2::XMLNode *thisAttributeNode)
		{
			// thisAttributeNode is the <attribute name='...'> so children are the values
			for( tinyxml2::XMLNode *valueNode=thisAttributeNode->FirstChild(); valueNode; valueNode=valueNode->NextSibling() )
			{
				::std::string strValue = valueNode->Value();
				if( ( strValue == "value") && ( !valueNode->NoChildren() ) )
				{
					// only one child of a value which is the value, get the value itself
					tinyxml2::XMLNode *value = valueNode->FirstChild();
					// get value string
					::std::string val = value->Value();
					// convert 
					::std::wstring wstrValue;
					MBtWC(wstrValue, val);
					// and set
					if( !Set(wstrValue) )
					{
						return false;
					}
				}				
			}			
			return true;
		}
		/**
		*
		*
		*/
		bool RealUID::GetXML(tinyxml2::XMLNode *thisAttributeNode)
		{			
			// insert value into passed attribute node 
			tinyxml2::XMLDocument* destDoc = thisAttributeNode->GetDocument();
			// create value xml
			tinyxml2::XMLElement *value = destDoc->NewElement( "value" );
			thisAttributeNode->InsertEndChild(value);
			// now have <value></value>, add child node for actual value
			::std::string strValue;
			// get value as wstring and convert to string
			WCtMB(this->GetString(), strValue);
			// set the value in this attribute node so... <value> strValue </value>
			value->SetText(strValue.c_str());

			return true;
		}

#pragma endregion RealUIDClass

#pragma region RealTimeClass
		/**
		*
		*	RealTime class (tm struct)
		*
		*/
		RealTime::RealTime(ODBTime val):
				RealValue(val)
		{
		}

		RealTime::RealTime() :
			RealValue({ 0 })
		{
		}
		/**
		*
		*
		*/
		RealTime::~RealTime()
		{
		}
		/**
		* copy ctor
		*
		*/
		//RealTime::RealTime(RealTime const &rhs):				
		//	RealValue(rhs._val)
		//{
		//}
		/**
		* assignment operator
		*
		*/
		RealTime& RealTime::operator=(RealTime const &rhs)			
		{
			RealValue::Set(Value(rhs));
			//_val = rhs._val;
			return *this;
		}
		/**
		*
		*
		*/
		RealValue<ODBTime>*	RealTime::Clone() const
		{ 
			return new RealTime(*this);					// call copy ctor
		}
		/**
		*
		* Set Value from wstring
		* format: '11/01/2016T20:17:22Z' (T denotes dtart of Time part, Z denotes UTC zone).
		*
		*/
		bool RealTime::Set(::std::wstring const &wStrValue)
		{
			const ::std::wstring dateTimeFormat( L"%d/%m/%y %H:%M:%S" );			
			// Create a stream which we will use to parse the string,
		   // which we provide to constructor of stream to fill the buffer.
		   ::std::wistringstream ss( wStrValue );
		   ss.imbue(std::locale("English_United Kingdom.1252"));

		   // Create a tm object to store the parsed date and time.
		   ::std::tm dt;
		   ::std::memset(&dt, 0, sizeof(dt));

		   // Now we read from buffer using get_time 
		   // and format the input 
		   ss >> ::std::get_time(&dt, dateTimeFormat.c_str());

		   RealValue::Set(dt);
		   //_val = dt;

		   return true;
		}
		/**
		*
		* Convert value to WString and return as WString
		* format: '11/01/2016T20:17:22Z' (T denotes dtart of Time part, Z denotes UTC zone).
		*
		*/
		::std::wstring RealTime::GetString()
		{
			::std::wstring timestr;
			const ::std::wstring dateTimeFormat( L"%d/%m/%y %H:%M:%S" );
			::std::wstringstream ss; 
			ss.imbue(std::locale("English_United Kingdom.1252"));
			ss << ::std::put_time(&Get(), dateTimeFormat.c_str()); 
			timestr = ss.str();
			return timestr;
		}
		/**
		*
		*
		*/
		bool RealTime::operator==(BaseValue const &rhs)
		{
			OutputDebugString(L"RealTime::operator== issue with mktime - change to std::time/chrono?");
			time_t lhsTime = mktime(&Get());
			ODBTime rhstm = (reinterpret_cast<RealValue<ODBTime> const &>(rhs).operator NM::ODB::ODBTime() ); 
			time_t rhsTime = mktime(&rhstm);
			double diff = difftime(lhsTime, rhsTime);
			return (diff == 0 ? true : false);
		}
		/**
		*
		*
		*/
		bool RealTime::operator<(BaseValue const &rhs)
		{
			time_t lhsTime = mktime(&Get());
			ODBTime rhstm = (reinterpret_cast<RealValue<ODBTime> const &>(rhs).operator NM::ODB::ODBTime() ); 
			time_t rhsTime = mktime(&rhstm);
			double diff = difftime(lhsTime, rhsTime);
			return (diff < 0 ? true : false);
		}
		/**
		*
		*
		*/
		bool RealTime::operator>(BaseValue const &rhs)
		{
			time_t lhsTime = mktime(&Get());
			ODBTime rhstm = (reinterpret_cast<RealValue<ODBTime> const &>(rhs).operator NM::ODB::ODBTime() ); 
			time_t rhsTime = mktime(&rhstm);
			double diff = difftime(lhsTime, rhsTime);
			return (diff > 0 ? true : false);
		}

		bool RealTime::SetXML(tinyxml2::XMLNode *thisAttributeNode)
		{
			// thisAttributeNode is the <attribute name='...'> so children are the values
			for( tinyxml2::XMLNode *valueNode=thisAttributeNode->FirstChild(); valueNode; valueNode=valueNode->NextSibling() )
			{
				::std::string strValue = valueNode->Value();
				if( ( strValue == "value") && ( !valueNode->NoChildren() ) )
				{
					// only one child of a value which is the value, get the value itself
					tinyxml2::XMLNode *value = valueNode->FirstChild();
					// get value string
					::std::string val = value->Value();
					// convert 
					::std::wstring wstrValue;
					MBtWC(wstrValue, val);
					// and set
					if( !Set(wstrValue) )
					{
						return false;
					}
				}				
			}			
			return true;
		}
		/**
		*
		*
		*/
		bool RealTime::GetXML(tinyxml2::XMLNode *thisAttributeNode)
		{
			// insert value into passed attribute node 
			tinyxml2::XMLDocument* destDoc = thisAttributeNode->GetDocument();
			// create value xml
			tinyxml2::XMLElement *value = destDoc->NewElement( "value" );
			thisAttributeNode->InsertEndChild(value);
			// now have <value></value>, add child node for actual value
			::std::string strValue;
			// get value as wstring and convert to string
			WCtMB(this->GetString(), strValue);
			// set the value in this attribute node so... <value> strValue </value>
			value->SetText(strValue.c_str());
			return true;
		}

#pragma endregion RealTimeClass

#pragma region RealInAddrClass
		/**
		*
		*	RealInAddr class (tm struct)
		*
		*/
	/*	RealInAddr::RealInAddr(ODBINADDR val):
				RealValue(val)
		{
		}*/

		RealInAddr::RealInAddr() :
			RealValue({ 0 })
		{
		}
		/**
		*
		*
		*/
		RealInAddr::~RealInAddr()
		{
		}
		/**
		* copy ctor
		*
		*/
		//RealInAddr::RealInAddr(RealInAddr const &rhs):				
		//	RealValue(rhs._val)
		//{
		//}

		RealInAddr::RealInAddr(ODBINADDR const &rhs) :
			RealValue(rhs)
		{
		}
		/**
		* assignment operator
		*
		*/
		RealInAddr& RealInAddr::operator=(RealInAddr const &rhs)			
		{
			RealValue::Set(Value(rhs));
			//_val = rhs._val;
			return *this;
		}

		RealInAddr& RealInAddr::operator=(ODBINADDR const &rhs)
		{
			RealValue::Set(rhs);
			//_val = rhs;
			return *this;
		}
		/**
		*
		*
		*/
		RealValue<ODBINADDR>*	RealInAddr::Clone() const
		{ 
			return new RealInAddr(*this);					// call copy ctor
		}
		/**
		*
		* Set Value from wstring
		*/
		bool RealInAddr::Set(::std::wstring const &wStrValue)
		{
			in_addr sa;
			::std::memset(&sa, 0, sizeof(sa));
			if( InetPton(AF_INET, wStrValue.c_str(), &(sa.S_un.S_addr)) == 1)
			{
				RealValue::Set(sa);
				//_val = sa;
				return true;
			}
			return false;
		}
		/**
		*
		* Convert value to WString and return as WString
		*/
		::std::wstring RealInAddr::GetString()
		{
			::std::wstring straddr = L"0.0.0.0";
			wchar_t str[INET_ADDRSTRLEN];

			unsigned long val = Get().S_un.S_addr;
			if( InetNtop(AF_INET, &val, str, INET_ADDRSTRLEN) != NULL)
			{
				straddr = str;
			}

			return straddr;
		}
		/**
		*
		*
		*/
		bool RealInAddr::operator==(BaseValue const &rhs)
		{
			ODBINADDR thisAddr = Get();
			ODBINADDR addr = (reinterpret_cast<RealValue<ODBINADDR> const &>(rhs).operator NM::ODB::ODBINADDR() ); 

			if(thisAddr.S_un.S_un_b.s_b1 != addr.S_un.S_un_b.s_b1 )
			{
				return false;
			}

			if(thisAddr.S_un.S_un_b.s_b2 != addr.S_un.S_un_b.s_b2 )
			{
				return false;
			}

			if(thisAddr.S_un.S_un_b.s_b3 != addr.S_un.S_un_b.s_b3 )
			{
				return false;
			}

			if(thisAddr.S_un.S_un_b.s_b4 != addr.S_un.S_un_b.s_b4 )
			{
				return false;
			}

			return true;

		}
		/**
		*
		*
		*/
		bool RealInAddr::operator<(BaseValue const &rhs)
		{
			ODBINADDR thisAddr = Get();
			ODBINADDR addr = (reinterpret_cast<RealValue<ODBINADDR> const &>(rhs).operator NM::ODB::ODBINADDR() ); 

			if(thisAddr.S_un.S_un_b.s_b1 != addr.S_un.S_un_b.s_b1 )
			{
				return thisAddr.S_un.S_un_b.s_b1 < addr.S_un.S_un_b.s_b1;
			}

			if(thisAddr.S_un.S_un_b.s_b2 != addr.S_un.S_un_b.s_b2 )
			{
				return thisAddr.S_un.S_un_b.s_b2 < addr.S_un.S_un_b.s_b2;
			}

			if(thisAddr.S_un.S_un_b.s_b3 != addr.S_un.S_un_b.s_b3 )
			{
				return  thisAddr.S_un.S_un_b.s_b3 < addr.S_un.S_un_b.s_b3;
			}

			if(thisAddr.S_un.S_un_b.s_b4 != addr.S_un.S_un_b.s_b4 )
			{
				return thisAddr.S_un.S_un_b.s_b4 < addr.S_un.S_un_b.s_b4;
			}

			return false;
		}
		/**
		*
		*
		*/
		bool RealInAddr::operator>(BaseValue const &rhs)
		{
			ODBINADDR thisAddr = Get();
			ODBINADDR addr = (reinterpret_cast<RealValue<ODBINADDR> const &>(rhs).operator NM::ODB::ODBINADDR() ); 

			if(thisAddr.S_un.S_un_b.s_b1 != addr.S_un.S_un_b.s_b1 )
			{
				return thisAddr.S_un.S_un_b.s_b1 > addr.S_un.S_un_b.s_b1 ;
			}

			if(thisAddr.S_un.S_un_b.s_b2 != addr.S_un.S_un_b.s_b2 )
			{
				return thisAddr.S_un.S_un_b.s_b2 > addr.S_un.S_un_b.s_b2;
			}

			if(thisAddr.S_un.S_un_b.s_b3 != addr.S_un.S_un_b.s_b3 )
			{
				return thisAddr.S_un.S_un_b.s_b3 > addr.S_un.S_un_b.s_b3;
			}

			if(thisAddr.S_un.S_un_b.s_b4 != addr.S_un.S_un_b.s_b4 )
			{
				return thisAddr.S_un.S_un_b.s_b4 > addr.S_un.S_un_b.s_b4 ;
			}

			return false;
		
		}

		bool RealInAddr::SetXML(tinyxml2::XMLNode *thisAttributeNode)
		{
			// thisAttributeNode is the <attribute name='...'> so children are the values
			for( tinyxml2::XMLNode *valueNode=thisAttributeNode->FirstChild(); valueNode; valueNode=valueNode->NextSibling() )
			{
				::std::string strValue = valueNode->Value();
				if( ( strValue == "value") && ( !valueNode->NoChildren() ) )
				{
					// only one child of a value which is the value, get the value itself
					tinyxml2::XMLNode *value = valueNode->FirstChild();
					// get value string
					::std::string val = value->Value();
					// convert 
					::std::wstring wstrValue;
					MBtWC(wstrValue, val);
					// and set
					if( !Set(wstrValue) )
					{
						return false;
					}
				}				
			}			
			return true;
		}
		/**
		*
		*
		*/
		bool RealInAddr::GetXML(tinyxml2::XMLNode *thisAttributeNode)
		{
			// insert value into passed attribute node 
			tinyxml2::XMLDocument* destDoc = thisAttributeNode->GetDocument();
			// create value xml
			tinyxml2::XMLElement *value = destDoc->NewElement( "value" );
			thisAttributeNode->InsertEndChild(value);
			// now have <value></value>, add child node for actual value
			::std::string strValue;
			// get value as wstring and convert to string
			WCtMB(this->GetString(), strValue);
			// set the value in this attribute node so... <value> strValue </value>
			value->SetText(strValue.c_str());
			return true;
		}


#pragma endregion RealInAddrClass

#pragma region RealByteClass

		/**
		*
		*	RealByte class 
		*
		*/
		RealByte::RealByte(ODBByte val):
				RealValue(val)
		{
		}

		RealByte::RealByte() :
			RealValue(0)
		{
		}
		/**
		*
		*
		*/
		RealByte::~RealByte()
		{
		}
		/**
		* copy ctor
		*
		*/
		//RealByte::RealByte(RealByte const &rhs):				
		//	RealValue(rhs._val)
		//{
		//}
		/**
		* assignment operator
		*
		*/
		RealByte& RealByte::operator=(RealByte const &rhs)		
		{
			RealValue::Set(Value(rhs));
			//_val = rhs._val;
			return *this;
		}
		RealValue<ODBByte>*	RealByte::Clone() const
		{ 
			return new RealByte(*this);			// call copy ctor
		}
		/**
		*
		* Set Value from wstring
		*/
		bool RealByte::Set(::std::wstring const &wStrValue)
		{
			if(wStrValue.size() > 0)
			{
				::std::string mb;
				WCtMB(wStrValue, mb);
				//_val = atoi(mb.c_str());
				RealValue::Set(atoi(mb.c_str()));
				//return true;
			}
			else
			{
				RealValue::Set(0);
				//_val = 0;
			}
			return true;
		}
		/**
		*
		* Convert value to WString and return as WString
		*/
		::std::wstring RealByte::GetString()
		{
			char str[4];
			::std::memset(&str, 0, sizeof(str));
			sprintf_s(str, 4, "%d", Get());
			::std::string strVal(str, 3);

			::std::wstring retVal;

			MBtWC(retVal, strVal);

			return retVal;
		}
		/**
		*
		*
		*/
		bool RealByte::operator==(BaseValue const &rhs)
		{
			return Get() == (reinterpret_cast<RealValue<ODBByte> const &>(rhs).operator NM::ODB::ODBByte() );
		}
		/**
		*
		*
		*/
		bool RealByte::operator<(BaseValue const &rhs)
		{
			return Get() < (reinterpret_cast<RealValue<ODBByte> const &>(rhs).operator NM::ODB::ODBByte() );
		}
		/**
		*
		*
		*/
		bool RealByte::operator>(BaseValue const &rhs)
		{
			return Get() > (reinterpret_cast<RealValue<ODBByte> const &>(rhs).operator NM::ODB::ODBByte() );
		}


		bool RealByte::SetXML(tinyxml2::XMLNode *thisAttributeNode)
		{
			// thisAttributeNode is the <attribute name='...'> so children are the values
			for( tinyxml2::XMLNode *valueNode=thisAttributeNode->FirstChild(); valueNode; valueNode=valueNode->NextSibling() )
			{
				::std::string strValue = valueNode->Value();
				if( ( strValue == "value") && ( !valueNode->NoChildren() ) )
				{
					// only one child of a value which is the value, get the value itself
					tinyxml2::XMLNode *value = valueNode->FirstChild();
					// get value string
					::std::string val = value->Value();
					// convert 
					::std::wstring wstrValue;
					MBtWC(wstrValue, val);
					// and set
					if( !Set(wstrValue) )
					{
						return false;
					}
				}				
			}			
			return true;
		}
		/**
		*
		*
		*/
		bool RealByte::GetXML(tinyxml2::XMLNode *thisAttributeNode)
		{
			// insert value into passed attribute node 
			tinyxml2::XMLDocument* destDoc = thisAttributeNode->GetDocument();
			// create value xml
			tinyxml2::XMLElement *value = destDoc->NewElement( "value" );
			thisAttributeNode->InsertEndChild(value);
			// now have <value></value>, add child node for actual value
			::std::string strValue;
			// get value as wstring and convert to string
			WCtMB(this->GetString(), strValue);
			// set the value in this attribute node so... <value> strValue </value>
			value->SetText(strValue.c_str());

			return true;
		}

#pragma endregion RealByteClass

#pragma region RealColorClass

		/**
		*
		*	RealColor class 
		*
		*/
		RealColor::RealColor(ODBColor val) :
			RealValue(val)
		{
		}
		RealColor::RealColor() :
			RealValue(0x00000000)
		{
		}
		/**
		*
		*
		*/
		RealColor::~RealColor()
		{
		}
		/**
		* copy ctor
		*
		*/
		//RealColor::RealColor(RealColor const &rhs) :
		//	RealValue(rhs._val)
		//{
		//}
		/*RealColor::RealColor(ODBColor const &rhs) :
			RealValue(rhs)
		{
		}*/
		/**
		* assignment operator
		*
		*/
		RealColor& RealColor::operator=(RealColor const &rhs)
		{
			RealValue::Set(Value(rhs));
			//_val = rhs._val;
			return *this;
		}

		RealColor& RealColor::operator=(ODBColor const &rhs)
		{
			RealValue::Set(rhs);
			//_val = rhs;
			return *this;
		}

		RealValue<ODBColor>* RealColor::Clone() const
		{
			return new RealColor(*this);			// call copy ctor
		}
		/**
		*
		* Set Value from wstring
		*/
		bool RealColor::Set(::std::wstring const &wStrValue)
		{
			if (wStrValue.size() > 0)
			{
				::std::string mb;
				WCtMB(wStrValue, mb);
				char* p = nullptr;
				RealValue::Set(std::strtoul(mb.c_str(), &p, 16));
				//_val = std::strtoul(mb.c_str(), &p, 16);
			}
			else
			{
				RealValue::Set(0);
				//_val = 0;
			}
			return true;
		}
		/**
		*
		* Convert value to WString and return as WString
		*/
		::std::wstring RealColor::GetString()
		{
			const int buf_size = 11;
			char str[buf_size];
			::std::memset(&str, 0, sizeof(str));
			sprintf_s(str, buf_size, "0x%08x", Get());		// 0x 00 BB GG RR
			::std::string strVal(str, buf_size);

			::std::wstring retVal;

			MBtWC(retVal, strVal);

			return retVal;
		}
		ODBColor RealColor::GetRed()
		{
			return GetRValue(Get());
		}
		ODBColor RealColor::GetGreen()
		{
			return GetGValue(Get());
		}
		ODBColor RealColor::GetBlue()
		{
			return GetBValue(Get());
		}
		/**
		*
		*
		*/
		bool RealColor::operator==(BaseValue const &rhs)
		{
			return Get() == (reinterpret_cast<RealValue<ODBColor> const &>(rhs).operator NM::ODB::ODBColor());
		}
		/**
		*
		*
		*/
		bool RealColor::operator<(BaseValue const &rhs)
		{
			return Get() < (reinterpret_cast<RealValue<ODBColor> const &>(rhs).operator NM::ODB::ODBColor());
		}
		/**
		*
		*
		*/
		bool RealColor::operator>(BaseValue const &rhs)
		{
			return Get() > (reinterpret_cast<RealValue<ODBColor> const &>(rhs).operator NM::ODB::ODBColor());
		}


		bool RealColor::SetXML(tinyxml2::XMLNode *thisAttributeNode)
		{
			// thisAttributeNode is the <attribute name='...'> so children are the values
			for (tinyxml2::XMLNode *valueNode = thisAttributeNode->FirstChild(); valueNode; valueNode = valueNode->NextSibling())
			{
				::std::string strValue = valueNode->Value();
				if ((strValue == "value") && (!valueNode->NoChildren()))
				{
					// only one child of a value which is the value, get the value itself
					tinyxml2::XMLNode *value = valueNode->FirstChild();
					// get value string
					::std::string val = value->Value();
					// convert 
					::std::wstring wstrValue;
					MBtWC(wstrValue, val);
					// and set
					if (!Set(wstrValue))
					{
						return false;
					}
				}
			}
			return true;
		}
		/**
		*
		*
		*/
		bool RealColor::GetXML(tinyxml2::XMLNode *thisAttributeNode)
		{
			// insert value into passed attribute node 
			tinyxml2::XMLDocument* destDoc = thisAttributeNode->GetDocument();
			// create value xml
			tinyxml2::XMLElement *value = destDoc->NewElement("value");
			thisAttributeNode->InsertEndChild(value);
			// now have <value></value>, add child node for actual value
			::std::string strValue;
			// get value as wstring and convert to string
			WCtMB(this->GetString(), strValue);
			// set the value in this attribute node so... <value> strValue </value>
			value->SetText(strValue.c_str());

			return true;
		}

#pragma endregion RealColorClass

//ns
	}
}