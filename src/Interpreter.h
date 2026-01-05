#pragma once
#include <string>
#include <vector>
#include <map>
#include <iostream>

enum TokenType {
    TOKEN_EOF,
    TOKEN_ID,
    TOKEN_NUMBER,
    TOKEN_LBRACE, // {
    TOKEN_RBRACE, // }
    TOKEN_LPAREN, // (
    TOKEN_RPAREN, // )
    TOKEN_SEMICOLON, // ;
    TOKEN_COMMA, // ,
    TOKEN_ASSIGN, // =
    TOKEN_LT, // <
    TOKEN_GT, // >
    TOKEN_IF,
    TOKEN_ELSE,
    TOKEN_INT,
    TOKEN_FLOAT,
    TOKEN_VOID,
    TOKEN_WHILE,
    TOKEN_DO,       // do
    TOKEN_FOR,      // for
    TOKEN_QUESTION, // ?
    TOKEN_COLON,    // :
    TOKEN_AND,      // &&
    TOKEN_OR,       // ||
    TOKEN_NOT,      // !
    TOKEN_PLUS,     // +
    TOKEN_MINUS,    // -
    TOKEN_STAR,     // *
    TOKEN_SLASH     // /
};

struct Token {
    TokenType type;
    std::string text;
    float numberValue;
};

class Interpreter {
public:
    Interpreter();
    
    void Load(const std::string& code);
    void Step(); // Execute one pass of loop()
    
    // Hardware Interface
    int GetPinValue(int pin); // 0-255
    void SetPinValue(int pin, int value);
    void SetSensorValue(int trigPin, int echoPin, float distance);
    void SetVariable(const std::string& name, float value);


    


private:
    std::string source;
    std::vector<Token> tokens;
    int currentToken;
    
    // Memory
    std::map<std::string, float> variables;
    std::map<int, int> pinValues; // Pin -> Value (0-255)
    std::map<int, float> sensorValues; // EchoPin -> Distance
    
    // Parsing
    void Tokenize();
    Token Peek(int offset = 0);
    Token Consume();
    bool Match(TokenType type);
    
    // Execution
    void ParseLoop();
    void ParseBlock(bool execute = true);
    void ParseStatement(bool execute = true);
    
    void ParseWhile(bool execute);
    void ParseDoWhile(bool execute);
    void ParseFor(bool execute);

    float ParseExpression();
    float ParseLogicalOr();
    float ParseLogicalAnd();
    float ParseComparison();
    float ParseSum();
    float ParseProduct();
    float ParseFactor();
    
    // Built-ins
    void ExecuteFunction(const std::string& name, bool execute = true);
};
