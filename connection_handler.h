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
					wraps a connection to the XMXLA server.
					calls Discover/Execute.
					one for each session.
*/

#pragma once

#define ALLOW_TRANSLATIONS

#include <vector>
#include <unordered_map>

#include "soapXMLAConnectionProxy.h"


#include "query_translator.h"
#include "mondrian_session_table.h"


#include "Log.h"

#include "config_data.h"
#include "pass_prompt.h"

#include "cross_join_reducer.h"
#include "execute_response.h"

class connection_handler
{
public:
	class out_of_bound : public std::runtime_error 
	{
	public:
		out_of_bound() : std::runtime_error("index out of bounds"){}
	};
private:
	std::string			m_session_id;
	cxmla__DiscoverResponse m_d_response;
	XMLAConnectionProxy m_proxy;
	std::string m_location;
	std::string m_user;
	std::string m_pass;
	std::string m_catalog;
	

public:
	session::session_data	m_sesion_data;
	mondrian_session_table	m_session_vars;
	bool					m_resize_coll_info;
private:
	void load_restrictions( ULONG cRestrictions, const VARIANT* rgRestrictions, xmlns__Restrictions& where, bool add_cat = true )
	{
		//TODO: validate memory consumption due to the strdup here

		where.RestrictionList.PropertyName = NULL;
		where.RestrictionList.CATALOG_USCORENAME = NULL;
		where.RestrictionList.CUBE_USCORENAME = NULL;
		where.RestrictionList.HIERARCHY_USCOREUNIQUE_USCORENAME = NULL;
		where.RestrictionList.MEMBER_USCOREUNIQUE_USCORENAME = NULL;
		where.RestrictionList.LEVEL_USCOREUNIQUE_USCORENAME = NULL;
		where.RestrictionList.TREE_USCOREOP = NULL;
		where.RestrictionList.PROPERTY_USCORETYPE = NULL;

		if ( !m_catalog.empty() && add_cat ) {
			where.RestrictionList.CATALOG_USCORENAME = _strdup(m_catalog.c_str());//Allocates for consistence (all the other do strdup)
		}

		//only handle what we know
		for( ULONG i = 0; i < cRestrictions; i++ ) 
		{
			switch (i) 
			{
			case 0://CATALOG_NAME
				if ( VT_BSTR == rgRestrictions[i].vt  ) {
					where.RestrictionList.CATALOG_USCORENAME = _strdup(CT2A(rgRestrictions[i].bstrVal, CP_UTF8));
				}
				break;
			case 2://CUBE_NAME
				if ( VT_BSTR == rgRestrictions[i].vt  ) {
					where.RestrictionList.CUBE_USCORENAME = _strdup(CT2A(rgRestrictions[i].bstrVal, CP_UTF8));
				}
				break;
			case 3://SET_NAME
				if ( VT_BSTR == rgRestrictions[i].vt && add_cat ) {
					where.RestrictionList.SET_USCORENAME = _strdup(CT2A(rgRestrictions[i].bstrVal, CP_UTF8));
				}
				break;
			case 4://HIERARCHY_UNIQUE_NAME
				if ( VT_BSTR == rgRestrictions[i].vt  ) {
					where.RestrictionList.HIERARCHY_USCOREUNIQUE_USCORENAME = _strdup(CT2A(rgRestrictions[i].bstrVal, CP_UTF8));
				}
				break;
			case 5://LEVEL_UNIQUE_NAME
				if ( VT_BSTR == rgRestrictions[i].vt  ) {
					where.RestrictionList.LEVEL_USCOREUNIQUE_USCORENAME = _strdup(CT2A(rgRestrictions[i].bstrVal, CP_UTF8));
				}
				break;
			case 8:
				if ( VT_BSTR == rgRestrictions[i].vt  ) {//MEMBER_UNIQUE_NAME
					where.RestrictionList.MEMBER_USCOREUNIQUE_USCORENAME = _strdup(CT2A(rgRestrictions[i].bstrVal, CP_UTF8));
				} else if ( VT_I2 == rgRestrictions[i].vt ) {//PROPERTY_TYPE
					char buf[20];
					_itoa_s( rgRestrictions[i].iVal, buf, 20, 10 );
					where.RestrictionList.PROPERTY_USCORETYPE = _strdup( buf );
				}
				break;
			case 11://TREE_OP
				if ( VT_UI4 == rgRestrictions[i].vt  ) {
					char buf[20];
					_itoa_s( rgRestrictions[i].uiVal, buf, 20, 10 );
					where.RestrictionList.TREE_USCOREOP = _strdup( buf );
				} //else if ( VT_EMPTY == rgRestrictions[i].vt  ) {
				//	where.RestrictionList.TREE_USCOREOP = "0";
				//}
				break;
			}
		}
	}

