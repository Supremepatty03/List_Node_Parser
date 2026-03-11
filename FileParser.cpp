#include "FileParser.h"
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <limits>
#include <charconv>


ParseResult FileListParser::parseFile ( const std::string& filename ) {
    std::ifstream ifs ( filename );
    if ( !ifs ) {
        if ( logger_ ) logger_->error ( "Cannot open file: " + filename );
        throw std::runtime_error ( "Cannot open file: " + filename );
    }
    return parse ( ifs );
}

int32_t FileListParser::parseRandIndex ( const std::string& token ) {
    // token должен представлять целое число (возможно со знаком)
    // используем std::from_chars для быстроты и безопасности
    long long value = 0;
    auto first = token.data ();
    auto last = token.data () + token.size ();
    std::from_chars_result res = std::from_chars ( first, last, value );
    if ( res.ec != std::errc () || res.ptr != last ) {
        throw std::runtime_error ( "Invalid rand index: '" + token + "'" );
    }
    if ( value < std::numeric_limits<int32_t>::min () || value > std::numeric_limits<int32_t>::max () ) {
        throw std::runtime_error ( "rand index out of int32 range: '" + token + "'" );
    }
    return static_cast< int32_t >( value );
}

ParseResult FileListParser::parse ( std::istream& in ) {
    ParseResult result;

    std::string line;
    size_t line_no = 0;
    // Предварительно не знаем кол-во строк; будем добавлять по мере чтения

    while ( std::getline ( in, line ) ) {
        ++line_no;
        // Пропускаем пустые строки (можно настроить иначе)
        if ( line.empty () ) {
            if ( logger_ ) logger_->warn ( "Empty line at " + std::to_string ( line_no ) + " skipped" );
            continue;
        }
        auto pos = line.rfind ( ';' );
        if ( pos == std::string::npos ) {
            if ( logger_ ) logger_->error ( "Bad format at line " + std::to_string ( line_no ) + ": missing ';'" );
            throw std::runtime_error ( "Bad format at line " + std::to_string ( line_no ) + ": missing ';'" );
        }

        std::string data = line.substr ( 0, pos );
        std::string rand_token = line.substr ( pos + 1 );

        // trim rand_token spaces
        auto start_it = rand_token.find_first_not_of ( " 	");
        auto end_it = rand_token.find_last_not_of ( " 	");
        if ( start_it == std::string::npos ) rand_token = "";
        else rand_token = rand_token.substr ( start_it, end_it - start_it + 1 );

        int32_t r = parseRandIndex ( rand_token );

        // Проверка длины data (по условию до 1000 символов)
        if ( data.size () > 1000 ) {
            if ( logger_ ) logger_->warn ( "Data too long at line " + std::to_string ( line_no ) + ", truncating to 1000 bytes" );
            data.resize ( 1000 );
        }
        // Создаём узел и сохраняем в вектор для владения
        auto node = std::make_unique<ListNode> ();
        node->data = std::move ( data );
        // prev/next/rand пока nullptr — свяжем позже

        result.nodes.push_back ( std::move ( node ) );
        result.rand_indices.push_back ( r );
    }

    // После чтения всех строк — проверяем корректность rand индексов
    const int32_t n = static_cast< int32_t >( result.nodes.size () );
    for ( size_t i = 0; i < result.rand_indices.size (); ++i ) {
        int32_t r = result.rand_indices[i];
        if ( r < -1 || r >= n ) {
            std::ostringstream oss;
            oss << "Invalid rand index at line " << ( i + 1 ) << ": " << r << " (nodes=" << n << ")";
            if ( logger_ ) logger_->error ( oss.str () );
            throw std::runtime_error ( oss.str () );
        }
    }
    // Линкуем next/prev и rand
    for ( int32_t i = 0; i < n; ++i ) {
        if ( i + 1 < n ) result.nodes[i]->next = result.nodes[i + 1].get ();
        if ( i - 1 >= 0 ) result.nodes[i]->prev = result.nodes[i - 1].get ();
        int32_t r = result.rand_indices[i];
        result.nodes[i]->rand = ( r == -1 ? nullptr : result.nodes[r].get () );
    }

    if ( logger_ ) logger_->info ( "Parsed " + std::to_string ( n ) + " nodes" );

    return result;
}