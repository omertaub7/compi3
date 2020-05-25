#include "utils.hpp"

//======================= global variables ===============
vector<Node*> globalPtrArr;
int nestedWhileCounter = 0;

GlobalSymbolTable symbolTable;
// TODO: put here the symbol table



//==========================Utils=========================
TypeN getIdType(Node* idNode) {
	return symbolTable.getSymbolType(idNode);
}

bool isByte(Node* pNode) {
	Num* pNum = (Num*)(pNode);
	return (pNum->value <= 255 && pNum->value >= 0);
}

bool isBool(Node* pNode) {
	return (pNode->getType() == TypeN::BOOL);
}

bool isNumeric(Node* pNode) {
	return (pNode->getType() == TypeN::INT || pNode->getType() == TypeN::BYTE);
}

bool isFunc(Node* pNode) {
	return symbolTable.checkSymbolIsFunction(pNode);
}

vector<TypeN> getFuncArgTypes(Node* pNode) {
	// TODO: implement with symbol table
	if (!isFunc(pNode)) {
		throw errorUndefFuncException(pNode->getName());
	}
	// dummy
	FuncDecl* func = dynamic_cast<FuncDecl*> pNode;
	return symbolTable.getFunctionArgs(func);
}

TypeN getFuncType(Node* pNode) {
	// TODO: implement with symbol table
	if (!isFunc(pNode)) {
		throw errorUndefFuncException(pNode->getName());
	}
	FuncDecl* func = dynamic_cast<FuncDecl*> pNode;
	return func->getType();
}

void insertNewVar(Node* pType, Node* pId) {
	symbolTable.insertVarible(pType); //Shai: pID is needed? I Push the node as itself to the table, it holds name&type which I Need
}

bool checkAssign(TypeN target, TypeN source) {
	if (target == TypeN::INT) {
		if (source == TypeN::INT || source == TypeN::BYTE) {
			return true;
		}
		return false;
	}
	if (target == TypeN::BOOL && source == TypeN::BOOL) {
		return true;
	}
	if (target == TypeN::BYTE && source == TypeN::BYTE) {
		return true;
	}
	return false;
}

TypeN getCurrFuncType() {
	// TODO: implement with symbol table
	// dummy
	return TypeN::INT;
}

bool inWhile() {
	if (nestedWhileCounter > 0) {
		return true;
	}
	return false;
}

