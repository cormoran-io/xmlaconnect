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
					a row in the hierarchy rowset
*/

#pragma once

#include "soapH.h"

class hierarchy_row
{
private:
	static const size_t MAX_BUF_SIZE = 10 * 1024 * sizeof( wchar_t );
private:
	void copy( const hierarchy_row& other )
	{
		m_schema = nullptr;

		m_catalog = _wcsdup( other.m_catalog );
		m_cube = _wcsdup( other.m_cube );
		m_dimension_unique_name = _wcsdup( other.m_dimension_unique_name );
		m_hierarchy_name = _wcsdup( other.m_hierarchy_name );
		m_hierarchy_unique_name = _wcsdup( other.m_hierarchy_unique_name );
		m_hierarchy_caption = _wcsdup( other.m_hierarchy_caption );		
		m_default_member = _wcsdup( other.m_default_member );
		m_all_member = _wcsdup( other.m_all_member );
		m_description = _wcsdup( other.m_description);
		m_dimension_type = other.m_dimension_type;
		m_hierarcy_cardinality = other.m_hierarcy_cardinality;
		m_structure = other.m_structure;
		m_hierarchy_origin = other.m_hierarchy_origin;

		m_hierarchy_origin = other.m_hierarchy_origin;
		m_is_virtual = other.m_is_virtual;
		m_is_readwrite = other.m_is_readwrite;
		m_dim_is_shared = other.m_dim_is_shared; 
		m_dim_is_visible = other.m_dim_is_visible;
		m_dim_unique_settings = other.m_dim_unique_settings;
		m_master_unique_settings = nullptr;
		m_hierarchy_display_folder = nullptr;
		m_instance_selection = other.m_instance_selection;
		m_grouping_behavior = other.m_grouping_behavior;
		
	}
public:
	wchar_t*         m_catalog;
	wchar_t*         m_schema;
	wchar_t*         m_cube;
	wchar_t*         m_dimension_unique_name;
	wchar_t*         m_hierarchy_name;
	wchar_t*         m_hierarchy_unique_name;
	GUID             m_hierarchy_guid;
	wchar_t*         m_hierarchy_caption;
	short            m_dimension_type;
	unsigned long    m_hierarcy_cardinality;
	wchar_t*         m_default_member;
	wchar_t*         m_all_member;
	wchar_t*         m_description;
	short            m_structure;
	bool			 m_is_visible;
	int				 m_hierarchy_origin;
	bool			 m_is_virtual;
	bool			 m_is_readwrite;
	bool			 m_dim_is_shared;
	bool			 m_dim_is_visible;
	int				 m_dim_unique_settings;
	unsigned long	 m_ordinal;
	wchar_t*		 m_master_unique_settings;
	wchar_t*		 m_hierarchy_display_folder;
	unsigned short	 m_instance_selection;
	short		     m_grouping_behavior;
	
	
	void dummy()
	{
		m_dimension_unique_name = _wcsdup( FROM_STRING( "[Date]", CP_UTF8 ) );
		m_hierarchy_name = _wcsdup( FROM_STRING( "the_date", CP_UTF8 ) );
		m_hierarchy_unique_name = _wcsdup( FROM_STRING(  "[Date].[the_date]", CP_UTF8 ) );
		m_hierarchy_caption = _wcsdup(FROM_STRING( "the_date", CP_UTF8 ));
		m_description = nullptr;
		
		m_all_member = _wcsdup( FROM_STRING( "[Date].[the_date].[All]", CP_UTF8 ) );
		m_default_member = _wcsdup( FROM_STRING( "[Date].[the_date].[All]", CP_UTF8 ) );
		

		m_dimension_type = 1;
		m_hierarcy_cardinality = 1001;
		m_structure = 0;//2;//MD_STRUCTURE_UNBALANCED;
		m_hierarchy_origin = 6;
		m_is_visible = false;
	
		m_dim_is_shared = true;
		m_dim_is_visible = true;
		m_dim_unique_settings = 1;
		m_is_readwrite = false;
		m_is_virtual = false;
		m_master_unique_settings = nullptr;
		m_hierarchy_display_folder = nullptr;
		m_instance_selection = 3;
		m_grouping_behavior = 2;
	}

