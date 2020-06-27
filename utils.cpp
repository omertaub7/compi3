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

//======================= static functions ================
static string to_llvm_type(TypeN type) {
	switch (type)
	{
	case TypeN::VOID:
		return "void";
		break;
	case TypeN::BOOL:
		return "i1";
		break;
	case TypeN::INT:
		//break;
	case TypeN::BYTE:
		return "i32";
		break;
	case TypeN::STRING:
		// break;
	default:
		assert(false); // should not get here
		break;
	}
	return "";
}

// creates a branch comand on a condition(cond), with false and true lists of the conditional expression
static void brOnCond(string cond, Exp* boolExp) {
	assert(isBool(boolExp));
	int buffer_index = codeBuffer.emit("br i1 " + cond + ", label @, label @");
	boolExp->truelist = codeBuffer.makelist({ buffer_index, FIRST });
	boolExp->falselist = codeBuffer.makelist({ buffer_index, SECOND });
}

// evaluates a bool expression to a register, made for 
// sets place attribute the expression's value
static void evaluateBoolExp(Exp* boolExp, TypeN output_type = TypeN::BOOL) {
	assert(isBool(boolExp));
	// TODO: avoid double evaluating. if it's place is not empty then it might be already evaluated
	string assign_label = newLabel();
	string br_to_assign = "br label %" + assign_label;

	string false_label = codeBuffer.genLabel();
	codeBuffer.emit(br_to_assign);
	string true_label = codeBuffer.genLabel();
	codeBuffer.emit(br_to_assign);

	codeBuffer.emit(assign_label + ":");
	boolExp->place = newTemp();
	string type = to_llvm_type(output_type);
	string phi_code = boolExp->place + " = phi " + type + " [ 0, %" + false_label + " ], [ 1, %" + true_label + " ]";
	codeBuffer.emit(phi_code);

	codeBuffer.bpatch(boolExp->truelist, true_label);
	codeBuffer.bpatch(boolExp->falselist, false_label);
}

// stores a value of exp into id, assumes that they are from the correct types and that id is a variable that 
// exists
static void storeVarFromExp(Id* id, Exp* exp) {
	// in case of boolean variables, we need to branch to assign 1 or 0
	// and to convert them to i32 to put in stack
	if (isBool(exp)) {
		evaluateBoolExp(exp, TypeN::INT);
	}
	string ptr = getVarPtr(id);
	codeBuffer.emit("store i32 " + exp->place + ", i32* " + ptr);
}

// creates a string to call a function in llvm
static string functionCall(Id* id) {
	assert(id);
	TypeN ret_type = getFuncType(id);

	return "call " + to_llvm_type(ret_type) + "@" + id->getName() + " ()";
}

static string functionCall(Id* id, vector<Exp*>& recieved_args) {
	assert(id);
	TypeN ret_type = getFuncType(id);
	stringstream call_code;
	
	call_code << "call " + to_llvm_type(ret_type) + "@" + id->getName() + " ( ";
	bool comma = false;
	for (Exp* p : recieved_args) {
		if (comma) {
			call_code << ", ";
		}
		if (p->getType() == TypeN::STRING) {
			call_code << "i8* getelementptr([" << p->n_bytes << " x i8], [" << p->n_bytes << " x i8]* " << p->place << ", i32 0, i32 0)";
		}
		else {
			call_code << to_llvm_type(p->getType()) <<" " << p->place;
		}
		comma = true;
	}
	call_code << ")";
	return call_code.str();
}

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
	// zero divide
	if (op == BinOp::DIV) {
		codeBuffer.emit("call void @check_zero_div(i32 " + exp2->place + ")");
	}
	if (type == TypeN::INT) {
		codeBuffer.emit(p->place + " = " + opcode + " i32 " + exp1->place + ", " + exp2->place);
	}
	// deal with overflow in the case of byte
	else {
		string temp = newTemp();
		codeBuffer.emit(temp + " = " + opcode + " i32 " + exp1->place + ", " + exp2->place);
		// truncate
		codeBuffer.emit(p->place + " = and i32 255, " + temp);
	}
	return p; 
}

Exp* expFromId(Node* pId) {
	CAST_PTR(Id, id, pId);

	TypeN type = getIdType(id);
	int offset = symbolTable->getVaribleOffset(id->getName());
	
	auto* p = new Exp(type);
	registerNode(p);

	// in function arguments
	if (offset < 0) {
		int arg_number = (offset * -1) - 1;
		p->place = "%" + to_string(arg_number);
	}
	// in stack
	else {
		p->place = newTemp();
		string ptr = getVarPtr(offset);
		codeBuffer.emit(p->place + " = load i32, i32* " + ptr);
		// if needs to convert to i1
		if (type == TypeN::BOOL) {
			string temp = newTemp();
			codeBuffer.emit(temp + " = trunc i32 " + p->place + " to i1");
			p->place = temp;
		}
	}
	// make the true and false list, and branch
	if (type == TypeN::BOOL) {
		brOnCond(p->place, p);
	}
	
	return p;
}

