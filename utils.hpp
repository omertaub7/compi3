#ifndef _UTILS
// This is the source file to be included in both the parser and lex, and include all the other parts

// TODO: create the tree by assigning the kids
// TODO: create automatic casting rules
// TODO: verify all type checks
// TODO: do we need to check for double returns?
#include "Node.hpp"
#include "hw3_output.hpp"
#include "Exceptions.hpp"
#include "SymbolTable.hpp"
#include "codeGenerationUtils.hpp"

#include <iostream>
#include <typeinfo>
#include <cassert>
#include <algorithm>

using std::exception;
using std::cout;
using std::endl;

#define YYSTYPE Node*



//========================= Debug Utils==========================
template<typename T>
bool checkPtr(Node* p) {
	T* temp = dynamic_cast<T*>(p);
	if (temp) {
		return true;
	}
	return false;
}

//=========================== Utils =============================
// for variables
TypeN getIdType(Node*);
// check that a number is byte size
bool isByte(Node*);
// check that an expression is boolean
bool isBool(Node*);
// check that an expression is numeric
bool isNumeric(Node*);
// check if an identifier is a declared function
bool isFunc(Node*);
// vector with function's arg list types
// if it has no args then returns an empty vector
vector<std::pair<string,TypeN>> getFuncArgTypes(Node*);
// the type of the function
TypeN getFuncType(Node*);
// insert a new variable to the table
void insertNewVar(Node* pType, Node* pId);
// check if target type can be assigned with source type
bool checkAssign(TypeN target, TypeN source);
// get the type of the function that we are inside of now
TypeN getCurrFuncType();
// checks if we are currently inside a while loop
bool inWhile();
// saves the pointer in the global pointer arr, and sets the children
void registerNode(Node* p);
// deletes all the pointer that where allocated
void clearMemory();
// to be called when the parser finished it's work
void endCompilation();
// convert a type vector to string vector
vector<string> typeVecToStringVec(const vector<std::pair<string,TypeN>> typeVec);
void initGlobalVars();
// emits 'getelementptr' to stack and returns a temporary variable holding the address of the variable
string getVarPtr(Id* id);
string getVarPtr(int offset);
// ====================scanner======================================
Num* getNum(const string& s);

Id* getId(const string& s);

String* getStr(const string& s);

BinOperator* getBinOp(BinOp o);

RelOperator* getRelOp(const string& s);
//=====================Exp Rules=====================================
// Exp -> ( Exp )
Exp* expFromExp(Node*);
// Exp -> Exp BINOP Exp
Exp* expFromBinop(Node*, Node*, BinOp);
// Exp -> ID
Exp* expFromId(Node*);
// Exp -> Call
Exp* expFromCall(Node*);
// Exp -> NUM
Exp* expFromNum(Node*);
// Exp -> NUM B
Exp* expFromByte(Node*);
// Exp -> STRING
Exp* expFromString(Node*);
// Exp -> TRUE
// Exp -> FALSE
Exp* expFromBool(bool);
// Exp -> NOT Exp
Exp* expFromNot(Node*);
// Exp -> Exp AND Exp
// Exp -> Exp OR Exp
Exp* expFromLogicop(Node* pExp1, Node* pM, Node* pExp2, LogicOp);
// Exp -> Exp RELOP Exp
Exp* expFromRelop(Node* pExp1, Node* pExp2, Node* pExp3);


//======================== Type Rules =========================
// Type -> int
Type* typeInt();
// Type -> byte
Type* typeByte();
// Type -> bool
Type* typeBool();

//=======================Exp List Rules =======================
// ExpList -> Exp
ExpList* expListFromExp(Node* pExp);
// ExpList -> Exp COMMA ExpList
ExpList* expListRightRec(Node* pExp, Node* pExpList);

//========================Call Rules ==========================
// Call -> ID ( ExpList )
Call* call(Node* pId, Node* pExpList);
// Call -> ID ( )
Call* call(Node* pId);

