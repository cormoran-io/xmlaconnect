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

#pragma once

#include <cctype>
#include <algorithm>
#include <string>
#include <regex>
#include <vector>

#include "set_row.h"
#include "member_row.h"

class connection_handler;

class mondrian_session_table
{	
public:
	struct entry
	{
	private:
		#define REGEX_MASK "\\s*CREATE\\s+SESSION\\s+(MEMBER|SET)\\s+(.*?)\\s+AS\\s+(.*)"	
	private:
		void copy( const entry& other )
		{
			type = other.type ;
			name = other.name ;
			owner = other.owner ;
			cube = other.cube ;
			formula = other.formula ;
			simple_name = other.simple_name ;
			
			if ( nullptr == other.m_member_row ) {
				m_member_row( nullptr );
			} else {
				m_member_row.reset ( new member_row( *( other.m_member_row.get() ) ) );
			}

			if ( nullptr == other.m_set_row ) {
				m_set_row( nullptr );
			} else {
				m_set_row.reset ( new set_row( *( other.m_set_row.get() ) ) );
			}
		}
	public:
		enum entry_type
		{
			MEASURE_TYPE,
			MEMBER_TYPE,
			SET_TYPE,
			INVALID_TYPE
		};
		
		entry_type type;
		std::string name;
		std::string owner;
		std::string cube;
		std::string formula;
		std::string simple_name;

		std::unique_ptr< member_row >	m_member_row;
		std::unique_ptr< set_row >	m_set_row;

		entry& operator =(const entry& other )
		{
			if ( this != &other ) { copy( other ); }
			
			return *this;
		}

		entry( const entry& other )
		{
			copy( other );
		}

		entry()
			: m_member_row( nullptr )
			, m_set_row( nullptr )
		{
			type = INVALID_TYPE;
		}

		entry( const std::string& statement )
			: m_member_row( nullptr )
			, m_set_row( nullptr )
		{
			std::match_results< std::string::const_iterator > matches;
			std::regex rule( REGEX_MASK, std::regex_constants::ECMAScript | std::regex_constants::icase );

			if ( !std::regex_match( statement, matches, rule ) ) 
			{ 
				type = INVALID_TYPE;
				return;
			}

			std::string type_as_string( *( matches.begin() + 1 ) );
			std::transform( type_as_string.begin(), type_as_string.end(), type_as_string.begin(), std::toupper );

			if (  type_as_string == "MEMBER" )
			{
				type = MEMBER_TYPE;
			} else if (  type_as_string == "SET" )
			{
				type = SET_TYPE;
			} else
			{ 
				type = INVALID_TYPE;
				return;
			}

			name = *( matches.begin() + 2 );

			//prefixed with cube
			std::size_t dot_pos = name.find_first_of('.');
			if ( std::string::npos != dot_pos )
			{
				cube = name.substr( 1, dot_pos - 2 );
				name = name.substr( dot_pos + 1, name.size() );
			}

			dot_pos = name.find_last_of('.');
			if ( std::string::npos != dot_pos )
			{
				owner = name.substr( 0, dot_pos );
				simple_name = name.substr( dot_pos + 2, name.length() - dot_pos - 3 );
			} else 
			{
				simple_name = name.substr( 1, name.length() - 2 );
			}

			formula = *( matches.begin() + 3 );
		}

		bool develop( connection_handler& handler )
		{
			if ( MEMBER_TYPE == type ) {
				if( develop_to_measure( handler ) ) { return true; }
				if( develop_to_member( handler ) ) { return true; }
			} else if ( SET_TYPE == type ) {
				if( develop_to_set( handler ) ) { return true; }
			}
			return false;
		}

	private:
		bool develop_to_set( connection_handler& handler );
		bool develop_to_measure( connection_handler& handler );
		bool develop_to_member( connection_handler& handler );
	};
private:
	#define REGEX_MASK_WIDTH "\\s*WITH\\s+(.*)"
public:
	
	const bool try_register_session_member( const std::string statement, connection_handler& handler )
	{

		entry new_entry( statement );

		if ( new_entry.type == entry::INVALID_TYPE ) { return false; }

		new_entry.develop( handler );

		switch ( new_entry.type )
		{
		case entry::MEASURE_TYPE:
		case entry::MEMBER_TYPE:
			m_session_members.push_back( new_entry );
			break;
		case entry::SET_TYPE:
			m_session_sets.push_back( new_entry );
			break;
		}
		return true;
	}

	bool transform( std::string& statement )
	{
		if ( m_session_members.empty() && m_session_sets.empty() ){ return false; }

		std::match_results< std::string::const_iterator > matches;
		std::regex rule( REGEX_MASK_WIDTH, std::regex_constants::ECMAScript | std::regex_constants::icase );

		if ( std::regex_match( statement, matches, rule ) ) 
		{ 
			statement = std::string(" ") + std::string( *( matches.begin() + 1 ) );
		}

		for ( container_vector_type::const_iterator i = m_session_members.begin(), e = m_session_members.end(); i != e; ++i )
		{

			statement = "MEMBER " + i->name + " AS '" + i->formula + "' " + statement;
		}

		for ( container_vector_type::const_iterator i = m_session_sets.begin(), e = m_session_sets.end(); i != e; ++i )
		{
				statement = "SET " + i->name + " AS '" + i->formula + "' " + statement;
		}

		statement = "WITH " + statement;
		return true;
	}	

	bool get_session_member( ULONG cRestrictions, const VARIANT* rgRestrictions, member_row& a_row )
	{
		#define TREEOP_SELF 8

		if ( cRestrictions >= 12 && ( ( VT_I4 == rgRestrictions[11].vt && TREEOP_SELF == rgRestrictions[11].lVal ) || (  VT_UI4 == rgRestrictions[11].vt && TREEOP_SELF == rgRestrictions[11].ulVal  ) ) )
		{
			std::string member_unique_name( ATL::CW2A( rgRestrictions[8].bstrVal, CP_UTF8 ) );
			container_vector_type::const_iterator match = std::find_if( m_session_members.begin(), m_session_members.end(), [&]( const entry& crt ) { return crt.name == member_unique_name; } );
			if ( match == m_session_members.end() ) { return false; }; 
			a_row = *((*match).m_member_row.get());
			return true;
		}
		return false;
	}

	std::vector< std::pair< std::string, std::string > >  get_session_measures()
	{
		std::vector< std::pair< std::string, std::string > > data;

		for ( container_vector_type::const_iterator i = m_session_members.begin(), e = m_session_members.end(); i != e; ++i )
		{
			if ( entry::MEASURE_TYPE != i->type ) { continue; }
			data.push_back( std::make_pair( i->name, i->simple_name ) );
		}
		return data;
	}

	std::vector< set_row > get_session_sets( ULONG cRestrictions, const VARIANT* rgRestrictions )
	{
		std::vector< set_row > data;

		std::string filter("");
		if ( cRestrictions >=4 && VT_BSTR == rgRestrictions[3].vt && nullptr != rgRestrictions[3].bstrVal )
		{
			filter = std::string( ATL::CW2A( rgRestrictions[3].bstrVal, CP_UTF8 ) );
		}

		for ( container_vector_type::const_iterator i = m_session_sets.begin(), e = m_session_sets.end(); i != e; ++i )
		{
			if ( !filter.empty() && filter != i->simple_name ) { continue; }
			data.push_back( *( i->m_set_row.get() ) );
		}
		return data;
	}

private:	
	typedef std::vector< entry > container_vector_type;

	container_vector_type	m_session_members;
	container_vector_type	m_session_sets;
};