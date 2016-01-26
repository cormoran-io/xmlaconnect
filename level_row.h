/*
	ODBO provider for XMLA data stores
    Copyright (C) 2014-2015  ARquery LTD
	http://www.arquery.com

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

	@description
					a row in the level rowset
*/

#pragma once

#include "soapH.h"

class level_row
{
private:
	static const size_t MAX_BUF_SIZE = 10 * 1024 * sizeof( wchar_t );
private:
	void copy( const level_row& other )
	{
		m_schema = nullptr;

		m_level_number = other.m_level_number;
		m_level_cardinality = other.m_level_cardinality;
		m_level_type = other.m_level_type;

		m_catalog = _wcsdup( other.m_catalog );
		m_cube = _wcsdup( other.m_cube );
		m_dimension_unique_name = _wcsdup( other.m_dimension_unique_name );
		m_hierarcy_unique_name = _wcsdup( other.m_hierarcy_unique_name );
		m_level_name = _wcsdup( other.m_level_name );
		m_level_unique_name = _wcsdup( other.m_level_unique_name );
		m_level_caption = _wcsdup( other.m_level_caption );
		m_description = _wcsdup( other.m_description );

		m_lvl_rollup_sett = other.m_lvl_rollup_sett;
		m_lvl_unique_sett = other.m_lvl_unique_sett;
		m_lvl_is_visible = other.m_lvl_is_visible;
		m_lvl_ordering_prop = _wcsdup(other.m_lvl_ordering_prop);
		m_lvl_dbtype = other.m_lvl_dbtype;

		m_lvl_m_unique_key = other.m_lvl_m_unique_key;
		m_lvl_name_sql_col = other.m_lvl_name_sql_col;
		m_lvl_key_sql = other.m_lvl_key_sql;
		m_lvl_u_name_sql_col = other.m_lvl_u_name_sql_col;
		m_lvl_attrib_hier_name = other.m_lvl_attrib_hier_name;
		m_lvl_key_cardinality = other.m_lvl_key_cardinality;
		m_lvl_origin = other.m_lvl_origin;
	}

public:
	wchar_t*			m_catalog;
	wchar_t*			m_schema;
	wchar_t*			m_cube;
	wchar_t*			m_dimension_unique_name;
	wchar_t*			m_hierarcy_unique_name;
	wchar_t*			m_level_name;
	wchar_t*			m_level_unique_name;
	GUID				m_level_guid;
	wchar_t*			m_level_caption;
	unsigned long		m_level_number;
	unsigned long		m_level_cardinality;
	long				m_level_type;
	wchar_t*			m_description;
	long				m_lvl_rollup_sett;
	long				m_lvl_unique_sett;
	bool				m_lvl_is_visible;
	wchar_t*			m_lvl_ordering_prop;
	long				m_lvl_dbtype;
	wchar_t*			m_lvl_m_unique_key;
	wchar_t*			m_lvl_name_sql_col;
	wchar_t*			m_lvl_key_sql;
	wchar_t*			m_lvl_u_name_sql_col;
	wchar_t*			m_lvl_attrib_hier_name;
	unsigned short		m_lvl_key_cardinality;
	unsigned short		m_lvl_origin;

