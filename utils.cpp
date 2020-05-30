#include "utils.hpp"

// for casting and testing for exact linenumber in assert
#define CAST_PTR(T, p_dst, p_src) assert((p_src));\
								  T *p_dst = dynamic_cast<T*>((p_src));\
								  assert((p_dst));
	

//======================= global variables ===============
vector<Node*> *globalPtrArr;
int nestedWhileCounter = 0;

GlobalSymbolTable* symbolTable;



//==========================Utils=========================
TypeN getIdType(Node* id) {
	return symbolTable->getSymbolType(id);
}

bool isByte(Num* num) {
	return (num->value <= 255 && num->value >= 0);
}

bool isBool(Node* exp) {
	return (exp->getType() == TypeN::BOOL);
}

bool isNumeric(Node* exp) {
	return (exp->getType() == TypeN::INT || exp->getType() == TypeN::BYTE);
}

bool isFunc(Node* id) {
	return symbolTable->checkSymbolIsFunction(id);
}

vector<std::pair<string,TypeN>> getFuncArgTypes(Node* id) {
	if (!isFunc(id)) {
		throw errorUndefFuncException(id->getName());
	}
	return symbolTable->getFunctionArgs(id);
}

TypeN getFuncType(Node* id) {
	if (!isFunc(id)) {
		throw errorUndefFuncException(id->getName());
	}
	return symbolTable->getFuncRetType(id->getName());
}

void insertNewVar(Node* type, Node* id) {
	symbolTable->insertVarible(id->getName(), type->getType());
}

bool checkAssign(TypeN target, TypeN source) {
	if (target == source) {
		return true;
	}
	if (target == TypeN::INT && source == TypeN::BYTE) {
		return true;
	}
	return false;
}

TypeN getCurrFuncType() {
	return symbolTable->getCurrentReturnType();
}

bool inWhile() {
	if (nestedWhileCounter > 0) {
		return true;
	}
	return false;
}

// saves the pointer in the global pointer arr, and sets the children
void registerNode(Node* p, Node* c1, Node* c2, Node* c3, Node* c4) {
	globalPtrArr->push_back(p);
	if (c1) {
		p->addChild(c1);
	}
	if (c2) {
		p->addChild(c2);
	}
	if (c3) {
		p->addChild(c3);
	}
	if (c4) {
		p->addChild(c4);
	}
}

void clearMemory() {
	for (auto p : *globalPtrArr) {
		delete p;
	}
	delete globalPtrArr;
	delete symbolTable;
}

void endCompilation() {
	symbolTable->endGlobalScope();
}

vector<string> typeVecToStringVec(const vector<std::pair<string,TypeN>> typeVec) {
	vector<string> stringVec;
	for (auto type : typeVec) {
		stringVec.push_back(to_string(type.second));
	}
	return stringVec;
}

void initGlobalVars() {
	globalPtrArr = new vector<Node*>();
	symbolTable = new GlobalSymbolTable();
}
// ====================scanner======================================
Num* getNum(const string& s) {
	auto p = new Num(s);
	registerNode(p);
	return p;
}
Id* getId(const string& s) {
	auto p = new Id(s);
	registerNode(p);
	return p;
}

//=====================Exp Rules=====================================
Exp* expFromExp(Node* pExp) {
	CAST_PTR(Exp, exp, pExp);
	
	auto* p = new Exp(exp->getType());
	registerNode(p, exp);
	return p;
}

Exp* expFromBinop(Node* pExp1, Node* pExp2) {
	CAST_PTR(Exp, exp1, pExp1);
	CAST_PTR(Exp, exp2, pExp2);

	TypeN type = TypeN::INT;
	if (!(isNumeric(exp1) && isNumeric(exp2))) {
		throw errorMismatchException();
	}
	if (exp1->getType() == TypeN::BYTE && exp2->getType() == TypeN::BYTE) {
		type = TypeN::BYTE;
	}
	auto* p = new Exp(type);
	registerNode(p, exp1, exp2);
	return p; 
}

Exp* expFromId(Node* pId) {
	CAST_PTR(Id, id, pId);

	auto* p = new Exp(getIdType(id));
	registerNode(p, id);
	return p;
}

