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

        if (c == '&' && i + 1 < source.length() && source[i+1] == '&') {
            Token t; t.text = "&&"; t.type = TOKEN_AND;
            tokens.push_back(t);
            i += 2;
            continue;
        }

        if (c == '|' && i + 1 < source.length() && source[i+1] == '|') {
            Token t; t.text = "||"; t.type = TOKEN_OR;
            tokens.push_back(t);
            i += 2;
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
            else if (text == "do") t.type = TOKEN_DO;
            else if (text == "for") t.type = TOKEN_FOR;
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
                case '!': t.type = TOKEN_NOT; break;
                case '+': t.type = TOKEN_PLUS; break;
                case '-': t.type = TOKEN_MINUS; break;
                case '*': t.type = TOKEN_STAR; break;
                case '/': t.type = TOKEN_SLASH; break;
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

void Interpreter::ParseBlock(bool execute) {
    if (Match(TOKEN_LBRACE)) {
        while (Peek().type != TOKEN_RBRACE && Peek().type != TOKEN_EOF) {
            ParseStatement(execute);
        }
        Match(TOKEN_RBRACE);
    }
}
void Interpreter::ParseStatement(bool execute) {
    Token t = Peek();
    
    if (t.type == TOKEN_INT || t.type == TOKEN_FLOAT) {
        // Declaration: int x = 10; or int x;
        Consume(); // Type
        std::string name = Consume().text;
        float val = 0.0f;
        if (Match(TOKEN_ASSIGN)) {
            val = ParseExpression();
        }
        if (execute) variables[name] = val;
        Match(TOKEN_SEMICOLON);
    } else if (t.type == TOKEN_ID) {
        // ... (Assignment/Call logic unchanged)
        if (Peek(1).type == TOKEN_LPAREN) {
            std::string funcName = Consume().text;
            ExecuteFunction(funcName, execute);
            Match(TOKEN_SEMICOLON);
        } else if (Peek(1).type == TOKEN_ASSIGN) {
            std::string name = Consume().text;
            Match(TOKEN_ASSIGN);
            float val = ParseExpression();
            if (execute) variables[name] = val;
            Match(TOKEN_SEMICOLON);
        } else {
            Consume();
        }
    } else if (t.type == TOKEN_IF) {
        // ... (If logic unchanged)
        Consume(); // if
        Match(TOKEN_LPAREN);
        float cond = ParseExpression();
        Match(TOKEN_RPAREN);
        
        bool conditionMet = execute && (cond != 0.0f);
        
        if (conditionMet) {
            ParseBlock(true);
            if (Peek().type == TOKEN_ELSE) {
                Consume();
                if (Peek().type == TOKEN_IF) ParseStatement(false);
                else ParseBlock(false);
            }
        } else {
            ParseBlock(false);
            if (Peek().type == TOKEN_ELSE) {
                Consume();
                if (Peek().type == TOKEN_IF) ParseStatement(execute);
                else ParseBlock(execute);
            }
        }
    } else if (t.type == TOKEN_WHILE) {
        ParseWhile(execute);
    } else if (t.type == TOKEN_DO) {
        ParseDoWhile(execute);
    } else if (t.type == TOKEN_FOR) {
        ParseFor(execute);
    } else {
        Consume(); // Skip
    }
}

void Interpreter::ParseWhile(bool execute) {
    Consume(); // while
    int startToken = currentToken;
    
    // Initial check
    Match(TOKEN_LPAREN);
    float cond = ParseExpression();
    Match(TOKEN_RPAREN);
    
    if (!execute) {
        ParseBlock(false);
        return;
    }
    
    while (cond != 0.0f) {
        if (updateCallback) updateCallback();
        ParseBlock(true);
        
        // Loop back
        currentToken = startToken;
        Match(TOKEN_LPAREN);
        cond = ParseExpression();
        Match(TOKEN_RPAREN);
    }
    
    // Skip block one last time to advance
    ParseBlock(false);
}

void Interpreter::ParseDoWhile(bool execute) {
    Consume(); // do
    int startToken = currentToken;
    
    if (!execute) {
        ParseBlock(false);
        Match(TOKEN_WHILE);
        Match(TOKEN_LPAREN);
        ParseExpression();
        Match(TOKEN_RPAREN);
        Match(TOKEN_SEMICOLON);
        return;
    }
    
    float cond = 0.0f;
    do {
        if (updateCallback) updateCallback();
        currentToken = startToken;
        ParseBlock(true);
        Match(TOKEN_WHILE);
        Match(TOKEN_LPAREN);
        cond = ParseExpression();
        Match(TOKEN_RPAREN);
        Match(TOKEN_SEMICOLON);
    } while (cond != 0.0f);
}

void Interpreter::ParseFor(bool execute) {
    Consume(); // for
    Match(TOKEN_LPAREN);
    
    // Init (Run once)
    // Can be declaration or assignment or empty
    // Simplified: Assume declaration or assignment
    if (Peek().type == TOKEN_INT || Peek().type == TOKEN_FLOAT) {
        // Declaration
        Consume(); // Type
        std::string name = Consume().text;
        float val = 0.0f;
        if (Match(TOKEN_ASSIGN)) {
            val = ParseExpression();
        }
        if (execute) variables[name] = val;
        Match(TOKEN_SEMICOLON);
    } else if (Peek().type == TOKEN_ID) {
        // Assignment
        std::string name = Consume().text;
        Match(TOKEN_ASSIGN);
        float val = ParseExpression();
        if (execute) variables[name] = val;
        Match(TOKEN_SEMICOLON);
    } else {
        Match(TOKEN_SEMICOLON); // Empty init
    }
    
    int condStart = currentToken;
    
    // Check Condition
    float cond = 1.0f;
    if (Peek().type != TOKEN_SEMICOLON) {
        cond = ParseExpression();
    }
    Match(TOKEN_SEMICOLON);
    
    int incStart = currentToken;
    
    // Skip Increment for now (to find body start)
    // We need to parse it without executing to find where it ends
    // This is tricky because we don't have a "SkipExpression" function easily.
    // Hack: ParseExpression but ignore result? 
    // But assignment is a statement, not expression in this parser usually?
    // In C++, inc is an expression.
    // Let's assume inc is "ID = Expr" or "ID++" (we don't have ++).
    // So "i = i + 1"
    
    // Scan forward until RPAREN?
    int bodyStart = currentToken;
    int parenCount = 0;
    while (Peek().type != TOKEN_EOF) {
        if (Peek().type == TOKEN_LPAREN) parenCount++;
        if (Peek().type == TOKEN_RPAREN) {
            if (parenCount == 0) break;
            parenCount--;
        }
        Consume();
    }
    int incEnd = currentToken; // Points to RPAREN
    Match(TOKEN_RPAREN);
    
    if (!execute) {
        ParseBlock(false);
        return;
    }
    
    // Loop
    while (cond != 0.0f) {
        if (updateCallback) updateCallback();
        ParseBlock(true);
        
        // Execute Increment
        int savedPos = currentToken;
        currentToken = incStart;
        
        // Parse Increment Expression(s)
        // We only support "ID = Expr" style for now in this slot?
        // Or we can reuse ParseStatement logic if it fits?
        // ParseStatement expects semicolon. For loop inc doesn't have semicolon.
        // Let's manually parse assignment: ID = Expr
        if (Peek().type == TOKEN_ID && Peek(1).type == TOKEN_ASSIGN) {
            std::string name = Consume().text;
            Match(TOKEN_ASSIGN);
            float val = ParseExpression();
            variables[name] = val;
        }
        
        currentToken = savedPos; // Restore to after body
        
        // Re-check condition
        currentToken = condStart;
        if (Peek().type != TOKEN_SEMICOLON) {
            cond = ParseExpression();
        } else {
            cond = 1.0f;
        }
        
        // Jump to body start if true
        if (cond != 0.0f) {
            // We need to skip init, cond, inc again to get to body?
            // No, we are at condStart. We just parsed cond.
            // We need to skip inc to get to body.
            // We already know where body starts: after incEnd + 1 (RPAREN)
            currentToken = incEnd + 1;
        }
    }
    
    // Done, move to after body
    // We are currently at condStart (after failed check).
    // We need to skip cond, inc, body.
    // Actually, if cond fails, we are at SEMICOLON (after cond).
    // We need to skip inc and body.
    // Skip inc:
    currentToken = incEnd + 1;
    ParseBlock(false);
}

// ... (Rest of file)

float Interpreter::ParseExpression() {
    // Ternary: Cond ? TrueVal : FalseVal
    float val = ParseLogicalOr();
    if (Match(TOKEN_QUESTION)) {
        float trueVal = ParseExpression();
        Match(TOKEN_COLON);
        float falseVal = ParseExpression();
        return (val != 0.0f) ? trueVal : falseVal;
    }
    return val;
}

float Interpreter::ParseLogicalOr() {
    float left = ParseLogicalAnd();
    while (Match(TOKEN_OR)) {
        float right = ParseLogicalAnd();
        left = ((left != 0.0f) || (right != 0.0f)) ? 1.0f : 0.0f;
    }
    return left;
}

float Interpreter::ParseLogicalAnd() {
    float left = ParseComparison();
    while (Match(TOKEN_AND)) {
        float right = ParseComparison();
        left = ((left != 0.0f) && (right != 0.0f)) ? 1.0f : 0.0f;
    }
    return left;
}

float Interpreter::ParseComparison() {
    // Simple expression parser (A < B)
    float left = ParseSum();
    if (Match(TOKEN_LT)) {
        float right = ParseSum();
        return (left < right) ? 1.0f : 0.0f;
    }
    if (Match(TOKEN_GT)) {
        float right = ParseSum();
        return (left > right) ? 1.0f : 0.0f;
    }
    return left;
}

float Interpreter::ParseSum() {
    float left = ParseProduct();
    while (Peek().type == TOKEN_PLUS || Peek().type == TOKEN_MINUS) {
        if (Match(TOKEN_PLUS)) {
            float right = ParseProduct();
            left += right;
        } else if (Match(TOKEN_MINUS)) {
            float right = ParseProduct();
            left -= right;
        }
    }
    return left;
}

float Interpreter::ParseProduct() {
    float left = ParseFactor();
    while (Peek().type == TOKEN_STAR || Peek().type == TOKEN_SLASH) {
        if (Match(TOKEN_STAR)) {
            float right = ParseFactor();
            left *= right;
        } else if (Match(TOKEN_SLASH)) {
            float right = ParseFactor();
            if (right != 0.0f) left /= right;
        }
    }
    return left;
}

float Interpreter::ParseFactor() {
    Token t = Peek();
    if (t.type == TOKEN_NOT) {
        Consume();
        float val = ParseFactor();
        return (val == 0.0f) ? 1.0f : 0.0f;
    }
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

void Interpreter::ExecuteFunction(const std::string& name, bool execute) {
    Match(TOKEN_LPAREN);
    if (name == "digitalWrite" || name == "analogWrite") {
        float pin = ParseExpression();
        Match(TOKEN_COMMA);
        float val = ParseExpression();
        if (execute) pinValues[(int)pin] = (int)val;
    } else if (name == "delay") {
        ParseExpression(); // Ignore delay for now
    } else if (name == "forward") {
        // Set pins for forward
        if (execute) {
            pinValues[5] = 255; pinValues[6] = 0;   // Left Fwd
            pinValues[9] = 255; pinValues[10] = 0;  // Right Fwd
        }
    } else if (name == "backward") {
        if (execute) {
            pinValues[5] = 0; pinValues[6] = 255;
            pinValues[9] = 0; pinValues[10] = 255;
        }
    } else if (name == "left") {
        // Snap Turn Left
        if (execute) pinValues[100] = 1;
    } else if (name == "right") {
        // Snap Turn Right
        if (execute) pinValues[100] = 2;
    } else if (name == "stop") {
        if (execute) {
            pinValues[5] = 0; pinValues[6] = 0;
            pinValues[9] = 0; pinValues[10] = 0;
        }
    }
    Match(TOKEN_RPAREN);
}