	void dummy(row& a_row)
	{
		m_hierarcy_unique_name = _wcsdup( FROM_STRING( "[Date].[the_date]", CP_UTF8 ) );
		if ( 0 == std::strcmp( a_row.LEVEL_USCORENAME, "All" ) ) {
			m_level_name = _wcsdup( FROM_STRING( "All", CP_UTF8 ) );
			m_level_unique_name = _wcsdup( FROM_STRING( "[Date].[the_date].[All]", CP_UTF8 ) );
			m_level_caption = _wcsdup( FROM_STRING( "All", CP_UTF8 ) );			
			m_level_number = 0;
			m_level_cardinality = 1;
			m_level_type = 1;
			m_lvl_dbtype = 3;
			m_lvl_ordering_prop = _wcsdup( FROM_STRING( "All", CP_UTF8 ) );
			m_lvl_m_unique_key = nullptr;
			m_lvl_name_sql_col = nullptr;
			m_lvl_key_sql = nullptr;
			m_lvl_u_name_sql_col = nullptr;
			m_lvl_attrib_hier_name = nullptr;
		} else {
			m_level_name = _wcsdup( FROM_STRING( "the_date", CP_UTF8 ) );
			m_level_unique_name = _wcsdup( FROM_STRING( "[Date].[the_date].[the_date]", CP_UTF8 ) );
			m_level_caption = _wcsdup( FROM_STRING( "the_date", CP_UTF8 ) );			
			m_level_number = 1;
			m_level_cardinality = 1000;
			m_level_type = 0;
			m_lvl_dbtype = 3;
			m_lvl_ordering_prop = _wcsdup( FROM_STRING( "the_date", CP_UTF8 ) );
			m_lvl_m_unique_key = nullptr;
			m_lvl_name_sql_col = nullptr;
			m_lvl_key_sql = nullptr;
			m_lvl_u_name_sql_col = nullptr;
			m_lvl_attrib_hier_name = _wcsdup( FROM_STRING( "the_date", CP_UTF8 ) );		}

		m_lvl_rollup_sett = 0;
		m_lvl_unique_sett = 0;
		m_lvl_is_visible = false;
		m_description = nullptr;
		
		m_lvl_key_cardinality = 1;
		m_lvl_origin = 6;

	}

	level_row( row& a_row )
	{
		m_schema = nullptr;

		m_catalog = _wcsdup( FROM_STRING( a_row.CATALOG_USCORENAME, CP_UTF8 ) );
		m_cube = _wcsdup( FROM_STRING( a_row.CUBE_USCORENAME, CP_UTF8 ) );		
		m_dimension_unique_name = _wcsdup( FROM_STRING( a_row.DIMENSION_USCOREUNIQUE_USCORENAME, CP_UTF8 ) );
		if ( 0 == std::strcmp( a_row.HIERARCHY_USCOREUNIQUE_USCORENAME, "[Date].[the_date]" ) ) {
			dummy(a_row);
			return;
		}
		m_hierarcy_unique_name = _wcsdup( FROM_STRING( a_row.HIERARCHY_USCOREUNIQUE_USCORENAME, CP_UTF8 ) );
		m_level_name = _wcsdup( FROM_STRING( a_row.LEVEL_USCORENAME, CP_UTF8 ) );
		m_level_unique_name = _wcsdup( FROM_STRING( a_row.LEVEL_USCOREUNIQUE_USCORENAME, CP_UTF8 ) );
		m_level_caption = _wcsdup( FROM_STRING( a_row.LEVEL_USCORECAPTION, CP_UTF8 ) );
		m_description = _wcsdup( FROM_STRING( a_row.DESCRIPTION, CP_UTF8 ) );

		m_level_number = get_int( a_row.LEVEL_USCORENUMBER );
		m_level_cardinality = get_int( a_row.LEVEL_USCORECARDINALITY );
		m_level_type = get_int( a_row.LEVEL_USCORETYPE );


		m_lvl_rollup_sett = 0;
		m_lvl_unique_sett = 0;
		m_lvl_is_visible = true;
		m_lvl_ordering_prop = _wcsdup( FROM_STRING( a_row.HIERARCHY_USCOREUNIQUE_USCORENAME, CP_UTF8 ) );
		m_lvl_dbtype = 130;
		m_lvl_m_unique_key = nullptr;
		m_lvl_name_sql_col = nullptr;
		m_lvl_key_sql = nullptr;
		m_lvl_u_name_sql_col = nullptr;
		m_lvl_attrib_hier_name= nullptr;
	}

	level_row( const level_row& other )
	{
		copy( other );
	}

	level_row& operator=( const level_row& other )
	{
		if ( this != &other )  {  copy( other ); }
		return *this;
	}

	~level_row()
	{
		delete[] m_catalog;
		delete[] m_cube;
		delete[] m_dimension_unique_name;
		delete[] m_hierarcy_unique_name;
		delete[] m_level_name;
		delete[] m_level_unique_name;
		delete[] m_level_caption;
		delete[] m_description;
		delete[] m_lvl_ordering_prop; 
	}

	EMPTY_CONSTRUCTOR(level_row);
	static char* schema_name() { return "MDSCHEMA_LEVELS"; }
	

