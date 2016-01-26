/*
	ODBO provider for XMLA data stores
    Copyright (C) 2014-2016  ARquery LTD
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
					wraps the data retured by Execute. One for each command
*/
#pragma once;

#include <cstring>
#include <cctype>
#include "soapXMLAConnectionProxy.h"

class execute_response
{
public:
class tabular_data_access
{
private:
	cxmla__ExecuteResponse& m_e_response;
	int m_col_count;
	ATL::ATLCOLUMNINFO* m_columns;

	void make_col_info()
	{
		m_columns = new ATL::ATLCOLUMNINFO[ m_col_count ];
		size_t offset = 0;
		for( int i = 0; i < m_col_count; ++i )
		{
			m_columns[i].pwszName = _wcsdup( ATL::CA2W(tabular_header( i ), CP_UTF8) );
			m_columns[i].pTypeInfo = (ITypeInfo*)NULL; 
			m_columns[i].iOrdinal = (ULONG)(i + 1); 
			m_columns[i].dwFlags = 0; 
			m_columns[i].wType = (DBTYPE) tabular_header_type(i); 
			switch ( m_columns[i].wType )
			{
			case DBTYPE_WSTR:
				m_columns[i].ulColumnSize = (ULONG)WORD_WIDTH; 
				m_columns[i].bPrecision = (BYTE)0xFF;
				m_columns[i].bScale = (BYTE)0xFF;
				m_columns[i].cbOffset = offset;
				offset += WORD_WIDTH * sizeof(wchar_t);
				break;
			case DBTYPE_R8:
				m_columns[i].ulColumnSize = (ULONG)sizeof(double); 
				m_columns[i].bPrecision = (BYTE)0xFF; 
				m_columns[i].bScale = (BYTE)0xFF;
				m_columns[i].cbOffset = offset;
				offset += sizeof(double);
				break;
			case DBTYPE_I4:
				m_columns[i].ulColumnSize = (ULONG)sizeof(int); 
				m_columns[i].bPrecision = (BYTE)0xFF;
				m_columns[i].bScale = (BYTE)0xFF;
				m_columns[i].cbOffset = offset;
				offset += sizeof(int);
				break;
			}
		}
	}

	void clean_column_info()
	{
		if ( nullptr == m_columns ){return;}
	
		for ( int i = 0; i < m_col_count; ++i )
		{
			delete[] m_columns[i].pwszName;
		}
		m_col_count = 0;
		delete[] m_columns;
		m_columns = nullptr;
	}

	const char* tabular_header( const int idx )
	{
		if ( 0 == m_e_response.cxmla__return__.root.__size ) { return ""; }//preffer empty to null
		if ( idx >= m_e_response.cxmla__return__.root.row[0].__size ){ return ""; }//preffer empty to null
		return m_e_response.cxmla__return__.root.row[0].__array[idx].elementName;
	}
	const DBTYPEENUM tabular_header_type( const int idx )
	{
		//the first row contains headers and counts. does not have typeinfo on the tags.
		if ( 2 > m_e_response.cxmla__return__.root.__size ) { return DBTYPE_WSTR; }//unknown is string
		if ( idx >= m_e_response.cxmla__return__.root.row[1].__size ){ return DBTYPE_WSTR; }//unknown is string
		if ( nullptr == (m_e_response.cxmla__return__.root.row[1].__array[idx].__xsi__type ) ) { return DBTYPE_WSTR; }//unknown is string
		const std::string type(m_e_response.cxmla__return__.root.row[1].__array[idx].__xsi__type);
		if ( type == "xsd:double" ) { return DBTYPE_R8; }
		if ( type == "xsd:int" ) { return DBTYPE_I4; }
		return DBTYPE_WSTR;
	}
public:
	tabular_data_access( cxmla__ExecuteResponse& response ) 
		: m_e_response( response )
		, m_columns( nullptr )
	{
		if ( 0 == m_e_response.cxmla__return__.root.__size ) 
		{
			m_col_count = 0;
			return;
		}

		if ( nullptr == m_e_response.cxmla__return__.root.row )
		{
			m_col_count = 0;
			return;
		}

		m_col_count = m_e_response.cxmla__return__.root.row[0].__size;
		make_col_info();
	}

	~tabular_data_access()
	{
		clean_column_info();
	}

