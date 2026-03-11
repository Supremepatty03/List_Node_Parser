#include <iostream>
#include "Core.h"
#include "FileParser.h"
#include "Logger.h"
#include "Serializer.h"
#include <filesystem>

using namespace std;

int main ( int argc, char** argv ) {

    while ( true ) {
        try {

            // выбор входного файла: argv[1] только на первой итерации
            string infile = "inlet.in";
            if ( argc >= 2 ) {
                infile = argv[1];
                argc = 1; // чтобы на следующих итерациях снова спрашивать
            }
            else {
                cout << "Enter path to input file (press Enter for default 'inlet.in'): ";
                string input_path;
                getline ( cin, input_path );
                if ( !input_path.empty () ) infile = input_path;
            }

            // выбор выходного файла
            string outfile;
            if ( argc >= 3 ) {
                outfile = argv[2];
            }
            else {
                filesystem::path abs_in = filesystem::absolute ( infile );
                filesystem::path parent = abs_in.parent_path ();

                if ( parent.empty () ) {
                    outfile = ( filesystem::current_path () / "outlet.out" ).string ();
                }
                else {
                    outfile = ( parent / "outlet.out" ).string ();
                }
            }

            ConsoleLogger logger;
            FileListParser parser ( &logger );

            auto res = parser.parseFile ( infile );

            cout << "Parsed nodes: " << res.nodes.size () << "\n";
            for ( size_t i = 0; i < res.nodes.size () && i < 10; ++i ) {
                auto* n = &res.nodes[i];
                cout << i << ": '" << n->data << "' rand_index=" << res.rand_indices[i] << "\n";
            }

            BinarySerializer ser ( &logger );
            ser.serializeToFile ( res, outfile );

            auto res2 = ser.deserializeFromFile ( outfile );

            cout << "Deserialized nodes: " << res2.nodes.size () << "\n";
            for ( size_t i = 0; i < res2.nodes.size () && i < 10; ++i ) {
                auto* n = &res2.nodes[i];
                cout << i << ": '" << n->data << "' rand_index=" << res2.rand_indices[i] << "\n";
            }

            // проверка соответствия
            if ( res2.nodes.size () != res.nodes.size () ) {
                cerr << "Mismatch after round-trip: sizes differ ("
                    << res.nodes.size () << " != " << res2.nodes.size () << ")\n";
            }
            else {
                size_t check_n = min<size_t> ( res.nodes.size (), 100 );
                bool ok = true;

                for ( size_t i = 0; i < check_n; ++i ) {
                    if ( res.nodes[i].data != res2.nodes[i].data ||
                        res.rand_indices[i] != res2.rand_indices[i] ) {

                        cerr << "Mismatch at node " << i << "\n";
                        ok = false;
                        break;
                    }
                }

                if ( !ok ) {
                    cerr << "Round-trip content mismatch\n";
                }
                else {
                    cout << "Round-trip OK\n";
                }
            }

            cout << "Output file written to: " << outfile << "\n";
        }
        catch ( const exception& ex ) {
            cerr << "Error: " << ex.what () << "\n";
        }

        // предложение повторить
        cout << "\nRun again? (y/n): ";
        string answer;
        getline ( cin, answer );

        if ( answer != "y" && answer != "Y" ) {
            cout << "Program finished.\n";
            break;
        }

        cout << "\n-----------------------------\n\n";
    }

    return 0;
}