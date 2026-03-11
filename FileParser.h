#pragma once
#include "Core.h"
#include "Logger.h"
#include <string>

using namespace std;

class FileListParser : public IListParser {
public:
    explicit FileListParser ( ILogger* logger = nullptr ) : logger_ ( logger ) {}

    ParseResult parse ( istream& in ) override;
    ParseResult parseFile ( const string& filename ) override;

private:
    ILogger* logger_ = nullptr;

    static int32_t parseRandIndex ( const string& token );
};