Exp* expFromCall(Node* pCall) {
	CAST_PTR(Call, call, pCall);

	auto* p = new Exp(call->getType());
	registerNode(p, call);
	return p;
}

Exp* expFromNum(Node* pNum) {
	CAST_PTR(Num, num, pNum);
	auto* p = new Exp(TypeN::INT);
	registerNode(p, num);
	return p;
}

Exp* expFromByte(Node* pNum) {
	CAST_PTR(Num, num, pNum);

	if (!isByte(num)) {
		throw errorByteTooLargeException(to_string(num->value));
	}
	auto* p = new Exp(TypeN::BYTE);
	registerNode(p, num);
	return p;
}

Exp* expFromString() {
	auto* p = new Exp(TypeN::STRING);
	registerNode(p);
	return p;
}

Exp* expFromBool() {
	auto* p = new Exp(TypeN::BOOL);
	registerNode(p);
	return p;
}

Exp* expFromNot(Node* pExp) {
	CAST_PTR(Exp, exp, pExp);

	if (!isBool(exp)) {
		throw errorMismatchException();
	}
	auto* p = new Exp(TypeN::BOOL);
	registerNode(p, exp);
	return p;
}

Exp* expFromLogicop(Node* pExp1, Node* pExp2) {
	CAST_PTR(Exp, exp1, pExp1);
	CAST_PTR(Exp, exp2, pExp2);

	if (!(isBool(exp1) && isBool(exp2))) {
		throw errorMismatchException();
	}
	auto* p = new Exp(TypeN::BOOL);
	registerNode(p, exp1, exp2);
	return p;
}

Exp* expFromRelop(Node* pExp1, Node* pExp2) {
	CAST_PTR(Exp, exp1, pExp1);
	CAST_PTR(Exp, exp2, pExp2);

	if (!(isNumeric(exp1) && isNumeric(exp2))) {
		throw errorMismatchException();
	}
	auto* p = new Exp(TypeN::BOOL);
	registerNode(p, exp1, exp2);
	return p;
}

//======================== Type Rules =========================
Type* typeInt() {
	auto* p = new Type(TypeN::INT);
	registerNode(p);
	return p;
}

Type* typeByte() {
	auto* p = new Type(TypeN::BYTE);
	registerNode(p);
	return p;
}

Type* typeBool() {
	auto* p = new Type(TypeN::BOOL);
	registerNode(p);
	return p;
}

//=======================Exp List Rules =======================
// ExpList -> Exp
ExpList* expListFromExp(Node* pExp) {
	CAST_PTR(Exp, exp, pExp);

	auto* p = new ExpList(exp);
	registerNode(p, exp);
	return p;
}

// ExpList -> Exp COMMA ExpList
ExpList* expListRightRec(Node* pExp, Node* pExpList) {
	CAST_PTR(Exp, exp, pExp);
	CAST_PTR(ExpList, expList, pExpList);

	auto* p = new ExpList(exp, expList);
	registerNode(p, exp, expList);
	return p;
}

//========================Call Rules ==========================
Call* call(Node* pId, Node* pExpList) {
	CAST_PTR(Id, id, pId);
	CAST_PTR(ExpList, expList, pExpList);

	vector<std::pair<string,TypeN>> excpectedArgTypes = getFuncArgTypes(id);
	vector<Exp*>& recievedExpressions = (expList)->expList;
	vector<string> argTypesStr = typeVecToStringVec(excpectedArgTypes);
	if (excpectedArgTypes.size() != recievedExpressions.size()) {
		throw errorPrototypeMismatchException(argTypesStr, id->getName());
	}
	for (int i = 0; i < excpectedArgTypes.size(); i++) {
		if (! checkAssign(excpectedArgTypes[i].second, recievedExpressions[i]->getType())) {
			throw errorPrototypeMismatchException(argTypesStr, id->getName());
		}
	}
	auto* p = new Call(getFuncType(id));
	registerNode(p, id, expList);
	return p;
}

Call* call(Node* pId) {
	CAST_PTR(Id, id, pId);

	vector<std::pair<string, TypeN>> argTypes = getFuncArgTypes(id);
	vector<string> argString = typeVecToStringVec(argTypes);
	if (argTypes.size() != 0) {
		throw errorPrototypeMismatchException(argString, id->getName());
	}
	auto* p = new Call(getFuncType(id));
	registerNode(p, id);
	return p;
}

