#pragma once

#include <string>
#include <iostream>

class ILogger {
public:
    virtual ~ILogger () = default;
    virtual void info ( const std::string& msg ) = 0;
    virtual void warn ( const std::string& msg ) = 0;
    virtual void error ( const std::string& msg ) = 0;
};

class ConsoleLogger : public ILogger {
public:
    void info ( const std::string& msg ) override { std::cerr << "[INFO] " << msg << "\n"; }
    void warn ( const std::string& msg ) override { std::cerr << "[WARN] " << msg << "\n"; }
    void error ( const std::string& msg ) override { std::cerr << "[ERROR] " << msg << "\n"; }
};