	void unload_restrictions( xmlns__Restrictions& where )
	{
		if ( NULL != where.RestrictionList.PROPERTY_USCORETYPE ) 
		{
			free( where.RestrictionList.PROPERTY_USCORETYPE );
			where.RestrictionList.PROPERTY_USCORETYPE = NULL;
		}
		if ( NULL != where.RestrictionList.PropertyName ) 
		{
			free( where.RestrictionList.PropertyName );
			where.RestrictionList.PropertyName = NULL;
		}
		if ( NULL != where.RestrictionList.CATALOG_USCORENAME )
		{
			free( where.RestrictionList.CATALOG_USCORENAME );
			where.RestrictionList.CATALOG_USCORENAME = NULL;
		}
		if ( NULL != where.RestrictionList.CUBE_USCORENAME )
		{
			free( where.RestrictionList.CUBE_USCORENAME );
			where.RestrictionList.CUBE_USCORENAME = NULL;
		}
		if ( NULL != where.RestrictionList.HIERARCHY_USCOREUNIQUE_USCORENAME )
		{
			free( where.RestrictionList.HIERARCHY_USCOREUNIQUE_USCORENAME );
			where.RestrictionList.HIERARCHY_USCOREUNIQUE_USCORENAME = NULL;
		}
		if ( NULL != where.RestrictionList.MEMBER_USCOREUNIQUE_USCORENAME )
		{
			free( where.RestrictionList.MEMBER_USCOREUNIQUE_USCORENAME );
			where.RestrictionList.MEMBER_USCOREUNIQUE_USCORENAME = NULL;
		}
		if ( NULL != where.RestrictionList.LEVEL_USCOREUNIQUE_USCORENAME )
		{
			free( where.RestrictionList.LEVEL_USCOREUNIQUE_USCORENAME );
			where.RestrictionList.LEVEL_USCOREUNIQUE_USCORENAME = NULL;
		}
		if ( NULL != where.RestrictionList.TREE_USCOREOP )
		{
			free( where.RestrictionList.TREE_USCOREOP );
			where.RestrictionList.TREE_USCOREOP = NULL; 
		}
		if ( NULL != where.RestrictionList.SET_USCORENAME )
		{
			free( where.RestrictionList.SET_USCORENAME );
			where.RestrictionList.SET_USCORENAME = NULL;
		}
	}

private:
	void begin_session()
	{
		m_proxy.header = new SOAP_ENV__Header();
		m_proxy.header->BeginSession = new BSessionType();
		m_proxy.header->BeginSession->element = NULL;
		m_proxy.header->BeginSession->xmlns = NULL;
		m_proxy.header->EndSession = NULL;
		m_proxy.header->Session = NULL;
	}

	void session()
	{
		m_proxy.header = new SOAP_ENV__Header();
		m_proxy.header->Session = new SessionType();
		m_proxy.header->Session->element = NULL;
		m_proxy.header->Session->xmlns = NULL;
		m_proxy.header->Session->SessionId = _strdup( m_session_id.c_str() );
		m_proxy.header->EndSession = NULL;
		m_proxy.header->BeginSession = NULL;
	}

