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
					session object
*/

#pragma once

#include <strsafe.h>
#include "config_data.h"

//from msmd.h
#define DBPROP_VISUALMODE				(DBPROP_SESS_AUTOCOMMITISOLEVELS  + 1)
//extern const OLEDBDECLSPEC GUID MDSCHEMA_MEASUREGROUPS					= {0xe1625ebf,0xfa96,0x42fd,{0xbe,0xa6,0xdb,0x90,0xad,0xaf,0xd9,0x6b}};



class catalog_rowset;
class cube_rowset;
class dimension_rowset;
class function_rowset;
class hierarchy_rowset;
class level_rowset;
class member_rowset;
class measure_rowset;
//class measuregroup_rowset;
class property_rowset;
class set_rowset;
class schema_rowset;
										
const GUID DISCOVER_SCHEMA_ROWSETS = {0xeea0302b,0x7922,0x4992,{0x89,0x91,0x0e,0x60,0x5d,0x0e,0x55,0x93}};

using namespace ATL;

class ATL_NO_VTABLE session : 
	public CComObjectRootEx<CComObjectThreadModel>,
	public IGetDataSourceImpl<session>,
	public IOpenRowsetImpl<session>,
	public ISessionPropertiesImpl<session>,
	public IObjectWithSiteSessionImpl<session>,
	public IDBSchemaRowsetImpl<session>,
	public IDBCreateCommandImpl<session, command>,
	public IInternalConnectionImpl<session>,
	public IGetSelf	, 
	public ISupportErrorInfo
{
private:
	std::auto_ptr< connection_handler > m_connection_handler;
public:
	struct session_data
	{
		enum server_type
		{
			UNDEFINED = 0,
			ORACLE = 1,
			MONDRIAN = 2,
			JEDOX = 3
		};

		server_type server;

		session_data() : server( UNDEFINED ){}

		void register_server( const char* server_data )
		{
			if ( server !=  UNDEFINED  ) { return; }
			if ( str_match( server_data, "Mondrian" ) ) { server = MONDRIAN; return; }
			if ( str_match( server_data, "PentahoXMLA" ) ) { server = MONDRIAN; return; }
			if ( str_match( server_data, "Arquery" ) ) { server = ORACLE; return; }
			if ( str_match( server_data, "Palo" ) ) { server = JEDOX; return; }
		}

		bool mondrian() 
		{
			if ( MONDRIAN == server ) return true;
			return false;
		}

	private:
		bool str_match( const char* src, const char* match )
		{
			while ( *src++ == *match++ )
			{
				if ( 0 == *src ) { return true; }
			}
			return 0 == *match;
		}
	};

	typedef std::vector< session* > session_table_type;

	static session_table_type& session_table()
	{
		static session_table_type instance;
		return instance;
	}
public:
	DECLARE_PROTECT_FINAL_CONSTRUCT()

	HRESULT FinalConstruct()
	{
		session_table().push_back( this );
		return FInit();
	}
	
	void FinalRelease() 	
	{
		session_table_type& st = session_table();
		
		st.erase( std::find( st.begin(), st.end(), this ) );
	}

	STDMETHOD(OpenRowset)(IUnknown *pUnk, DBID *pTID, DBID *pInID, REFIID riid,
					   ULONG cSets, DBPROPSET rgSets[], IUnknown **ppRowset)
	{
		rowset* pRowset;
		return CreateRowset(pUnk, pTID, pInID, riid, cSets, rgSets, ppRowset, pRowset);
	}

	void SetRestrictions(ULONG cRestrictions, GUID* rguidSchema, ULONG* rgRestrictions)
	{
		for (ULONG l=0; l<cRestrictions; l++)
		{
			if (InlineIsEqualGUID(rguidSchema[l], DBSCHEMA_TABLES))
				rgRestrictions[l] = 0x04;
			else if (InlineIsEqualGUID(rguidSchema[l], DBSCHEMA_COLUMNS))
				rgRestrictions[l] = 0x04;
			else if (InlineIsEqualGUID(rguidSchema[l], DBSCHEMA_PROVIDER_TYPES))
				rgRestrictions[l] = 0x00;
		}
	}

//IGetSelf
	STDMETHODIMP GetSelf( void** pSelf )
	{
		if ( NULL == pSelf ) {
			return E_INVALIDARG;
		}
		*pSelf = this;
		return S_OK;
	}

//ISupportErrorInfo
	HRESULT STDMETHODCALLTYPE InterfaceSupportsErrorInfo (REFIID riid)
	{
		return S_OK;
	}

HRESULT init_connection_handler();

static connection_handler* connection_handler( IUnknown* aSession )
{
	session* session_obj;
	IGetSelf* pGetSelf;
	aSession->QueryInterface( __uuidof( IGetSelf ) , ( void** ) &pGetSelf );
	pGetSelf->GetSelf( (void**)&session_obj );
	pGetSelf->Release();

	if ( nullptr == session_obj->m_connection_handler.get() ) { session_obj->init_connection_handler(); }

	return session_obj->m_connection_handler.get();
}


BEGIN_PROPSET_MAP(session)
	BEGIN_PROPERTY_SET_COND(config_data::visual_totals(), DBPROPSET_SESSION)
		PROPERTY_INFO_ENTRY( CURRENTCATALOG )
		PROPERTY_INFO_ENTRY_VALUE( VISUALMODE, MDPROPVAL_VISUAL_MODE_VISUAL )
	ELSE_PROPERTY_SET_COND(DBPROPSET_SESSION)
		PROPERTY_INFO_ENTRY( CURRENTCATALOG )
	END_PROPERTY_SET_COND(DBPROPSET_SESSION)
END_PROPSET_MAP()

BEGIN_COM_MAP(session)
	COM_INTERFACE_ENTRY(IGetDataSource)
	COM_INTERFACE_ENTRY(IOpenRowset)
	COM_INTERFACE_ENTRY(ISessionProperties)
	COM_INTERFACE_ENTRY(IObjectWithSite)
	COM_INTERFACE_ENTRY(IDBCreateCommand)
	COM_INTERFACE_ENTRY(IDBSchemaRowset)
	COM_INTERFACE_ENTRY(IInternalConnection)
	COM_INTERFACE_ENTRY(ISupportErrorInfo)
	COM_INTERFACE_ENTRY(IGetSelf)
END_COM_MAP()





BEGIN_SCHEMA_MAP(session)
	SCHEMA_ENTRY( DBSCHEMA_CATALOGS , catalog_rowset )
	SCHEMA_ENTRY( MDSCHEMA_CUBES , cube_rowset )
	SCHEMA_ENTRY( MDSCHEMA_DIMENSIONS, dimension_rowset )
	SCHEMA_ENTRY( MDSCHEMA_FUNCTIONS, function_rowset )
	SCHEMA_ENTRY( MDSCHEMA_HIERARCHIES, hierarchy_rowset )
	SCHEMA_ENTRY( MDSCHEMA_LEVELS, level_rowset )
	SCHEMA_ENTRY( MDSCHEMA_MEMBERS, member_rowset )
	SCHEMA_ENTRY( MDSCHEMA_MEASURES, measure_rowset )
//	SCHEMA_ENTRY( MDSCHEMA_MEASUREGROUPS, measuregroup_rowset )
	SCHEMA_ENTRY( MDSCHEMA_PROPERTIES, property_rowset )
	SCHEMA_ENTRY( MDSCHEMA_SETS, set_rowset )
//	SCHEMA_ENTRY( DISCOVER_SCHEMA_ROWSETS, schema_rowset )
	
END_SCHEMA_MAP()

};

