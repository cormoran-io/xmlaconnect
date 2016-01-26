/*
	ODBO provider for XMLA m_data_exchange stores
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
					a row in the axis rowset
*/

#pragma once

class axis_row
{
private:
	static const size_t MAX_BUF_SIZE = 10 * 1024 * sizeof( wchar_t );
	bool m_should_fix_aliases;
private:
	char*					m_data_exchange;
	xmlns__Axis*			m_axis;
	xmlns__AxisInfo*		m_axis_info;
	ATLCOLUMNINFO*			m_col_info;
	unsigned int			m_col_info_cnt;
	DBLENGTH				m_buf_size;

	struct member_def
	{
		bool has_parent_uniq_name;
		bool has_member_uniq_name;
		bool has_member_type;
		std::vector< std::string > custom_props;

		member_def() : has_parent_uniq_name(false), has_member_uniq_name(false), has_member_type(false) {}
	};

	std::vector<member_def> m_member_defs;
public:
	axis_row()
	{
		m_data_exchange = nullptr;
		m_axis = nullptr;
		m_col_info = nullptr;
	}

	~axis_row()
	{
		clear();
	}
	

	void setup_data( DBCOUNTITEM idx, execute_response::md_data_access& data, bool should_fix_aliases ) {

		clear();
		m_should_fix_aliases = should_fix_aliases;

		data.get_axis( idx, m_axis, m_axis_info );
		
		m_col_info_cnt = 1;//TUPLE_ORDINAL;
		for ( unsigned int i = 0, e = m_axis_info->__size; i < e; ++i ) {
			m_member_defs.push_back( member_def() );
			m_col_info_cnt += 5;//required columns;
			xmlns__HierarchyInfo hInfo = m_axis_info->HierarchyInfo[i];
			if ( nullptr != hInfo.PARENT_USCOREUNIQUE_USCORENAME ) {
				m_member_defs[i].has_parent_uniq_name = true;
				++m_col_info_cnt;
			}
			if ( nullptr != hInfo.MEMBER_USCORENAME ) {
				m_member_defs[i].has_member_uniq_name = true;
				++m_col_info_cnt;
			}
			if ( nullptr != hInfo.MEMBER_USCORETYPE ) {
				m_member_defs[i].has_member_type = true;
				++m_col_info_cnt;
			}

			for ( int j = 0; j < hInfo.__userProp.__size; ++j )
			{
				m_member_defs[i].custom_props.push_back( hInfo.__userProp.__array[j].elementName ) ;
				++m_col_info_cnt;
			}
		}
		m_buf_size = 0;

		m_col_info = new ATLCOLUMNINFO[ m_col_info_cnt ];

		m_col_info[0].pwszName = _wcsdup( L"TUPLE_ORDINAL" );
		m_col_info[0].pTypeInfo = (ITypeInfo*)nullptr;
		m_col_info[0].iOrdinal = 1;
		m_col_info[0].dwFlags = DBCOLUMNFLAGS_ISFIXEDLENGTH;
		m_col_info[0].ulColumnSize = sizeof(unsigned long);
		m_col_info[0].wType = DBTYPE_UI4;
		m_col_info[0].bPrecision = 10;
		m_col_info[0].bScale = 0xFF;
		m_col_info[0].cbOffset = m_buf_size;
		memset( &( m_col_info[0].columnid ), 0, sizeof( DBID ));
		m_buf_size += m_col_info[0].ulColumnSize;

		unsigned int crtColInfo = 1;

		for ( unsigned int i = 0, e = m_axis_info->__size; i < e; ++i ) {
			xmlns__HierarchyInfo hInfo = m_axis_info->HierarchyInfo[i];
			m_col_info[crtColInfo].pwszName = _wcsdup( CA2W( hInfo.UName.name, CP_UTF8 ) );
			m_col_info[crtColInfo].pTypeInfo = (ITypeInfo*)nullptr;
			m_col_info[crtColInfo].iOrdinal = crtColInfo+1;
			m_col_info[crtColInfo].dwFlags = DBCOLUMNFLAGS_ISFIXEDLENGTH;
			m_col_info[crtColInfo].ulColumnSize = MAX_BUF_SIZE;
			m_col_info[crtColInfo].wType = DBTYPE_WSTR;
			m_col_info[crtColInfo].bPrecision = 0xFF;
			m_col_info[crtColInfo].bScale = 0xFF;
			m_col_info[crtColInfo].cbOffset = m_buf_size;
			memset( &( m_col_info[crtColInfo].columnid ), 0, sizeof( DBID ));
			m_buf_size += m_col_info[crtColInfo].ulColumnSize;
			++crtColInfo;

			m_col_info[crtColInfo].pwszName = _wcsdup( CA2W( hInfo.Caption.name, CP_UTF8 ) );
			m_col_info[crtColInfo].pTypeInfo = (ITypeInfo*)nullptr;
			m_col_info[crtColInfo].iOrdinal = crtColInfo+1;
			m_col_info[crtColInfo].dwFlags = DBCOLUMNFLAGS_ISFIXEDLENGTH;
			m_col_info[crtColInfo].ulColumnSize = MAX_BUF_SIZE;
			m_col_info[crtColInfo].wType = DBTYPE_WSTR;
			m_col_info[crtColInfo].bPrecision = 0xFF;
			m_col_info[crtColInfo].bScale = 0xFF;
			m_col_info[crtColInfo].cbOffset = m_buf_size;
			memset( &( m_col_info[crtColInfo].columnid ), 0, sizeof( DBID ));
			m_buf_size += m_col_info[crtColInfo].ulColumnSize;
			++crtColInfo;

			m_col_info[crtColInfo].pwszName = _wcsdup( CA2W( hInfo.LName.name, CP_UTF8 ) );
			m_col_info[crtColInfo].pTypeInfo = (ITypeInfo*)nullptr;
			m_col_info[crtColInfo].iOrdinal = crtColInfo+1;
			m_col_info[crtColInfo].dwFlags = DBCOLUMNFLAGS_ISFIXEDLENGTH;
			m_col_info[crtColInfo].ulColumnSize = MAX_BUF_SIZE;
			m_col_info[crtColInfo].wType = DBTYPE_WSTR;
			m_col_info[crtColInfo].bPrecision = 0xFF;
			m_col_info[crtColInfo].bScale = 0xFF;
			m_col_info[crtColInfo].cbOffset = m_buf_size;
			memset( &( m_col_info[crtColInfo].columnid ), 0, sizeof( DBID ));
			m_buf_size += m_col_info[crtColInfo].ulColumnSize;
			++crtColInfo;

			m_col_info[crtColInfo].pwszName = _wcsdup( CA2W( hInfo.LNum.name, CP_UTF8 ) );
			m_col_info[crtColInfo].pTypeInfo = (ITypeInfo*)nullptr;
			m_col_info[crtColInfo].iOrdinal = crtColInfo+1;
			m_col_info[crtColInfo].dwFlags = DBCOLUMNFLAGS_ISFIXEDLENGTH;
			m_col_info[crtColInfo].ulColumnSize = sizeof(long);
			m_col_info[crtColInfo].wType = DBTYPE_I4;
			m_col_info[crtColInfo].bPrecision = 10;
			m_col_info[crtColInfo].bScale = 0xFF;
			m_col_info[crtColInfo].cbOffset = m_buf_size;
			memset( &( m_col_info[crtColInfo].columnid ), 0, sizeof( DBID ));
			m_buf_size += m_col_info[crtColInfo].ulColumnSize;
			++crtColInfo;

			m_col_info[crtColInfo].pwszName = _wcsdup( CA2W( hInfo.DisplayInfo.name, CP_UTF8 ) );
			m_col_info[crtColInfo].pTypeInfo = (ITypeInfo*)nullptr;
			m_col_info[crtColInfo].iOrdinal = crtColInfo+1;
			m_col_info[crtColInfo].dwFlags = DBCOLUMNFLAGS_ISFIXEDLENGTH;
			m_col_info[crtColInfo].ulColumnSize = sizeof(unsigned long);
			m_col_info[crtColInfo].wType = DBTYPE_UI4;
			m_col_info[crtColInfo].bPrecision = 10;
			m_col_info[crtColInfo].bScale = 0xFF;
			m_col_info[crtColInfo].cbOffset = m_buf_size;
			memset( &( m_col_info[crtColInfo].columnid ), 0, sizeof( DBID ));
			m_buf_size += m_col_info[crtColInfo].ulColumnSize;
			++crtColInfo;

			if ( nullptr != hInfo.PARENT_USCOREUNIQUE_USCORENAME ) {
				m_col_info[crtColInfo].pwszName = _wcsdup( CA2W( hInfo.PARENT_USCOREUNIQUE_USCORENAME->name, CP_UTF8 ) );
				m_col_info[crtColInfo].pTypeInfo = (ITypeInfo*)nullptr;
				m_col_info[crtColInfo].iOrdinal = crtColInfo+1;
				m_col_info[crtColInfo].dwFlags = DBCOLUMNFLAGS_ISFIXEDLENGTH;
				m_col_info[crtColInfo].ulColumnSize = MAX_BUF_SIZE;
				m_col_info[crtColInfo].wType = DBTYPE_WSTR;
				m_col_info[crtColInfo].bPrecision = 0xFF;
				m_col_info[crtColInfo].bScale = 0xFF;
				m_col_info[crtColInfo].cbOffset = m_buf_size;
				memset( &( m_col_info[crtColInfo].columnid ), 0, sizeof( DBID ));
				m_buf_size += m_col_info[crtColInfo].ulColumnSize;
				++crtColInfo;
			}
			if ( nullptr != hInfo.MEMBER_USCORENAME ) {
				m_col_info[crtColInfo].pwszName = _wcsdup( CA2W( hInfo.MEMBER_USCORENAME->name, CP_UTF8 ) );
				m_col_info[crtColInfo].pTypeInfo = (ITypeInfo*)nullptr;
				m_col_info[crtColInfo].iOrdinal = crtColInfo+1;
				m_col_info[crtColInfo].dwFlags = DBCOLUMNFLAGS_ISFIXEDLENGTH;
				m_col_info[crtColInfo].ulColumnSize = MAX_BUF_SIZE;
				m_col_info[crtColInfo].wType = DBTYPE_WSTR;
				m_col_info[crtColInfo].bPrecision = 0xFF;
				m_col_info[crtColInfo].bScale = 0xFF;
				m_col_info[crtColInfo].cbOffset = m_buf_size;
				memset( &( m_col_info[crtColInfo].columnid ), 0, sizeof( DBID ));
				m_buf_size += m_col_info[crtColInfo].ulColumnSize;
				++crtColInfo;
			}
			if ( nullptr != hInfo.MEMBER_USCORETYPE ) {
				m_col_info[crtColInfo].pwszName = _wcsdup( CA2W( hInfo.MEMBER_USCORETYPE->name, CP_UTF8 ) );
				m_col_info[crtColInfo].pTypeInfo = (ITypeInfo*)nullptr;
				m_col_info[crtColInfo].iOrdinal = crtColInfo+1;
				m_col_info[crtColInfo].dwFlags = DBCOLUMNFLAGS_ISFIXEDLENGTH;
				m_col_info[crtColInfo].ulColumnSize = sizeof(long);
				m_col_info[crtColInfo].wType = DBTYPE_I4;
				m_col_info[crtColInfo].bPrecision = 10;
				m_col_info[crtColInfo].bScale = 0xFF;
				m_col_info[crtColInfo].cbOffset = m_buf_size;
				memset( &( m_col_info[crtColInfo].columnid ), 0, sizeof( DBID ));
				m_buf_size += m_col_info[crtColInfo].ulColumnSize;
				++crtColInfo;
			}

			for ( unsigned int j = 0, je = hInfo.__userProp.__size; j < je; ++j ) {
				m_col_info[crtColInfo].pwszName = _wcsdup( CA2W( hInfo.__userProp.__array[j].elementName, CP_UTF8 ) );
				m_col_info[crtColInfo].pTypeInfo = (ITypeInfo*)nullptr;
				m_col_info[crtColInfo].iOrdinal = crtColInfo+1;
				m_col_info[crtColInfo].dwFlags = DBCOLUMNFLAGS_ISFIXEDLENGTH;
				m_col_info[crtColInfo].ulColumnSize = MAX_BUF_SIZE;
				m_col_info[crtColInfo].wType = DBTYPE_WSTR;
				m_col_info[crtColInfo].bPrecision = 0xFF;
				m_col_info[crtColInfo].bScale = 0xFF;
				m_col_info[crtColInfo].cbOffset = m_buf_size;
				memset( &( m_col_info[crtColInfo].columnid ), 0, sizeof( DBID ));
				m_buf_size += m_col_info[crtColInfo].ulColumnSize;
				++crtColInfo;
			}
		}

		m_data_exchange = new char[ m_buf_size ];
	}