	ATL::ATLCOLUMNINFO* GetColumnInfo( DBORDINAL* pcCols )
	{
		* pcCols = m_col_count;
		return m_columns;
	}

	void load_at( int idx, wchar_t* data )
	{
		if ( 0 > idx ) { return; }
		if ( idx >= m_e_response.cxmla__return__.root.__size ){ return; }

		for( int i = 0; i < m_col_count; ++i )
		{
			switch ( m_columns[i].wType )
			{
			case DBTYPE_WSTR:
				wcscpy_s( ( wchar_t*) ((char*)data + m_columns[i].cbOffset), WORD_WIDTH, ATL::CA2W(m_e_response.cxmla__return__.root.row[idx].__array[i].value, CP_UTF8) );
				break;
			case DBTYPE_R8:
				{
					double val = atof( m_e_response.cxmla__return__.root.row[idx].__array[i].value );
					CopyMemory( (char*)data + m_columns[i].cbOffset, &val, sizeof( val ) );
				}
				break;
			case DBTYPE_I4:
				{
					int val = atoi( m_e_response.cxmla__return__.root.row[idx].__array[i].value );
					CopyMemory( (char*)data + m_columns[i].cbOffset, &val, sizeof( val ) );
				}
				break;
			}
		}
	}

	const int col_count() const { return m_col_count; }
	const int row_count() const { return m_e_response.cxmla__return__.root.__size; }
	const int data_size() const { return m_col_count * WORD_WIDTH * sizeof(wchar_t); }
};
class md_data_access
{
private:
	cxmla__ExecuteResponse& m_e_response;
	ATL::ATLCOLUMNINFO* m_execute_colls;
	size_t m_execute_col_count;

	std::vector<int> m_cell_data;
	size_t m_cell_ordinal_pos;
	std::vector< std::pair< std::string, int > > m_indirection;//0 will be value, all user props will substract 1
	bool m_report_single_column;

public:
	md_data_access( cxmla__ExecuteResponse& response, bool report_single_column ) 
		: m_e_response( response )
		, m_execute_colls( nullptr )
		, m_report_single_column( report_single_column )
	{
			get_cell_data();
			form_column_headers(); 
	}

	~md_data_access()
	{
		if ( nullptr != m_execute_colls ) 
		{ 
			for ( size_t i = 0; i < m_execute_col_count; ++i )
			{
				delete[] m_execute_colls[i].pwszName;
			}
			delete[] m_execute_colls;
			m_execute_colls = nullptr;
		}
		m_indirection.clear();
	}

private:
	DBTYPEENUM dbtype_from_xsd( const char* xsd )
	{
		if ( nullptr == xsd ) { return DBTYPE_WSTR; }
		if ( 0 == strcmp( "xsd:string", xsd ) ) { return DBTYPE_WSTR; }
		if ( 0 == strcmp( "xsd:int", xsd ) ) { return DBTYPE_I4; }
		if ( 0 == strcmp( "xsd:unsignedInt", xsd ) ) { return DBTYPE_UI4; }
		if ( 0 == strcmp( "xsd:short", xsd ) ) { return DBTYPE_I2; }
		if ( 0 == strcmp( "xsd:unsignedShort", xsd ) ) { return DBTYPE_UI2; }
		return DBTYPE_WSTR;
	}

	void get_cell_data() 
	{
		if ( NULL == m_e_response.cxmla__return__.root.CellData ) return;
		if ( NULL == m_e_response.cxmla__return__.root.Axes ) return; 

		int size = 1;
		//get cellData size with empty values
		for( int i = 0; i < m_e_response.cxmla__return__.root.Axes->__size; ++i ) {
			if (  strcmp (m_e_response.cxmla__return__.root.Axes->Axis[i].name,"SlicerAxis") != 0) {
				size *= m_e_response.cxmla__return__.root.Axes->Axis[i].Tuples.__size;
			}
		}

		m_cell_data.assign (size,-1);

		for( int i = 0; i < m_e_response.cxmla__return__.root.CellData->__size; ++i ) {
			m_cell_data[ atoi(m_e_response.cxmla__return__.root.CellData->Cell[i].CellOrdinal) ] = i;
		}
	}