	hierarchy_row( row& a_row )
	{
		m_schema = nullptr;

		m_catalog = _wcsdup( FROM_STRING( a_row.CATALOG_USCORENAME, CP_UTF8 ) );
		m_cube = _wcsdup( FROM_STRING( a_row.CUBE_USCORENAME, CP_UTF8 ) );
		if ( 0 == strcmp( a_row.HIERARCHY_USCORENAME, "the_date" ) ){
			dummy();
			return;
		}
		m_dimension_unique_name = _wcsdup( FROM_STRING( a_row.DIMENSION_USCOREUNIQUE_USCORENAME, CP_UTF8 ) );
		m_hierarchy_name = _wcsdup( FROM_STRING( a_row.HIERARCHY_USCORENAME, CP_UTF8 ) );
		m_hierarchy_unique_name = _wcsdup( FROM_STRING( a_row.HIERARCHY_USCOREUNIQUE_USCORENAME, CP_UTF8 ) );
		m_hierarchy_caption = _wcsdup(FROM_STRING( a_row.HIERARCHY_USCORECAPTION, CP_UTF8 ));
		m_description = _wcsdup( FROM_STRING( a_row.DESCRIPTION, CP_UTF8 ) );
		
		if ( nullptr != a_row.ALL_USCOREMEMBER ) {
			m_all_member = _wcsdup( FROM_STRING( a_row.ALL_USCOREMEMBER, CP_UTF8 ) );
		} else {
			m_all_member = nullptr;
		}

		if ( nullptr != a_row.DEFAULT_USCOREMEMBER ) {
			m_default_member = _wcsdup( FROM_STRING( a_row.DEFAULT_USCOREMEMBER, CP_UTF8 ) );
		} else {
			m_default_member = nullptr;
		}
		m_dimension_type = get_int( a_row.DIMENSION_USCORETYPE );
		m_hierarcy_cardinality = get_int( a_row.HIERARCHY_USCORECARDINALITY );
		m_structure = get_int( a_row.STRUCTURE );//2;//MD_STRUCTURE_UNBALANCED;
		m_hierarchy_origin = 1; //default MD_USER_DEFINED
		m_is_visible = true;
		if ( nullptr != a_row.HIERARCHY__USCOREORIGIN ) { m_hierarchy_origin = get_int( a_row.HIERARCHY__USCOREORIGIN ); }
		m_dim_is_shared = true;
		m_dim_is_visible = true;
		m_dim_unique_settings = 1;
		m_is_readwrite = false;
		m_is_virtual = false;
		m_master_unique_settings = nullptr;

		m_hierarchy_display_folder = nullptr;
		m_grouping_behavior = 1;
	}

	hierarchy_row( const hierarchy_row& other )
	{
		copy( other );
	}

	hierarchy_row& operator=( const hierarchy_row& other )
	{
		if ( this != &other )  {  copy( other ); }
		return *this;
	}

	~hierarchy_row()
	{
		delete[] m_catalog;
		delete[] m_cube;
		delete[] m_dimension_unique_name;
		delete[] m_hierarchy_name;
		delete[] m_hierarchy_unique_name;
		delete[] m_hierarchy_caption;
		if ( m_all_member ) { delete[] m_all_member; }
		if ( m_default_member) { delete[] m_default_member; }
		delete[] m_description; 
	}

	EMPTY_CONSTRUCTOR(hierarchy_row);

	static char* schema_name() { return "MDSCHEMA_HIERARCHIES"; }

	

