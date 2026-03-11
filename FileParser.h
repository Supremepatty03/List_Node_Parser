#pragma once
#include "Core.h"
#include "Logger.h"
#include <string>

class FileListParser : public IListParser {
public:
    explicit FileListParser ( ILogger* logger = nullptr ) : logger_ ( logger ) {}

    ParseResult parse ( std::istream& in ) override;
    ParseResult parseFile ( const std::string& filename ) override;

private:
    ILogger* logger_ = nullptr;

    static int32_t parseRandIndex ( const std::string& token );
};