Exp* expFromCall(Node* pCall) {
	CAST_PTR(Call, call, pCall);
	stringstream code;

	auto* p = new Exp(call->getType(), newTemp());
	registerNode(p);

	codeBuffer.emit(p->place + " = " + call->function_call_string);

	// if it is bool then we want to add to it a true and false list
	if (isBool(p)) {
		brOnCond(p->place, p);
	}
	
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

	int buffer_index = codeBuffer.emit("br label @");
	if (b) {
		p->truelist = codeBuffer.makelist({ buffer_index, FIRST });
	}
	else {
		p->falselist = codeBuffer.makelist({ buffer_index, FIRST });
	}

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
	brOnCond(temp, p);

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

ExpList* expListFromExp(Node* pExp) {
	CAST_PTR(Exp, exp, pExp);

	auto* p = new ExpList(exp);
	registerNode(p);

	return p;
}

ExpList* expListRightRec(Node* pExp, Node* pExpList) {
	CAST_PTR(Exp, exp, pExp);
	CAST_PTR(ExpList, expList, pExpList);

	auto* p = new ExpList(exp, expList);
	registerNode(p);

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

	p->function_call_string = functionCall(id, recievedExpressions);
	

	registerNode(p);
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

	p->function_call_string = functionCall(id);
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
	CAST_PTR(Type, pType, pNode1);
	CAST_PTR(Id, pId, pNode2);

	insertNewVar(pType, pId);
	auto* p = new Statement();
	registerNode(p);
	// initialize the new var to 0
	string ptr = getVarPtr(pId);
	codeBuffer.emit("store i32 0, i32* " + ptr);

	return p;
}

Statement* statementVarDeclInit(Node* pNode1, Node* pNode2, Node* pNode3) {
	CAST_PTR(Type, pType, pNode1);
	CAST_PTR(Id, pId, pNode2);
	CAST_PTR(Exp, pExp, pNode3);
	
	if (! checkAssign(pType->getType(), pExp->getType())) {
		throw errorMismatchException();
	}
	insertNewVar(pType, pId);
	auto* p = new Statement();
	registerNode(p);

	storeVarFromExp(pId, pExp);
	
	return p;
}

Statement* statementAssign(Node* pNode1, Node* pNode2) {
	CAST_PTR(Id, pId, pNode1);
	CAST_PTR(Exp, pExp, pNode2);
	
	if (!symbolTable->checkVarExists(pId->getName())) {
		throw errorUndefException(pId->getName());
	}
	if (! checkAssign(getIdType(pId), pExp->getType())) {
		throw errorMismatchException();
	}
	auto* p = new Statement();
	registerNode(p);

	// check if it is a parameter, if so, then remove it from the parameters list and copy its value to the stack
	int offset = symbolTable->getVaribleOffset(pId->getName());
	if (offset < 0) {
		symbolTable->make_parameter_writeable(pId->getName());
	}

	storeVarFromExp(pId, pExp);
	
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
	CAST_PTR(Exp, pExp, pNode);

	if (! checkAssign(getCurrFuncType(), pExp->getType())) {
		throw errorMismatchException();
	}
	auto* p = new Statement();
	registerNode(p);

	// bool need to do branch trick
	if (isBool(pExp)) {
		string true_label = codeBuffer.genLabel();
		codeBuffer.emit("ret i1 1");

		string false_label = codeBuffer.genLabel();
		codeBuffer.emit("ret i1 0");
		
		codeBuffer.bpatch(pExp->falselist, false_label);
		codeBuffer.bpatch(pExp->truelist, true_label);
	}
	else {
		stringstream code;
		code << "ret i32 " << pExp->place;
		codeBuffer.emit(code.str());
	}
	
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
	CAST_PTR(RetType, ret_type, pNode1);
	CAST_PTR(Id, id, pNode2);
	CAST_PTR(Formals, formals, pNode3);
	CAST_PTR(Statements, statements, pNode4);

	// check if there are any loose edges that need to be backpatched
	if (statements->nextlist.size() > 0) {
		string label = codeBuffer.genLabel();
		codeBuffer.bpatch(statements->nextlist, label);
	}
	switch (ret_type->getType()) {
	case TypeN::VOID:
		codeBuffer.emit("ret void");
		break;
	case TypeN::BOOL:
		codeBuffer.emit("ret i1 0");
		break;
	case TypeN::BYTE:
	case TypeN::INT:
		codeBuffer.emit("ret i32 0");
		break;
	default:
		assert(false); // should not get here
	}

	exitScope();
	codeBuffer.emit("}");

	auto* p = new FuncDecl(ret_type, id, formals);
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

//===================== function arguments evaluation =============

void evaluateExp(Node* pExp) {
	CAST_PTR(Exp, exp, pExp);
	if (isBool(exp)) {
		evaluateBoolExp(exp);
	}
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
	// divide by zero
	codeBuffer.emit("@.zero_divide_msg = constant [24 x i8] c\"Error division by zero\\0A\\00\"");
	codeBuffer.emit("define void @check_zero_div(i32) {");
	codeBuffer.emit("%x = icmp eq i32 0, %0");
	codeBuffer.emit("br i1 %x, label %zero, label %no_zero");
	codeBuffer.emit("zero:");
	codeBuffer.emit("call i32 (i8*, ...) @printf(i8* getelementptr ([24 x i8], [24 x i8]* @.zero_divide_msg, i32 0, i32 0))");
	codeBuffer.emit("call void @exit(i32 1)");
	codeBuffer.emit("br label %no_zero");
	codeBuffer.emit("no_zero:");
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
	bool first = true;
	for (auto p : f->argTypes) {
		if (!first) {
			code << ", ";
		}
		first = false;
		code << to_llvm_retType(p.second);
	}
	code << ") ";
	code << "{";
	codeBuffer.emit(code.str());
	//allocate local stack
	codeBuffer.emit("%stack = alloca [50 x i32]");
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
	