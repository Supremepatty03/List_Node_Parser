#include "Serializer.h"
#include <fstream>
#include <stdexcept>
#include <cstdint>
#include <unordered_map>

using namespace std;

void BinarySerializer::serializeToFile ( const ParseResult& parsed, const string& filename ) {
    ofstream out ( filename, ios::binary );
    if ( !out ) {
        if ( logger_ ) logger_->error ( "Cannot open file for writing: " + filename );
        throw runtime_error ( "Cannot open file for writing: " + filename );
    }

    const uint32_t n = static_cast< uint32_t >( parsed.nodes.size () );
    out.write ( reinterpret_cast< const char* >( &n ), sizeof ( n ) );

    for ( uint32_t i = 0; i < n; ++i ) {
        const string& s = parsed.nodes[i].data;
        uint32_t len = static_cast< uint32_t > ( s.size () );
        if ( len > 1000u ) {
            if ( logger_ ) logger_->warn ( "Data length > 1000 at node " + to_string ( i ) + ", truncating" );
            len = 1000u;
        }
        out.write ( reinterpret_cast< const char* >( &len ), sizeof ( len ) );
        if ( len ) out.write ( s.data (), len );

        int32_t r = -1;
        if ( i < parsed.rand_indices.size () ) r = parsed.rand_indices[i];
        out.write ( reinterpret_cast< const char* > ( &r ), sizeof ( r ) );
    }

    if ( !out ) {
        if ( logger_ ) logger_->error ( "Failed during writing file: " + filename );
        throw runtime_error ( "Failed during writing file: " + filename );
    }
    if ( logger_ ) logger_->info ( "Serialized " + to_string ( n ) + " nodes to " + filename );
}
ParseResult BinarySerializer::deserializeFromFile ( const string& filename ) {
    ifstream in ( filename, ios::binary );
    if ( !in ) {
        if ( logger_ ) logger_->error ( "Cannot open file for reading: " + filename );
        throw runtime_error ( "Cannot open file for reading: " + filename );
    }

    uint32_t n = 0;
    in.read ( reinterpret_cast< char* >( &n ), sizeof ( n ) );
    if ( !in ) throw runtime_error ( "Failed to read node count from file" );

    ParseResult res;
    res.nodes.reserve ( n );
    res.rand_indices.reserve ( n );

    for ( uint32_t i = 0; i < n; ++i ) {
        uint32_t len = 0;
        in.read ( reinterpret_cast< char* > ( &len ), sizeof ( len ) );
        if ( !in ) throw runtime_error ( "Failed to read data length" );

        string s;
        s.resize ( len );
        if ( len ) in.read ( &s[0], len );
        if ( !in ) throw runtime_error ( "Failed to read data bytes" );

        int32_t r = -1;
        in.read ( reinterpret_cast< char* >( &r ), sizeof ( r ) );
        if ( !in ) throw runtime_error ( "Failed to read rand index" );

        ListNode node;
        node.data = move ( s );
        node.prev = nullptr;
        node.next = nullptr;
        node.rand = nullptr;
        res.nodes.push_back ( move ( node ) );
        res.rand_indices.push_back ( r );
    }
    const int32_t ni = static_cast< int32_t >( res.nodes.size () );
    for ( int32_t i = 0; i < ni; ++i ) {
        if ( i + 1 < ni ) res.nodes[i].next = &res.nodes[i + 1];
        if ( i - 1 >= 0 ) res.nodes[i].prev = &res.nodes[i - 1];
        int32_t r = res.rand_indices[i];
        res.nodes[i].rand = ( r == -1 ? nullptr : &res.nodes[r] );
    }

    if ( logger_ ) logger_->info ( "Deserialized " + to_string ( ni ) + " nodes from " + filename );
    return res;
}