#pragma once
#include "Core.h"
#include "Logger.h"
#include <string>

using namespace std;

class ISerializer {
public:
    virtual ~ISerializer () = default;

    virtual void serializeToFile ( const ParseResult& parsed, const string& filename ) = 0;

    virtual ParseResult deserializeFromFile ( const string& filename ) = 0;
};

class BinarySerializer : public ISerializer {
public:
    explicit BinarySerializer ( ILogger* logger = nullptr ) : logger_ ( logger ) {}

    void serializeToFile ( const ParseResult& parsed, const string& filename ) override;
    ParseResult deserializeFromFile ( const string& filename ) override;

private:
    ILogger* logger_ = nullptr;
};
