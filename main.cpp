#include <iostream>
#include "Core.h"
#include "FileParser.h"
#include "Logger.h"
#include "Serializer.h"
#include <filesystem>

int main ( int argc, char** argv ) {
    try {
        // 1) Сначала выбираем infile (аргументы / ввод / значение по умолчанию)
        std::string infile = "inlet.in";
        std::string outfile_arg; // если задан явно как argv[2]

        if ( argc >= 2 ) {
            infile = argv[1];
        }
        else {
            std::cout << "Enter path to input file (press Enter for default 'inlet.in'): ";
            std::string input_path;
            std::getline ( std::cin, input_path );
            if ( !input_path.empty () ) infile = input_path;
        }

        if ( argc >= 3 ) {
            outfile_arg = argv[2];
        }

        // 2) Теперь строим путь output'а: если пользователь явно указал argv[2] — используем его,
        //    иначе кладём outlet.out в ту же папку, что и входной файл.
        std::filesystem::path out_path;
        if ( !outfile_arg.empty () ) {
            out_path = std::filesystem::path ( outfile_arg );
        }
        else {
            // приводим infile к абсолютному пути, чтобы корректно взять parent_path()
            std::filesystem::path abs_in = std::filesystem::absolute ( infile );
            std::filesystem::path parent = abs_in.parent_path ();
            if ( parent.empty () ) {
                // на всякий случай — если parent пуст (маловероятно после absolute),
                // положим в текущую рабочую директорию
                out_path = std::filesystem::current_path () / "outlet.out";
            }
            else {
                out_path = parent / "outlet.out";
            }
        }

        // Приводим к строке при вызове сериализации
        const std::string outfile = out_path.string ();

        ConsoleLogger logger;
        FileListParser parser ( &logger );

        auto res = parser.parseFile ( infile );

        std::cout << "Parsed nodes: " << res.nodes.size () << " ";
        for ( size_t i = 0; i < res.nodes.size () && i < 10; ++i ) {
            auto* n = res.nodes[i].get ();
            std::cout << i << ": '" << n->data << "' rand_index=" << res.rand_indices[i] << " ";
        }

        BinarySerializer ser ( &logger );
        ser.serializeToFile ( res, outfile );

        // Для проверки: десериализуем обратно и сравним количество
        auto res2 = ser.deserializeFromFile ( outfile );
        std::cout << "Deserialized nodes: " << res2.nodes.size () << " ";
        if ( res2.nodes.size () != res.nodes.size () ) {
            std::cerr << "Mismatch after round-trip!" << std::endl;
            return 3;
        }

        std::cout << "Round-trip OK" << std::endl;
        std::cout << "Output file written to: " << outfile << std::endl;
        return 0;
    }
    catch ( const std::exception& ex ) {
        std::cerr << "Fatal error: " << ex.what () << " ";
        return 2;
    }
}