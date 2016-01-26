#pragma once

#include <stack>
#include <assert.h>
#include <iostream>

class cross_join_reducer
{
private:
#define W_CROSSJOIN "NonEmptyCrossJoin("
#define W_HIERARCHIZE "Hierarchize("
#define W_ADDCALCULATEDMEMBERS "AddCalculatedMembers("
#define W_DRILLDOWNLEVEL "{DrilldownLevel("
#define W_DRILLDOWNLEVELDOUBLE "{{DrilldownLevel("
#define W_DRILLDOWNMEMBER "DrilldownMember("
#define W_DRILLDOWNMEMBERDOUBLE "{{DrilldownMember("
#define S_OPENBRACE "{"

	enum state_type {
		ST_IDLE,
		ST_LEFT_SET,
		ST_RIGHT_SET,
		ST_DRILL_L,		
		ST_DRILL_PARAM2,
		ST_DRILL_PARAM3,
		ST_SET,
		ST_DRILL_M,
		ST_DRILL_M_PARAM2,
		ST_DRILL_M_PARAM3,
		ST_MEMBER,
		ST_SKIP_FCT_END,
		ST_SKIP_BRACE_END,
		ST_END
	};

	typedef std::vector< std::string > set_type;
	typedef std::vector< std::string > atoms_type;

	std::vector< atoms_type > m_total_atoms;

	struct drill_struct {
		set_type set;
		set_type set_for_member;
		std::unique_ptr< drill_struct > drill;
		std::string level;
		drill_struct* parent;
		atoms_type atoms;

		drill_struct()
			: drill( nullptr )
			, level("")
			, parent( nullptr )
		{

		}

		drill_struct( drill_struct* parent )
			: drill( nullptr )
			, level("")
			, parent( parent )
		{
		}

		void compute()
		{
			if ( nullptr != drill ) { drill->compute(); }
			if ( set_for_member.empty() ) 
			{
				//DrillDownLevel;
				if ( !set.empty() ){ atoms = set; }
				else { atoms = drill->atoms; }
				if ( level.empty() ) {
					atoms.push_back( atoms[0] + ".FirstChild.Level.AllMembers" );
				} else {
					atoms.push_back( level + ".Members.Item(0).FirstChild.Level.AllMembers" );
				}
			} else {
				//DrillDownMember;
				if ( !set.empty() ){ atoms = set; }
				else { atoms = drill->atoms; }
				for ( set_type::const_iterator i = set_for_member.begin(), e = set_for_member.end(); i != e; ++i )
				{
					atoms.push_back( *i + ".Children" );
				}
			}
		}

		void print ( std::string indent )
		{
			std::cout << indent;
			if ( set_for_member.empty() ) { std::cout << "L:"; }
			else  { std::cout << "M:"; }
			if ( nullptr != drill )
			{
				std::cout << std::endl;
				drill->print( indent + "\t" );
				std::cout << indent;
			} else 
			{
				std::cout << "{";
				for ( set_type::const_iterator i = set.begin(), e = set.end(); i != e; ++i )
				{
					std::cout<< *i << " ";
				}
				std::cout << "}\t";
			}
			if ( !set_for_member.empty() )
			{
				std::cout << "{";
				for ( set_type::const_iterator i = set_for_member.begin(), e = set_for_member.end(); i != e; ++i )
				{
					std::cout<< *i << " ";
				}
				std::cout << "}\t";
			} else if ( !level.empty() )
			{
				std::cout << level << "\t";
			}
			std::cout << std::endl;
		}
	};

	state_type m_state;
	std::string m_input;
	size_t m_offset;
	size_t m_start;


	bool m_left_hierarchize;
	bool m_right_hierarchize;

	bool m_left_add_calc;
	bool m_right_add_calc;

	bool m_success;

	drill_struct m_left_drill;
	drill_struct m_right_drill;

	drill_struct* m_crt_drill;
	set_type* m_crt_set;
	std::string* m_crt_member;
	
	std::stack<state_type> m_next_state;

