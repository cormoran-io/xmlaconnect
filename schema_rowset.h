#pragma once

#include "soapH.h"

class schema_row
{
private:
	static const size_t MAX_BUF_SIZE = 10 * 1024 * sizeof( wchar_t );
private:
	void copy( const schema_row& other )
	{
		//m_name = _wcsdup(other.m_name);
		//m_descrip = _wcsdup( other.m_descrip);
		//				m_restriction;
		//m_restric_mask = other.m_restric_mask;
		m_name = _wcsdup( other.m_name ); 
		m_descrip = _wcsdup( other.m_descrip); 
		
	}
public:
	wchar_t*				m_name;
	wchar_t*				m_descrip;
	GUID					m_guid;
//	SAFEARRAY*				m_restriction;
	int						m_restric_mask;


	schema_row( row& a_row )
	{
		//m_name = _wcsdup( FROM_STRING( a_row.PROPERTY_USCORENAME, CP_UTF8 ) );
		m_name =  _wcsdup( FROM_STRING( a_row.DataSourceName, CP_UTF8 ) ); 
		m_descrip = _wcsdup( FROM_STRING( a_row.DataSourceName, CP_UTF8 ) ); 
	}

	schema_row( const schema_row& other )
	{
		copy( other );
	}

	schema_row& operator=( const schema_row& other )
	{
		if ( this != &other )  {  copy( other ); }
		return *this;
	}

	~schema_row()
	{
		if ( nullptr != m_name ) { delete[] m_name;}
		if ( nullptr != m_descrip ) { delete[] m_descrip;}
	}


	EMPTY_CONSTRUCTOR(schema_row);

	static char* schema_name() { return "DISCOVER_SCHEMA_ROWSETS"; }

	BEGIN_PROVIDER_COLUMN_MAP( schema_row )
	PROVIDER_COLUMN_ENTRY_VAR_WSTR( "SchemaName", 1, MAX_BUF_SIZE, m_name )
	PROVIDER_COLUMN_ENTRY_VAR_WSTR( "SchemaGuid", 2, VT_CLSID, m_guid )
//	PROVIDER_COLUMN_ENTRY_VAR_WSTR( "Restrictions", 3, VT_ARRAY, m_restriction )
	PROVIDER_COLUMN_ENTRY_VAR_WSTR( "Description", 3, MAX_BUF_SIZE, m_descrip )
	PROVIDER_COLUMN_ENTRY_VAR_WSTR( "RestrictionsMask", 4, VT_UI8, m_restric_mask )
	END_PROVIDER_COLUMN_MAP()
};