#include "catalog_row.h"
#include "cube_row.h"
#include "dimension_row.h"
#include "function_row.h"
#include "hierarchy_row.h"
#include "level_row.h"
#include "measure_row.h"
//#include "measuregroup_row.h"
#include "member_row.h"
#include "property_row.h"
#include "set_row.h"
#include "base_rowset.h"
#include "schema_rowset.h"


class schema_rowset : public base_rowset< schema_rowset, schema_row, session >{};
class catalog_rowset : public base_rowset< catalog_rowset, catalog_row, session >{};
class cube_rowset : public base_rowset< cube_rowset, cube_row, session >{};
class dimension_rowset : public base_rowset< dimension_rowset, dimension_row, session >{};
class function_rowset : public base_rowset< function_rowset, function_row, session >{};
class hierarchy_rowset : public base_rowset< hierarchy_rowset, hierarchy_row, session >{};
class level_rowset : public base_rowset< level_rowset, level_row, session >{};
class member_rowset : public base_rowset< member_rowset, member_row, session >
{
public:

	//IDBSchemaRowset implementation
	HRESULT Execute( DBROWCOUNT* pcRowsAffected, ULONG cRestrictions, const VARIANT* rgRestrictions ) 
	{
		connection_handler* handler = session::connection_handler( m_spUnkSite );


		member_row buf;
		if ( handler->get_session_member( cRestrictions, rgRestrictions, buf ) )
		{
			m_rgRowData.Add( buf );
			*pcRowsAffected = 1;
			return S_OK;
		}

		int result = handler->discover( member_row::schema_name(), cRestrictions, rgRestrictions);
		if ( handler->no_session() ) {
			result = handler->discover( member_row::schema_name(), cRestrictions, rgRestrictions);
		}
		if ( S_OK != result ) {
			make_error( FROM_STRING( handler->fault_string(), CP_UTF8 ) );
			return E_FAIL;
		}

		for ( int i = 0, e = handler->discover_response().cxmla__return__.root.__rows.__size; i < e; ++i ) {			
			m_rgRowData.Add( member_row( handler->discover_response().cxmla__return__.root.__rows.row[i] ) );
		}


		*pcRowsAffected = (LONG) m_rgRowData.GetCount();
		return S_OK;
	}
};
class measure_rowset : public base_rowset< measure_rowset, measure_row, session >
{
public:

	//IDBSchemaRowset implementation
	HRESULT Execute( DBROWCOUNT* pcRowsAffected, ULONG cRestrictions, const VARIANT* rgRestrictions ) 
	{
		connection_handler* handler = session::connection_handler( m_spUnkSite );

		int result = handler->discover( measure_row::schema_name(), cRestrictions, rgRestrictions);
		if ( handler->no_session() ) {
			result = handler->discover( measure_row::schema_name(), cRestrictions, rgRestrictions);
		}
		if ( S_OK != result ) {
			make_error( FROM_STRING( handler->fault_string(), CP_UTF8 ) );
			return E_FAIL;
		}

		for ( int i = 0, e = handler->discover_response().cxmla__return__.root.__rows.__size; i < e; ++i ) {			
			m_rgRowData.Add( measure_row( handler->discover_response().cxmla__return__.root.__rows.row[i] ) );
		}

		if ( m_rgRowData.IsEmpty() ) { return S_OK; }

		std::vector< std::pair< std::string, std::string > > mondrian_session_measures = handler->get_session_measures();

		for ( size_t i = 0; i < mondrian_session_measures.size(); ++i )
		{
			measure_row row(m_rgRowData.GetAt(0) );

			delete[] row.m_measure_name;
			delete[] row.m_measure_unique_name;
			delete[] row.m_measure_caption;
			delete[] row.m_description;

			row.m_measure_name = _wcsdup( FROM_STRING( mondrian_session_measures[i].second.c_str(), CP_UTF8 ) );
			row.m_measure_unique_name = _wcsdup( FROM_STRING( mondrian_session_measures[i].first.c_str(), CP_UTF8 ) );
			row.m_measure_caption = _wcsdup( FROM_STRING( mondrian_session_measures[i].second.c_str(), CP_UTF8 ) );
			row.m_description = _wcsdup( FROM_STRING( mondrian_session_measures[i].second.c_str(), CP_UTF8 ) );

			m_rgRowData.Add( row );
		}


		*pcRowsAffected = (LONG) m_rgRowData.GetCount();
		return S_OK;
	}
};
class property_rowset : public base_rowset< property_rowset, property_row, session >{};
//class measuregroup_rowset : public base_rowset< measuregroup_rowset, measuregroup_row, session >{};
class set_rowset : public base_rowset< set_rowset, set_row, session >
{
public:

	//IDBSchemaRowset implementation
	HRESULT Execute( DBROWCOUNT* pcRowsAffected, ULONG cRestrictions, const VARIANT* rgRestrictions ) 
	{
		connection_handler* handler = session::connection_handler( m_spUnkSite );

		int result = handler->discover( set_row::schema_name(), cRestrictions, rgRestrictions);
		if ( handler->no_session() ) {
			result = handler->discover( set_row::schema_name(), cRestrictions, rgRestrictions);
		}
		if ( S_OK != result ) {
			make_error( FROM_STRING( handler->fault_string(), CP_UTF8 ) );
			return E_FAIL;
		}

		for ( int i = 0, e = handler->discover_response().cxmla__return__.root.__rows.__size; i < e; ++i ) {			
			m_rgRowData.Add( set_row( handler->discover_response().cxmla__return__.root.__rows.row[i] ) );
		}

		std::vector< set_row > mondrian_session_sets = handler->get_session_sets( cRestrictions, rgRestrictions );

		for ( size_t i = 0; i < mondrian_session_sets.size(); ++i )
		{

			m_rgRowData.Add( mondrian_session_sets[i] );
		}


		*pcRowsAffected = (LONG) m_rgRowData.GetCount();
		return S_OK;
	}
};