	void form_column_headers()
	{
		if ( nullptr == m_e_response.cxmla__return__.root.OlapInfo ) { return; }

		m_execute_colls = new ATL::ATLCOLUMNINFO[ m_e_response.cxmla__return__.root.OlapInfo->CellInfo.__cellProps.__size + 1 ];	
		DBLENGTH pos = 0;
		size_t crt = 0;
		m_cell_ordinal_pos = -1;

		if ( nullptr != m_e_response.cxmla__return__.root.OlapInfo->CellInfo.Value )
		{			
			m_execute_colls[crt].pwszName = _wcsdup( FROM_STRING( m_e_response.cxmla__return__.root.OlapInfo->CellInfo.Value->name, CP_UTF8 ) );
			m_execute_colls[crt].pTypeInfo = (ITypeInfo*)nullptr;
			m_execute_colls[crt].iOrdinal = crt + 1;
			m_execute_colls[crt].dwFlags = DBCOLUMNFLAGS_ISFIXEDLENGTH;
			m_execute_colls[crt].ulColumnSize = sizeof(VARIANT);
			m_execute_colls[crt].wType = DBTYPE_VARIANT;
			m_execute_colls[crt].bPrecision = 0;
			m_execute_colls[crt].bScale = 0;
			m_execute_colls[crt].cbOffset = pos;
			memset( &( m_execute_colls[crt].columnid ), 0, sizeof( DBID ));
	
			pos += m_execute_colls[crt].ulColumnSize;
			++crt;
			m_indirection.push_back( std::make_pair( std::string("Value"), 0 )  );
		}

		for ( int i = 0; i < m_e_response.cxmla__return__.root.OlapInfo->CellInfo.__cellProps.__size; ++i )
		{
			m_execute_colls[crt].pwszName = _wcsdup( FROM_STRING( m_e_response.cxmla__return__.root.OlapInfo->CellInfo.__cellProps.__array[i].name, CP_UTF8 ) );
			if ( 0 == wcscmp( m_execute_colls[crt].pwszName, L"CELL_ORDINAL" ) )
			{
				m_cell_ordinal_pos = crt;
			}
			m_execute_colls[crt].pTypeInfo = (ITypeInfo*)nullptr;
			m_execute_colls[crt].iOrdinal = crt + 1;
			m_execute_colls[crt].dwFlags = DBCOLUMNFLAGS_ISFIXEDLENGTH;
			m_execute_colls[crt].wType = dbtype_from_xsd( m_e_response.cxmla__return__.root.OlapInfo->CellInfo.__cellProps.__array[i].type );
			switch ( m_execute_colls[crt].wType )
			{
			case DBTYPE_WSTR:
				m_execute_colls[crt].dwFlags = DBCOLUMNFLAGS_MAYBENULL;
				m_execute_colls[crt].ulColumnSize = (ULONG)WORD_WIDTH; 
				m_execute_colls[crt].bPrecision = 0;
				m_execute_colls[crt].bScale = 0;
				m_execute_colls[crt].cbOffset = pos;
				pos += WORD_WIDTH * sizeof(wchar_t);
				break;
			case DBTYPE_UI4:
				m_execute_colls[crt].ulColumnSize = (ULONG)sizeof(unsigned int); 
				m_execute_colls[crt].bPrecision = (BYTE)0xFF; 
				m_execute_colls[crt].bScale = (BYTE)0xFF;
				m_execute_colls[crt].cbOffset = pos;
				pos += sizeof(unsigned int);
				break;
			case DBTYPE_I4:
				m_execute_colls[crt].ulColumnSize = (ULONG)sizeof(int); 
				m_execute_colls[crt].bPrecision = (BYTE)0xFF;
				m_execute_colls[crt].bScale = (BYTE)0xFF;
				m_execute_colls[crt].cbOffset = pos;
				pos += sizeof(int);
				break;
			case DBTYPE_UI2:
				m_execute_colls[crt].ulColumnSize = (ULONG)sizeof(unsigned short); 
				m_execute_colls[crt].bPrecision = (BYTE)0xFF; 
				m_execute_colls[crt].bScale = (BYTE)0xFF;
				m_execute_colls[crt].cbOffset = pos;
				pos += sizeof(unsigned short);
				break;
			case DBTYPE_I2:
				m_execute_colls[crt].ulColumnSize = (ULONG)sizeof(short); 
				m_execute_colls[crt].bPrecision = (BYTE)0xFF;
				m_execute_colls[crt].bScale = (BYTE)0xFF;
				m_execute_colls[crt].cbOffset = pos;
				pos += sizeof(short);
				break;
			}
			memset( &( m_execute_colls[crt].columnid ), 0, sizeof( DBID ));
			++crt;
			m_indirection.push_back( std::make_pair( std::string( m_e_response.cxmla__return__.root.OlapInfo->CellInfo.__cellProps.__array[i].elementName ), -1 )  );
		}
		m_execute_col_count = crt;
	}
public:
	ATL::ATLCOLUMNINFO* column_info( DBORDINAL* pcInfo )
	{
		if ( m_report_single_column ) {
			*pcInfo = 1;
		} else {
			*pcInfo = m_execute_col_count;
		}
		return m_execute_colls;
	}

