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
					maintains session members/sets for mondrian while 
					we are waiting for CREATE statement
*/
#include "stdafx.h"
#include "mondrian_session_table.h"
#include "rowset.h"
#include "command.h"
#include "session.h"
#include "connection_handler.h"


bool mondrian_session_table::entry::develop_to_set( connection_handler& handler )
{
	xmlns__Restrictions restrictions;
	restrictions.RestrictionList.CUBE_USCORENAME = _strdup( cube.c_str() );
			
	xmlns__Properties properties;			
	properties.PropertyList.LocaleIdentifier = CP_UTF8;
		
	cxmla__DiscoverResponse&  response = handler.raw_discover( "MDSCHEMA_CUBES", restrictions, properties );

	if ( 1 != response.cxmla__return__.root.__rows.__size ) { return false; }

	m_set_row.reset( new set_row() );

	row crt_row = response.cxmla__return__.root.__rows.row[0];

	m_set_row->m_schema = nullptr;
	m_set_row->m_description = nullptr;
	m_set_row->m_scope = 2;
	m_set_row->m_eval_context = 1;

	m_set_row->m_catalog = _wcsdup( FROM_STRING( crt_row.CATALOG_USCORENAME, CP_UTF8 ) );
	m_set_row->m_cube = _wcsdup( FROM_STRING( crt_row.CUBE_USCORENAME, CP_UTF8 ) );
	m_set_row->m_set_name = _wcsdup( FROM_STRING( simple_name.c_str(), CP_UTF8 ) );
	m_set_row->m_expression = _wcsdup( FROM_STRING( "", CP_UTF8 ));
	
	std::size_t open_pos = formula.find_first_of('[');
	if ( std::string::npos != open_pos )
	{
		std::size_t close_pos =  formula.find_first_of( ']', open_pos );
		if ( std::string::npos != close_pos )
		{
			m_set_row->m_dimension = _wcsdup( FROM_STRING( formula.substr(open_pos, close_pos - open_pos + 1 ).c_str(), CP_UTF8 ) );
		}
	}
	
	
	m_set_row->m_set_caption = _wcsdup( FROM_STRING( simple_name.c_str(), CP_UTF8 ) );
	m_set_row->m_display_folder = _wcsdup( FROM_STRING( "", CP_UTF8 ) );

	return true;
}

bool mondrian_session_table::entry::develop_to_measure( connection_handler& handler )
{
	xmlns__Restrictions restrictions;
	restrictions.RestrictionList.HIERARCHY_USCOREUNIQUE_USCORENAME = _strdup( owner.c_str() );
	restrictions.RestrictionList.CUBE_USCORENAME = _strdup( cube.c_str() );
			
	xmlns__Properties properties;			
	properties.PropertyList.LocaleIdentifier = CP_UTF8;
		
	cxmla__DiscoverResponse&  response = handler.raw_discover( "MDSCHEMA_HIERARCHIES", restrictions, properties );

	if ( 1 != response.cxmla__return__.root.__rows.__size ) { return false; }

	type = MEASURE_TYPE;

	m_member_row.reset( new member_row() );
	row crt_row = response.cxmla__return__.root.__rows.row[0];

	m_member_row->m_catalog = _wcsdup( FROM_STRING( crt_row.CATALOG_USCORENAME, CP_UTF8 ) ) ;
	m_member_row->m_schema = nullptr;
	m_member_row->m_cube = _wcsdup(FROM_STRING( crt_row.CUBE_USCORENAME, CP_UTF8 ));
	m_member_row->m_dimension_unique_name = _wcsdup( FROM_STRING( crt_row.DIMENSION_USCOREUNIQUE_USCORENAME, CP_UTF8 ) );
	m_member_row->m_hierarchy_unique_name = _wcsdup( FROM_STRING( crt_row.HIERARCHY_USCOREUNIQUE_USCORENAME, CP_UTF8 ) );
		
	m_member_row->m_level_unique_name = nullptr;//_wcsdup( FROM_STRING( crt_row.LEVEL_USCOREUNIQUE_USCORENAME, CP_UTF8 ) );
	m_member_row->m_member_name = _wcsdup( FROM_STRING( simple_name.c_str(), CP_UTF8 ) );
	m_member_row->m_member_unique_name = _wcsdup( FROM_STRING( name.c_str(), CP_UTF8 ) );
	m_member_row->m_member_caption = _wcsdup( FROM_STRING( simple_name.c_str(), CP_UTF8 ) );
	m_member_row->m_parent_unique_name = nullptr;//_wcsdup( FROM_STRING( crt_row.PARENT_USCOREUNIQUE_USCORENAME, CP_UTF8 ) );
	m_member_row->m_description = nullptr;//_wcsdup( FROM_STRING( crt_row.DESCRIPTION, CP_UTF8 ) );
		
	//m_level_number = get_int(crt_row.LEVEL_USCORENUMBER);
	m_member_row->m_member_ordinal = 0;//get_int( crt_row.MEMBER_USCOREORDINAL );
	m_member_row->m_member_type = MDMEMBER_TYPE_FORMULA;//get_int( crt_row.MEMBER_USCORETYPE );
	m_member_row->m_children_cardinality = 0;//get_int(crt_row.CHILDREN_USCORECARDINALITY);
	m_member_row->m_parent_level = 0;//get_int(crt_row.PARENT_USCORELEVEL);
	m_member_row->m_parent_count = 0;//get_int(crt_row.PARENT_USCORECOUNT);

	return true;
}