	class string_replacer
	{
	private:
		std::string ret;
	public:
		string_replacer( char* in, const std::string& what, const std::string&  with )
			: ret( in )
		{
			size_t idx = ret.find( what );
			if ( std::string::npos == idx || 0 != idx ) { return; }
			ret.replace( 0, what.length(), with );
		}
		operator const char*()
		{
			return ret.c_str();
		}
	};
		
	inline const char& get_fix_aliases( size_t index ) const
	{
		xmlns__Tuple& crtTuple = m_axis->Tuples.Tuple[index];
		DBLENGTH offset = 0;
		size_t idx = 0;

		//Tuple Ordinal
		*( ( DBLENGTH *) ( m_data_exchange + offset ) ) = (DBLENGTH)index;
		offset += m_col_info[idx].ulColumnSize;
		++idx;

		for ( int i = 0; i < crtTuple.__size; ++i )
		{
			std::string hier_candidate(crtTuple.Member[ i ].Hierarchy);

			//when the dimension name and hierarchy name are the same there is a reduction rule:
			//foo.foo will be replaced by [foo] only in the member unique name.
			const size_t the_size = hier_candidate.size();
			if ( 1 == ( the_size % 2 ) && '.'== hier_candidate[the_size/2] && hier_candidate.substr(0, the_size / 2 ) == hier_candidate.substr( the_size / 2 + 1, the_size / 2 ) )
			{
				hier_candidate = hier_candidate.substr(0, the_size / 2 );
			}

			std::string hier_uname = std::string("[") + hier_candidate + "]";
			std::string u_name(crtTuple.Member[ i ].UName);

			//mondrian 4 is fully XMLA compliant. should skip this entirely;
			std::size_t first_pos = u_name.find("].[");
			bool is_xmla_comp = false;
			if ( std::string::npos != first_pos )
			{
				is_xmla_comp = u_name.substr( first_pos + 2, hier_uname.length() ) == hier_uname;
			}


			if ( is_xmla_comp || u_name.substr(0, hier_uname.length() ) == hier_uname )
			{
				//UName
				wcscpy_s(  ( wchar_t* )( m_data_exchange + offset ), m_col_info[idx].ulColumnSize / 2, CA2W( crtTuple.Member[ i ].UName, CP_UTF8 ) );
				offset += m_col_info[idx++].ulColumnSize;
				//Caption
				wcscpy_s(  ( wchar_t* )( m_data_exchange + offset ), m_col_info[idx].ulColumnSize / 2, CA2W( crtTuple.Member[ i ].Caption, CP_UTF8 ) );
				offset += m_col_info[idx++].ulColumnSize;
				//LName
				wcscpy_s(  ( wchar_t* )( m_data_exchange + offset ), m_col_info[idx].ulColumnSize / 2, CA2W( crtTuple.Member[ i ].LName, CP_UTF8 ) );
				offset += m_col_info[idx++].ulColumnSize;
				//LNum
				*( ( long* )( m_data_exchange + offset ) ) = atol( crtTuple.Member[ i ].LNum );
				offset += m_col_info[idx++].ulColumnSize;
				//DisplayInfo
				*( ( unsigned long* )( m_data_exchange + offset ) ) = atol( crtTuple.Member[ i ].DisplayInfo );
				offset += m_col_info[idx++].ulColumnSize;

				if ( m_member_defs[i].has_parent_uniq_name )
				{
					char* prop = crtTuple.Member[ i ].PARENT_USCOREUNIQUE_USCORENAME;
					if ( nullptr != prop ) {
						wcscpy_s(  ( wchar_t* )( m_data_exchange + offset ), m_col_info[idx].ulColumnSize / 2, CA2W( prop, CP_UTF8 ) );
					} else {
						*( ( wchar_t* )( m_data_exchange + offset ) ) = 0;
					}
					offset += m_col_info[idx++].ulColumnSize;
				}
			} else {
				std::string to_replace = u_name.substr( 0, u_name.find(']')+1);

				//UName
				wcscpy_s(  ( wchar_t* )( m_data_exchange + offset ), m_col_info[idx].ulColumnSize / 2, CA2W( string_replacer(crtTuple.Member[ i ].UName, to_replace, hier_uname )  , CP_UTF8 ) );
				offset += m_col_info[idx++].ulColumnSize;
				//Caption
				wcscpy_s(  ( wchar_t* )( m_data_exchange + offset ), m_col_info[idx].ulColumnSize / 2, CA2W( crtTuple.Member[ i ].Caption, CP_UTF8 ) );
				offset += m_col_info[idx++].ulColumnSize;
				//LName
				wcscpy_s(  ( wchar_t* )( m_data_exchange + offset ), m_col_info[idx].ulColumnSize / 2, CA2W( string_replacer(crtTuple.Member[ i ].LName, to_replace, hier_uname ), CP_UTF8 ) );
				offset += m_col_info[idx++].ulColumnSize;
				//LNum
				*( ( long* )( m_data_exchange + offset ) ) = atol( crtTuple.Member[ i ].LNum );
				offset += m_col_info[idx++].ulColumnSize;
				//DisplayInfo
				*( ( unsigned long* )( m_data_exchange + offset ) ) = atol( crtTuple.Member[ i ].DisplayInfo );
				offset += m_col_info[idx++].ulColumnSize;

				if ( m_member_defs[i].has_parent_uniq_name )
				{
					char* prop = crtTuple.Member[ i ].PARENT_USCOREUNIQUE_USCORENAME;
					if ( nullptr != prop ) {
						wcscpy_s(  ( wchar_t* )( m_data_exchange + offset ), m_col_info[idx].ulColumnSize / 2, CA2W( string_replacer( prop, to_replace, hier_uname ), CP_UTF8 ) );
					} else {
						*( ( wchar_t* )( m_data_exchange + offset ) ) = 0;
					}
					offset += m_col_info[idx++].ulColumnSize;
				}
			}

			if ( m_member_defs[i].has_member_uniq_name )
			{
				char* prop = crtTuple.Member[ i ].MEMBER_USCORENAME;
				if ( nullptr != prop ) {
					wcscpy_s(  ( wchar_t* )( m_data_exchange + offset ), m_col_info[idx].ulColumnSize / 2, CA2W( prop, CP_UTF8 ) );
				} else {
					*( ( wchar_t* )( m_data_exchange + offset ) ) = 0;
				}
				offset += m_col_info[idx++].ulColumnSize;
			}

			if ( m_member_defs[i].has_member_type )
			{
				static const std::string regular("MDMEMBER_TYPE_REGULAR");
				static const std::string all("MDMEMBER_TYPE_ALL");
				static const std::string measure("MDMEMBER_TYPE_MEASURE");
				static const std::string formula("MDMEMBER_TYPE_FORMULA");
				const char* type = crtTuple.Member[ i ].MEMBER_USCORETYPE;
				if ( regular == type ) {
					*( ( long* )( m_data_exchange + offset ) ) = MDMEMBER_TYPE_REGULAR;
				} else if ( all == type ){
					*( ( long* )( m_data_exchange + offset ) ) = MDMEMBER_TYPE_ALL;
				} else if ( measure == type ) {
					*( ( long* )( m_data_exchange + offset ) ) = MDMEMBER_TYPE_MEASURE;
				} else if ( formula == type ) {
					*( ( long* )( m_data_exchange + offset ) ) = MDMEMBER_TYPE_FORMULA;
				} else {//UNKNOWN
					*( ( long* )( m_data_exchange + offset ) ) = MDMEMBER_TYPE_UNKNOWN;
				}
				offset += m_col_info[idx++].ulColumnSize;
			}

			for ( size_t j = 0; j < m_member_defs[i].custom_props.size(); ++j )
			{
				*( ( wchar_t* )( m_data_exchange + offset ) ) = 0;

				for( int k = 0; k < crtTuple.Member[ i ].__userProp.__size; ++ k )
				{
					if ( m_member_defs[i].custom_props[j] == crtTuple.Member[ i ].__userProp.__array[k].elementName )
					{
						wcscpy_s(  ( wchar_t* )( m_data_exchange + offset ), m_col_info[idx].ulColumnSize / 2, CA2W( crtTuple.Member[ i ].__userProp.__array[k].value, CP_UTF8 ) );
						break;
					}
				}

				offset += m_col_info[idx++].ulColumnSize;
			}
		}
		return *m_data_exchange;

	}