	VARIANT at( DBORDINAL index, size_t indirection )
	{
		VARIANT result;
		result.vt = VT_NULL;

		if ( NULL == m_e_response.cxmla__return__.root.CellData ) {
			throw std::runtime_error( "no query response");
		}

		if ( index >= m_cell_data.size() ) {
			//empty response
			result.bstrVal = NULL;
			result.vt = VT_BSTR;
			return result;
		}

		if ( -1 == m_cell_data[index] ) {
			//empty response
			result.bstrVal = NULL;
			result.vt = VT_BSTR;
			return result;
		}

		//will only test once for user defined props

		if ( -1 == m_indirection[indirection-1].second )
		{
			m_indirection[indirection-1].second = -2;
			for ( int i = 0; i < m_e_response.cxmla__return__.root.CellData->Cell[m_cell_data[index]].__cellProps.__size; ++i )
			{
				if ( 0 == strcmp( m_indirection[indirection-1].first.c_str(), m_e_response.cxmla__return__.root.CellData->Cell[m_cell_data[index]].__cellProps.__array[i].elementName ) )
				{
					m_indirection[indirection-1].second = i+1;
					break;
				}
			}
		}

		switch ( m_indirection[indirection-1].second )
		{

		case 0://value
		{
			_Value& val = m_e_response.cxmla__return__.root.CellData->Cell[m_cell_data[index]].Value;

			if ( nullptr == val.__v )
			{
					result.bstrVal = NULL;
					result.vt = VT_BSTR;
					return result;
			}
			
			if ( 0 == strcmp( val.xsi__type, "xsd:double" ) ) {
				result.vt = VT_R8;
				if ( NULL == val.__v ) {
					result.bstrVal = NULL;
					result.vt = VT_BSTR;
				} else {
					char* end_pos;
					result.dblVal = strtod(val.__v,&end_pos);
					if ( 0 == result.dblVal && 0 != *end_pos )
					{
						result.bstrVal = SysAllocString( ATL::CA2W( val.__v, CP_UTF8 ) );
						result.vt = VT_BSTR;
					}
				}
			} else if ( 0 == strcmp( val.xsi__type, "xsd:string" ) ) {
				result.vt = VT_BSTR;
				if ( NULL != val.__v ) {
					result.bstrVal = SysAllocString( ATL::CA2W( val.__v, CP_UTF8 ) );
				} else {
					result.bstrVal = NULL;
				}
			} else if ( 0 == strcmp( val.xsi__type, "xsd:empty" ) ) {
				result.bstrVal = NULL;
				result.vt = VT_BSTR;
			} else if ( 0 == strcmp( val.xsi__type, "xsd:int" ) ) {
				result.vt = VT_I4;
				if ( NULL == val.__v ) {
					result.bstrVal = NULL;
					result.vt = VT_BSTR;
				} else {
					char* end_pos;
					result.intVal = strtol( val.__v, &end_pos, 10 );
					if ( 0 == result.intVal && 0 != *end_pos )
					{
						result.bstrVal = SysAllocString( ATL::CA2W( val.__v, CP_UTF8 ) );
						result.vt = VT_BSTR;
					}
				}
			} else if ( 0 == strcmp( val.xsi__type, "xsd:boolean" ) ) { 
				if ( NULL == val.__v ) {
					result.bstrVal = NULL;
					result.vt = VT_BSTR;
				} else {
					result.vt = VT_BOOL;
					result.boolVal = (std::strcmp("true", val.__v) == 0);
				}
			} else {
				//handle unknown as string
				result.vt = VT_BSTR;
				if ( NULL != val.__v ) {
					result.bstrVal = SysAllocString( ATL::CA2W( val.__v, CP_UTF8 ) );
				} else {
					result.bstrVal = NULL;
				}
			}
		}

		break;

		default:
		{			
			char* data = nullptr;
			if ( m_e_response.cxmla__return__.root.CellData->Cell[m_cell_data[index]].__cellProps.__size >= m_indirection[indirection-1].second )
			{
				data = const_cast<char*>( m_e_response.cxmla__return__.root.CellData->Cell[m_cell_data[index]].__cellProps.__array[m_indirection[indirection-1].second-1].value );
			}
			if ( data ) {
				DBTYPE data_type = m_execute_colls[indirection-1].wType;
				switch ( data_type )
				{
				case DBTYPE_WSTR:
					{
						result.vt = VT_BSTR;
						result.bstrVal = SysAllocString( ATL::CA2W( data, CP_UTF8 ) );
					}
					break;
				}
			} else {
				result.vt = VT_EMPTY;
			}	
		}
		break;

		case -2:
			result.vt = VT_EMPTY;
			break;

		}

		return result;
	}