	void prompt_initialize( HWND parent_window_handle )
	{
		pass_prompt_ui login_prompt;
		_tcscpy_s( login_prompt.m_user, 256, CA2T( m_user.c_str(), CP_UTF8) );
		_tcscpy_s( login_prompt.m_pass, 256, CA2T( m_pass.c_str(), CP_UTF8) );

		if ( IDOK == login_prompt.DoModal( parent_window_handle ) ) {
			m_user.assign(CT2A( login_prompt.m_user, CP_UTF8 ));
			m_pass.assign(CT2A( login_prompt.m_pass, CP_UTF8 ));
		}
	}
public:
	connection_handler( const std::string& location, const std::string& user, const std::string& pass, const std::string& catalog )
		: m_location(location)
		, m_user(user)
		, m_pass(pass)
		, m_catalog( catalog )
	{		
		config_data::ssl_init( &m_proxy );

		config_data::get_proxy( m_location.c_str(), m_proxy.proxy_host, m_proxy.proxy_port );		

		m_proxy.soap_endpoint = m_location.c_str();
		soap_omode(&m_proxy, SOAP_XML_DEFAULTNS | SOAP_C_UTFSTRING | SOAP_IO_KEEPALIVE | SOAP_IO_CHUNK );
		soap_imode(&m_proxy, SOAP_C_UTFSTRING | SOAP_IO_KEEPALIVE | SOAP_IO_CHUNK );		

		if ( m_pass.empty() )
		{
			config_data::cred_iterator match = config_data::m_credentials.find( m_location+m_catalog );
			if ( config_data::m_credentials.end() != match )  {
				m_user = match->second.first;
				m_pass = match->second.second;			
			}
		}
	}

	const std::string& user() const { return m_user; }
	const std::string& pass() const { return m_pass; }

	bool check_login( HWND parent_window_handle )
	{
		::execute_response resp;
		for (  int i = 0; i < 3; ++i ){		
			if ( S_OK != execute("",resp) && !valid_credentials() ) {
				prompt_initialize( parent_window_handle );//has side effect. changes m_user and m_pass;
			} else {
				config_data::m_credentials[m_location+m_catalog] = config_data::key_val_type(m_user, m_pass);
				return true;
			}
		}
		return false;
	}

	bool get_session_member( ULONG cRestrictions, const VARIANT* rgRestrictions, member_row& a_row )
	{
		return m_session_vars.get_session_member( cRestrictions, rgRestrictions, a_row );
	}

	std::vector< std::pair< std::string, std::string > > get_session_measures()
	{
		return m_session_vars.get_session_measures();
	}

	std::vector< set_row > get_session_sets( ULONG cRestrictions, const VARIANT* rgRestrictions )
	{
		return m_session_vars.get_session_sets( cRestrictions, rgRestrictions );
	}

	cxmla__DiscoverResponse& raw_discover( char* endpoint, xmlns__Restrictions& restrictions, xmlns__Properties& properties )
	{
		m_proxy.Discover( endpoint, restrictions, properties, m_d_response );
		return m_d_response;
	}

	int discover( char* endpoint, ULONG cRestrictions, const VARIANT* rgRestrictions, bool skip_seesion = false)
	{
		if ( m_session_id.empty() && !skip_seesion ) {
			begin_session();
			m_proxy.userid = m_user.c_str();
			m_proxy.passwd = m_pass.c_str();
		} else if ( !skip_seesion ) {
			//palo requires credentials inside the session
			m_proxy.userid = m_user.c_str();
			m_proxy.passwd = m_pass.c_str();
			session();
		}	

		bool loadProperties = false;
		xmlns__Restrictions restrictions;
		load_restrictions( cRestrictions, rgRestrictions, restrictions, strcmp("DISCOVER_LITERALS", endpoint) != 0 && strcmp("MDSCHEMA_FUNCTIONS", endpoint) != 0 );

		xmlns__Properties props;
		props.PropertyList.Catalog = const_cast<char*>(m_catalog.c_str());//make Palo happy	
		props.PropertyList.LocaleIdentifier = CP_UTF8;
		int result = m_proxy.Discover( endpoint, restrictions, props, m_d_response );


		if ( NULL != m_proxy.header && NULL != m_proxy.header->Session && NULL != m_proxy.header->Session->SessionId ) {
			m_session_id =  m_proxy.header->Session->SessionId;
		}
		unload_restrictions( restrictions );
		return result;
	}

