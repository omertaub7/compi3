#include "utils.hpp"
// TODO: do we even have to keep the hierarchy of the Nodes?

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

// saves the pointer in the global pointer arr
void registerNode(Node* p) {
	globalPtrArr->push_back(p);
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

string getVarPtr(Id* id) {
	assert(id);
	int offset = symbolTable->getVaribleOffset(id->getName());
	return getVarPtr(offset);
}

string getVarPtr(int offset) {
	// TODO: what if we want to assign value to one of the arguments?
	// in c it is allowd...
	assert(offset >= 0);	// not argument
	stringstream get_ptr_code;
	string ptr = newTemp();
	get_ptr_code << ptr << " = getelementptr [50 x i32], [50 x i32]* %stack, i32 0, i32 " << offset;
	codeBuffer.emit(get_ptr_code.str());
	return ptr;
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

String* getStr(const string& s) {
	auto p = new String(s);
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
	if (s == "<") {
		op = RelOp::LT;
	}
	else if (s == ">") {
		op = RelOp::GT;
	}
	else if (s == "<=") {
		op = RelOp::LEQ;
	}
	else if (s == ">=") {
		op = RelOp::GEQ;
	}
	else if (s == "==") {
		op = RelOp::EQ;
	}
	else if (s == "!=") {
		op = RelOp::NEQ;
	}
	else {
		assert(false);
	}
	auto p = new RelOperator(op);
	registerNode(p);
	return p;
}

//=====================Exp Rules=====================================
Exp* expFromExp(Node* pExp) {
	CAST_PTR(Exp, exp, pExp);
	auto* p = new Exp(*exp);
	registerNode(p);

	return p;
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

	auto* p = new Exp(type, newTemp());
	registerNode(p);

	string opcode;
	switch (op)
	{
	case BinOp::PLUS:	opcode = "add"; break;
	case BinOp::MINUS:	opcode = "sub"; break;
	case BinOp::MUL:	opcode = "mul"; break;
	case BinOp::DIV:	opcode = "sdiv"; break; //TODO: Print error of division by zero
	}

	if (type == TypeN::INT) {
		stringstream code;
		code << p->place << " = " << opcode << " i32 " << exp1->place << ", " << exp2->place;
		codeBuffer.emit(code.str());
	}
	// deal with overflow in the case of byte
	else {
		string temp = newTemp();
		stringstream temp_code;
		temp_code << temp << " = " << opcode << " i32 " << exp1->place << ", " << exp2->place;
		stringstream code;
		// clear the upper bits
		code << p->place << " = and i32 255, " << temp;		
		codeBuffer.emit(temp_code.str());
		codeBuffer.emit(code.str());
	}
	return p; 
}

Exp* expFromId(Node* pId) {
	CAST_PTR(Id, id, pId);

	TypeN type = getIdType(id);
	int offset = symbolTable->getVaribleOffset(id->getName());
	string place;
	// in function arguments
	if (offset < 0) {
		int arg_number = (offset * -1) - 1;
		place = "%";
		place += to_string(arg_number);
	}
	// in stack
	else {
		place = newTemp();
		string ptr = getVarPtr(offset);
		stringstream load_code;
		load_code << place << " = load i32, i32* " << ptr;
		codeBuffer.emit(load_code.str());
		// in case it is bool need to conver it to i1
		if (type == TypeN::BOOL) {
			string temp = newTemp();
			stringstream convert_code;
			convert_code << temp << " = trunc i32 " << place << " to i1";
			place = temp;
			codeBuffer.emit(convert_code.str());
		}
	}
	
	auto* p = new Exp(type, place);
	registerNode(p);
	return p;
}

Exp* expFromCall(Node* pCall) {
	CAST_PTR(Call, call, pCall);
	stringstream code;
	string place = newTemp();
	
	auto* p = new Exp(call->getType(), place);

	code << place << " = " << call->function_call_string;
	codeBuffer.emit(code.str());
	
	registerNode(p);
	return p;
}

Exp* expFromNum(Node* pNum) {
	CAST_PTR(Num, num, pNum);

	auto p = new Exp(TypeN::INT, to_string(num->value));
	registerNode(p);

	return p;
}

Exp* expFromByte(Node* pNum) {
	CAST_PTR(Num, num, pNum);

	if (!isByte(num)) {
		throw errorByteTooLargeException(to_string(num->value));
	}
	auto p = new Exp(TypeN::BYTE, to_string(num->value));
	registerNode(p);

	return p;
}

Exp* expFromString(Node* pNode) {
	CAST_PTR(String, pString, pNode);
	
	auto* p = new Exp(TypeN::STRING, newString());
	registerNode(p);
	// emit to the global buffer
	stringstream string_declaration;
	// add \00" to the end
	string str = pString->s;
	// taking out the qoutes(-2) and adding \00 (+1)
	int n_bytes = str.length() - 1;
	p->n_bytes = n_bytes;
	str[str.length() - 1] = '\\';
	str += "00\""; 
	string_declaration << p->place << " = " << "constant [" << n_bytes << " x i8] c" << str;
	codeBuffer.emitGlobal(string_declaration.str());

	return p;
}

Exp* expFromBool(bool b) {
	auto* p = new Exp(TypeN::BOOL);
	registerNode(p);

	stringstream br_code;
	br_code << "br i1 " << b << ", label @, label @";
	int buffer_index = codeBuffer.emit(br_code.str());
	p->truelist = codeBuffer.makelist({ buffer_index, FIRST });
	p->falselist = codeBuffer.makelist({ buffer_index, SECOND });

	return p;
}

Exp* expFromNot(Node* pExp) {
	CAST_PTR(Exp, exp, pExp);

	if (!isBool(exp)) {
		throw errorMismatchException();
	}
	auto* p = new Exp(TypeN::BOOL);
	registerNode(p);
	
	p->truelist = exp->falselist;
	p->falselist = exp->truelist;

	return p;
}

Exp* expFromLogicop(Node* pExp1, Node* pM, Node* pExp2, LogicOp op) {
	CAST_PTR(Exp, exp1, pExp1);
	CAST_PTR(Exp, exp2, pExp2);
	CAST_PTR(M, m, pM);

	if (!(isBool(exp1) && isBool(exp2))) {
		throw errorMismatchException();
	}
	auto* p = new Exp(TypeN::BOOL, exp1->place);
	registerNode(p);
	
	switch (op)
	{
	case LogicOp::AND:
		// if exp1 is true, then go evaluate exp2
		codeBuffer.bpatch(exp1->truelist, m->label);
		p->truelist = exp2->truelist;
		p->falselist = codeBuffer.merge(exp1->falselist, exp2->falselist);
		break;
	case LogicOp::OR:
		// if exp1 is false, then go evaluate exp2
		codeBuffer.bpatch(exp1->falselist, m->label);
		p->falselist = exp2->falselist;
		p->truelist = codeBuffer.merge(exp1->truelist, exp2->truelist);
		break;
	default:
		assert(false); // should not get here
		break;
	}

	return p;
}

Exp* expFromRelop(Node* pExp1, Node* pExp2, Node* pExp3) {
	CAST_PTR(Exp, exp1, pExp1);
	CAST_PTR(RelOperator, rel_op, pExp2);
	CAST_PTR(Exp, exp2, pExp3);

	if (!(isNumeric(exp1) && isNumeric(exp2))) {
		throw errorMismatchException();
	}
	auto* p = new Exp(TypeN::BOOL);
	registerNode(p);
	
	string op;
	switch (rel_op->op)
	{
	case RelOp::LT:
		op = "slt";
		break;
	case RelOp::GT:
		op = "sgt";
		break;
	case RelOp::LEQ:
		op = "sle";
		break;
	case RelOp::GEQ:
		op = "sge";
		break;
	case RelOp::EQ:
		op = "eq";
		break;
	case RelOp::NEQ:
		op = "ne";
		break;
	default:
		assert(false); //should not get here
		break;
	}

	string temp = newTemp();
	stringstream icmp_code;
	icmp_code << temp << " = icmp " << op << " i32 " << exp1->place << ", " << exp2->place;
	codeBuffer.emit(icmp_code.str());
	stringstream br_code;
	br_code << "br i1 " << temp << ", label @, label @";
	int buffer_index = codeBuffer.emit(br_code.str());
	p->truelist = codeBuffer.makelist({ buffer_index, FIRST });
	p->falselist = codeBuffer.makelist({ buffer_index, SECOND });

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
	registerNode(p);
	return p;
}

// ExpList -> Exp COMMA ExpList
ExpList* expListRightRec(Node* pExp, Node* pExpList) {
	CAST_PTR(Exp, exp, pExp);
	CAST_PTR(ExpList, expList, pExpList);

	auto* p = new ExpList(exp, expList);
	registerNode(p);
	return p;
}

//========================Call Rules ==========================
Call* call(Node* pId, Node* pExpList) {
	// TODO: move out, should be in 'statementCall' or 'expFromCall'
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
	p->function_call_string = emitFunctionCall(getFuncType(id), id->getName(), recievedExpressions);
	//TODO: need to emit: call function.ret_type function.arg_types function.name function.arg_list
	registerNode(p);
	return p;
}

Call* call(Node* pId) {
	CAST_PTR(Id, id, pId);
	// TODO: implement
	vector<std::pair<string, TypeN>> argTypes = getFuncArgTypes(id);
	vector<string> argString = typeVecToStringVec(argTypes);
	if (argTypes.size() != 0) {
		throw errorPrototypeMismatchException(argString, id->getName());
	}
	auto* p = new Call(getFuncType(id));
	vector<Exp*> dummy;
	p->function_call_string = emitFunctionCall(getFuncType(id), id->getName(), dummy);
	registerNode(p);
	return p;
}

//======================  Statement Rules =====================
Statement* statementList(Node* pNode) {
	// TODO: implement
	CAST_PTR(Statements, pStatements, pNode);

	auto* p = new Statement();
	registerNode(p);

	p->nextlist = pStatements->nextlist;
	p->breakList = pStatements->breakList;
	p->continueList = pStatements->continueList;
	return p;
}

Statement* statementVarDecl(Node* pNode1, Node* pNode2) {
	// TODO: implement
	CAST_PTR(Type, pType, pNode1);
	CAST_PTR(Id, pId, pNode2);

	insertNewVar(pType, pId);
	auto* p = new Statement();
	registerNode(p);
	// initialize the new var to 0
	string ptr = getVarPtr(pId);
	stringstream init_code;
	init_code << "store i32 0, i32* " << ptr;
	codeBuffer.emit(init_code.str());

	return p;
}

Statement* statementVarDeclInit(Node* pNode1, Node* pNode2, Node* pNode3) {
	// TODO: only works for int, implement also for byte and bool
	CAST_PTR(Type, pType, pNode1);
	CAST_PTR(Id, pId, pNode2);
	CAST_PTR(Exp, pExp, pNode3);
	
	if (! checkAssign(pType->getType(), pExp->getType())) {
		throw errorMismatchException();
	}
	insertNewVar(pType, pId);
	auto* p = new Statement();
	registerNode(p);

	// store the value of exp in the correct offset
	int offset = symbolTable->getVaribleOffset(pId->getName());
	// check that it is not an argument
	assert(offset >= 0);

	string ptr = getVarPtr(offset);
	stringstream store_code;
	store_code << "store i32 " << pExp->place << ", i32* " << ptr;
	codeBuffer.emit(store_code.str());
	
	return p;
}

Statement* statementAssign(Node* pNode1, Node* pNode2) {
	CAST_PTR(Id, pId, pNode1);
	CAST_PTR(Exp, pExp, pNode2);
	
	if (! checkAssign(getIdType(pId), pExp->getType())) {
		throw errorMismatchException();
	}
	auto* p = new Statement();
	registerNode(p);
	// TODO: what about assignment to one of the functions argument?
	// get the local variable address, and store the exp into it
	string ptr = getVarPtr(pId);
	stringstream store_code;
	store_code << "store i32 " << pExp->place << ", i32* " << ptr;
	codeBuffer.emit(store_code.str());
	return p;
}

Statement* statementCall(Node* pCall) {
	CAST_PTR(Call, call, pCall);

	codeBuffer.emit(call->function_call_string);

	auto* p = new Statement();

	registerNode(p);
	return p;
}

Statement* statementReturn() {
	// TODO: implement
	if (! (getCurrFuncType() == TypeN::VOID)) {
		throw errorMismatchException();
	}
	auto* p = new Statement();
	codeBuffer.emit("ret void");
	registerNode(p);
	return p;
}

Statement* statementReturn(Node* pNode) {
	// TODO: implement
	CAST_PTR(Exp, pExp, pNode);

	if (! checkAssign(getCurrFuncType(), pExp->getType())) {
		throw errorMismatchException();
	}
	auto* p = new Statement();
	stringstream code;
	code << "ret " << to_llvm_retType(pExp->getType()) << pExp->place;
	codeBuffer.emit(code.str());
	registerNode(p);
	return p;
}

Statement* statementIfElse(Node* pExp, Node* pM1, Node* pStatement1, Node* pN1,
	Node* pM2, Node* pStatement2, Node* pN2) {
	// TODO: implement
	CAST_PTR(Exp, exp, pExp);
	CAST_PTR(M, m1, pM1);
	CAST_PTR(Statement, statement1, pStatement1);
	CAST_PTR(N, n1, pN1);
	CAST_PTR(M, m2, pM2);
	CAST_PTR(Statement, statement2, pStatement2);
	CAST_PTR(N, n2, pN2);

	if (!isBool(exp)) {
		throw errorMismatchException();
	}

	auto* p = new Statement();
	registerNode(p);
	// backpatching
	codeBuffer.bpatch(exp->truelist, m1->label);
	codeBuffer.bpatch(exp->falselist, m2->label);
	p->nextlist = codeBuffer.merge(statement1->nextlist, statement2->nextlist);
	p->nextlist = codeBuffer.merge(p->nextlist, n1->nextlist);
	p->nextlist = codeBuffer.merge(p->nextlist, n2->nextlist);

	p->breakList = codeBuffer.merge(statement1->breakList, statement2->breakList);
	p->continueList = codeBuffer.merge(statement1->continueList, statement2->continueList);

	return p;
}


//WHILE LPAREN N M Exp RPAREN EnterWhile EnterScope M Statement N ExitScope ExitWhile ELSE EnterScope M Statement N ExitScope
/*
Evaluation_label: 
	//Evaluate bool exp
	br i1 %temp label %while_block_label, label %else_block_label
while_block_label:
	//Statements of the while block
		//Break statements goto end_else_label
		//Continue statements goto evaluation_label
 	br evaluation_label
else_block_label:
	//Statements of the else block
	br end_else_label
end_else_label:
	//Next code statements
*/
Statement* statementWhileElse(Node* pN1, Node* pM1, Node* pExp, Node* pM2, Node* pStatements1,
	Node* pN2, Node* pM3, Node* pStatements2, Node* pN3) {
	CAST_PTR(N, n1, pN1);
	CAST_PTR(M, m1, pM1);
	CAST_PTR(Exp, exp, pExp);
	CAST_PTR(M, m2, pM2);
	CAST_PTR(Statement, statements1, pStatements1);
	CAST_PTR(N, n2, pN2);
	CAST_PTR(M, m3, pM3);
	CAST_PTR(Statement, statements2, pStatements2);
	CAST_PTR(N, n3, pN3);

	if (!isBool(exp)) {
		throw errorMismatchException();
	}

	auto* p = new Statement();
	registerNode(p);

	string evaluation_label = m1->label; 
	string while_block_label = m2->label;
	string else_block_label = m3->label;
	string end_else_label = codeBuffer.genLabel();


	codeBuffer.bpatch(exp->falselist, else_block_label);
	codeBuffer.bpatch(exp->truelist, while_block_label);
	
	codeBuffer.bpatch(n2->nextlist, evaluation_label);
	codeBuffer.bpatch(n1->nextlist, evaluation_label);
	codeBuffer.bpatch(n3->nextlist, end_else_label);

	codeBuffer.bpatch(statements1->breakList, end_else_label);
	codeBuffer.bpatch(statements1->continueList, evaluation_label);
	codeBuffer.bpatch(statements1->nextlist, else_block_label);
	codeBuffer.bpatch(statements2->nextlist, end_else_label);
	codeBuffer.bpatch(statements2->nextlist, end_else_label);

	return p;
}

Statement* statementIf(Node* pExp, Node* pM, Node* pStatement, Node* pN) {
	// TODO: implemnet
	CAST_PTR(Exp, exp, pExp);
	CAST_PTR(M, m, pM);
	CAST_PTR(Statement, statement, pStatement);
	CAST_PTR(N, n, pN);
	
	if (!isBool(exp)) {
		throw errorMismatchException();
	}
	auto* p = new Statement();
	registerNode(p);

	// backpatch the truelist of exp to go into the statement
	codeBuffer.bpatch(exp->truelist, m->label);
	// set the parent's Statement a nextlist attribute
	p->nextlist = codeBuffer.merge(statement->nextlist, exp->falselist); 
	p->nextlist = codeBuffer.merge(p->nextlist, n->nextlist);

	p->breakList = statement->breakList;
	p->continueList = statement->continueList;
	return p;
}

Statement* statementWhile(Node* pN, Node* pM1, Node* pExp, Node* pM2, Node* pStatement, Node* pN2) {
	CAST_PTR(Exp, exp, pExp);
	CAST_PTR(M, m1, pM1);
	CAST_PTR(M, m2, pM2);
	CAST_PTR(Statement, statement, pStatement);
	CAST_PTR(N, n1, pN);
	CAST_PTR(N, n2, pN2);
	
	if (!isBool(exp)) {
		throw errorMismatchException();
	}
	auto* p = new Statement();
	registerNode(p);

	string label_1 = m1->label; // Exp evaluation
	string label_2 = m2->label;	// Exp is true - begin of statements

	string label_3 = codeBuffer.genLabel(); // End of statements - re-evaluate the expression
	codeBuffer.emit("br label %" + label_1);

	string label_4 = codeBuffer.genLabel(); // statements outside of loop
	codeBuffer.bpatch(n1->nextlist, label_1);
	codeBuffer.bpatch(n2->nextlist, label_3);
	// backpatch the truelist of exp to go into the statement
	codeBuffer.bpatch(exp->falselist, label_4); 
	codeBuffer.bpatch(exp->truelist, label_2);
	// backpatch the break statements to end of while loop and continue statements back to bool exp evaluation
	codeBuffer.bpatch(statement->continueList, label_1);
	codeBuffer.bpatch(statement->breakList, label_4);
	codeBuffer.bpatch(statement->nextlist, label_3);
	
	return p;
}

Statement* statementBreak() {
	// TODO: implement
	if (!inWhile()) {
		throw errorUnexpectedBreakException();
	}
	auto* p = new Statement();
	int buffer_index = codeBuffer.emit("br label @");
	BackpatchList breakList = codeBuffer.makelist({buffer_index, FIRST});
	p->breakList = breakList;
	registerNode(p);
	return p;
}

Statement* statementContinue() {
	// TODO:
	if (!inWhile()) {
		throw errorUnexpectedContinueException();
	}
	int buffer_index = codeBuffer.emit("br label @");
	BackpatchList continueList = codeBuffer.makelist({buffer_index, FIRST});
	auto* p = new Statement();
	registerNode(p);
	p->continueList = continueList;
	return p;
}

//====================== Statements Rules =====================

Statements* statementsEnd(Node* pNode) {
	CAST_PTR(Statement, pStatement, pNode);

	auto* p = new Statements();
	registerNode(p);

	p->nextlist = pStatement->nextlist;
	p->continueList = pStatement->continueList;
	p->breakList = pStatement->breakList;
	return p;
}


Statements* statementsLeftRec(Node* pNode1, Node* pNode2) {
	CAST_PTR(Statements, pStatements, pNode1);
	CAST_PTR(Statement, pStatement, pNode2);

	auto* p = new Statements();
	registerNode(p);

	// if pStatements had a nextlist it sould have already been bapatched in backpatchStatements()
	p->nextlist = pStatement->nextlist;
	p->continueList = codeBuffer.merge(pStatement->continueList, pStatements->continueList);
	p->breakList = codeBuffer.merge(pStatement->breakList, pStatements->breakList);
	return p;
}

//====================== Program Rules ========================

Program* program(Node* pNode) {
	CAST_PTR(Funcs, pFuncs, pNode);

	endCompilation();

	auto* p = new Program();
	registerNode(p);
	return p;
}

//====================== Funcs Rules ========================

Funcs* funcs() {
	auto* p = new Funcs();
	registerNode(p);
	return p;
}

Funcs* funcsRightRec(Node* pNode1, Node* pNode2) {
	CAST_PTR(FuncDecl, pFuncDecl, pNode1);
	CAST_PTR(Funcs, pFuncs, pNode2);

	auto* p = new Funcs();
	registerNode(p);
	return p;
}

//====================== FuncDecl Rules ========================

FuncDecl* funcDecl(Node* pNode1, Node* pNode2, Node* pNode3, Node* pNode4) {
	CAST_PTR(RetType, pRetType, pNode1);
	CAST_PTR(Id, pId, pNode2);
	CAST_PTR(Formals, pFormals, pNode3);
	CAST_PTR(Statements, pStatements, pNode4);

	emitFuncEnd(pRetType->getType());
	auto* p = new FuncDecl((RetType*)pRetType, (Id*) pId, (Formals*)pFormals);
	registerNode(p);
	return p;
}

//====================== RetType Rules ========================

RetType* retType(Node* pNode) {
	CAST_PTR(Type, pType, pNode);

	auto* p = new RetType((Type*)pType);
	registerNode(p);
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

Formals* formals(Node* pNode) {
	CAST_PTR(FormalsList, pFormalsList, pNode);

	auto* p = new Formals(pFormalsList);
	registerNode(p);
	return p;
}

//====================== FormalsList Rules ========================

FormalsList* formalsList(Node* pNode) {
	CAST_PTR(FormalDecl, pFormalDecl, pNode);

	auto* p = new FormalsList(pFormalDecl);
	registerNode(p);
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
	registerNode(p);
	return p;
}

//====================== FormalDecl Rules ========================
FormalDecl* formalDecl(Node* pNode1, Node* pNode2) {
	CAST_PTR(Type, pType, pNode1);
	CAST_PTR(Id, pId, pNode2);

	auto* p = new FormalDecl(pType, pId);
	registerNode(p);
	return p;
}

//=========================Markers================================
M* m() {
	auto p = new M();
	registerNode(p);
	p->label = codeBuffer.genLabel();
	return p;
}

N* n() {
	auto p = new N();
	registerNode(p);
	int buffer_index = codeBuffer.emit("br label @");
	p->nextlist = codeBuffer.makelist({ buffer_index, FIRST });
	return p;
}

void backpatchStatements(Node* pStatements) {
	CAST_PTR(Statements, statements, pStatements);
	if (statements->nextlist.size() > 0) {
		string label = codeBuffer.genLabel();
		codeBuffer.bpatch(statements->nextlist, label);
	}
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

void exitFunc(Node* pStatements) {
	CAST_PTR(Statements, statements, pStatements);
	
	if (statements->nextlist.size() > 0) {
		string label = codeBuffer.genLabel();
		codeBuffer.bpatch(statements->nextlist, label);
	}
	TypeN type = getCurrFuncType();
	switch (type)
	{
	case TypeN::VOID:
		codeBuffer.emit("ret void");
		break;
	case TypeN::BOOL:
		codeBuffer.emit("ret i1 0");
		break;
	case TypeN::INT:
		codeBuffer.emit("ret i32 0");
		break;
	case TypeN::BYTE:
		codeBuffer.emit("ret i32 0");
		break;
	default:
		assert(false); // sould not get here
		break;
	}
	
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
string to_llvm_retType(TypeN type) {
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
	codeBuffer.emit("%stack = alloca [50 x i32]");
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

string loadValueToReg (string address_reg) {
	std::stringstream load_statement;
	string value_reg = newTemp();
	load_statement << value_reg;
	load_statement << " = load i32, i32* ";
	load_statement << address_reg;
	codeBuffer.emit(load_statement.str());
	return value_reg;
}

string emitFunctionCall(TypeN retType, string id, vector<Exp*>& recieved_args) {
	std::stringstream code;
	std::stringstream args_list;
	code << "call " << to_llvm_retType(retType) << "@" << id << " (";
	bool skip_comma = true;
	for (Exp* p : recieved_args) {
		if (!skip_comma) {
			args_list << ", ";
		}
		if (p->getType() == TypeN::STRING) {
			//Single argument - pointer to global string varible
			args_list << "i8* getelementptr([" << p->n_bytes << " x i8], [" << p->n_bytes << " x i8]* " << p->place <<", i32 0, i32 0)";
		} else {
			args_list << "i32 " << p->place;
		}
		skip_comma = false;
	}	
	code << args_list.str();
	code << ")";
	return code.str();
}
	