	size_t m_limit;
	std::unique_ptr< cross_join_reducer > m_left_reducer;
	std::unique_ptr< cross_join_reducer > m_right_reducer;

	std::vector< std::vector< std::string > > m_prepared_atoms;
public:
	cross_join_reducer( std::string& input )
		: m_state( ST_IDLE )
		, m_input( input )
		, m_offset( 0 )

		, m_left_hierarchize( false )
		, m_right_hierarchize( false )
		, m_left_add_calc( false )
		, m_right_add_calc( false )

		, m_success( false )
		, m_limit( input.size() )

		, m_left_reducer( nullptr )
		, m_right_reducer( nullptr )
	{
	}

	void compute()
	{
		while ( advance() ) { }
	}

	bool success() { return m_success; }

	void print(const std::string indent = std::string("\t"))
	{
		if ( nullptr != m_left_reducer ) { m_left_reducer->print( indent ); }
		else {
			m_left_drill.print( indent );
		}
		if ( nullptr != m_right_reducer ) { m_right_reducer->print( indent ); }
		else {
			m_right_drill.print( indent );
		}
	}

	std::string create_canonical_query()
	{
		get_atoms();
		create_canonical_sets();

		std::string query( "" );
		bool add_calc = true;
		const std::string cross_join( "NonEmptyCrossJoin" );
		const std::string union_string("Union");
		for ( std::vector< std::vector< std::string > >::const_reverse_iterator i = m_prepared_atoms.rbegin(), e = m_prepared_atoms.rend(); i != e; ++i )
		{
			std::string crt;
			for ( std::vector< std::string >::const_reverse_iterator ii = i->rbegin(), ee = i->rend(); ii != ee; ++ii )
			{
				if ( crt.empty() ){
					crt += std::string( add_calc ? "AddCalculatedMembers(" : "" ) + "{" + *ii + "}" + ( add_calc ? ")" : "" );
				} else {
					crt = cross_join + "(" + ( add_calc ? "AddCalculatedMembers(" : "" ) + "{" + *ii + "}" + ( add_calc ? ")" : "" ) + ","  + crt + ")";
				}
			}

			if ( query.empty() )
			{
				query = crt;
			} else 
			{
				query = union_string + "(" + crt + "," + query + ")";
			}
		}
		query = "Hierarchize(" + query + ") ";
		
		m_input = m_input.replace( m_start, m_offset-m_start + 1, query );

		return m_input;
	}

	void print_atoms()
	{
		std::cout << std::endl;
		for ( std::vector< atoms_type >::const_iterator i = m_total_atoms.begin(), e = m_total_atoms.end(); i != e; ++i  )
		{
			for ( atoms_type::const_iterator ii = i->begin(), ee = i->end(); ii != ee; ++ii )
			{
				std::cout << *ii << " ";
			}
			std::cout << std::endl;
		}
	}

private:
	
	void get_atoms()
	{
		if ( nullptr != m_left_reducer ) 
		{ 
			m_left_reducer->get_atoms(  ); 
			m_total_atoms.insert( m_total_atoms.end(), m_left_reducer->m_total_atoms.begin(), m_left_reducer->m_total_atoms.end() );
		}
		else {
			m_left_drill.compute();
			m_total_atoms.push_back( m_left_drill.atoms );
		}
		if ( nullptr != m_right_reducer ) { 
			m_right_reducer->get_atoms(  ); 
			m_total_atoms.insert( m_total_atoms.end(), m_right_reducer->m_total_atoms.begin(), m_right_reducer->m_total_atoms.end() );
		}
		else {
			m_right_drill.compute();
			m_total_atoms.push_back( m_right_drill.atoms );
		}
	}

