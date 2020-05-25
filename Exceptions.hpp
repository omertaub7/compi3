#ifndef _SEMANTIC_EXCEPTIONS
#define _SEMANTIC_EXCEPTIONS

#include <string>
#include <vector>
using std::vector;
using std::string;

//======================ID Based Exception - Def, Undef, UndefFunc======================
class idException : public std::exception {
    string id;

 public:
    idException (const string& name) : id(name) {}
    string getID() const {
        return id;
    }
};

class errorUndefException : public idException {
public:
    errorUndefException(const string& s) : idException(s) { }
};

class errorDefException : public idException {
public:
    errorDefException(const string& s) : idException(s){ }
};

class errorUndefFuncException : public idException {
public:
    errorUndefFuncException(const string& s) : idException(s) { }
};

//======================Value Based Exception - ByteTooLarge======================

class errorByteTooLargeException : public std::exception {
    string v;

public:
    errorByteTooLargeException (const string& value) : v(value) { }
    string getValue() const {
        return v;
    }
};

//======================Vector Based Exception - PrototypeMismatchException======================
class errorPrototypeMismatchException : public std::exception {
    vector<string> argTypes;
    string id;

public:
    errorPrototypeMismatchException (const vector<string>& argTypes, const string& id) : argTypes(argTypes), id(id) {}
    vector<string> getArgTypes() const {
        return argTypes;
    }
    string getID() const {
        return id;
    }
};

//======================YYLENENO Based Exceptions======================
class errorMismatchException :  public std::exception {};
class errorUnexpectedBreakException : public std::exception {};
class errorUnexpectedContinueException : public std::exception {};
class errorMainMissingException : public  std::exception {};
class lexException : public std::exception {};
class synException : public std::exception {};

#endif
