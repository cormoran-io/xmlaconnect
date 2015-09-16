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
					session methods
*/

#include "stdafx.h"
#include "rowset.h"
#include "tabular_rowset.h"
#include "command.h"
#include "session.h"
#include "connection_handler.h"


HRESULT session::init_connection_handler()
{
	std::string location;
	std::string user;
	std::string pass;
	std::string catalog;

	HRESULT					hr;
	IGetDataSource*			pDataSource = NULL;
	IDBProperties*			pProperties = NULL;
	IMalloc*				pIMalloc = NULL;

	ULONG propCount;
	DBPROPSET* props;

	HWND					parent_window_handle;

	if FAILED( hr = CoGetMalloc( 1, &pIMalloc ) ) {
		return hr;
	}

	if FAILED( hr = QueryInterface(__uuidof(IGetDataSource),(void**)&pDataSource) ) {
		pIMalloc->Release();
		return hr;
	}

	if FAILED( hr = pDataSource->GetDataSource( __uuidof(IDBProperties), ( IUnknown** ) &pProperties ) )
	{
		pIMalloc->Release();
		pDataSource->Release();
		return hr;
	}

	//Session catalog has lower precendence than db catalog
	ISessionProperties* pISessionProperties = NULL;
	if SUCCEEDED( QueryInterface(__uuidof(ISessionProperties),(void**)&pISessionProperties) ) {
		pISessionProperties->GetProperties( 0, NULL, &propCount, &props );

		for ( ULONG i = 0; i < propCount; i++ )
		{
			for ( ULONG j =0; j < props[i].cProperties; j++ )
			{
				if ( IsEqualGUID( props[i].guidPropertySet, DBPROPSET_SESSION ) ) {
					if ( DBPROP_CURRENTCATALOG == props[i].rgProperties[j].dwPropertyID ) {
						std::string buf = CT2A(props[i].rgProperties[j].vValue.bstrVal, CP_UTF8);
						if ( !buf.empty() )  {
							std::swap( catalog, buf );
						}
					}
				}
				VariantClear( &(props[i].rgProperties[j].vValue) );
			}
			pIMalloc->Free( props[i].rgProperties );
		}
		pIMalloc->Free( props );

		pISessionProperties->Release();
	}


	pProperties->GetProperties( 0, NULL, &propCount, &props );

	for ( ULONG i = 0; i < propCount; i++ )
	{
		for ( ULONG j =0; j < props[i].cProperties; j++ )
		{
			if ( IsEqualGUID( props[i].guidPropertySet,DBPROPSET_DBINIT ) )
			{
				DBPROP  crtProp = props[i].rgProperties[j];
				switch ( crtProp.dwPropertyID )
				{
				case DBPROP_INIT_LOCATION:
					location = CT2A(crtProp.vValue.bstrVal, CP_UTF8);
					break;
				case DBPROP_AUTH_USERID:
					user = CT2A(crtProp.vValue.bstrVal, CP_UTF8);
					break;
				case DBPROP_AUTH_PASSWORD:
					pass = CT2A(crtProp.vValue.bstrVal, CP_UTF8);
					break;
				case DBPROP_INIT_CATALOG:
					if ( crtProp.vValue.bstrVal )  { catalog = CT2A(crtProp.vValue.bstrVal, CP_UTF8); }
					break;
					case DBPROP_INIT_HWND:
#ifdef _WIN64 
						//(HWND)crtProp.vValue.llVal; 64 bit does not set the handle right.
						//Sometimes I see here uninitialized data in the high order long.
						//I quess the trick below will handle :) it as the documentation hints it is only the low order 32 bits that matter
						parent_window_handle = IsWindow( (HWND)crtProp.vValue.llVal ) ? (HWND)crtProp.vValue.llVal : (HWND)( (LONG)crtProp.vValue.llVal );
#else
						parent_window_handle = (HWND)crtProp.vValue.lVal;
#endif

						break;
				}
			}
			VariantClear( &(props[i].rgProperties[j].vValue) );
		}
		pIMalloc->Free( props[i].rgProperties );
	}
	pIMalloc->Free( props );

	pIMalloc->Release();
	pProperties->Release();
	pDataSource->Release();

	m_connection_handler.reset( new ::connection_handler( location, user, pass, catalog) );

	if ( m_connection_handler->check_login( parent_window_handle ) ) { return S_OK; } 
	else { return E_FAIL; }

	return S_OK;
}

