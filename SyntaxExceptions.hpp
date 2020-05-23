#include <string>
#include <vector>
/*======================ID Based Exception - Def, Undef, UndefFunc======================*/
class idException : public std::exception {
    string id;

    public:
    errorDef (string& name) : id(name) {}
    string& getID() {
        return id;
    }
}

class errorUndefException : idException {
    public:
    errorUndefException(string& s) {
        idException(s);
    }
}

class errorDefException : idException {
    public:
    errorDefException(string& s) {
        idException(s);
    }
}

class errorUndefFuncException : idException {
    public:
    errorUndefFuncException(string& s) {
        idException(s);
    }
}

/*======================Value Based Exception - ByteTooLarge======================*/

class errorByteTooLargeException : public std::exception {
    string v;

    public:
    valueException (string& value) : v(value) {}
    string& getValue() {
        return v;
    }
}

/*======================Vector Based Exception - PrototypeMismatchException======================*/
class errorPrototypeMismatchException : public std::exception {
    vector<string> argTypes;
    string id;

    public:
    argListException (vector<string>& argTypes, string& id) : argTypes(argTypes), id(id) {}
    vector<string>& getArgTypes() {
        return argTypes;
    }
    string& getID() {
        return id;
    }
}

/*======================YYLENENO Based Exceptions======================*/
class errorMismatchException :  public std::exception {}
class errorUnexpectedBreakException : public std::exception {}
class errorUnexpectedContinueException : public std::exception {}
class errorMainMissingException : public  std::exception {}