	void get_axis( DBCOUNTITEM idx, xmlns__Axis*& axisData, xmlns__AxisInfo*& axisInfo )
	{
		if ( NULL == m_e_response.cxmla__return__.root.Axes ) {
			throw std::runtime_error( "no query response");
		}

		if ( NULL == m_e_response.cxmla__return__.root.OlapInfo ) {
			throw std::runtime_error( "no query response");
		}

		std::string axisName;
		if ( MDAXIS_SLICERS == idx ) {
			axisName = "SlicerAxis";
		} else {
			std::stringstream buf;
			buf << "Axis";
			buf << idx;
			axisName = buf.str();
		}
		
		for ( int i = 0, e = m_e_response.cxmla__return__.root.Axes->__size; i < e; ++i ) {
			if ( axisName == m_e_response.cxmla__return__.root.Axes->Axis[i].name ) {
				axisData = &( m_e_response.cxmla__return__.root.Axes->Axis[i] );
			}
		}

		for ( DBCOUNTITEM i = 0, e = ( DBCOUNTITEM ) m_e_response.cxmla__return__.root.OlapInfo->AxesInfo.__size; i < e; ++i ) {
			if ( axisName == m_e_response.cxmla__return__.root.OlapInfo->AxesInfo.AxisInfo[i].name ) {
				axisInfo = &(m_e_response.cxmla__return__.root.OlapInfo->AxesInfo.AxisInfo[i] );
			}
		}
	}

	void get_axis_info( DBCOUNTITEM   *pcAxes, MDAXISINFO   **prgAxisInfo )
	{
		if ( nullptr == m_e_response.cxmla__return__.root.OlapInfo ) {
			throw std::runtime_error( "no query response");
		}

		if ( nullptr == m_e_response.cxmla__return__.root.OlapInfo ){
			throw std::runtime_error( "the server returned an invalid answer");
		}

		if ( nullptr == m_e_response.cxmla__return__.root.Axes ){
			throw std::runtime_error( "the server returned an invalid answer");
		}

		*pcAxes			= ( DBCOUNTITEM ) m_e_response.cxmla__return__.root.OlapInfo->AxesInfo.__size;
		if ( 0 == *pcAxes ) {
			return;
		}

		//Mondrian gives an empty slicer
		DBCOUNTITEM idx = 0;

		MDAXISINFO* axisInfo = new MDAXISINFO[*pcAxes];
		for ( DBCOUNTITEM i = 0; i < *pcAxes; ++i ) {
			axisInfo[idx].cbSize = sizeof( MDAXISINFO );
			axisInfo[idx].rgcColumns = nullptr;
			axisInfo[idx].rgpwszDimensionNames = nullptr;
			axisInfo[idx].cCoordinates = m_e_response.cxmla__return__.root.Axes->Axis[idx].Tuples.__size;//count on the same order
			axisInfo[idx].cDimensions = m_e_response.cxmla__return__.root.OlapInfo->AxesInfo.AxisInfo[idx].__size;
			std::string name( m_e_response.cxmla__return__.root.OlapInfo->AxesInfo.AxisInfo[idx].name );
			std::transform( name.begin(), name.end(), name.begin(), std::tolower );
			if ( name.substr( 0, 4 ) == "axis" ) {
				axisInfo[idx].iAxis = atoi( name.substr(4, name.size() ).c_str() );
			} else {
				axisInfo[idx].iAxis = MDAXIS_SLICERS;
				if ( 0 == m_e_response.cxmla__return__.root.OlapInfo->AxesInfo.AxisInfo[idx].__size ) {
					//slicer was present but it was empty										
					axisInfo[idx].cCoordinates = 0;
					axisInfo[idx].cDimensions = 0;
					continue;
				}
			}
			axisInfo[idx].rgcColumns = new DBORDINAL[ axisInfo[idx].cDimensions ];
			axisInfo[idx].rgpwszDimensionNames = new LPOLESTR[ axisInfo[idx].cDimensions ];
			for ( DBCOUNTITEM j = 0; j < axisInfo[idx].cDimensions; ++j ) {
				xmlns__HierarchyInfo hInfo = m_e_response.cxmla__return__.root.OlapInfo->AxesInfo.AxisInfo[idx].HierarchyInfo[j];
				DBORDINAL col_count = 5;//required columns;
				if ( NULL != hInfo.PARENT_USCOREUNIQUE_USCORENAME ) {
					++col_count;
				}
				if ( NULL != hInfo.MEMBER_USCORENAME ) {
					++col_count;
				}
				if ( NULL != hInfo.MEMBER_USCORETYPE ) {
					++col_count;
				}
				col_count += hInfo.__userProp.__size;

				axisInfo[idx].rgcColumns[j] = col_count;
				axisInfo[idx].rgpwszDimensionNames[j] =  _wcsdup( ATL::CA2W( m_e_response.cxmla__return__.root.OlapInfo->AxesInfo.AxisInfo[idx].HierarchyInfo[j].name, CP_UTF8 ) );

			}
			++idx;
		}

		DWORD* leak = new DWORD[1];

		*prgAxisInfo	= axisInfo;
	}

