#include "Serializer.h"
#include <fstream>
#include <stdexcept>
#include <cstdint>
#include <unordered_map>

void BinarySerializer::serializeToFile ( const ParseResult& parsed, const std::string& filename ) {
    std::ofstream out ( filename, std::ios::binary );
    if ( !out ) {
        if ( logger_ ) logger_->error ( "Cannot open file for writing: " + filename );
        throw std::runtime_error ( "Cannot open file for writing: " + filename );
    }

    const uint32_t n = static_cast< uint32_t >( parsed.nodes.size () );
    out.write ( reinterpret_cast< const char* >( &n ), sizeof ( n ) );

    // Создаём map указатель -> индекс
    std::unordered_map<const ListNode*, uint32_t> idx;
    idx.reserve ( n );
    for ( uint32_t i = 0; i < n; ++i ) idx[parsed.nodes[i].get ()] = i;

    for ( uint32_t i = 0; i < n; ++i ) {
        const std::string& s = parsed.nodes[i]->data;
        uint32_t len = static_cast< uint32_t > ( s.size () );
        if ( len > 1000u ) {
            if ( logger_ ) logger_->warn ( "Data length > 1000 at node " + std::to_string ( i ) + ", truncating" );
            len = 1000u;
        }
        out.write ( reinterpret_cast< const char* > ( &len ), sizeof ( len ) );
        if ( len ) out.write ( s.data (), len );

        int32_t r = -1;
        if ( parsed.nodes[i]->rand != nullptr ) {
            auto it = idx.find ( parsed.nodes[i]->rand );
            if ( it == idx.end () ) {
                throw std::runtime_error ( "rand pointer points outside of parsed nodes" );
            }
            r = static_cast< int32_t >( it->second );
        }
        out.write ( reinterpret_cast< const char* >( &r ), sizeof ( r ) );
    }

    if ( !out ) {
        if ( logger_ ) logger_->error ( "Failed during writing file: " + filename );
        throw std::runtime_error ( "Failed during writing file: " + filename );
    }
    if ( logger_ ) logger_->info ( "Serialized " + std::to_string ( n ) + " nodes to " + filename );
}

ParseResult BinarySerializer::deserializeFromFile ( const std::string& filename ) {
    std::ifstream in ( filename, std::ios::binary );
    if ( !in ) {
        if ( logger_ ) logger_->error ( "Cannot open file for reading: " + filename );
        throw std::runtime_error ( "Cannot open file for reading: " + filename );
    }

    uint32_t n = 0;
    in.read ( reinterpret_cast< char* >( &n ), sizeof ( n ) );
    if ( !in ) throw std::runtime_error ( "Failed to read node count from file" );

    ParseResult res;
    res.nodes.reserve ( n );
    res.rand_indices.reserve ( n );

    for ( uint32_t i = 0; i < n; ++i ) {
        uint32_t len = 0;
        in.read ( reinterpret_cast< char* > ( &len ), sizeof ( len ) );
        if ( !in ) throw std::runtime_error ( "Failed to read data length" );

        if ( len > 1000u ) {
            if ( logger_ ) logger_->warn ( "Data length in file > 1000 for node " + std::to_string ( i ) + ", will be read but truncated later" );
        }

        std::string s;
        s.resize ( len );
        if ( len ) in.read ( &s[0], len );
        if ( !in ) throw std::runtime_error ( "Failed to read data bytes" );

        int32_t r = -1;
        in.read ( reinterpret_cast< char* >( &r ), sizeof ( r ) );
        if ( !in ) throw std::runtime_error ( "Failed to read rand index" );

        auto node = std::make_unique<ListNode> ();
        node->data = std::move ( s );
        res.nodes.push_back ( std::move ( node ) );
        res.rand_indices.push_back ( r );
    }

    // Восстанавливаем next/prev/rand
    const int32_t ni = static_cast< int32_t >( res.nodes.size () );
    for ( int32_t i = 0; i < ni; ++i ) {
        if ( i + 1 < ni ) res.nodes[i]->next = res.nodes[i + 1].get ();
        if ( i - 1 >= 0 ) res.nodes[i]->prev = res.nodes[i - 1].get ();
        int32_t r = res.rand_indices[i];
        res.nodes[i]->rand = ( r == -1 ? nullptr : res.nodes[r].get () );
    }

    if ( logger_ ) logger_->info ( "Deserialized " + std::to_string ( ni ) + " nodes from " + filename );
    return res;
}