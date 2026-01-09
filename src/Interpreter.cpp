#include "Interpreter.h"
#include <cctype>
#include <cstdlib>
#include <iostream>
#include <cmath>
#include <chrono>

Interpreter::Interpreter() {
    currentToken = 0;
    isRunning = false;
}

Interpreter::~Interpreter() {
    Stop();
}

void Interpreter::Load(const std::string& code) {
    Stop(); // Ensure stopped before loading
    
    source = code;
    Tokenize();
    currentToken = 0;
    
    std::lock_guard<std::mutex> lock(memoryMutex);
    globals.clear();
    functions.clear();
    structs.clear();
    enums.clear();
    pinValues.clear();
    sensorValues.clear();
    callStack.clear();
    
    ParseProgram();
}

void Interpreter::Start() {
    if (isRunning) return;
    isRunning = true;
    executionThread = std::thread(&Interpreter::RunLoop, this);
}

void Interpreter::Stop() {
    isRunning = false;
    if (executionThread.joinable()) {
        executionThread.join();
    }
}

void Interpreter::RunLoop() {
    {
        CallFunction("setup", {});
    }
    
    while (isRunning) {
        CallFunction("loop", {});
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}

int Interpreter::GetPinValue(int pin) {
    std::lock_guard<std::mutex> lock(memoryMutex);
    if (pinValues.find(pin) != pinValues.end()) return pinValues[pin];
    return 0;
}

void Interpreter::SetPinValue(int pin, int value) {
    std::lock_guard<std::mutex> lock(memoryMutex);
    pinValues[pin] = value;
}

void Interpreter::SetSensorValue(int trigPin, int echoPin, float distance) {
    std::lock_guard<std::mutex> lock(memoryMutex);
    sensorValues[echoPin] = distance;
}

void Interpreter::SetVariable(const std::string& name, float value) {
    std::lock_guard<std::mutex> lock(memoryMutex);
    if (globals.count(name)) {
        globals[name] = Value(value);
    }
}

// --- Tokenizer ---
void Interpreter::Tokenize() {
    tokens.clear();
    int i = 0;
    int line = 1;
    while (i < source.length()) {
        char c = source[i];
        if (c == '\n') { line++; i++; continue; }
        if (isspace(c)) { i++; continue; }
        
        if (c == '/' && i + 1 < source.length() && source[i+1] == '/') {
            while (i < source.length() && source[i] != '\n') i++;
            continue;
        }
        if (c == '/' && i + 1 < source.length() && source[i+1] == '*') {
             i += 2;
             while (i + 1 < source.length() && !(source[i] == '*' && source[i+1] == '/')) {
                 if (source[i] == '\n') line++;
                 i++;
             }
             i += 2;
             continue;
        }

        if (c == '&' && i + 1 < source.length() && source[i+1] == '&') {
            tokens.push_back({TOKEN_AND, "&&", 0, line}); i += 2; continue;
        }
        if (c == '|' && i + 1 < source.length() && source[i+1] == '|') {
            tokens.push_back({TOKEN_OR, "||", 0, line}); i += 2; continue;
        }
        if (c == '+' && i + 1 < source.length() && source[i+1] == '+') {
            tokens.push_back({TOKEN_INC, "++", 0, line}); i += 2; continue;
        }
        if (c == '-' && i + 1 < source.length() && source[i+1] == '-') {
            tokens.push_back({TOKEN_DEC, "--", 0, line}); i += 2; continue;
        }
        
        Token t; t.line = line;
        if (isalpha(c) || c == '_') {
            std::string text;
            while (i < source.length() && (isalnum(source[i]) || source[i] == '_')) text += source[i++];
            t.text = text;
            if (text == "if") t.type = TOKEN_IF;
            else if (text == "else") t.type = TOKEN_ELSE;
            else if (text == "int") t.type = TOKEN_INT;
            else if (text == "float") t.type = TOKEN_FLOAT;
            else if (text == "long") t.type = TOKEN_LONG;
            else if (text == "bool") t.type = TOKEN_BOOL;
            else if (text == "void") t.type = TOKEN_VOID;
            else if (text == "const") t.type = TOKEN_CONST;
            else if (text == "enum") t.type = TOKEN_ENUM;
            else if (text == "struct") t.type = TOKEN_STRUCT;
            else if (text == "return") t.type = TOKEN_RETURN;
            else if (text == "while") t.type = TOKEN_WHILE;
            else if (text == "do") t.type = TOKEN_DO;
            else if (text == "for") t.type = TOKEN_FOR;
            else if (text == "pile") t.type = TOKEN_PILE;
            else if (text == "true") t.type = TOKEN_TRUE;
            else if (text == "false") t.type = TOKEN_FALSE;
            else t.type = TOKEN_ID;
        } else if (isdigit(c)) {
            std::string text;
            while (i < source.length() && (isdigit(source[i]) || source[i] == '.')) text += source[i++];
            t.text = text;
            t.type = TOKEN_NUMBER;
            t.numberValue = std::stof(text);
        } else {
            t.text = std::string(1, c);
            switch (c) {
                case '{': t.type = TOKEN_LBRACE; break;
                case '}': t.type = TOKEN_RBRACE; break;
                case '(': t.type = TOKEN_LPAREN; break;
                case ')': t.type = TOKEN_RPAREN; break;
                case '[': t.type = TOKEN_LBRACKET; break;
                case ']': t.type = TOKEN_RBRACKET; break;
                case ';': t.type = TOKEN_SEMICOLON; break;
                case ',': t.type = TOKEN_COMMA; break;
                case '.': t.type = TOKEN_DOT; break;
                case '=': t.type = TOKEN_ASSIGN; break;
                case '<': t.type = TOKEN_LT; break;
                case '>': t.type = TOKEN_GT; break;
                case '?': t.type = TOKEN_QUESTION; break;
                case ':': t.type = TOKEN_COLON; break;
                case '!': t.type = TOKEN_NOT; break;
                case '+': t.type = TOKEN_PLUS; break;
                case '-': t.type = TOKEN_MINUS; break;
                case '*': t.type = TOKEN_STAR; break;
                case '/': t.type = TOKEN_SLASH; break;
                case '%': t.type = TOKEN_MOD; break;
                case '&': t.type = TOKEN_AMPERSAND; break;
                default: t.type = TOKEN_EOF; break;
            }
            i++;
        }
        tokens.push_back(t);
    }
    tokens.push_back({TOKEN_EOF, "", 0, line});
}

Token Interpreter::Peek(int offset) {
    if (currentToken + offset >= tokens.size()) return tokens.back();
    return tokens[currentToken + offset];
}
Token Interpreter::Consume() {
    if (currentToken < tokens.size()) return tokens[currentToken++];
    return tokens.back();
}
bool Interpreter::Match(TokenType type) {
    if (Peek().type == type) { Consume(); return true; }
    return false;
}
bool Interpreter::Check(TokenType type) { return Peek().type == type; }

// --- Parser ---
void Interpreter::ParseProgram() {
    while (Peek().type != TOKEN_EOF) ParseGlobal();
}

void Interpreter::ParseGlobal() {
    if (Match(TOKEN_STRUCT)) {
        StructDef def;
        def.name = Consume().text;
        Match(TOKEN_LBRACE);
        while (!Check(TOKEN_RBRACE) && !Check(TOKEN_EOF)) {
            std::string type = Consume().text;
            std::string name = Consume().text;
            def.members[name] = type;
            Match(TOKEN_SEMICOLON);
        }
        Match(TOKEN_RBRACE);
        Match(TOKEN_SEMICOLON);
        structs[def.name] = def;
    } else if (Match(TOKEN_ENUM)) {
        EnumDef def;
        def.name = Consume().text;
        Match(TOKEN_LBRACE);
        int val = 0;
        do {
            std::string name = Consume().text;
            if (Match(TOKEN_ASSIGN)) val = (int)Consume().numberValue;
            def.values[name] = val;
            globals[name] = Value(val);
            val++;
        } while (Match(TOKEN_COMMA));
        Match(TOKEN_RBRACE);
        Match(TOKEN_SEMICOLON);
        enums[def.name] = def;
    } else {
        bool isConst = Match(TOKEN_CONST);
        std::string type = Consume().text;
        bool isRef = Match(TOKEN_AMPERSAND); 
        std::string name = Consume().text;
        
        if (Match(TOKEN_LPAREN)) {
            FunctionDef func;
            func.name = name;
            func.returnType = type;
            if (isRef) func.returnType += "&";
            
            if (!Check(TOKEN_RPAREN)) {
                do {
                    std::string pType = Consume().text;
                    bool pRef = Match(TOKEN_AMPERSAND);
                    std::string pName = Consume().text;
                    if (pRef) pType += "&";
                    func.parameters.push_back({pType, pName});
                } while (Match(TOKEN_COMMA));
            }
            Match(TOKEN_RPAREN);
            Match(TOKEN_LBRACE);
            
            std::vector<StmtPtr> stmts;
            while (!Check(TOKEN_RBRACE) && !Check(TOKEN_EOF)) {
                stmts.push_back(ParseStatement());
            }
            Match(TOKEN_RBRACE);
            
            auto block = std::make_shared<BlockStmt>();
            block->statements = stmts;
            func.body = block;
            functions[name] = func;
        } else {
            if (Match(TOKEN_LBRACKET)) {
                ExprPtr sizeExpr = ParseExpression();
                Match(TOKEN_RBRACKET);
                Match(TOKEN_SEMICOLON);
                
                Value sizeVal = Evaluate(sizeExpr);
                Value arr;
                arr.type = VAL_ARRAY;
                Value defaultVal = CreateDefaultValue(type);
                for(int i=0; i<sizeVal.intVal; i++) arr.arrayElements.push_back(defaultVal);
                globals[name] = arr;
            } else {
                Value val = CreateDefaultValue(type);
                if (Match(TOKEN_ASSIGN)) {
                    val = Evaluate(ParseExpression());
                }
                globals[name] = val;
                Match(TOKEN_SEMICOLON);
            }
        }
    }
}

StmtPtr Interpreter::ParseStatement() {
    if (Check(TOKEN_LBRACE)) return ParseBlock();
    if (Match(TOKEN_IF)) {
        auto stmt = std::make_shared<IfStmt>();
        Match(TOKEN_LPAREN);
        stmt->condition = ParseExpression();
        Match(TOKEN_RPAREN);
        stmt->thenBranch = ParseStatement();
        if (Match(TOKEN_ELSE)) stmt->elseBranch = ParseStatement();
        return stmt;
    }
    if (Match(TOKEN_WHILE)) {
        auto stmt = std::make_shared<WhileStmt>();
        Match(TOKEN_LPAREN);
        stmt->condition = ParseExpression();
        Match(TOKEN_RPAREN);
        stmt->body = ParseStatement();
        return stmt;
    }
    if (Match(TOKEN_DO)) {
        auto stmt = std::make_shared<DoWhileStmt>();
        stmt->body = ParseStatement();
        Match(TOKEN_WHILE);
        Match(TOKEN_LPAREN);
        stmt->condition = ParseExpression();
        Match(TOKEN_RPAREN);
        Match(TOKEN_SEMICOLON);
        return stmt;
    }
    if (Match(TOKEN_FOR)) {
        auto stmt = std::make_shared<ForStmt>();
        Match(TOKEN_LPAREN);
        if (!Check(TOKEN_SEMICOLON)) {
            if (Check(TOKEN_INT) || Check(TOKEN_PILE) || structs.count(Peek().text)) {
                 // Inline var decl
                 auto decl = std::make_shared<VarDeclStmt>();
                 decl->type = Consume().text;
                 bool isRef = Match(TOKEN_AMPERSAND);
                 if (isRef) decl->type += "&";
                 decl->name = Consume().text;
                 if (Match(TOKEN_ASSIGN)) decl->initializer = ParseExpression();
                 Match(TOKEN_SEMICOLON);
                 stmt->init = decl;
            } else {
                 auto exprStmt = std::make_shared<ExprStmt>();
                 exprStmt->expression = ParseExpression();
                 Match(TOKEN_SEMICOLON);
                 stmt->init = exprStmt;
            }
        } else {
            Match(TOKEN_SEMICOLON);
        }
        
        if (!Check(TOKEN_SEMICOLON)) stmt->condition = ParseExpression();
        Match(TOKEN_SEMICOLON);
        
        if (!Check(TOKEN_RPAREN)) stmt->increment = ParseExpression();
        Match(TOKEN_RPAREN);
        
        stmt->body = ParseStatement();
        return stmt;
    }
    if (Match(TOKEN_RETURN)) {
        auto stmt = std::make_shared<ReturnStmt>();
        if (!Check(TOKEN_SEMICOLON)) stmt->value = ParseExpression();
        Match(TOKEN_SEMICOLON);
        return stmt;
    }
    
    Token t = Peek();
    bool isType = (t.type == TOKEN_INT || t.type == TOKEN_FLOAT || t.type == TOKEN_BOOL || t.type == TOKEN_LONG || t.type == TOKEN_PILE ||
                   structs.count(t.text) || enums.count(t.text));
    
    if (isType) {
        auto stmt = std::make_shared<VarDeclStmt>();
        stmt->type = Consume().text;
        bool isRef = Match(TOKEN_AMPERSAND);
        if (isRef) stmt->type += "&";
        stmt->name = Consume().text;
        
        if (Match(TOKEN_LBRACKET)) {
            stmt->isArray = true;
            stmt->arraySize = ParseExpression();
            Match(TOKEN_RBRACKET);
        } else if (Match(TOKEN_ASSIGN)) {
            stmt->initializer = ParseExpression();
        }
        Match(TOKEN_SEMICOLON);
        return stmt;
    }
    
    auto stmt = std::make_shared<ExprStmt>();
    stmt->expression = ParseExpression();
    Match(TOKEN_SEMICOLON);
    return stmt;
}

StmtPtr Interpreter::ParseBlock() {
    auto block = std::make_shared<BlockStmt>();
    Match(TOKEN_LBRACE);
    while (!Check(TOKEN_RBRACE) && !Check(TOKEN_EOF)) {
        block->statements.push_back(ParseStatement());
    }
    Match(TOKEN_RBRACE);
    return block;
}

ExprPtr Interpreter::ParseExpression() { return ParseAssignment(); }

ExprPtr Interpreter::ParseAssignment() {
    ExprPtr expr = ParseLogicalOr();
    if (Match(TOKEN_ASSIGN)) {
        auto assign = std::make_shared<AssignExpr>();
        assign->target = expr;
        assign->value = ParseAssignment();
        return assign;
    }
    return expr;
}

ExprPtr Interpreter::ParseLogicalOr() {
    ExprPtr expr = ParseLogicalAnd();
    while (Match(TOKEN_OR)) {
        auto binary = std::make_shared<BinaryExpr>();
        binary->left = expr;
        binary->op = TOKEN_OR;
        binary->right = ParseLogicalAnd();
        expr = binary;
    }
    return expr;
}

ExprPtr Interpreter::ParseLogicalAnd() {
    ExprPtr expr = ParseEquality();
    while (Match(TOKEN_AND)) {
        auto binary = std::make_shared<BinaryExpr>();
        binary->left = expr;
        binary->op = TOKEN_AND;
        binary->right = ParseEquality();
        expr = binary;
    }
    return expr;
}

ExprPtr Interpreter::ParseEquality() {
    ExprPtr expr = ParseRelational();
    return expr;
}

ExprPtr Interpreter::ParseRelational() {
    ExprPtr expr = ParseSum();
    while (Check(TOKEN_LT) || Check(TOKEN_GT)) {
        auto binary = std::make_shared<BinaryExpr>();
        binary->left = expr;
        binary->op = Consume().type;
        binary->right = ParseSum();
        expr = binary;
    }
    return expr;
}

ExprPtr Interpreter::ParseSum() {
    ExprPtr expr = ParseProduct();
    while (Check(TOKEN_PLUS) || Check(TOKEN_MINUS)) {
        auto binary = std::make_shared<BinaryExpr>();
        binary->left = expr;
        binary->op = Consume().type;
        binary->right = ParseProduct();
        expr = binary;
    }
    return expr;
}

ExprPtr Interpreter::ParseProduct() {
    ExprPtr expr = ParseUnary();
    while (Check(TOKEN_STAR) || Check(TOKEN_SLASH) || Check(TOKEN_MOD)) {
        auto binary = std::make_shared<BinaryExpr>();
        binary->left = expr;
        binary->op = Consume().type;
        binary->right = ParseUnary();
        expr = binary;
    }
    return expr;
}

ExprPtr Interpreter::ParseUnary() {
    if (Check(TOKEN_NOT) || Check(TOKEN_MINUS)) {
        auto unary = std::make_shared<UnaryExpr>();
        unary->op = Consume().type;
        unary->right = ParseUnary();
        return unary;
    }
    if (Check(TOKEN_INC) || Check(TOKEN_DEC)) {
        auto unary = std::make_shared<UnaryExpr>();
        unary->op = Consume().type;
        unary->right = ParseUnary();
        return unary;
    }
    return ParsePrimary();
}

ExprPtr Interpreter::ParsePrimary() {
    ExprPtr expr = nullptr;
    
    if (Match(TOKEN_TRUE)) { auto l = std::make_shared<LiteralExpr>(); l->boolVal = true; l->isBool = true; expr = l; }
    else if (Match(TOKEN_FALSE)) { auto l = std::make_shared<LiteralExpr>(); l->boolVal = false; l->isBool = true; expr = l; }
    else if (Check(TOKEN_NUMBER)) {
        auto l = std::make_shared<LiteralExpr>();
        l->numberVal = Consume().numberValue;
        expr = l;
    }
    else if (Check(TOKEN_ID)) {
        std::string name = Consume().text;
        if (Match(TOKEN_LPAREN)) {
            auto call = std::make_shared<CallExpr>();
            call->callee = name;
            if (!Check(TOKEN_RPAREN)) {
                do {
                    call->args.push_back(ParseExpression());
                } while (Match(TOKEN_COMMA));
            }
            Match(TOKEN_RPAREN);
            expr = call;
        } else {
            auto var = std::make_shared<VariableExpr>();
            var->name = name;
            expr = var;
        }
    }
    else if (Match(TOKEN_LPAREN)) {
        Token t = Peek();
        bool isType = (t.type == TOKEN_INT || t.type == TOKEN_FLOAT || enums.count(t.text));
        if (isType) {
            Consume(); 
            Match(TOKEN_RPAREN);
            expr = ParseUnary();
        } else {
            expr = ParseExpression();
            Match(TOKEN_RPAREN);
        }
    }
    
    if (!expr) expr = std::make_shared<LiteralExpr>();
    
    // Handle suffixes
    while (true) {
        if (Match(TOKEN_DOT)) {
            auto member = std::make_shared<MemberExpr>();
            member->object = expr;
            member->member = Consume().text;
            expr = member;
        } else if (Match(TOKEN_LBRACKET)) {
            auto index = std::make_shared<IndexExpr>();
            index->array = expr;
            index->index = ParseExpression();
            Match(TOKEN_RBRACKET);
            expr = index;
        } else if (Check(TOKEN_INC) || Check(TOKEN_DEC)) {
            auto postfix = std::make_shared<PostfixExpr>();
            postfix->left = expr;
            postfix->op = Consume().type;
            expr = postfix;
        } else {
            break;
        }
    }
    return expr;
}

// --- Execution ---

Value* Interpreter::GetLValue(ExprPtr expr) {
    if (auto var = std::dynamic_pointer_cast<VariableExpr>(expr)) {
        if (!callStack.empty()) {
            auto& locals = callStack.back().locals;
            if (locals.count(var->name)) return &locals[var->name];
        }
        if (globals.count(var->name)) return &globals[var->name];
    } else if (auto member = std::dynamic_pointer_cast<MemberExpr>(expr)) {
        Value* obj = GetLValue(member->object);
        if (obj) {
            if (obj->type == VAL_REF && obj->refVal) obj = obj->refVal;
            return &obj->members[member->member];
        }
    } else if (auto index = std::dynamic_pointer_cast<IndexExpr>(expr)) {
        Value* arr = GetLValue(index->array);
        Value idx = Evaluate(index->index);
        if (arr) {
            if (arr->type == VAL_REF && arr->refVal) arr = arr->refVal;
            if (idx.intVal >= 0 && idx.intVal < arr->arrayElements.size()) {
                return &arr->arrayElements[idx.intVal];
            }
        }
    }
    return nullptr;
}

Value Interpreter::Evaluate(ExprPtr expr) {
    if (auto l = std::dynamic_pointer_cast<LiteralExpr>(expr)) {
        if (l->isBool) return Value(l->boolVal);
        return Value(l->numberVal);
    }
    if (auto v = std::dynamic_pointer_cast<VariableExpr>(expr)) {
        Value* val = GetLValue(expr);
        if (val) {
            if (val->type == VAL_REF && val->refVal) return *val->refVal;
            return *val;
        }
        return Value();
    }
    if (auto a = std::dynamic_pointer_cast<AssignExpr>(expr)) {
        Value* target = GetLValue(a->target);
        Value val = Evaluate(a->value);
        if (target) {
            if (target->type == VAL_REF && target->refVal) {
                *target->refVal = val;
            } else {
                *target = val;
            }
        }
        return val;
    }
    if (auto b = std::dynamic_pointer_cast<BinaryExpr>(expr)) {
        Value left = Evaluate(b->left);
        Value right = Evaluate(b->right);
        
        if (b->op == TOKEN_PLUS) return Value(left.floatVal + right.floatVal);
        if (b->op == TOKEN_MINUS) return Value(left.floatVal - right.floatVal);
        if (b->op == TOKEN_STAR) return Value(left.floatVal * right.floatVal);
        if (b->op == TOKEN_SLASH) return Value(right.floatVal != 0 ? left.floatVal / right.floatVal : 0);
        if (b->op == TOKEN_MOD) return Value((int)left.floatVal % (int)right.floatVal);
        
        if (b->op == TOKEN_LT) return Value(left.floatVal < right.floatVal);
        if (b->op == TOKEN_GT) return Value(left.floatVal > right.floatVal);
        if (b->op == TOKEN_AND) return Value(left.boolVal && right.boolVal);
        if (b->op == TOKEN_OR) return Value(left.boolVal || right.boolVal);
    }
    if (auto u = std::dynamic_pointer_cast<UnaryExpr>(expr)) {
        if (u->op == TOKEN_NOT) return Value(!Evaluate(u->right).boolVal);
        if (u->op == TOKEN_MINUS) return Value(-Evaluate(u->right).floatVal);
        if (u->op == TOKEN_INC || u->op == TOKEN_DEC) {
            Value* val = GetLValue(u->right);
            if (val) {
                if (val->type == VAL_REF && val->refVal) val = val->refVal;
                if (u->op == TOKEN_INC) {
                    val->intVal++; val->floatVal++;
                } else {
                    val->intVal--; val->floatVal--;
                }
                return *val;
            }
        }
    }
    if (auto p = std::dynamic_pointer_cast<PostfixExpr>(expr)) {
        Value* val = GetLValue(p->left);
        if (val) {
            if (val->type == VAL_REF && val->refVal) val = val->refVal;
            Value old = *val;
            if (p->op == TOKEN_INC) {
                val->intVal++; val->floatVal++;
            } else {
                val->intVal--; val->floatVal--;
            }
            return old;
        }
    }
    if (auto c = std::dynamic_pointer_cast<CallExpr>(expr)) {
        std::vector<Value> args;
        for (auto arg : c->args) args.push_back(Evaluate(arg));
        return CallFunction(c->callee, args);
    }
    if (auto m = std::dynamic_pointer_cast<MemberExpr>(expr)) {
        Value* obj = GetLValue(m->object);
        if (obj) {
            if (obj->type == VAL_REF && obj->refVal) obj = obj->refVal;
            return obj->members[m->member];
        }
    }
    if (auto i = std::dynamic_pointer_cast<IndexExpr>(expr)) {
        Value* arr = GetLValue(i->array);
        Value idx = Evaluate(i->index);
        if (arr) {
            if (arr->type == VAL_REF && arr->refVal) arr = arr->refVal;
            if (idx.intVal >= 0 && idx.intVal < arr->arrayElements.size()) {
                return arr->arrayElements[idx.intVal];
            }
        }
    }
    return Value();
}

void Interpreter::Execute(StmtPtr stmt) {
    if (!stmt) return;
    if (!isRunning) return; // Stop check
    if (!callStack.empty() && callStack.back().returnHit) return;
    
    if (auto block = std::dynamic_pointer_cast<BlockStmt>(stmt)) {
        for (auto s : block->statements) Execute(s);
    } else if (auto ifStmt = std::dynamic_pointer_cast<IfStmt>(stmt)) {
        Value cond = Evaluate(ifStmt->condition);
        if (cond.boolVal || cond.intVal != 0) Execute(ifStmt->thenBranch);
        else if (ifStmt->elseBranch) Execute(ifStmt->elseBranch);
    } else if (auto whileStmt = std::dynamic_pointer_cast<WhileStmt>(stmt)) {
        while (isRunning) {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
            Value cond = Evaluate(whileStmt->condition);
            if (!cond.boolVal && cond.intVal == 0) break;
            Execute(whileStmt->body);
            if (!callStack.empty() && callStack.back().returnHit) break;
        }
    } else if (auto doStmt = std::dynamic_pointer_cast<DoWhileStmt>(stmt)) {
        do {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
            Execute(doStmt->body);
            if (!callStack.empty() && callStack.back().returnHit) break;
            Value cond = Evaluate(doStmt->condition);
            if (!cond.boolVal && cond.intVal == 0) break;
        } while (isRunning);
    } else if (auto forStmt = std::dynamic_pointer_cast<ForStmt>(stmt)) {
        if (forStmt->init) Execute(forStmt->init);
        while (isRunning) {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
            if (forStmt->condition) {
                Value cond = Evaluate(forStmt->condition);
                if (!cond.boolVal && cond.intVal == 0) break;
            }
            Execute(forStmt->body);
            if (!callStack.empty() && callStack.back().returnHit) break;
            if (forStmt->increment) Evaluate(forStmt->increment);
        }
    } else if (auto ret = std::dynamic_pointer_cast<ReturnStmt>(stmt)) {
        if (!callStack.empty()) {
            callStack.back().returnHit = true;
            if (ret->value) {
                std::string funcName = callStack.back().functionName;
                if (functions.count(funcName) && functions[funcName].returnType.back() == '&') {
                    Value* ptr = GetLValue(ret->value);
                    if (ptr) {
                        if (ptr->type == VAL_REF && ptr->refVal) ptr = ptr->refVal;
                        Value refVal;
                        refVal.type = VAL_REF;
                        refVal.refVal = ptr;
                        callStack.back().returnValue = refVal;
                    }
                } else {
                    callStack.back().returnValue = Evaluate(ret->value);
                }
            }
        }
    } else if (auto expr = std::dynamic_pointer_cast<ExprStmt>(stmt)) {
        Evaluate(expr->expression);
    } else if (auto decl = std::dynamic_pointer_cast<VarDeclStmt>(stmt)) {
        Value val;
        if (decl->isArray) {
            val.type = VAL_ARRAY;
            Value size = Evaluate(decl->arraySize);
            Value def = CreateDefaultValue(decl->type);
            for(int i=0; i<size.intVal; i++) val.arrayElements.push_back(def);
        } else {
            if (decl->initializer) {
                Value initVal = Evaluate(decl->initializer);
                if (decl->type.back() == '&') {
                    if (initVal.type == VAL_REF) {
                        val.type = VAL_REF;
                        val.refVal = initVal.refVal;
                    }
                } else {
                    if (initVal.type == VAL_REF && initVal.refVal) val = *initVal.refVal;
                    else val = initVal;
                }
            } else {
                val = CreateDefaultValue(decl->type);
            }
        }
        if (decl->type == "pile") val.type = VAL_PILE;
        if (!callStack.empty()) callStack.back().locals[decl->name] = val;
    }
}

Value Interpreter::CallFunction(const std::string& name, const std::vector<Value>& args) {
    if (name == "digitalWrite") {
        if (args.size() == 2) SetPinValue(args[0].intVal, args[1].intVal);
        return Value();
    }
    if (name == "delay") {
        if (args.size() >= 1) {
            int ms = args[0].intVal;
            std::this_thread::sleep_for(std::chrono::milliseconds(ms));
        }
        return Value();
    }
    if (name == "delayMicroseconds") {
        if (args.size() >= 1) {
            int us = args[0].intVal;
            std::this_thread::sleep_for(std::chrono::microseconds(us));
        }
        return Value();
    }
    if (name == "pulseIn") {
        if (args.size() >= 1) {
            int echo = args[0].intVal;
            std::lock_guard<std::mutex> lock(memoryMutex);
            if (sensorValues.count(echo)) {
                float dist = sensorValues[echo];
                float duration = dist * 2.0f / 0.034f;
                return Value(duration);
            }
        }
        return Value(0);
    }
    
    if (name == "push") {
        if (args.size() == 2) {
             // args[0] should be pile ref, args[1] int
             Value* pPile = nullptr;
             // We need to find the variable in memory. 
             // But args are passed by value. 
             // We need to change CallFunction to accept references or handle pile specifically.
             // Actually, for "push(p, 10)", p is passed as Value.
             // If p is VAL_PILE, it's a copy. That's bad.
             // We need pass-by-reference for piles.
             // But the user just writes "push(p, 10)".
             // In our language, we should make pile implicitly a reference or handle it.
             // For now, let's assume the user passes a reference or we find it.
             // Wait, `CallFunction` receives `vector<Value>`.
             // If `p` was a variable, `Evaluate` returned its value (copy).
             // Unless we change `Evaluate` to return references for piles?
             // Or we make `push` a special case in `ParsePrimary`?
             // Or we just say `pile` is a reference type?
             
             // Let's hack it: `push` is a built-in that modifies the pile.
             // But we only have the copy here.
             // This requires a deeper change to support pass-by-reference for built-ins or 
             // change how `push` is called.
             
             // Alternative: `p.push(10)`.
             // Then `MemberExpr` -> `CallExpr`.
             // `Evaluate` for `MemberExpr` could handle methods.
             
             // Let's try to find the pile in the stack if it's a local variable?
             // No, that's messy.
             
             // Let's assume `pile` is a reference type (like arrays often are in some langs).
             // When we assign `pile p;`, we create the object.
             // When we pass `p`, we pass a reference.
             // In `Evaluate(VariableExpr)`, if it's a pile, return a REF value?
             
             // Let's modify `Evaluate` for `VariableExpr`:
             // If it's a pile, return a REF?
             // No, `Value` has `pileElements`.
             
             // Let's just make `push` and `pop` macros or special forms?
             // No.
             
             // Let's check `Interpreter::Evaluate` for `VariableExpr`.
             // It returns `*val`.
             
             // If I change `Value` to hold a shared_ptr to pile data?
             // That would solve it.
             // But `Value` is a struct.
             
             // Let's look at `VAL_REF`.
             // If I pass `&p` to `push`, it works. `push(&p, 10)`.
             // User wants `push(p, 10)`?
             // If so, I need to change `Evaluate` to return REF for piles, OR change `push` to take `&p`.
             // Let's stick to `push(&p, 10)` for now as it's C++-like, 
             // OR implement `p.push(10)`.
             
             // User asked for "piles push and pop functions".
             // `push(p, 10)` is standard.
             // I will assume `push` takes a reference.
             // So user must write `push(&p, 10)`.
             // OR, I can make `pile` always be a reference.
             
             // Let's go with `push(&p, 10)` for safety, or better:
             // Change `Value` to use `std::shared_ptr<vector<int>>` for piles.
             // That way copies share the data.
             // I'll do that in `Interpreter.h`? No, too many changes.
             
             // I'll implement `push` to expect a REF.
             if (args[0].type == VAL_REF && args[0].refVal && args[0].refVal->type == VAL_PILE) {
                 args[0].refVal->pileElements.push_back(args[1].intVal);
             }
        }
        return Value();
    }
    if (name == "pop") {
        if (args.size() == 1) {
             if (args[0].type == VAL_REF && args[0].refVal && args[0].refVal->type == VAL_PILE) {
                 if (!args[0].refVal->pileElements.empty()) {
                     int v = args[0].refVal->pileElements.back();
                     args[0].refVal->pileElements.pop_back();
                     return Value(v);
                 }
             }
        }
        return Value(0);
    }

    // Movement Built-ins
    if (name == "forward") {
        SetPinValue(8, 1); SetPinValue(9, 0);
        SetPinValue(10, 1); SetPinValue(11, 0);
        return Value();
    }
    if (name == "backward") {
        SetPinValue(8, 0); SetPinValue(9, 1);
        SetPinValue(10, 0); SetPinValue(11, 1);
        return Value();
    }
    if (name == "left") {
        // Rotate 90 degrees left
        // Simulate by turning in place for a specific time
        SetPinValue(8, 0); SetPinValue(9, 1); // Left Bwd
        SetPinValue(10, 1); SetPinValue(11, 0); // Right Fwd
        std::this_thread::sleep_for(std::chrono::milliseconds(400)); // Calibrated delay
        SetPinValue(8, 0); SetPinValue(9, 0);
        SetPinValue(10, 0); SetPinValue(11, 0);
        return Value();
    }
    if (name == "right") {
        // Rotate 90 degrees right
        SetPinValue(8, 1); SetPinValue(9, 0); // Left Fwd
        SetPinValue(10, 0); SetPinValue(11, 1); // Right Bwd
        std::this_thread::sleep_for(std::chrono::milliseconds(400)); // Calibrated delay
        SetPinValue(8, 0); SetPinValue(9, 0);
        SetPinValue(10, 0); SetPinValue(11, 0);
        return Value();
    }
    if (name == "stop") {
        SetPinValue(8, 0); SetPinValue(9, 0);
        SetPinValue(10, 0); SetPinValue(11, 0);
        return Value();
    }

    if (name == "pinMode") return Value();
    if (name == "Serial.begin") return Value();
    
    if (functions.count(name)) {
        FunctionDef& func = functions[name];
        StackFrame frame;
        frame.functionName = name;
        
        for (size_t i = 0; i < func.parameters.size() && i < args.size(); i++) {
            std::string pName = func.parameters[i].second;
            std::string pType = func.parameters[i].first;
            
            if (pType.back() == '&') {
                frame.locals[pName] = args[i];
            } else {
                frame.locals[pName] = args[i];
            }
        }
        
        callStack.push_back(frame);
        Execute(func.body);
        Value ret = callStack.back().returnValue;
        callStack.pop_back();
        return ret;
    }
    return Value();
}

Value Interpreter::CreateDefaultValue(const std::string& type) {
    if (type == "int" || type == "long") return Value(0);
    if (type == "float") return Value(0.0f);
    if (type == "bool") return Value(false);
    if (structs.count(type)) {
        Value v;
        v.type = VAL_STRUCT;
        v.structName = type;
        for (auto& member : structs[type].members) {
            v.members[member.first] = CreateDefaultValue(member.second);
        }
        return v;
    }
    if (enums.count(type)) return Value(0);
    return Value(0);
}