//======================  Statement Rules =====================
Statement* statementList(Node* pStatements) {
	assert(pStatements);
	assert(checkPtr<Statements>(pStatements));

	auto* p = new Statement();
	registerNode(p, pStatements);
	return p;
}

Statement* statementVarDecl(Node* pType, Node* pId) {
	assert(pType);
	assert(checkPtr<Type>(pType));
	assert(pId);
	assert(checkPtr<Id>(pId));

	insertNewVar(pType, pId);
	auto* p = new Statement();
	registerNode(p, pType, pId);
	return p;
}

Statement* statementVarDeclInit(Node* pType, Node* pId, Node* pExp) {
	assert(pType);
	assert(checkPtr<Type>(pType));
	assert(pId);
	assert(checkPtr<Id>(pId));
	assert(pExp);
	assert(checkPtr<Exp>(pExp));
	
	if (! checkAssign(pType->getType(), pExp->getType())) {
		throw errorMismatchException();
	}
	insertNewVar(pType, pId);
	auto* p = new Statement();
	registerNode(p, pType, pId, pExp);
	return p;
}

Statement* statementAssign(Node* pId, Node* pExp) {
	assert(pId);
	assert(checkPtr<Id>(pId));
	assert(pExp);
	assert(checkPtr<Exp>(pExp));
	
	if (! checkAssign(getIdType(pId), pExp->getType())) {
		throw errorMismatchException();
	}
	auto* p = new Statement();
	registerNode(p, pId, pExp);
	return p;
}

Statement* statementCall(Node* pCall) {
	assert(pCall);
	assert(checkPtr<Call>(pCall));

	auto* p = new Statement();
	registerNode(p, pCall);
	return p;
}

Statement* statementReturn() {
	if (! (getCurrFuncType() == TypeN::VOID)) {
		throw errorMismatchException();
	}
	auto* p = new Statement();
	registerNode(p);
	return p;
}

Statement* statementReturn(Node* pExp) {
	assert(pExp);
	assert(checkPtr<Exp>(pExp));

	if (! checkAssign(getCurrFuncType(), pExp->getType())) {
		throw errorMismatchException();
	}
	auto* p = new Statement();
	registerNode(p, pExp);
	return p;
}

Statement* statementIfElse(Node* pExp, Node* pStatement1, Node* pStatement2) {
	assert(pExp);
	assert(checkPtr<Exp>(pExp));
	assert(pStatement1);
	assert(checkPtr<Statement>(pStatement1));
	assert(pStatement2);
	assert(checkPtr<Statement>(pStatement2));

	if (! isBool(pExp)) {
		throw errorMismatchException();
	}
	auto* p = new Statement();
	registerNode(p, pExp, pStatement1, pStatement2);
	return p;
}

Statement* statementIf(Node* pExp, Node* pStatement) {
	assert(pExp);
	assert(checkPtr<Exp>(pExp));
	assert(pStatement);
	assert(checkPtr<Statement>(pStatement));
	
	if (!isBool(pExp)) {
		throw errorMismatchException();
	}
	auto* p = new Statement();
	registerNode(p, pExp, pStatement);
	return p;
}

Statement* statementBreak() {
	if (!inWhile()) {
		throw errorUnexpectedBreakException();
	}
	auto* p = new Statement();
	registerNode(p);
	return p;
}

Statement* statementContinue() {
	if (!inWhile()) {
		throw errorUnexpectedContinueException();
	}
	auto* p = new Statement();
	registerNode(p);
	return p;
}

//====================== Statements Rules =====================

Statements* statementsEnd(Node* pStatement) {
	assert(pStatement);
	assert(checkPtr<Statement>(pStatement));

	auto* p = new Statements();
	registerNode(p, pStatement);
	return p;
}


Statements* statementsLeftRec(Node* pStatements, Node* pStatement) {
	assert(pStatements);
	assert(checkPtr<Statements>(pStatements));
	assert(pStatement);
	assert(checkPtr<Statement>(pStatement));

	auto* p = new Statements();
	registerNode(p, pStatements, pStatement);
	return p;
}

//====================== Program Rules ========================