	void create_canonical_sets()
	{
		typedef std::vector< atoms_type::const_iterator > pointer_type;
		pointer_type begins;
		pointer_type ends;
		pointer_type crts;

		m_prepared_atoms.clear();

		for ( std::vector< atoms_type >::const_iterator i = m_total_atoms.begin(), e = m_total_atoms.end(); i != e; ++i  )
		{
			begins.push_back( i->begin() );
			crts.push_back( i->begin() );
			ends.push_back( i->end() );
		}

		while ( true )
		{
			std::vector< std::string > crt;
			for ( pointer_type::const_iterator i = crts.begin(), e = crts.end(); i != e; ++i )
			{
				crt.push_back( **i );
			}
			m_prepared_atoms.push_back( crt );

			pointer_type::reverse_iterator i = crts.rbegin();
			pointer_type::reverse_iterator b = begins.rbegin();
			pointer_type::reverse_iterator e = ends.rbegin();
			while( true )
			{
				if ( i == crts.rend() ) { return; }
				++(*i);
				if ( (*e) == (*i) ) { (*i) = (*b); } 
				else { break; }
				++i;
				++b;
				++e;

			}
		}
	}

	bool advance()
	{
		std::string foo = m_input.substr( m_offset, 10000 );
		size_t pos;
		switch ( m_state )
		{
		case ST_IDLE:
			pos = m_input.find( W_CROSSJOIN, m_offset );
			if ( std::string::npos == pos ) {
				m_state = ST_END;
				return false;
			}
			m_start = pos;
			m_offset += pos +strlen( W_CROSSJOIN );
			m_state = ST_LEFT_SET;
			m_next_state.push( ST_RIGHT_SET );
			return true;
		case ST_LEFT_SET:
			pos = m_input.find( W_CROSSJOIN, m_offset );
			if ( m_offset == pos ) {
				m_left_reducer.reset( new cross_join_reducer( m_input.substr( m_offset, m_input.length() ) ) );
				m_left_reducer->compute();
				m_offset += m_left_reducer->m_offset;
				assert( !m_next_state.empty() );
				m_state = m_next_state.top();
				m_next_state.pop();
				return m_left_reducer->success();
			}

			pos = m_input.find( W_HIERARCHIZE, m_offset );
			if ( m_offset == pos ) {
				m_left_hierarchize = true;
				m_offset += strlen( W_HIERARCHIZE );
				m_next_state.push( ST_SKIP_FCT_END );
			}

			pos = m_input.find( W_ADDCALCULATEDMEMBERS, m_offset );
			if ( m_offset == pos ) {
				m_left_add_calc = true;
				m_offset += strlen( W_ADDCALCULATEDMEMBERS );
				m_next_state.push( ST_SKIP_FCT_END );
			}

			pos = m_input.find( W_DRILLDOWNLEVEL, m_offset );
			if ( m_offset == pos ) {
				m_offset += strlen( W_DRILLDOWNLEVEL );
				m_state = ST_DRILL_L;
				m_crt_drill = &m_left_drill;
				return true;
			}

			pos = m_input.find( W_DRILLDOWNMEMBER, m_offset );
			if ( m_offset == pos ) {
				m_offset += strlen( W_DRILLDOWNMEMBER );
				m_state = ST_DRILL_M;
				m_crt_drill = &m_left_drill;
				return true;
			}
			return false;
		case ST_DRILL_L:
			pos = m_input.find( W_DRILLDOWNLEVEL, m_offset );
			if ( m_offset == pos ) {
				m_offset += strlen( W_DRILLDOWNLEVEL );
				m_state = ST_DRILL_L;
				m_next_state.push( ST_DRILL_PARAM2 );
				m_crt_drill->drill.reset( new drill_struct( m_crt_drill ) );
				m_crt_drill = m_crt_drill->drill.get();
				return true;
			}
			pos = m_input.find( W_DRILLDOWNMEMBER, m_offset );
			if ( m_offset == pos ) {
				m_offset += strlen( W_DRILLDOWNMEMBER );
				m_state = ST_DRILL_M;
				m_next_state.push( ST_DRILL_PARAM2 );
				m_crt_drill->drill.reset( new drill_struct( m_crt_drill ) );
				m_crt_drill = m_crt_drill->drill.get();
				return true;
			}
			pos = m_input.find( S_OPENBRACE, m_offset );
			{
				m_offset += strlen( S_OPENBRACE );
				m_state = ST_SET;
				m_next_state.push( ST_DRILL_PARAM2 );
				m_crt_set = &m_crt_drill->set;
				return true;
			}
			return false;
		case ST_SET:
			switch ( m_input[m_offset] )
			{
			case '}':
				++m_offset;
				assert( !m_next_state.empty() );
				m_state = m_next_state.top();
				m_next_state.pop();
				return !m_crt_set->empty();
			case ',':
				++m_offset;
				return true;
			case '[':
				m_state = ST_MEMBER;
				m_next_state.push( ST_SET );
				return true;
			}
			return false;
		case ST_MEMBER:			
			{
				size_t bracket = 1;
				size_t m_max_offset = m_input.size();
				size_t start = m_offset;
				while ( m_offset != m_max_offset )
				{
					++m_offset;
					switch ( m_input[ m_offset ] )
					{
					case '[':
						if ( 0 != bracket ) { return false; }
						++bracket;
						break;
					case ']':
						if ( 1 != bracket ) { return false; }
						--bracket;
						break;
					case ',':
					case '}':
					case ')':
						if ( 0 == bracket )
						{
							assert( !m_next_state.empty() );
							m_state = m_next_state.top();
							m_next_state.pop();
							switch ( m_state )
							{
							case ST_SET:
								m_crt_set->push_back(m_input.substr( start, m_offset - start ) );
								break;
							default:
								*m_crt_member = m_input.substr( start, m_offset - start );
							}
							return true;
						}
						break;
					}		
				}
			}
			return false;
		case ST_DRILL_PARAM2:
			switch ( m_input[ m_offset ] )
			{
			case ')':
				++m_offset;
				if ( '}' != m_input[ m_offset ] ) return false;
				++m_offset;
				assert( !m_next_state.empty() );
				m_state = m_next_state.top();
				m_next_state.pop();
				m_crt_drill = m_crt_drill->parent;
				return true;
			case ',':
				++m_offset;
				m_state = ST_MEMBER;
				m_next_state.push( ST_DRILL_PARAM3 );
				m_crt_member = &m_crt_drill->level;
				return '[' == m_input[ m_offset ];
			}
			return false;
		case ST_DRILL_M_PARAM2:
			switch ( m_input[ m_offset ] )
			{
			case ')':
				++m_offset;
				if ( '}' != m_input[ m_offset ] ) return false;
				++m_offset;
				assert( !m_next_state.empty() );
				m_state = m_next_state.top();
				m_next_state.pop();
				m_crt_drill = m_crt_drill->parent;
				return true;
			case ',':
				++m_offset;
				while ( m_offset < m_limit && ' ' == m_input[ m_offset ] ) { ++m_offset; }
				if ( '{' != m_input[ m_offset ] ) return false;
				++m_offset;
				m_state = ST_SET;
				m_next_state.push( ST_DRILL_M_PARAM3 );
				m_crt_set = &m_crt_drill->set_for_member;

				return '[' == m_input[ m_offset ];
			}
			return false;
		case ST_DRILL_PARAM3:
			if ( ')' != m_input[ m_offset ] ) return false;
			++m_offset;
			if ( '}' != m_input[ m_offset ] ) return false;
			++m_offset;
			assert( !m_next_state.empty() );
			m_state = m_next_state.top();
			m_next_state.pop();
			m_crt_drill = m_crt_drill->parent;
			return true;
		case ST_DRILL_M_PARAM3:
			if ( ')' != m_input[ m_offset ] ) return false;
			++m_offset;
			assert( !m_next_state.empty() );
			m_state = m_next_state.top();
			m_next_state.pop();
			m_crt_drill = m_crt_drill->parent;
			return true;
		case ST_SKIP_FCT_END:
			if ( ')' != m_input[ m_offset ] ) return false;
			++m_offset;
			assert( !m_next_state.empty() );
			m_state = m_next_state.top();
			m_next_state.pop();
			return true;
		case ST_SKIP_BRACE_END:
			if ( '}' != m_input[ m_offset ] ) return false;
			++m_offset;
			assert( !m_next_state.empty() );
			m_state = m_next_state.top();
			m_next_state.pop();
			return true;
		case ST_RIGHT_SET:
			if ( ',' != m_input[ m_offset ] ) return false;
			++m_offset;
			pos = m_input.find( W_CROSSJOIN, m_offset );
			if ( m_offset == pos ) {
				m_right_reducer.reset( new cross_join_reducer( m_input.substr( m_offset, m_input.length() ) ) );
				m_right_reducer->compute();
				m_offset += m_right_reducer->m_offset;
				m_state = ST_END;
				return m_left_reducer->success();
			}

			m_next_state.push( ST_END );

			pos = m_input.find( W_HIERARCHIZE, m_offset );
			if ( m_offset == pos ) {
				m_right_hierarchize = true;
				m_offset += strlen( W_HIERARCHIZE );
				m_next_state.push( ST_SKIP_FCT_END );
			}

			pos = m_input.find( W_ADDCALCULATEDMEMBERS, m_offset );
			if ( m_offset == pos ) {
				m_right_add_calc = true;
				m_offset += strlen( W_ADDCALCULATEDMEMBERS );
				m_next_state.push( ST_SKIP_FCT_END );
			}

			pos = m_input.find( W_DRILLDOWNLEVEL, m_offset );
			if ( m_offset == pos ) {
				m_offset += strlen( W_DRILLDOWNLEVEL );
				m_state = ST_DRILL_L;
				m_crt_drill = &m_right_drill;
				return true;
			}

			pos = m_input.find( W_DRILLDOWNMEMBER, m_offset );
			if ( m_offset == pos ) {
				m_offset += strlen( W_DRILLDOWNMEMBER );
				m_state = ST_DRILL_M;
				m_crt_drill = &m_right_drill;
				return true;
			}
			return false;
		case ST_DRILL_M:
			pos = m_input.find( W_DRILLDOWNLEVELDOUBLE, m_offset );
			if ( m_offset == pos ) {
				m_offset += strlen( W_DRILLDOWNLEVELDOUBLE );
				m_state = ST_DRILL_L;
				m_next_state.push( ST_DRILL_M_PARAM2 );
				m_next_state.push( ST_SKIP_BRACE_END );
				m_crt_drill->drill.reset( new drill_struct( m_crt_drill ) );
				m_crt_drill = m_crt_drill->drill.get();
				return true;
			}			
			pos = m_input.find( W_DRILLDOWNLEVEL, m_offset );
			if ( m_offset == pos ) {
				m_offset += strlen( W_DRILLDOWNLEVEL );
				m_state = ST_DRILL_L;
				m_next_state.push( ST_DRILL_M_PARAM2 );
				m_crt_drill->drill.reset( new drill_struct( m_crt_drill ) );
				m_crt_drill = m_crt_drill->drill.get();
				return true;
			}	
			pos = m_input.find( W_DRILLDOWNMEMBERDOUBLE, m_offset );
			if ( m_offset == pos ) {
				m_offset += strlen( W_DRILLDOWNMEMBERDOUBLE );
				m_state = ST_DRILL_M;
				m_next_state.push( ST_DRILL_M_PARAM2 );
				m_next_state.push( ST_SKIP_BRACE_END );
				m_next_state.push( ST_SKIP_BRACE_END );
				m_crt_drill->drill.reset( new drill_struct( m_crt_drill ) );
				m_crt_drill = m_crt_drill->drill.get();
				return true;
			}	
			pos = m_input.find( S_OPENBRACE, m_offset );
			{
				m_offset += strlen( S_OPENBRACE );
				m_state = ST_SET;
				m_next_state.push( ST_DRILL_M_PARAM2 );
				m_crt_set = &m_crt_drill->set;
				return true;
			}
			return false;
		case ST_END:
			assert( ')' ==  m_input[ m_offset ] );
			m_success = true;
			++m_offset;
			return false;
		}
		return false;
	}
};