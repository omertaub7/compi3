#include "utils.hpp"

// for casting and testing for exact linenumber in assert
#define CAST_PTR(T, p_dst, p_src) assert((p_src));\
								  T *p_dst = dynamic_cast<T*>((p_src));\
								  assert((p_dst));
	

//======================= global variables ===============
vector<Node*> *globalPtrArr;
int nestedWhileCounter = 0;

GlobalSymbolTable* symbolTable;
CodeBuffer& codeBuffer = CodeBuffer::instance();

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

StringNode* getStr(const string& s) {
	auto p = new StringNode(s);
	registerNode(p);
	return p;
}

BinOperator* getBinOp(BinOp o) {
	auto p = new BinOperator(o);
	registerNode(p);
	return p;
}

RelOperator* getRelOp(const string& s) {
	RelOp op;
	//C++ strings are not fun to handle
	if (s == "<") {
		op = RelOp::LT;
	} else if (s == ">") {
		op = RelOp::GT;
	} else if (s == "<=") {
		op = RelOp::LEQ;
	} else if (s == ">=") {
		op = RelOp::GEQ;
	} else {
		assert(false);
	}
	auto p = new RelOperator(op);
	registerNode(p);
	return p;
}

EqualOperator* getEqOp(const string& s) {
	EqOp op;
	if (s == "==") {
		op = EqOp::NEQ;
	} else if (s == "!=") {
		op = EqOp::EQ;
	} else {
		assert(false);
	}
	auto p = new EqualOperator(op);
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

static inline int valueFromBinOp (int x, int y, BinOp op) {
	switch (op) {
		case BinOp::PLUS:
			return x+y;
		case BinOp::MINUS:
			return x-y;
		case BinOp::MUL:
			return x*y;
		case BinOp::DIV:
			//TODO: emit code handler to exit if y=0
			return x/y;
	}
}

Exp* expFromBinop(Node* pExp1, Node* pExp2, BinOp op) {
	CAST_PTR(Exp, exp1, pExp1);
	CAST_PTR(Exp, exp2, pExp2);

	TypeN type = TypeN::INT;
	if (!(isNumeric(exp1) && isNumeric(exp2))) {
		throw errorMismatchException();
	}
	if (exp1->getType() == TypeN::BYTE && exp2->getType() == TypeN::BYTE) {
		type = TypeN::BYTE;
	}
	emitBinOpCode(exp1, exp2, op);
	auto* p = new Exp(type, valueFromBinOp(exp1->value, exp2->value, op));
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
	auto* p = new Exp(TypeN::INT,num->value);
	registerNode(p, num);
	return p;
}

Exp* expFromByte(Node* pNum) {
	CAST_PTR(Num, num, pNum);

	if (!isByte(num)) {
		throw errorByteTooLargeException(to_string(num->value));
	}
	auto* p = new Exp(TypeN::BYTE, num->value);
	registerNode(p, num);
	return p;
}

Exp* expFromString(Node* string) {
	CAST_PTR(StringNode, exp, string);
	auto* p = new Exp(TypeN::STRING, exp->s_value);
	add_global_string(exp->s_value);
	emitGlobalString(exp->s_value);
	registerNode(p);
	return p;
}

Exp* expFromBool(bool flag) {
	auto* p = new Exp(TypeN::BOOL, flag);
	registerNode(p);
	return p;
}

Exp* expFromNot(Node* pExp) {
	CAST_PTR(Exp, exp, pExp);

	if (!isBool(exp)) {
		throw errorMismatchException();
	}
	auto* p = new Exp(TypeN::BOOL, !exp->b_value);
	registerNode(p, exp);
	return p;
}

Exp* expFromLogicop(Node* pExp1, Node* pExp2, LogicOp op) {
	CAST_PTR(Exp, exp1, pExp1);
	CAST_PTR(Exp, exp2, pExp2);

	if (!(isBool(exp1) && isBool(exp2))) {
		throw errorMismatchException();
	}
	bool flag = false;
	switch (op) {
		case LogicOp::AND:
			flag = exp1->b_value && exp2->b_value;
			break;
		case LogicOp::OR:
			flag = exp1->b_value || exp2->b_value;
			break;
		default:
			assert(false); //Should not get here
	}
	auto* p = new Exp(TypeN::BOOL, flag);
	registerNode(p, exp1, exp2);
	return p;
}

static inline bool compareByRelOp(int x, int y, RelOp op) {
	switch (op) {
		case RelOp::GT:
			return x>y;
		case RelOp::LT:
			return x<y;
		case RelOp::GEQ:
			return x>=y;
		case RelOp::LEQ:
			return x<=y;
		default:
			assert(false);
	}
}

Exp* expFromRelop(Node* pExp1, Node* pExp2, Node* pExp3) {
	CAST_PTR(Exp, exp1, pExp1);
	CAST_PTR(Exp, exp3, pExp3);
	CAST_PTR(RelOperator, exp2, pExp2)
	if (!(isNumeric(exp1) && isNumeric(exp3))) {
		throw errorMismatchException();
	}
	auto* p = new Exp(TypeN::BOOL, compareByRelOp(exp1->value, exp3->value, exp2->op));
	registerNode(p, exp1, exp2);
	return p;
}

static inline bool compareByEqOp(int x, int y, EqOp op) {
	switch (op) {
		case EqOp::EQ:
			return x==y;
		case EqOp::NEQ:
			return x!=y;
		default:
			assert(false);
	}
}

Exp* expFromEqualOp(Node* pExp1, Node* pExp2, Node* pExp3) {
	CAST_PTR(Exp, exp1, pExp1);
	CAST_PTR(Exp, exp3, pExp3);
	CAST_PTR(EqualOperator, exp2, pExp2)
	if (!(isNumeric(exp1) && isNumeric(exp3))) {
		throw errorMismatchException();
	}
	auto* p = new Exp(TypeN::BOOL, compareByEqOp(exp1->value, exp3->value, exp2->op));
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
	emitFunctionCall(getFuncType(id), id->getName(), recievedExpressions);
	//TODO: need to emit: call function.ret_type function.arg_types function.name function.arg_list
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
	emitNewVarDeclInit(symbolTable->getVaribleOffset(pId->getName()), pExp);
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
	emitFuncEnd(pRetType->getType());
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
	emitFuncDef(t, iden, f);
}

void exitFunc() {
	exitScope();
}

//====================== LLVM Code handlers ============================
void init_global_prog() {
    codeBuffer.emit("declare i32 @printf(i8*,...)");
	codeBuffer.emit("declare void @exit(i32)");
	codeBuffer.emit("@.str_specifier = constant [4 x i8] c\"%s\\0A\\00\"");
	codeBuffer.emit("@.int_specifier = constant [4 x i8] c\"%d\\0A\\00\"");
	codeBuffer.emit("define void @printi(i32) {"); 
    codeBuffer.emit("call i32 (i8*, ...) @printf(i8* getelementptr ([4 x i8], [4x i8]* @.int_specifier, i32 0, i32 0), i32 %0)");
    codeBuffer.emit("ret void");
	codeBuffer.emit("}");
	codeBuffer.emit("define void @print(i8*) {");
 	codeBuffer.emit("call i32 (i8*, ...) @printf(i8* getelementptr ([4 x i8], [4x i8]* @.str_specifier, i32 0, i32 0), i8* %0)");
	codeBuffer.emit("ret void");
    codeBuffer.emit("}");
}

void end_global_prog() {
	codeBuffer.printGlobalBuffer();
	codeBuffer.printCodeBuffer();
}

//====================== Buffer printers ============================
void emitBinOpCode(Exp* x, Exp* y, BinOp op) {
	return;
/*
	bool is_byte = x->getType() == TypeN::BYTE && y->getType() == TypeN::BYTE;
	string op_type = is_byte?"i8":"i32",
	exp_1 = x->getName();
	exp_1 = 
	switch (op) {
		case BinOp::PLUS:
			codeBuffer.emit("add" + op_type + exp_1 + " , " + exp_2);
			break;
		case BinOp::MINUS:
			codeBuffer.emit("sub" + op_type + exp_1 + " , " + exp_2);
			break;
		case BinOp::MUL:
			codeBuffer.emit("mul" + op_type + exp_1 + " , " + exp_2);
			break;
		case BinOp::DIV:
			//
			break;
		default:
			assert(false);
	}
	*/
}

static inline string to_llvm_retType(TypeN type) {
	switch (type) {
		case TypeN::INT:
			return "i32 ";
		case TypeN::BYTE:
			return "i8 ";
		case TypeN::VOID:
			return "void ";
		case TypeN::BOOL:
			return "i1 ";
	}
}

void emitFuncDef(RetType* type, Id* id, Formals* f) {
	//print function defenition
	std::stringstream code;
	code << "define ";
	code << to_llvm_retType(type->getType());
	code << "@";
	code << id->getName();
	code << "(";
	for (auto p : f->argTypes) {
		code << to_llvm_retType(p.second);
	}
	code << ") ";
	code << "{";
	codeBuffer.emit(code.str());
	//allocate local stack
	codeBuffer.emit(newStack() + " = alloca [50 x i32]");
}

void emitFuncEnd(TypeN type) {
	//TODO: return the correct value
	//For now ret void to be able to run
	switch (type) {
		case TypeN::VOID:
			codeBuffer.emit("ret void");
			break;
		case TypeN::INT:
			codeBuffer.emit("ret i32 0");
			break;
		case TypeN::BYTE:
			codeBuffer.emit("ret i8 0");
			break;
		case TypeN::BOOL:
			codeBuffer.emit("ret i1 0");
			break;
		default:
			assert(false);
	}
	codeBuffer.emit("}");
}

void emitNewVarDeclInit (int offset, Node* exp) {
	std::stringstream code;

}

void emitGlobalString(string value) {
	std::stringstream code;
	code << get_global_string(value);
	code << " = constant [";
    code << std::to_string(value.length()-1);
    code << " x i8] c\"";
	value.erase(std::remove(value .begin(), value .end(), '"'), value .end());
    code << value;
	code << "\\00\"";
    //code << "\\0A\\00\"";
    codeBuffer.emitGlobal(code.str());
}

void emitFunctionCall(TypeN retType, string id, vector<Exp*>& recieved_args) {
	std::stringstream code;
	code << "call ";
	code << to_llvm_retType(retType);
	code << "@";
	code << id;
	code << " (";
	for (auto p : recieved_args) {
		if (p->getType() == TypeN::STRING) {
			//Single argument - pointer to global string varible
			int size = (p->s_value).length() - 1;
			code << "i8* getelementptr([";
			code << std::to_string(size);
			code << " x i8], [";
			code << std::to_string(size);
			code << " x i8]* ";
			code << get_global_string(p->s_value);
			code << ", i32 0, i32 0)";
		}
	}
	code << ")";
	codeBuffer.emit(code.str());
}
	