Program* program(Node* pFuncs) {
	assert(pFuncs);
	assert(checkPtr<Funcs>(pFuncs));

	endCompilation();

	auto* p = new Program();
	registerNode(p, pFuncs);
	return p;
}

//====================== Funcs Rules ========================

Funcs* funcs() {
	auto* p = new Funcs();
	registerNode(p);
	return p;
}

Funcs* funcsRightRec(Node* pFuncDecl, Node* pFuncs) {
	assert(pFuncDecl);
	assert(checkPtr<FuncDecl>(pFuncDecl));
	assert(pFuncs);
	assert(checkPtr<Funcs>(pFuncs));

	auto* p = new Funcs();
	registerNode(p, pFuncDecl, pFuncs);
	return p;
}

//====================== FuncDecl Rules ========================

FuncDecl* funcDecl(Node* pRetType, Node* pId, Node* pFormals, Node* pStatements) {
	assert(pRetType);
	assert(checkPtr<RetType>(pRetType));
	assert(pId);
	assert(checkPtr<Id>(pId));
	assert(pFormals);
	assert(checkPtr<Formals>(pFormals));
	assert(pStatements);
	assert(checkPtr<Statements>(pStatements));

	auto* p = new FuncDecl((RetType*)pRetType, (Id*) pId, (Formals*)pFormals);
	registerNode(p, pRetType, pId, pFormals, pStatements);
	return p;
}

//====================== RetType Rules ========================

RetType* retType(Node* pType) {
	assert(pType);
	assert(checkPtr<Type>(pType));

	auto* p = new RetType((Type*)pType);
	registerNode(p, pType);
	return p;
}

RetType* retType() {
	auto* p = new RetType();
	registerNode(p);
	return p;
}

//====================== Formals Rules ========================

Formals* formals() {
	auto* p = new Formals();
	registerNode(p);
	return p;
}

Formals* formals(Node* pFormalsList) {
	assert(pFormalsList);
	assert(checkPtr<FormalsList>(pFormalsList));

	auto* p = new Formals((FormalsList*)pFormalsList);
	registerNode(p, pFormalsList);
	return p;
}

//====================== FormalsList Rules ========================

FormalsList* formalsList(Node* pFormalDecl) {
	assert(pFormalDecl);
	assert(checkPtr<FormalDecl>(pFormalDecl));

	auto* p = new FormalsList((FormalDecl*)pFormalDecl);
	registerNode(p, pFormalDecl);
	return p;
}

FormalsList* formalsListRightRec(Node* pFormalDecl, Node* pFormalsList) {
	CAST_PTR(FormalDecl, formalDecl, pFormalDecl);
	CAST_PTR(FormalsList, formalsList, pFormalsList);

	// check that we do not have to arguments with the same name
	string currName = formalDecl->getName();
	for (std::pair<string, TypeN>& nameType : formalsList->argTypes) {
		if (currName == nameType.first) {
			throw errorDefException(currName);
		}
	}
	auto* p = new FormalsList(formalDecl, formalsList);
	registerNode(p, pFormalDecl, pFormalsList);
	return p;
}

//====================== FormalDecl Rules ========================
FormalDecl* formalDecl(Node* pType, Node* pId) {
	assert(pType);
	assert(checkPtr<Type>(pType));
	assert(pId);
	assert(checkPtr<Id>(pId));

	auto* p = new FormalDecl((Type*)pType, (Id*)pId);
	registerNode(p, pType, pId);
	return p;
}

//====================== nested loops ============================
void enterWhile() {
	nestedWhileCounter++;
}

void exitWhile() {
	nestedWhileCounter--;
}

//====================== Scope handler ============================
void enterScope() {
	symbolTable->addNewScope();
}

void exitScope() {
	symbolTable->popScope();
}

void setReturnType(Node* retType) {
	CAST_PTR(RetType, t, retType);
	symbolTable->setCurrentReturnType(t->getType());
}

void addFunc(Node* retType, Node* identifier, Node* formals) {
	CAST_PTR(RetType, t, retType);
	CAST_PTR(Id, iden, identifier);
	CAST_PTR(Formals, f, formals);

	symbolTable->insertFunction(t, iden, f);
}

void exitFunc() {
	exitScope();
}