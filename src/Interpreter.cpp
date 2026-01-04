#include "Interpreter.h"
#include <cctype>
#include <cstdlib>
#include <iostream>

Interpreter::Interpreter() {
    currentToken = 0;
}

void Interpreter::Load(const std::string& code) {
    source = code;
    Tokenize();
    currentToken = 0;
    
    // Reset Memory
    variables.clear();
    pinValues.clear();
}

int Interpreter::GetPinValue(int pin) {
    if (pinValues.find(pin) != pinValues.end()) {
        return pinValues[pin];
    }
    return 0;
}

void Interpreter::SetPinValue(int pin, int value) {
    pinValues[pin] = value;
}

void Interpreter::SetSensorValue(int trigPin, int echoPin, float distance) {
    sensorValues[echoPin] = distance;
}

void Interpreter::SetVariable(const std::string& name, float value) {
    variables[name] = value;
}



void Interpreter::Step() {
    // Reset token pointer to start of loop()
    // For simplicity, we just re-parse the whole thing every frame looking for 'loop'
    // A real interpreter would cache the AST or function entry point.
    currentToken = 0;
    
    while (Peek().type != TOKEN_EOF) {
        if (Peek().type == TOKEN_VOID && Peek(1).text == "loop") {
            Consume(); // void
            Consume(); // loop
            Consume(); // (
            Consume(); // )
            ParseBlock();
            return;
        }
        Consume();
    }
}

// --- Tokenizer ---

void Interpreter::Tokenize() {
    tokens.clear();
    int i = 0;
    while (i < source.length()) {
        char c = source[i];
        
        if (isspace(c)) {
            i++;
            continue;
        }
        
        if (c == '/' && i + 1 < source.length() && source[i+1] == '/') {
            // Comment
            while (i < source.length() && source[i] != '\n') i++;
            continue;
        }
        
        Token t;
        if (isalpha(c)) {
            // ID or Keyword
            std::string text;
            while (i < source.length() && (isalnum(source[i]) || source[i] == '_')) {
                text += source[i++];
            }
            t.text = text;
            if (text == "if") t.type = TOKEN_IF;
            else if (text == "else") t.type = TOKEN_ELSE;
            else if (text == "int") t.type = TOKEN_INT;
            else if (text == "float") t.type = TOKEN_FLOAT;
            else if (text == "void") t.type = TOKEN_VOID;
            else if (text == "while") t.type = TOKEN_WHILE;
            else t.type = TOKEN_ID;
        } else if (isdigit(c)) {
            // Number
            std::string text;
            while (i < source.length() && (isdigit(source[i]) || source[i] == '.')) {
                text += source[i++];
            }
            t.text = text;
            t.type = TOKEN_NUMBER;
            t.numberValue = std::stof(text);
        } else {
            // Symbols
            t.text = std::string(1, c);
            switch (c) {
                case '{': t.type = TOKEN_LBRACE; break;
                case '}': t.type = TOKEN_RBRACE; break;
                case '(': t.type = TOKEN_LPAREN; break;
                case ')': t.type = TOKEN_RPAREN; break;
                case ';': t.type = TOKEN_SEMICOLON; break;
                case ',': t.type = TOKEN_COMMA; break;
                case '=': t.type = TOKEN_ASSIGN; break;
                case '<': t.type = TOKEN_LT; break;
                case '>': t.type = TOKEN_GT; break;
                case '?': t.type = TOKEN_QUESTION; break;
                case ':': t.type = TOKEN_COLON; break;
                default: t.type = TOKEN_EOF; break;
            }
            i++;
        }
        tokens.push_back(t);
    }
    Token eof;
    eof.type = TOKEN_EOF;
    tokens.push_back(eof);
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
    if (Peek().type == type) {
        Consume();
        return true;
    }
    return false;
}

// --- Parser ---

void Interpreter::ParseBlock() {
    if (Match(TOKEN_LBRACE)) {
        while (Peek().type != TOKEN_RBRACE && Peek().type != TOKEN_EOF) {
            ParseStatement();
        }
        Match(TOKEN_RBRACE);
    }
}