	BEGIN_PROVIDER_COLUMN_MAP( hierarchy_row )
	PROVIDER_COLUMN_ENTRY_VAR_WSTR( "CATALOG_NAME", 1, MAX_BUF_SIZE, m_catalog )
	PROVIDER_COLUMN_ENTRY_VAR_WSTR( "SCHEMA_NAME", 2, MAX_BUF_SIZE, m_schema )
	PROVIDER_COLUMN_ENTRY_VAR_WSTR( "CUBE_NAME", 3, MAX_BUF_SIZE, m_cube )
	PROVIDER_COLUMN_ENTRY_VAR_WSTR( "DIMENSION_UNIQUE_NAME", 4, MAX_BUF_SIZE, m_dimension_unique_name )
	PROVIDER_COLUMN_ENTRY_VAR_WSTR( "HIERARCHY_NAME", 5, MAX_BUF_SIZE, m_hierarchy_name )
	PROVIDER_COLUMN_ENTRY_VAR_WSTR( "HIERARCHY_UNIQUE_NAME", 6, MAX_BUF_SIZE, m_hierarchy_unique_name )
	PROVIDER_COLUMN_ENTRY_TYPE( "HIERARCHY_GUID", 7, VT_CLSID, m_hierarchy_guid )
	PROVIDER_COLUMN_ENTRY_VAR_WSTR("HIERARCHY_CAPTION", 8, MAX_BUF_SIZE, m_hierarchy_caption)
	PROVIDER_COLUMN_ENTRY_TYPE( "DIMENSION_TYPE", 9, VT_I2, m_dimension_type )
	PROVIDER_COLUMN_ENTRY_TYPE( "HIERARCHY_CARDINALITY", 10, VT_UI4, m_hierarcy_cardinality )
	PROVIDER_COLUMN_ENTRY_VAR_WSTR( "DEFAULT_MEMBER", 11, MAX_BUF_SIZE, m_default_member )
	PROVIDER_COLUMN_ENTRY_VAR_WSTR( "ALL_MEMBER", 12, MAX_BUF_SIZE, m_all_member )
	PROVIDER_COLUMN_ENTRY_VAR_WSTR( "DESCRIPTION", 13, MAX_BUF_SIZE, m_description )
	PROVIDER_COLUMN_ENTRY_TYPE( "STRUCTURE", 14, VT_I2, m_structure )

	PROVIDER_COLUMN_ENTRY_TYPE( "IS_VIRTUAL", 15, VT_BOOL, m_is_virtual )
	PROVIDER_COLUMN_ENTRY_TYPE( "IS_READWRITE", 16, VT_BOOL, m_is_readwrite )
	PROVIDER_COLUMN_ENTRY_TYPE( "DIMENSION_UNIQUE_SETTINGS", 17, VT_I4, m_dim_unique_settings )
	PROVIDER_COLUMN_ENTRY_TYPE( "DIMENSION_MASTER_UNIQUE_NAME", 18, MAX_BUF_SIZE, m_structure )
	PROVIDER_COLUMN_ENTRY_TYPE( "DIMENSION_IS_VISIBLE", 19, VT_BOOL, m_dim_is_visible )
	PROVIDER_COLUMN_ENTRY_TYPE( "HIERARCHY_ORDINAL", 20, VT_UI4, m_ordinal )
	PROVIDER_COLUMN_ENTRY_TYPE( "DIMENSION_IS_SHARED", 21, VT_BOOL, m_dim_is_shared )

	PROVIDER_COLUMN_ENTRY_TYPE( "HIERARCHY_IS_VISIBLE", 22, VT_BOOL, m_is_visible )
	PROVIDER_COLUMN_ENTRY_TYPE( "HIERARCHY_ORIGIN", 23, VT_I4, m_hierarchy_origin )
	
	PROVIDER_COLUMN_ENTRY_TYPE( "HIERARCHY_DISPLAY_FOLDER", 24, MAX_BUF_SIZE, m_hierarchy_display_folder )
	PROVIDER_COLUMN_ENTRY_TYPE( "INSTANCE_SELECTION", 25, VT_UI2, m_instance_selection )
	PROVIDER_COLUMN_ENTRY_TYPE( "GROUPING_BEHAVIOR", 26, VT_I2, m_grouping_behavior )
	END_PROVIDER_COLUMN_MAP()
};