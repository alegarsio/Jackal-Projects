#include "lexer.h"

static char peek(Lexer* L) {
    return L->src[L->pos];
}

static char get(Lexer* L) {
    return L->src[L->pos++];
}

static void skip_ws(Lexer* L) {
    while (isspace(peek(L))) get(L);
}

void lexer_init(Lexer* L, const char* src) {
    L->src = src;
    L->pos = 0;
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

    switch (get(L)) {
        case '+': tk.kind = TOKEN_PLUS; strcpy(tk.text, "+"); break;
        case '-': tk.kind = TOKEN_MINUS; strcpy(tk.text, "-"); break;
        case '*': tk.kind = TOKEN_STAR; strcpy(tk.text, "*"); break;
        case '/': tk.kind = TOKEN_SLASH; strcpy(tk.text, "/"); break;
        case '=': tk.kind = TOKEN_ASSIGN; strcpy(tk.text, "="); break;
        case '(': tk.kind = TOKEN_LPAREN; strcpy(tk.text, "("); break;
        case ')': tk.kind = TOKEN_RPAREN; strcpy(tk.text, ")"); break;
        case ';': tk.kind = TOKEN_SEMI; strcpy(tk.text, ";"); break;
        default:
            tk.kind = TOKEN_INVALID;
            snprintf(tk.text, sizeof(tk.text), "%c", c);
            break;
    }

    return tk;
}