	void free_axis_info( DBCOUNTITEM   cAxes, MDAXISINFO   *rgAxisInfo )
	{
		for ( DBCOUNTITEM i = 0; i < cAxes; ++i ) {
			if ( 0 == rgAxisInfo[i].cDimensions ) { continue; }
			delete[] rgAxisInfo[i].rgcColumns;
			for ( DBCOUNTITEM j = 0; j < rgAxisInfo[i].cDimensions; ++j ) {
				delete[] rgAxisInfo[i].rgpwszDimensionNames[j];
			}
			delete[] rgAxisInfo[i].rgpwszDimensionNames;
		}
		delete[] rgAxisInfo;
	}

	unsigned int row_count()
	{
		if ( NULL == m_e_response.cxmla__return__.root.CellData ) {
			return 0;
		}
		return m_e_response.cxmla__return__.root.CellData->__size;
	}

	bool  is_cell_ordinal( size_t indirection )
	{
		return m_cell_ordinal_pos == indirection-1;
	}
};
private:
	cxmla__ExecuteResponse m_e_response;
	std::unique_ptr<tabular_data_access> m_tabular_data_access;
	std::unique_ptr<md_data_access> m_md_data_access;

	bool m_should_fix_aliases;
	bool m_resize_coll_info;
public:
	execute_response()	
		: m_should_fix_aliases( false )
		, m_resize_coll_info( false )
	{
		m_tabular_data_access.reset( nullptr );
		m_md_data_access.reset( nullptr );
	}

	cxmla__ExecuteResponse& get_response()
	{
		return m_e_response;
	}

	void switch_to_tabular_result()
	{
		m_tabular_data_access.reset( new tabular_data_access( m_e_response ) );
		m_md_data_access.reset( nullptr );
	}

	const bool has_tabular_data() const 
	{ 
		return nullptr != m_tabular_data_access.get(); 
	}

	tabular_data_access& access_tab_data()
	{
		return *( m_tabular_data_access.get() );
	}

	void switch_to_multidimensional_result()
	{
		m_md_data_access.reset( new md_data_access( m_e_response, m_should_fix_aliases && m_resize_coll_info ) );
		m_tabular_data_access.reset( nullptr );
	}

	const bool has_md_data() const 
	{ 
		return nullptr != m_md_data_access.get(); 
	}

	md_data_access& access_md_data()
	{
		return *( m_md_data_access.get() );
	}

	void set_should_fix_aliases( bool value )
	{
		m_should_fix_aliases = value;
	}

	bool should_fix_aliases()
	{
		return m_should_fix_aliases;
	}

	void set_resize_coll_info( bool value )
	{
		m_resize_coll_info = value;
	}
	

};