	inline const char& operator[]( size_t index ) const
	{
		if ( index >= (size_t)m_axis->Tuples.__size ) {
			throw std::runtime_error( "index out of bounds" );
		}

		if ( m_should_fix_aliases )
		{
			return get_fix_aliases( index );
		}

		xmlns__Tuple& crtTuple = m_axis->Tuples.Tuple[index];
		DBLENGTH offset = 0;
		size_t idx = 0;

		//Tuple Ordinal
		*( ( DBLENGTH *) ( m_data_exchange + offset ) ) = (DBLENGTH)index;
		offset += m_col_info[idx].ulColumnSize;
		++idx;

		for ( int i = 0; i < crtTuple.__size; ++i )
		{
			//UName
			wcscpy_s(  ( wchar_t* )( m_data_exchange + offset ), m_col_info[idx].ulColumnSize / 2, CA2W( crtTuple.Member[ i ].UName, CP_UTF8 ) );
			offset += m_col_info[idx++].ulColumnSize;
			//Caption
			wcscpy_s(  ( wchar_t* )( m_data_exchange + offset ), m_col_info[idx].ulColumnSize / 2, CA2W( crtTuple.Member[ i ].Caption, CP_UTF8 ) );
			offset += m_col_info[idx++].ulColumnSize;
			//LName
			wcscpy_s(  ( wchar_t* )( m_data_exchange + offset ), m_col_info[idx].ulColumnSize / 2, CA2W( crtTuple.Member[ i ].LName, CP_UTF8 ) );
			offset += m_col_info[idx++].ulColumnSize;
			//LNum
			*( ( long* )( m_data_exchange + offset ) ) = atol( crtTuple.Member[ i ].LNum );
			offset += m_col_info[idx++].ulColumnSize;
			//DisplayInfo
			*( ( unsigned long* )( m_data_exchange + offset ) ) = atol( crtTuple.Member[ i ].DisplayInfo );
			offset += m_col_info[idx++].ulColumnSize;

			if ( m_member_defs[i].has_parent_uniq_name )
			{
				char* prop = crtTuple.Member[ i ].PARENT_USCOREUNIQUE_USCORENAME;
				if ( nullptr != prop ) {
					wcscpy_s(  ( wchar_t* )( m_data_exchange + offset ), m_col_info[idx].ulColumnSize / 2, CA2W( prop, CP_UTF8 ) );
				} else {
					*( ( wchar_t* )( m_data_exchange + offset ) ) = 0;
				}
				offset += m_col_info[idx++].ulColumnSize;
			}

			if ( m_member_defs[i].has_member_uniq_name )
			{
				char* prop = crtTuple.Member[ i ].MEMBER_USCORENAME;
				if ( nullptr != prop ) {
					wcscpy_s(  ( wchar_t* )( m_data_exchange + offset ), m_col_info[idx].ulColumnSize / 2, CA2W( prop, CP_UTF8 ) );
				} else {
					*( ( wchar_t* )( m_data_exchange + offset ) ) = 0;
				}
				offset += m_col_info[idx++].ulColumnSize;
			}

			if ( m_member_defs[i].has_member_type )
			{
				static const std::string regular("MDMEMBER_TYPE_REGULAR");
				static const std::string all("MDMEMBER_TYPE_ALL");
				static const std::string measure("MDMEMBER_TYPE_MEASURE");
				static const std::string formula("MDMEMBER_TYPE_FORMULA");
				const char* type = crtTuple.Member[ i ].MEMBER_USCORETYPE;
				if ( regular == type ) {
					*( ( long* )( m_data_exchange + offset ) ) = MDMEMBER_TYPE_REGULAR;
				} else if ( all == type ){
					*( ( long* )( m_data_exchange + offset ) ) = MDMEMBER_TYPE_ALL;
				} else if ( measure == type ) {
					*( ( long* )( m_data_exchange + offset ) ) = MDMEMBER_TYPE_MEASURE;
				} else if ( formula == type ) {
					*( ( long* )( m_data_exchange + offset ) ) = MDMEMBER_TYPE_FORMULA;
				} else {//UNKNOWN
					*( ( long* )( m_data_exchange + offset ) ) = MDMEMBER_TYPE_UNKNOWN;
				}
				offset += m_col_info[idx++].ulColumnSize;
			}

			for ( size_t j = 0; j < m_member_defs[i].custom_props.size(); ++j )
			{
				*( ( wchar_t* )( m_data_exchange + offset ) ) = 0;

				for( int k = 0; k < crtTuple.Member[ i ].__userProp.__size; ++ k )
				{
					if ( m_member_defs[i].custom_props[j] == crtTuple.Member[ i ].__userProp.__array[k].elementName )
					{
						wcscpy_s(  ( wchar_t* )( m_data_exchange + offset ), m_col_info[idx].ulColumnSize / 2, CA2W( crtTuple.Member[ i ].__userProp.__array[k].value, CP_UTF8 ) );
						break;
					}
				}

				offset += m_col_info[idx++].ulColumnSize;
			}
		}
		return *m_data_exchange;
	}

//prerequisites for a row in the rowset

	inline size_t GetCount()
	{
		return m_axis ? m_axis->Tuples.__size : 0;
	}

	void RemoveAll()
	{
		clear();
	}

	ATLCOLUMNINFO* GetColumnInfo( DBORDINAL* pcInfo )
	{
		*pcInfo = m_col_info_cnt;
		return m_col_info;
	}

private:
	void clear()
	{
		if ( nullptr != m_col_info ) {
			for ( unsigned int i = 0; i < m_col_info_cnt; ++i ) {
				delete[] m_col_info[i].pwszName;
			}
			delete[] m_col_info;
		}

		if ( nullptr != m_data_exchange ) {
			delete[] m_data_exchange;
		}

		m_col_info = nullptr;
		m_data_exchange = nullptr;
		m_axis = nullptr;
		m_member_defs.clear();
	}
};


