#pragma once
#include "Core.h"
#include "Logger.h"
#include <string>

class ISerializer {
public:
    virtual ~ISerializer () = default;

    // Сериализовать ParseResult в бинарный файл
    virtual void serializeToFile ( const ParseResult& parsed, const std::string& filename ) = 0;

    // Десериализовать и вернуть ParseResult
    virtual ParseResult deserializeFromFile ( const std::string& filename ) = 0;
};

class BinarySerializer : public ISerializer {
public:
    explicit BinarySerializer ( ILogger* logger = nullptr ) : logger_ ( logger ) {}

    void serializeToFile ( const ParseResult& parsed, const std::string& filename ) override;
    ParseResult deserializeFromFile ( const std::string& filename ) override;

private:
    ILogger* logger_ = nullptr; // не владеет
};