	BEGIN_PROVIDER_COLUMN_MAP( level_row )
	PROVIDER_COLUMN_ENTRY_VAR_WSTR( "CATALOG_NAME", 1, MAX_BUF_SIZE, m_catalog )
	PROVIDER_COLUMN_ENTRY_VAR_WSTR( "SCHEMA_NAME", 2, MAX_BUF_SIZE, m_schema )
	PROVIDER_COLUMN_ENTRY_VAR_WSTR( "CUBE_NAME", 3, MAX_BUF_SIZE, m_cube )
	PROVIDER_COLUMN_ENTRY_VAR_WSTR( "DIMENSION_UNIQUE_NAME", 4, MAX_BUF_SIZE, m_dimension_unique_name )
	PROVIDER_COLUMN_ENTRY_VAR_WSTR( "HIERARCHY_UNIQUE_NAME", 5, MAX_BUF_SIZE, m_hierarcy_unique_name )
	PROVIDER_COLUMN_ENTRY_VAR_WSTR( "LEVEL_NAME", 6, MAX_BUF_SIZE, m_level_name )
	PROVIDER_COLUMN_ENTRY_VAR_WSTR( "LEVEL_UNIQUE_NAME", 7, MAX_BUF_SIZE, m_level_unique_name )
	PROVIDER_COLUMN_ENTRY_TYPE( "LEVEL_GUID", 8, VT_CLSID, m_level_guid )
	PROVIDER_COLUMN_ENTRY_VAR_WSTR( "LEVEL_CAPTION", 9, MAX_BUF_SIZE, m_level_caption )
	PROVIDER_COLUMN_ENTRY_TYPE( "LEVEL_NUMBER", 10, VT_UI4, m_level_number )
	PROVIDER_COLUMN_ENTRY_TYPE( "LEVEL_CARDINALITY", 11, VT_UI4, m_level_cardinality )
	PROVIDER_COLUMN_ENTRY_TYPE( "LEVEL_TYPE", 12, VT_I4, m_level_type )
	PROVIDER_COLUMN_ENTRY_VAR_WSTR( "DESCRIPTION", 13, MAX_BUF_SIZE, m_description )

	PROVIDER_COLUMN_ENTRY_TYPE( "CUSTOM_ROLLUP_SETTINGS", 14, VT_I4, m_lvl_rollup_sett )
	PROVIDER_COLUMN_ENTRY_TYPE( " LEVEL_UNIQUE_SETTINGS ", 15, VT_I4, m_lvl_unique_sett )
	PROVIDER_COLUMN_ENTRY_TYPE( "LEVEL_IS_VISIBLE", 16, VT_BOOL, m_lvl_is_visible )
	PROVIDER_COLUMN_ENTRY_VAR_WSTR( "LEVEL_ORDERING_PROPERTY", 17, MAX_BUF_SIZE, m_lvl_ordering_prop )
	PROVIDER_COLUMN_ENTRY_TYPE( "LEVEL_DBTYPE", 18, VT_I4, m_lvl_dbtype )

	PROVIDER_COLUMN_ENTRY_TYPE( "LEVEL_MASTER_UNIQUE_NAME", 19, MAX_BUF_SIZE, m_lvl_m_unique_key )
	PROVIDER_COLUMN_ENTRY_TYPE( "LEVEL_NAME_SQL_COLUMN_NAME", 20, MAX_BUF_SIZE, m_lvl_name_sql_col )
	PROVIDER_COLUMN_ENTRY_TYPE( "LEVEL_KEY_SQL_COLUMN_NAME", 21, MAX_BUF_SIZE, m_lvl_key_sql )
	PROVIDER_COLUMN_ENTRY_TYPE( "LEVEL_UNIQUE_NAME_SQL_COLUMN_NAME", 22, MAX_BUF_SIZE, m_lvl_u_name_sql_col )
	PROVIDER_COLUMN_ENTRY_TYPE( "LEVEL_ATTRIBUTE_HIERARCHY_NAME", 23, MAX_BUF_SIZE, m_lvl_attrib_hier_name )
	PROVIDER_COLUMN_ENTRY_TYPE( "LEVEL_KEY_CARDINALITY", 24, VT_UI2, m_lvl_key_cardinality )
	PROVIDER_COLUMN_ENTRY_TYPE( "LEVEL_ORIGIN", 25, VT_UI2, m_lvl_origin )

	END_PROVIDER_COLUMN_MAP()
};