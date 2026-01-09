#pragma once
#include <string>
#include <vector>
#include <map>
#include <memory>
#include <functional>
#include <thread>
#include <mutex>
#include <atomic>

enum TokenType {
    TOKEN_EOF, TOKEN_ID, TOKEN_NUMBER,
    TOKEN_LBRACE, TOKEN_RBRACE, TOKEN_LPAREN, TOKEN_RPAREN,
    TOKEN_LBRACKET, TOKEN_RBRACKET, TOKEN_SEMICOLON, TOKEN_COMMA, TOKEN_DOT,
    TOKEN_ASSIGN, TOKEN_LT, TOKEN_GT, TOKEN_IF, TOKEN_ELSE,
    TOKEN_INT, TOKEN_FLOAT, TOKEN_LONG, TOKEN_BOOL, TOKEN_VOID,
    TOKEN_CONST, TOKEN_ENUM, TOKEN_STRUCT, TOKEN_RETURN,
    TOKEN_WHILE, TOKEN_DO, TOKEN_FOR, TOKEN_PILE,
    TOKEN_QUESTION, TOKEN_COLON,
    TOKEN_AND, TOKEN_OR, TOKEN_NOT,
    TOKEN_PLUS, TOKEN_MINUS, TOKEN_STAR, TOKEN_SLASH, TOKEN_MOD,
    TOKEN_INC, TOKEN_DEC,
    TOKEN_TRUE, TOKEN_FALSE, TOKEN_AMPERSAND
};

struct Token {
    TokenType type;
    std::string text;
    float numberValue;
    int line;
};

// --- AST ---

struct Stmt;
struct Expr;

typedef std::shared_ptr<Stmt> StmtPtr;
typedef std::shared_ptr<Expr> ExprPtr;

struct Stmt {
    virtual ~Stmt() = default;
};

struct Expr {
    virtual ~Expr() = default;
};

// Statements
struct BlockStmt : Stmt {
    std::vector<StmtPtr> statements;
};

struct IfStmt : Stmt {
    ExprPtr condition;
    StmtPtr thenBranch;
    StmtPtr elseBranch;
};

struct WhileStmt : Stmt {
    ExprPtr condition;
    StmtPtr body;
};

struct DoWhileStmt : Stmt {
    StmtPtr body;
    ExprPtr condition;
};

struct ForStmt : Stmt {
    StmtPtr init;
    ExprPtr condition;
    ExprPtr increment;
    StmtPtr body;
};

struct ReturnStmt : Stmt {
    ExprPtr value;
};

struct ExprStmt : Stmt {
    ExprPtr expression;
};

struct VarDeclStmt : Stmt {
    std::string type;
    std::string name;
    ExprPtr initializer;
    bool isArray = false;
    ExprPtr arraySize;
};

// Expressions
struct BinaryExpr : Expr {
    ExprPtr left;
    TokenType op;
    ExprPtr right;
};

struct UnaryExpr : Expr {
    TokenType op;
    ExprPtr right;
};

struct PostfixExpr : Expr {
    ExprPtr left;
    TokenType op;
};

struct LiteralExpr : Expr {
    float numberVal;
    bool boolVal;
    bool isBool = false;
};

struct VariableExpr : Expr {
    std::string name;
};

struct CallExpr : Expr {
    std::string callee;
    std::vector<ExprPtr> args;
};

struct MemberExpr : Expr {
    ExprPtr object;
    std::string member;
};

struct IndexExpr : Expr {
    ExprPtr array;
    ExprPtr index;
};

struct AssignExpr : Expr {
    ExprPtr target;
    ExprPtr value;
};

// --- Definitions ---

struct FunctionDef {
    std::string name;
    std::string returnType;
    std::vector<std::pair<std::string, std::string>> parameters;
    StmtPtr body;
};

struct StructDef {
    std::string name;
    std::map<std::string, std::string> members;
};

struct EnumDef {
    std::string name;
    std::map<std::string, int> values;
};

// --- Runtime Values ---

enum ValueType { VAL_VOID, VAL_INT, VAL_FLOAT, VAL_BOOL, VAL_STRUCT, VAL_ARRAY, VAL_REF, VAL_PILE };

struct Value {
    ValueType type = VAL_VOID;
    int intVal = 0;
    float floatVal = 0.0f;
    bool boolVal = false;
    std::string structName;
    std::map<std::string, Value> members;
    std::vector<Value> arrayElements;
    std::vector<int> pileElements;
    Value* refVal = nullptr;

    Value() {}
    Value(int v) : type(VAL_INT), intVal(v), floatVal((float)v) {}
    Value(float v) : type(VAL_FLOAT), floatVal(v), intVal((int)v) {}
    Value(bool v) : type(VAL_BOOL), boolVal(v), intVal(v), floatVal(v) {}
};

// --- Interpreter ---

class Interpreter {
public:
    Interpreter();
    ~Interpreter(); // Destructor to stop thread
    
    void Load(const std::string& code);
    void Start(); // Start execution thread
    void Stop();  // Stop execution thread
    bool IsRunning() const { return isRunning; }
    
    int GetPinValue(int pin);
    void SetPinValue(int pin, int value);
    void SetSensorValue(int trigPin, int echoPin, float distance);
    void SetVariable(const std::string& name, float value);
    
    std::function<void()> updateCallback;

private:
    std::string source;
    std::vector<Token> tokens;
    int currentToken;
    
    // Threading
    std::atomic<bool> isRunning;
    std::thread executionThread;
    std::mutex memoryMutex; // Protects globals, pins, sensors
    
    // Globals
    std::map<std::string, Value> globals;
    std::map<std::string, FunctionDef> functions;
    std::map<std::string, StructDef> structs;
    std::map<std::string, EnumDef> enums;
    
    std::map<int, int> pinValues;
    std::map<int, float> sensorValues;
    
    struct StackFrame {
        std::string functionName;
        std::map<std::string, Value> locals;
        bool returnHit = false;
        Value returnValue;
    };
    std::vector<StackFrame> callStack;
    
    // Parsing
    void Tokenize();
    Token Peek(int offset = 0);
    Token Consume();
    bool Match(TokenType type);
    bool Check(TokenType type);
    
    void ParseProgram();
    void ParseGlobal();
    StmtPtr ParseStatement();
    StmtPtr ParseBlock();
    ExprPtr ParseExpression();
    ExprPtr ParseAssignment();
    ExprPtr ParseLogicalOr();
    ExprPtr ParseLogicalAnd();
    ExprPtr ParseEquality();
    ExprPtr ParseRelational();
    ExprPtr ParseSum();
    ExprPtr ParseProduct();
    ExprPtr ParseUnary();
    ExprPtr ParsePrimary();
    
    // Execution
    void Execute(StmtPtr stmt);
    Value Evaluate(ExprPtr expr);
    Value* GetLValue(ExprPtr expr); // Get pointer to value for assignment
    
    Value CreateDefaultValue(const std::string& type);
    Value CallFunction(const std::string& name, const std::vector<Value>& args);
    
    void RunLoop(); // The thread loop
};
