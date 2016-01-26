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
					rowset methods and the instantiation of the data_source
*/

#include "stdafx.h"
#include "rowset.h"
#include "data_source.h"
#include "XMLAMamespaces.nsmap"

//#define ARQUERY_SERVER

//Excel 2010 reads the subqueries property before setting the location 
//so there is no way to query it from the server
#ifdef	ARQUERY_SERVER
__declspec( thread ) bool data_source::thread_has_subqueries = true;
#else
__declspec( thread ) bool data_source::thread_has_subqueries = false;
#endif


ATL::ATLCOLUMNINFO* row_data::GetColumnInfo( void* pv, DBORDINAL* pcInfo )
{
	return (static_cast<rowset*>(pv))->m_md_data_access->column_info( pcInfo );
}

HRESULT rowset::Execute(DBPARAMS * /*pParams*/, DBROWCOUNT* pcRowsAffected)
{
	execute_response* data_holder;
	IExtendCommand* pIExtCommand = NULL;
	HRESULT hr = m_spUnkSite->QueryInterface( __uuidof( IExtendCommand ), ( void**) &pIExtCommand );
	
	if FAILED( hr ) { return hr; }

	pIExtCommand->GetDataHolder( (void**)&data_holder );
	pIExtCommand->Release();

	if ( nullptr == data_holder ) { return E_FAIL; }

	m_md_data_access = &( data_holder->access_md_data() );

	if ( nullptr != pcRowsAffected )
	{
		*pcRowsAffected = m_md_data_access->row_count();
	}

	return S_OK;
}


STDMETHODIMP rowset::GetCellData(
	    HACCESSOR   hAccessor,
	    DBORDINAL   ulStartCell,
	    DBORDINAL   ulEndCell,
	    VOID       *pData ) 
{
	ATLBINDINGS*									bind		= ( ATLBINDINGS* ) hAccessor;
	DBSTATUS										status;
	DBLENGTH										rowSize		= GetRowSize(hAccessor);
	HRESULT											result		= S_OK;


	for ( DBORDINAL crtCell = ulStartCell; crtCell <= ulEndCell; crtCell++ )
	{
		for (DBCOUNTITEM  i = 0; i < bind->cBindings; i++ ) 
		{
			status = DBSTATUS_E_CANTCONVERTVALUE;

			if ( DBPART_LENGTH == ( DBPART_LENGTH & bind->pBindings[i].dwPart ) )
			{
				*(( DWORD* )((( char* ) pData ) + bind->pBindings[i].obLength + rowSize*(crtCell - ulStartCell) ) ) = sizeof(VARIANT*);
			}

			if ( DBPART_VALUE == ( DBPART_VALUE & bind->pBindings[i].dwPart ) )
			{
				if ( DBTYPE_VARIANT == bind->pBindings[i].wType )
				{
					if ( m_md_data_access->is_cell_ordinal( bind->pBindings[i].iOrdinal ) )
					{
						VARIANT cell_data;
						cell_data.vt = VT_UI4;
						cell_data.ulVal = (ULONG) crtCell;
						*(( VARIANT* )((( char* ) pData ) + bind->pBindings[i].obValue + rowSize*(crtCell - ulStartCell) ) ) = cell_data;
					} 
					else
					{
						try
						{
							VARIANT cell_data = m_md_data_access->at( crtCell, bind->pBindings[i].iOrdinal );
							*(( VARIANT* )((( char* ) pData ) + bind->pBindings[i].obValue + rowSize*(crtCell - ulStartCell) ) ) = cell_data;
							status = DBSTATUS_S_OK;
						}
						catch( connection_handler::out_of_bound& )
						{
							status = DB_S_ENDOFROWSET;
							result = DB_S_ENDOFROWSET;
						}
					}
				}
			}
			else
			{
				status = DBSTATUS_S_OK;
			}

			if ( DBPART_STATUS == ( DBPART_STATUS & bind->pBindings[i].dwPart ) )
			{
				*(( DBSTATUS* )((( char* ) pData ) + bind->pBindings[i].obStatus + rowSize*(crtCell - ulStartCell) ) ) = status;
			}

			if ( DB_S_ENDOFROWSET == result ) {
				break;
			}
		}
	}

	return result;
}

STDMETHODIMP rowset::GetAxisInfo( DBCOUNTITEM   *pcAxes, MDAXISINFO   **prgAxisInfo )
{
	m_md_data_access->get_axis_info( pcAxes, prgAxisInfo );
	return S_OK;
}

STDMETHODIMP rowset::FreeAxisInfo( DBCOUNTITEM cAxes, MDAXISINFO *rgAxisInfo ) 
{
	m_md_data_access->free_axis_info( cAxes, rgAxisInfo );
	return S_OK;
}

STDMETHODIMP rowset::GetAxisRowset (
									IUnknown     *pUnkOuter,
									DBCOUNTITEM   iAxis,
									REFIID        riid,
									ULONG         cPropertySets,
									DBPROPSET     rgPropertySets[],
									IUnknown      **ppRowset )
{
	IExtendCommand* pIExtCommand = NULL;
	HRESULT hr = m_spUnkSite->QueryInterface( __uuidof( IExtendCommand ), ( void**) &pIExtCommand );
	
	if FAILED( hr ) return hr;

	hr = pIExtCommand->GetAxisRowset( pUnkOuter, riid, NULL, (DBROWCOUNT*) &iAxis, ppRowset );

	pIExtCommand->Release();

	return hr;
}