	int execute ( char* statement, execute_response& response )
	{
		bool tabular_result = false;	
		if ( m_session_id.empty() ) {
			begin_session();
			m_proxy.userid = m_user.c_str();
			m_proxy.passwd = m_pass.c_str();
		} else {
			//palo requires credentials inside the session
			m_proxy.userid = m_user.c_str();
			m_proxy.passwd = m_pass.c_str();
			session();
		}	

		xmlns__Command command;

		std::string translation( statement );
		m_resize_coll_info = false;
		
		if ( m_sesion_data.mondrian() )
		{
			if ( m_session_vars.try_register_session_member( translation, *this ) ){
				translation.clear(); 
			} else {
				m_session_vars.transform( translation );
			}

			size_t pos = translation.find("CELL PROPERTIES VALUE");
			size_t total = translation.size();

			if ( (std::string::npos != pos) && ( (pos+21) == translation.size() )) {
				response.set_resize_coll_info( true );
			}

		}

		query_translator::translator().translate( translation, m_sesion_data.server );

		cross_join_reducer reducer( translation );
		reducer.compute();
		if ( reducer.success() )
		{
			translation = reducer.create_canonical_query();
		}

		statement = const_cast<char*>( translation.c_str() );

		command.Statement = statement;
		xmlns__Properties Properties;
		Properties.PropertyList.LocaleIdentifier = CP_UTF8;
		Properties.PropertyList.Content = "Data";
		Properties.PropertyList.AxisFormat = "TupleFormat";
		
		std::string drill_through_test(statement, statement+strlen("DRILLTHROUGH")); 
		std::transform( drill_through_test.begin(), drill_through_test.end(), drill_through_test.begin(), std::toupper );
		
		if ( drill_through_test == "DRILLTHROUGH" )
		{
			tabular_result = true;			
			Properties.PropertyList.Format = "Tabular";
		} else
		{
			Properties.PropertyList.Format = "Multidimensional";
		}

		Properties.PropertyList.Catalog = const_cast<char*>(m_catalog.c_str());

		int result = m_proxy.Execute( NULL, command, Properties, response.get_response() );
		
		if ( NULL != m_proxy.header && NULL != m_proxy.header->Session && NULL != m_proxy.header->Session->SessionId ) {
			m_session_id = m_proxy.header->Session->SessionId;
		}

		response.set_should_fix_aliases(m_sesion_data.server == session::session_data::MONDRIAN);
		if ( tabular_result ) { response.switch_to_tabular_result(); }
		else { response.switch_to_multidimensional_result(); }

		return result;
	}

	bool no_session() 
	{
		bool result = false;
		if ( m_proxy.fault ) {
			 result = (0 == strcmp(m_proxy.fault->faultstring,"Invalid Session id"));
			 if ( !result ) {
				  char * pch;
				  pch = strstr(m_proxy.fault->faultstring, " unknown XMLA session");
				  result = ( nullptr != pch );
			 }
		}
		if ( result ) {
			m_session_id.clear();
		}
		return result;
	}

	const cxmla__DiscoverResponse& discover_response() const
	{
		return m_d_response;
	}

	const char* fault_string()
	{
		if  ( NULL == m_proxy.fault ) {
			static const char* noInfo = "No further information.";
			return noInfo;
		}

		if ( nullptr != m_proxy.fault->detail && nullptr != m_proxy.fault->detail->error.desc )
		{
			return m_proxy.fault->detail->error.desc;
		}

		return m_proxy.fault->faultstring;
	}

	const bool valid_credentials()
	{
		if  ( NULL == m_proxy.fault ) { return true; }
		if  ( 401 ==  m_proxy.error ) 
		{
			m_session_id.clear();
			return false; 
		}
		const bool is_valid =  ( NULL == strstr( m_proxy.fault->faultstring, "ORA-01005" )  && NULL == strstr( m_proxy.fault->faultstring, "ORA-01017" ) );
		if ( !is_valid )
		{
			m_session_id.clear();
		}
		return is_valid;
	}

	void detect_server()
	{
		if ( m_session_id.empty() ) {
			begin_session();
			m_proxy.userid = m_user.c_str();
			m_proxy.passwd = m_pass.c_str();
		} else {
			session();
			m_proxy.userid = m_user.c_str();
			m_proxy.passwd = m_pass.c_str();
		}	
		xmlns__Restrictions restrictions;
		xmlns__Properties props;
		props.PropertyList.LocaleIdentifier = CP_UTF8;
		int result = m_proxy.Discover( "DISCOVER_DATASOURCES", restrictions, props, m_d_response );
		if ( 0 != m_d_response.cxmla__return__.root.__rows.__size ){ m_sesion_data.register_server( m_d_response.cxmla__return__.root.__rows.row[0].ProviderName ); }
	}
};