// saves the pointer in the global pointer arr, and sets the children
void registerNode(Node* p, Node* c1, Node* c2, Node* c3, Node* c4) {
	globalPtrArr.push_back(p);
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

void clearNodes() {
	for (auto p : globalPtrArr) {
		delete p;
	}
}

void endCompilation() {
	clearNodes();
	symbolTable.endGlobalScope();
}

vector<string> typeVecToStringVec(const vector<TypeN> typeVec) {
	vector<string> stringVec;
	for (auto type : typeVec) {
		stringVec.push_back(to_string(type));
	}
	return stringVec;
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
Exp* expFromExp(Node* pNode) {
	assert(pNode);
	assert(checkPtr<Exp>(pNode));
	
	auto* p = new Exp(pNode->getType());
	registerNode(p, pNode);
	return p;
}

Exp* expFromBinop(Node* pNode1, Node* pNode2) {
	assert(pNode1);
	assert(pNode2);
	assert(checkPtr<Exp>(pNode1));
	assert(checkPtr<Exp>(pNode2));

	TypeN type = TypeN::INT;
	if (!(isNumeric(pNode1) && isNumeric(pNode2))) {
		throw errorMismatchException();
	}
	if (pNode1->getType() == TypeN::BYTE && pNode2->getType() == TypeN::BYTE) {
		type = TypeN::BYTE;
	}
	auto* p = new Exp(type);
	registerNode(p, pNode1, pNode2);
	return p; 
}

Exp* expFromId(Node* pNode) {
	assert(pNode);
	assert(checkPtr<Id>(pNode));

	auto* p = new Exp(getIdType(pNode));
	registerNode(p, pNode);
	return p;
}

Exp* expFromCall(Node* pNode) {
	assert(pNode);
	assert(checkPtr<Call>(pNode));

	auto* p = new Exp(pNode->getType());
	registerNode(p, pNode);
	return p;
}

Exp* expFromNum(Node* pNode) {
	assert(pNode);
	assert(checkPtr<Num>(pNode));

	auto* p = new Exp(TypeN::INT);
	registerNode(p, pNode);
	return p;
}

Exp* expFromByte(Node* pNode) {
	assert(pNode);
	assert(checkPtr<Num>(pNode));

	if (!isByte(pNode)) {
		throw errorByteTooLargeException(to_string(((Num*)pNode)->value));
	}
	auto* p = new Exp(TypeN::BYTE);
	registerNode(p, pNode);
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

Exp* expFromNot(Node* pNode) {
	assert(pNode);
	assert(checkPtr<Exp>(pNode));

	if (!isBool(pNode)) {
		throw errorMismatchException();
	}
	auto* p = new Exp(TypeN::BOOL);
	registerNode(p, pNode);
	return p;
}

Exp* expFromLogicop(Node* pNode1, Node* pNode2) {
	assert(pNode1);
	assert(checkPtr<Exp>(pNode1));
	assert(pNode2);
	assert(checkPtr<Exp>(pNode2));

	if (!(isBool(pNode1) && isBool(pNode2))) {
		throw errorMismatchException();
	}
	auto* p = new Exp(TypeN::BOOL);
	registerNode(p, pNode1, pNode2);
	return p;
}

Exp* expFromRelop(Node* pNode1, Node* pNode2) {
	assert(pNode1);
	assert(checkPtr<Exp>(pNode1));
	assert(pNode2);
	assert(checkPtr<Exp>(pNode2));

	if (!(isNumeric(pNode1) && isNumeric(pNode2))) {
		throw errorMismatchException();
	}
	auto* p = new Exp(TypeN::BOOL);
	registerNode(p, pNode1, pNode2);
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
	assert(pExp);
	assert(checkPtr<Exp>(pExp));

	auto* p = new ExpList((Exp*)pExp);
	registerNode(p, pExp);
	return p;
}

// ExpList -> Exp COMMA ExpList
ExpList* expListRightRec(Node* pExp, Node* pExpList) {
	assert(pExp);
	assert(checkPtr<Exp>(pExp));
	assert(pExpList);
	assert(checkPtr<ExpList>(pExpList));

	auto* p = new ExpList((Exp*)pExp, (ExpList*)pExpList);
	registerNode(p, pExp, pExpList);
	return p;
}

//========================Call Rules ==========================
Call* call(Node* pId, Node* pExpList) {
	assert(pId);
	assert(checkPtr<Id>(pId));
	assert(pExpList);
	assert(checkPtr<ExpList>(pExpList));

	vector<TypeN> excpectedArgTypes = getFuncArgTypes(pId);
	vector<Exp*>& recievedExpressions = ((ExpList*)pExpList)->expList;
	vector<string> argTypesStr = typeVecToStringVec(excpectedArgTypes);
	if (excpectedArgTypes.size() != recievedExpressions.size()) {
		throw errorPrototypeMismatchException(argTypesStr, pId->getName());
	}
	for (int i = 0; i < excpectedArgTypes.size(); i++) {
		if (excpectedArgTypes[i] != recievedExpressions[i]->getType()) {
			throw errorPrototypeMismatchException(argTypesStr, pId->getName());
		}
	}
	auto* p = new Call(getFuncType(pId));
	registerNode(p, pId, pExpList);
	return p;
}

Call* call(Node* pId) {
	assert(pId);
	assert(checkPtr<Id>(pId));

	vector<TypeN> argTypes = getFuncArgTypes(pId);
	vector<string> argString = typeVecToStringVec(argTypes);
	if (argTypes.size() != 0) {
		throw errorPrototypeMismatchException(argString, pId->getName());
	}
	auto* p = new Call(getFuncType(pId));
	registerNode(p, pId);
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

	auto* p = new FuncDecl((RetType*)pRetType, (Formals*)pFormals);
	try {
		symbolTable.InsertFunction(p);
	} catch (errorDefException& e) {
		delete p;
		throw e;
	}
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
	assert(pFormalDecl);
	assert(checkPtr<FormalDecl>(pFormalDecl));
	assert(pFormalsList);
	assert(checkPtr<FormalsList>(pFormalsList));

	// TODO check that we do not have to arguments with the same name
	auto* p = new FormalsList((FormalDecl*)pFormalDecl, (FormalsList*)pFormalsList);
	registerNode(p, pFormalDecl, pFormalsList);
	return p;
}

//====================== FormalDecl Rules ========================
FormalDecl* formalDecl(Node* pType, Node* pId) {
	assert(pType);
	assert(checkPtr<Type>(pType));
	assert(pId);
	assert(checkPtr<Id>(pId));

	auto* p = new FormalDecl((Type*)pType);
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
	symbolTable.addNewScope();
}

void exitScope() {
	symbolTable.popScope();
}