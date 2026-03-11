#include "FileParser.h"
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <limits>
#include <charconv>

using namespace std;

ParseResult FileListParser::parseFile ( const string& filename ) {
    ifstream ifs_count ( filename );
    if ( !ifs_count ) {
        if ( logger_ ) logger_->error ( "Cannot open file: " + filename );
        throw runtime_error ( "Cannot open file: " + filename );
    }
    size_t count = 0;
    string tmp;
    while ( getline ( ifs_count, tmp ) ) {
        if ( !tmp.empty () ) ++count;
    }
    ifs_count.close ();

    ifstream ifs ( filename );
    if ( !ifs ) {
        if ( logger_ ) logger_->error ( "Cannot open file (2nd pass): " + filename );
        throw runtime_error ( "Cannot open file (2nd pass): " + filename );
    }

    ParseResult result;
    result.nodes.reserve ( count );
    result.rand_indices.reserve ( count );

    string line;
    size_t line_no = 0;
    while ( getline ( ifs, line ) ) {
        ++line_no;
        if ( line.empty () ) {
            if ( logger_ ) logger_->warn ( "Empty line at " + to_string ( line_no ) + " skipped" );
            continue;
        }

        auto pos = line.rfind ( ';' );
        if ( pos == string::npos ) {
            if ( logger_ ) logger_->error ( "Bad format at line " + to_string ( line_no ) + ": missing ';'" );
            throw runtime_error ( "Bad format at line " + to_string ( line_no ) + ": missing ';'" );
        }

        string data = line.substr ( 0, pos );
        string rand_token = line.substr ( pos + 1 );

        auto start_it = rand_token.find_first_not_of ( " \t\r\n" );
        auto end_it = rand_token.find_last_not_of ( " \t\r\n" );
        if ( start_it == string::npos ) rand_token = ""; else rand_token = rand_token.substr ( start_it, end_it - start_it + 1 );

        int32_t r = parseRandIndex ( rand_token );

        if ( data.size () > 1000 ) {
            if ( logger_ ) logger_->warn ( "Data too long at line " + to_string ( line_no ) + ", truncating to 1000 bytes" );
            data.resize ( 1000 );
        }

        ListNode node;
        node.data = std::move ( data );
        node.prev = nullptr;
        node.next = nullptr;
        node.rand = nullptr;
        result.nodes.push_back ( std::move ( node ) );
        result.rand_indices.push_back ( r );
    }

    const int32_t n = static_cast< int32_t >( result.nodes.size () );
    for ( size_t i = 0; i < result.rand_indices.size (); ++i ) {
        int32_t r = result.rand_indices[i];
        if ( r < -1 || r >= n ) {
            ostringstream oss;
            oss << "Invalid rand index at line " << ( i + 1 ) << ": " << r << " (nodes=" << n << ")";
            if ( logger_ ) logger_->error ( oss.str () );
            throw runtime_error ( oss.str () );
        }
    }

    for ( int32_t i = 0; i < n; ++i ) {
        if ( i + 1 < n ) result.nodes[i].next = &result.nodes[i + 1];
        if ( i - 1 >= 0 ) result.nodes[i].prev = &result.nodes[i - 1];
        int32_t r = result.rand_indices[i];
        result.nodes[i].rand = ( r == -1 ? nullptr : &result.nodes[r] );
    }

    if ( logger_ ) logger_->info ( "Parsed " + to_string ( n ) + " nodes" );
    return result;
}

int32_t FileListParser::parseRandIndex ( const string& token ) {
    long long value = 0;
    auto first = token.data ();
    auto last = token.data () + token.size ();
    from_chars_result res = from_chars ( first, last, value );
    if ( res.ec != errc () || res.ptr != last ) {
        throw runtime_error ( "Invalid rand index: '" + token + "'" );
    }
    if ( value < numeric_limits<int32_t>::min () || value > numeric_limits<int32_t>::max () ) {
        throw runtime_error ( "rand index out of int32 range: '" + token + "'" );
    }
    return static_cast< int32_t >( value );
}

ParseResult FileListParser::parse ( istream& in ) {
    ParseResult result;

    vector<string> tmp_data;
    vector<int32_t> tmp_rand;
    tmp_data.reserve ( 1024 );
    tmp_rand.reserve ( 1024 );

    string line;
    size_t line_no = 0;

    // чтение потока в временные векторы
    while ( getline ( in, line ) ) {
        ++line_no;
        if ( line.empty () ) {
            if ( logger_ ) logger_->warn ( "Empty line at " + to_string ( line_no ) + " skipped" );
            continue;
        }

        auto pos = line.rfind ( ';' );
        if ( pos == string::npos ) {
            if ( logger_ ) logger_->error ( "Bad format at line " + to_string ( line_no ) + ": missing ';'" );
            throw runtime_error ( "Bad format at line " + to_string ( line_no ) + ": missing ';'" );
        }

        string data = line.substr ( 0, pos );
        string rand_token = line.substr ( pos + 1 );

        auto start_it = rand_token.find_first_not_of ( " \t\r\n" );
        auto end_it = rand_token.find_last_not_of ( " \t\r\n" );
        if ( start_it == string::npos ) rand_token.clear ();
        else rand_token = rand_token.substr ( start_it, end_it - start_it + 1 );

        int32_t r = parseRandIndex ( rand_token );

        // проверка длины data
        if ( data.size () > 1000 ) {
            if ( logger_ ) logger_->warn ( "Data too long at line " + to_string ( line_no ) + ", truncating to 1000 bytes" );
            data.resize ( 1000 );
        }

        tmp_data.push_back ( std::move ( data ) );
        tmp_rand.push_back ( r );
    }

    // валидация rand индексов относительно количества считанных строк
    const int32_t n = static_cast< int32_t >( tmp_data.size () );
    for ( size_t i = 0; i < tmp_rand.size (); ++i ) {
        int32_t r = tmp_rand[i];
        if ( r < -1 || r >= n ) {
            ostringstream oss;
            oss << "Invalid rand index at line " << ( i + 1 ) << ": " << r << " (nodes=" << n << ")";
            if ( logger_ ) logger_->error ( oss.str () );
            throw runtime_error ( oss.str () );
        }
    }

    // перенос данных в result.nodes
    result.nodes.reserve ( n );
    for ( int32_t i = 0; i < n; ++i ) {
        ListNode node;
        node.prev = nullptr;
        node.next = nullptr;
        node.rand = nullptr;
        node.data = std::move ( tmp_data[i] );
        result.nodes.push_back ( std::move ( node ) );
    }

    result.rand_indices = std::move ( tmp_rand );

    for ( int32_t i = 0; i < n; ++i ) {
        if ( i + 1 < n ) result.nodes[i].next = &result.nodes[i + 1];
        if ( i - 1 >= 0 )  result.nodes[i].prev = &result.nodes[i - 1];
        int32_t r = result.rand_indices[i];
        result.nodes[i].rand = ( r == -1 ? nullptr : &result.nodes[r] );
    }

    if ( logger_ ) logger_->info ( "Parsed " + to_string ( n ) + " nodes" );
    return result;
}