//======================  Statement Rules =====================
// Statement -> { Statements }
Statement* statementList(Node* pStatements);
// Statement -> Type ID ;
Statement* statementVarDecl(Node* pType, Node* pId);
// Statement -> Type ID = Exp ;
Statement* statementVarDeclInit(Node* pType, Node* pId, Node* pExp);
// Statement -> ID = Exp ;
Statement* statementAssign(Node* pId, Node* pExp);
// Statement -> Call ;
Statement* statementCall(Node* pCall);
// Statement -> RETURN ;
Statement* statementReturn();
// Statement -> RETURN Exp ;
Statement* statementReturn(Node* pExp);
// Statement -> IF ( Exp ) Statement ELSE Statement
Statement* statementIfElse(Node* pExp, Node* pM1, Node* pStatement1, Node* pN1,
	Node* pM2, Node* pStatement2, Node* pN2);
// Statement -> WHILE ( Exp ) Statement ELSE Statement
Statement* statementWhileElse(Node* pN1, Node* pM1, Node* pExp, Node* pM2, Node* pStatements1,
	Node* pN2, Node* pM3, Node* pStatements2, Node* pN3);
// Statement -> IF ( Exp ) Statement
Statement* statementIf(Node* pExp, Node* pM, Node* pStatement, Node* pN);
// Statement -> WHILE ( Exp ) Statement
Statement* statementWhile(Node* pN1, Node* pM1, Node* pExp, Node* pM2, Node* pStatement, Node* pN2);
// Statement -> BREAK ;
Statement* statementBreak();
// Statement -> CONTINUE ;
Statement* statementContinue();


//====================== Statements Rules =====================
// Statements -> Statement
Statements* statementsEnd(Node* pStatement);
// Statements -> Statements Statement
Statements* statementsLeftRec(Node* pStatements, Node* pStatement);

//====================== Program Rules ========================
// Program -> Funcs
Program* program(Node* pFuncs);

//====================== Funcs Rules ========================
// Funcs ->	epsilon
Funcs* funcs();
// Funcs -> FuncDecl Funcs
Funcs* funcsRightRec(Node* pFuncDecl, Node* pFuncs);

//====================== FuncDecl Rules ========================
// FuncDecl -> RetType ID ( Formals ) { Statements }
FuncDecl* funcDecl(Node* pRetType, Node* pId, Node* pFormals, Node* pStatemets);

//====================== RetType Rules ========================
// RetType -> Type
RetType* retType(Node* pType);
// RetType -> VOID
RetType* retType();

//====================== Formals Rules ========================
// Formals -> epsilon 
Formals* formals();
// Formals -> FormalsList
Formals* formals(Node* pFormalsList);

//====================== FormalsList Rules ========================
// FormalsList -> FormalDecl
FormalsList* formalsList(Node* pFormalDecl);
// FormalsList -> FormalDecl , FormalsList
FormalsList* formalsListRightRec(Node* pFormalDecl, Node* pFormalsList);

//====================== FormalDecl Rules ========================
// FormalDecl -> Type ID
FormalDecl* formalDecl(Node* pType, Node* pId);

//========================= Markers ==============================
// creates and empty label
M* m();
// creates an empty unconditional br, with nextlist
N* n();
// checks if the last statements have a non-empty next list, and backpatches them before next statement
void backpatchStatements(Node* pStatements);

//====================== nested loops ============================
void enterWhile();
void exitWhile();

//====================== Scope handler ============================
void enterScope();
void exitScope();

//===================== function arguments evaluation =============
// if a bool expression is passed to a function than its value is needed to be evaluated
// before it is passed.
// this needs to act in the middle of the rule becasue of the derivation order of right recursion
void evaluateExp(Node* pExp);

//====================== Functions handler ============================
void setReturnType(Node* retType);
void addFunc(Node* retType, Node* ID, Node* Formals);


//====================== LLVM Code handlers ============================
void init_global_prog();
void end_global_prog();


#endif // !_UTILS