void Interpreter::ParseStatement() {
    Token t = Peek();
    
    if (t.type == TOKEN_INT || t.type == TOKEN_FLOAT) {
        // Declaration: int x = 10;
        Consume(); // Type
        std::string name = Consume().text;
        Match(TOKEN_ASSIGN);
        float val = ParseExpression();
        variables[name] = val;
        Match(TOKEN_SEMICOLON);
    } else if (t.type == TOKEN_ID) {
        // Assignment or Function Call
        if (Peek(1).type == TOKEN_LPAREN) {
            // Function Call: foo();
            ExecuteFunction(Consume().text);
            Match(TOKEN_SEMICOLON);
        } else if (Peek(1).type == TOKEN_ASSIGN) {
            // Assignment: x = 10;
            std::string name = Consume().text;
            Match(TOKEN_ASSIGN);
            float val = ParseExpression();
            variables[name] = val;
            Match(TOKEN_SEMICOLON);
        } else {
            Consume(); // Skip unknown
        }
    } else if (t.type == TOKEN_IF) {
        Consume(); // if
        Match(TOKEN_LPAREN);
        float cond = ParseExpression();
        Match(TOKEN_RPAREN);
        
        if (cond != 0) {
            ParseBlock();
            if (Peek().type == TOKEN_ELSE) {
                Consume();
                // Skip else block
                int depth = 0;
                if (Match(TOKEN_LBRACE)) {
                    depth = 1;
                    while (depth > 0 && Peek().type != TOKEN_EOF) {
                        if (Peek().type == TOKEN_LBRACE) depth++;
                        if (Peek().type == TOKEN_RBRACE) depth--;
                        Consume();
                    }
                }
            }
        } else {
            // Skip if block
            int depth = 0;
            if (Match(TOKEN_LBRACE)) {
                depth = 1;
                while (depth > 0 && Peek().type != TOKEN_EOF) {
                    if (Peek().type == TOKEN_LBRACE) depth++;
                    if (Peek().type == TOKEN_RBRACE) depth--;
                    Consume();
                }
            }
            if (Match(TOKEN_ELSE)) {
                ParseBlock();
            }
        }
    } else {
        Consume(); // Skip
    }
}

float Interpreter::ParseExpression() {
    // Ternary: Cond ? TrueVal : FalseVal
    float val = ParseComparison();
    if (Match(TOKEN_QUESTION)) {
        float trueVal = ParseExpression();
        Match(TOKEN_COLON);
        float falseVal = ParseExpression();
        return (val != 0.0f) ? trueVal : falseVal;
    }
    return val;
}

float Interpreter::ParseComparison() {
    // Simple expression parser (A < B)
    float left = ParseTerm();
    if (Match(TOKEN_LT)) {
        float right = ParseTerm();
        return (left < right) ? 1.0f : 0.0f;
    }
    if (Match(TOKEN_GT)) {
        float right = ParseTerm();
        return (left > right) ? 1.0f : 0.0f;
    }
    return left;
}

float Interpreter::ParseTerm() {
    return ParseFactor();
}

float Interpreter::ParseFactor() {
    Token t = Peek();
    if (t.type == TOKEN_NUMBER) {
        Consume();
        return t.numberValue;
    } else if (t.type == TOKEN_ID) {
        // Variable or Function Call (returning value)
        if (Peek(1).type == TOKEN_LPAREN) {
            // Function call returning value
            std::string name = Consume().text;
            Match(TOKEN_LPAREN);
            // Handle specific return functions
            if (name == "readUltrasonic") {
                float trig = ParseExpression();
                Match(TOKEN_COMMA);
                float echo = ParseExpression();
                Match(TOKEN_RPAREN);
                return sensorValues[(int)echo];
            }
            Match(TOKEN_RPAREN);
        } else {
            Consume();
            return variables[t.text];
        }
    }
    return 0;
}

void Interpreter::ExecuteFunction(const std::string& name) {
    Match(TOKEN_LPAREN);
    if (name == "digitalWrite" || name == "analogWrite") {
        float pin = ParseExpression();
        Match(TOKEN_COMMA);
        float val = ParseExpression();
        pinValues[(int)pin] = (int)val;
    } else if (name == "delay") {
        ParseExpression(); // Ignore delay for now
    } else if (name == "forward") {
        // Set pins for forward
        pinValues[5] = 255; pinValues[6] = 0;   // Left Fwd
        pinValues[9] = 255; pinValues[10] = 0;  // Right Fwd
    } else if (name == "backward") {
        pinValues[5] = 0; pinValues[6] = 255;
        pinValues[9] = 0; pinValues[10] = 255;
    } else if (name == "left") {
        // Snap Turn Left
        // We use a special pin or variable to signal the simulation?
        // Or just set a "command" variable.
        // Let's use a special pin 100 for commands: 1=Left, 2=Right
        pinValues[100] = 1;
    } else if (name == "right") {
        // Snap Turn Right
        pinValues[100] = 2;
    } else if (name == "stop") {
        pinValues[5] = 0; pinValues[6] = 0;
        pinValues[9] = 0; pinValues[10] = 0;
    }
    Match(TOKEN_RPAREN);
}
