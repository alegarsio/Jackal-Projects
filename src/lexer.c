#include "lexer.h"
#include <string.h>
#include <ctype.h>

static char peek(Lexer* L) {
    return L->src[L->pos];
}

static char get(Lexer* L) {
    return L->src[L->pos++];
}

static void skip_ws(Lexer* L) {
    while (1) {
        char c = peek(L);
        if (isspace(c)) {
            get(L);
        } else if (c == '/' && L->src[L->pos + 1] == '/') {
            while (peek(L) != '\n' && peek(L) != '\0') {
                get(L);
            }
        } else {
            break;
        }
    }
}

void lexer_init(Lexer* L, const char* src) {
    L->src = src;
    L->pos = 0;
}

static int match(Lexer* L, char c) {
    if (peek(L) == c) {
        get(L);
        return 1;
    }
    return 0;
}


Token lexer_next(Lexer* L) {
    skip_ws(L);
    char c = peek(L);
    Token tk = { TOKEN_INVALID, "", 0.0 };

    if (c == '\0') {
        tk.kind = TOKEN_END;
        return tk;
    }

    if (isalpha(c) || c == '_') {
        int i = 0;
        while (isalnum(peek(L)) || peek(L) == '_') {
            if (i < 63) tk.text[i++] = get(L);
            else get(L);
        }
        tk.text[i] = '\0';

        if (strcmp(tk.text, "let") == 0) tk.kind = TOKEN_LET;
        else if (strcmp(tk.text, "print") == 0) tk.kind = TOKEN_PRINT;
        else if (strcmp(tk.text, "if") == 0) tk.kind = TOKEN_IF;
        else if (strcmp(tk.text, "else") == 0) tk.kind = TOKEN_ELSE;
        else if (strcmp(tk.text, "function") == 0) tk.kind = TOKEN_FUNCTION;
        else if (strcmp(tk.text, "return") == 0) tk.kind = TOKEN_RETURN;
        else if (strcmp(tk.text, "while") == 0) tk.kind = TOKEN_WHILE;
        else if (strcmp(tk.text, "for") == 0) tk.kind = TOKEN_FOR;
        else if (strcmp(tk.text, "class") == 0) tk.kind = TOKEN_CLASS;
        else if (strcmp(tk.text, "this") == 0) tk.kind = TOKEN_THIS;
        else tk.kind = TOKEN_IDENT;

        return tk;
    }

    if (isdigit(c) || (c == '.' && isdigit(L->src[L->pos + 1]))) {
        int i = 0;
        int dot = 0;
        while (isdigit(peek(L)) || peek(L) == '.') {
            if (peek(L) == '.') {
                if (dot) break;
                dot = 1;
            }
            if (i < 63) tk.text[i++] = get(L);
            else get(L);
        }
        tk.text[i] = '\0';
        tk.kind = TOKEN_NUMBER;
        tk.number = atof(tk.text);
        return tk;
    }

    if (c == '"') {
        get(L);
        int i = 0;
        while (peek(L) != '"' && peek(L) != '\0') {
            if (i < 63) tk.text[i++] = get(L);
            else get(L);
        }
        tk.text[i] = '\0';

        if (peek(L) == '"') get(L);
        
        tk.kind = TOKEN_STRING;
        return tk;
    }

    switch (get(L)) {
        case '+': 
            tk.kind = match(L, '+') ? TOKEN_PLUS_PLUS : TOKEN_PLUS; 
            break;
        case '-': 
            tk.kind = match(L, '-') ? TOKEN_MINUS_MINUS : TOKEN_MINUS; 
            break;
        case '*': tk.kind = TOKEN_STAR; strcpy(tk.text, "*"); break;
        case '/': tk.kind = TOKEN_SLASH; strcpy(tk.text, "/"); break;
        case '(': tk.kind = TOKEN_LPAREN; strcpy(tk.text, "("); break;
        case ')': tk.kind = TOKEN_RPAREN; strcpy(tk.text, ")"); break;
        case ';': tk.kind = TOKEN_SEMI; strcpy(tk.text, ";"); break;
        case '{': tk.kind = TOKEN_LBRACE; strcpy(tk.text, "{"); break;
        case '}': tk.kind = TOKEN_RBRACE; strcpy(tk.text, "}"); break;
        case ',': tk.kind = TOKEN_COMMA; strcpy(tk.text, ","); break;
        case '[': tk.kind = TOKEN_LBRACKET; strcpy(tk.text, "["); break;
        case ']': tk.kind = TOKEN_RBRACKET; strcpy(tk.text, "]"); break;
        case '.': tk.kind = TOKEN_DOT; strcpy(tk.text, "."); break;

        case '!': 
            tk.kind = match(L, '=') ? TOKEN_BANG_EQUAL : TOKEN_BANG; 
            break;
        case '=': 
            tk.kind = match(L, '=') ? TOKEN_EQUAL_EQUAL : TOKEN_ASSIGN; 
            break;
        case '<': 
            tk.kind = match(L, '=') ? TOKEN_LESS_EQUAL : TOKEN_LESS; 
            break;
        case '>': 
            tk.kind = match(L, '=') ? TOKEN_GREATER_EQUAL : TOKEN_GREATER; 
            break;
        case '&':
            if (match(L, '&')) tk.kind = TOKEN_AND_AND;
            else tk.kind = TOKEN_INVALID;
            break;

        default:
            tk.kind = TOKEN_INVALID;
            snprintf(tk.text, sizeof(tk.text), "%c", c);
            break;
    }

    return tk;
}