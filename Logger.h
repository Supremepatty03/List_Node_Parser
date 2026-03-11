#pragma once

#include <string>
#include <iostream>

using namespace std;

class ILogger {
public:
    virtual ~ILogger () = default;
    virtual void info ( const string& msg ) = 0;
    virtual void warn ( const string& msg ) = 0;
    virtual void error ( const string& msg ) = 0;
};

class ConsoleLogger : public ILogger {
public:
    void info ( const string& msg ) override { cerr << "[INFO] " << msg << "\n"; }
    void warn ( const string& msg ) override { cerr << "[WARN] " << msg << "\n"; }
    void error ( const string& msg ) override { cerr << "[ERROR] " << msg << "\n"; }
};