bool mondrian_session_table::entry::develop_to_member( connection_handler& handler )
{
	xmlns__Restrictions restrictions;
	restrictions.RestrictionList.MEMBER_USCOREUNIQUE_USCORENAME = _strdup( owner.c_str() );
	restrictions.RestrictionList.CUBE_USCORENAME = _strdup( cube.c_str() );
	restrictions.RestrictionList.TREE_USCOREOP = _strdup( "8" );//TREEOP_SELF
			
	xmlns__Properties properties;			
	properties.PropertyList.LocaleIdentifier = CP_UTF8;
		
	cxmla__DiscoverResponse&  response = handler.raw_discover( "MDSCHEMA_MEMBERS", restrictions, properties );

	if ( 1 != response.cxmla__return__.root.__rows.__size ) { return false; }

	m_member_row.reset( new member_row() );
	row crt_row = response.cxmla__return__.root.__rows.row[0];

	m_member_row->m_catalog = _wcsdup( FROM_STRING( crt_row.CATALOG_USCORENAME, CP_UTF8 ) ) ;
	m_member_row->m_schema = nullptr;
	m_member_row->m_cube = _wcsdup(FROM_STRING( crt_row.CUBE_USCORENAME, CP_UTF8 ));
	m_member_row->m_dimension_unique_name = _wcsdup( FROM_STRING( crt_row.DIMENSION_USCOREUNIQUE_USCORENAME, CP_UTF8 ) );
	m_member_row->m_hierarchy_unique_name = _wcsdup( FROM_STRING( crt_row.HIERARCHY_USCOREUNIQUE_USCORENAME, CP_UTF8 ) );
		
	m_member_row->m_level_unique_name = nullptr;//_wcsdup( FROM_STRING( crt_row.LEVEL_USCOREUNIQUE_USCORENAME, CP_UTF8 ) );
	m_member_row->m_member_name = _wcsdup( FROM_STRING( simple_name.c_str(), CP_UTF8 ) );
	m_member_row->m_member_unique_name = _wcsdup( FROM_STRING( name.c_str(), CP_UTF8 ) );
	m_member_row->m_member_caption = _wcsdup( FROM_STRING( simple_name.c_str(), CP_UTF8 ) );
	m_member_row->m_parent_unique_name = _wcsdup( FROM_STRING( crt_row.MEMBER_USCOREUNIQUE_USCORENAME, CP_UTF8 ) );
	m_member_row->m_description = nullptr;//_wcsdup( FROM_STRING( crt_row.DESCRIPTION, CP_UTF8 ) );
		
	//m_level_number = get_int(crt_row.LEVEL_USCORENUMBER);
	m_member_row->m_member_ordinal = 0;//get_int( crt_row.MEMBER_USCOREORDINAL );
	m_member_row->m_member_type = MDMEMBER_TYPE_FORMULA;//get_int( crt_row.MEMBER_USCORETYPE );
	m_member_row->m_children_cardinality = 0;//get_int(crt_row.CHILDREN_USCORECARDINALITY);
	m_member_row->m_parent_level = 0;//get_int(crt_row.PARENT_USCORELEVEL);
	m_member_row->m_parent_count = 0;//get_int(crt_row.PARENT_USCORECOUNT);

	return true;
}