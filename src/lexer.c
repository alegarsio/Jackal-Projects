#include "lexer.h"
#include <string.h>
#include <ctype.h>

/**
 * @brief Peeks at the current character without consuming it.
 * @param L Pointer to the Lexer.
 */
static char peek(Lexer *L)
{
    return L->src[L->pos];
}
/**
 * @brief Gets the current character and advances the position.
 * @param L Pointer to the Lexer.
 */
static char get(Lexer *L)
{
    return L->src[L->pos++];
}

/**
 * @brief Skips whitespace and comments in the source code.
 * @param L Pointer to the Lexer.
 */
static void skip_ws(Lexer *L)
{
    while (1)
    {
        char c = peek(L);

        if (isspace(c))
        {
            get(L);
            continue;
        }

        if (c == '/')
        {
            char next_char = L->src[L->pos + 1];

            if (next_char == '/')
            {
                while (peek(L) != '\n' && peek(L) != '\0')
                {
                    get(L);
                }
                continue;
            }
            else if (next_char == '*')
            {

                get(L);
                get(L);

                while (peek(L) != '\0')
                {
                    if (peek(L) == '*' && L->src[L->pos + 1] == '/')
                    {
                        get(L);
                        get(L);
                        break;
                    }
                    get(L);
                }
                continue;
            }
            else
            {
                break;
            }
        }
        else
        {
            break;
        }
    }
}

/**
 * @brief Initializes the lexer with the given source code.
 * @param L Pointer to the Lexer to be initialized.
 * @param src The source code to be lexed.
 */
void lexer_init(Lexer *L, const char *src)
{
    L->src = src;
    L->pos = 0;
}

/**
 * @brief Matches the current character with the expected character.
 * If they match, advances the position.
 * @param L Pointer to the Lexer.
 * @param c The character to match.
 * @return 1 if matched, 0 otherwise.
 */
static int match(Lexer *L, char c)
{
    if (peek(L) == c)
    {
        get(L);
        return 1;
    }
    return 0;
}
/**
 * @brief Retrieves the next token from the source code.
 * @param L Pointer to the Lexer.
 * @return The next Token.
 */
Token lexer_next(Lexer *L)
{
    skip_ws(L);
    char c = peek(L);
    Token tk = {TOKEN_INVALID, "", 0.0};

    if (c == '\0')
    {
        tk.kind = TOKEN_END;
        return tk;
    }

    if (isalpha(c) || c == '_')
    {
        int i = 0;
        while (isalnum(peek(L)) || peek(L) == '_')
        {
            if (i < 63)
                tk.text[i++] = get(L);
            else
                get(L);
        }
        tk.text[i] = '\0';

        if (strcmp(tk.text, "let") == 0)
            tk.kind = TOKEN_LET;
        else if (strcmp(tk.text, "const") == 0)
            tk.kind = TOKEN_CONST;
        // else if (strcmp(tk.text, "print") == 0) tk.kind = TOKEN_PRINT;
        else if (strcmp(tk.text, "if") == 0)
            tk.kind = TOKEN_IF;
        else if (strcmp(tk.text, "else") == 0)
            tk.kind = TOKEN_ELSE;
        else if (strcmp(tk.text, "func") == 0)
            tk.kind = TOKEN_FUNCTION;
        else if (strcmp(tk.text, "return") == 0)
            tk.kind = TOKEN_RETURN;
        else if (strcmp(tk.text, "while") == 0)
            tk.kind = TOKEN_WHILE;
        else if (strcmp(tk.text, "record") == 0)
            tk.kind = TOKEN_RECORD; 
        // else if (strcmp(tk.text,"") == 0)
        //     tk.kind = TOKEN_TYPE;
        else if (strcmp(tk.text, "every") == 0)
            tk.kind = TOKEN_EVERY;
        else if (strcmp(tk.text, "until") == 0)
            tk.kind = TOKEN_UNTIL;
        else if (strcmp(tk.text, "true") == 0)
            tk.kind = TOKEN_TRUE;
        else if (strcmp(tk.text, "false") == 0)
            tk.kind = TOKEN_FALSE;
        else if (strcmp(tk.text, "observe") == 0)
            tk.kind = TOKEN_OBSERVE;
        else if (strcmp(tk.text, "on") == 0)
            tk.kind = TOKEN_ON;
        else if (strcmp(tk.text,"struct") == 0)
            tk.kind = TOKEN_STRUCT;
        else if (strcmp(tk.text, "object") == 0)
            tk.kind = TOKEN_OBJECT;
        else if (strcmp(tk.text, "of") == 0)
            tk.kind = TOKEN_OF;
        else if (strcmp(tk.text,"step") == 0)
            tk.kind = TOKEN_STEP;
        else if (strcmp(tk.text, "break") == 0)
            tk.kind = TOKEN_BREAK;
        else if (strcmp(tk.text, "continue") == 0)
            tk.kind = TOKEN_CONTINUE;
        else if (strcmp(tk.text, "when") == 0)
            tk.kind = TOKEN_WHEN;
        else if (strcmp(tk.text, "private") == 0)
            tk.kind = TOKEN_PRIVATE;
        else if (strcmp(tk.text, "for") == 0)
            tk.kind = TOKEN_FOR;
        else if (strcmp(tk.text, "class") == 0)
            tk.kind = TOKEN_CLASS;
        else if (strcmp(tk.text, "extends") == 0)
            tk.kind = TOKEN_EXTENDS;
        else if (strcmp(tk.text, "this") == 0)
            tk.kind = TOKEN_THIS;
        else if (strcmp(tk.text, "import") == 0)
            tk.kind = TOKEN_IMPORT;
        else if (strcmp(tk.text, "match") == 0)
            tk.kind = TOKEN_MATCH;
        else if (strcmp(tk.text, "default") == 0)
            tk.kind = TOKEN_DEFAULT;
        else if (strcmp(tk.text, "interface") == 0)
            tk.kind = TOKEN_INTERFACE;
        else if (strcmp(tk.text, "enum") == 0)
            tk.kind = TOKEN_ENUM;
        else if (strcmp(tk.text, "try") == 0)
            tk.kind = TOKEN_TRY;
        else if (strcmp(tk.text,"trait") == 0)
            tk.kind = TOKEN_TRAIT;
        else if (strcmp(tk.text, "catch") == 0)
            tk.kind = TOKEN_CATCH;
        else if (strcmp(tk.text, "throw") == 0)
            tk.kind = TOKEN_THROW;
        else if (strcmp(tk.text, "in") == 0)
            tk.kind = TOKEN_IN;
        else if (strcmp(tk.text,"any") == 0)
            tk.kind = TOKEN_ANY;
        else if (strcmp(tk.text,"with") == 0)
            tk.kind = TOKEN_WITH;
        else if (strcmp(tk.text,"namespace") == 0 )
            tk.kind = TOKEN_NAMESPACE;
        
        else if (strcmp(tk.text,"using") == 0 )
            tk.kind = TOKEN_USING;
        
        else if (strcmp(tk.text, "implements") == 0)
            tk.kind = TOKEN_IMPLEMENTS;

        else if (strcmp(tk.text, "to") == 0)
            tk.kind = TOKEN_TO;

        // else if (strcmp(tk.text,"..") == 0){
        //     tk.kind = TOKEN_RANGE;
        // }

        
        else
            tk.kind = TOKEN_IDENT;

        return tk;
    }

    if (isdigit(c) || (c == '.' && isdigit(L->src[L->pos + 1])))
    {
        int i = 0;
        int dot = 0;
        while (isdigit(peek(L)) || peek(L) == '.')
        {
            if (peek(L) == '.')
            {
                if (dot)
                    break;
                dot = 1;
            }
            if (i < 63)
                tk.text[i++] = get(L);
            else
                get(L);
        }
        tk.text[i] = '\0';
        tk.kind = TOKEN_NUMBER;
        tk.number = atof(tk.text);
        return tk;
    }

    if (c == '"')
    {
        get(L);
        int i = 0;

        while (peek(L) != '"' && peek(L) != '\0')
        {
            char ch = get(L);
            if (ch == '\\')
            {
                if (peek(L) == '\0')
                    break;
                ch = get(L);
            }

            if (i < 63)
                tk.text[i++] = ch;
            else
            {
            }
        }
        tk.text[i] = '\0';

        if (peek(L) == '"')
            get(L);

        tk.kind = TOKEN_STRING;
        return tk;
    }

    switch (get(L))
    {
    case '+':
        tk.kind = match(L, '+') ? TOKEN_PLUS_PLUS : TOKEN_PLUS;
        break;
    case '-':
    {
        char next_char = peek(L);

        if (next_char == '-')
        {

            get(L);
            tk.kind = TOKEN_MINUS_MINUS;
        }
        else if (next_char == '>')
        {
            get(L);
            tk.kind = TOKEN_ARROW_TO_RETURN;
        }
        else
        {
            tk.kind = TOKEN_MINUS;
        }
    }
    break;
    case '*':
        tk.kind = TOKEN_STAR;
        strcpy(tk.text, "*");
        break;
    case '/':
        tk.kind = TOKEN_SLASH;
        strcpy(tk.text, "/");
        break;
    case '(':
        tk.kind = TOKEN_LPAREN;
        strcpy(tk.text, "(");
        break;
  
    case ')':
        tk.kind = TOKEN_RPAREN;
        strcpy(tk.text, ")");
        break;
    case ';':
        tk.kind = TOKEN_SEMI;
        strcpy(tk.text, ";");
        break;
    case '{':
        tk.kind = TOKEN_LBRACE;
        strcpy(tk.text, "{");
        break;
    case '}':
        tk.kind = TOKEN_RBRACE;
        strcpy(tk.text, "}");
        break;
    case ',':
        tk.kind = TOKEN_COMMA;
        strcpy(tk.text, ",");
        break;
    case '%':
        tk.kind = TOKEN_PERCENT;
        strcpy(tk.text, "%");
        break;
    case '[':
        tk.kind = TOKEN_LBRACKET;
        strcpy(tk.text, "[");
        break;
    case ']':
        tk.kind = TOKEN_RBRACKET;
        strcpy(tk.text, "]");
        break;
    
    case '.':
        if (match(L, '.'))
        {
            tk.kind = TOKEN_DOT_DOT;
            strcpy(tk.text, "..");
        }
        else
        {
            tk.kind = TOKEN_DOT;
            strcpy(tk.text, ".");
        }
        break;
    case ':':
        tk.kind = TOKEN_COLON;
        strcpy(tk.text, ":");
        break;
    case '@':
        tk.kind = TOKEN_AT;
        strcpy(tk.text, "@");
        break;
    // @deprecated
    // case '->':
    //     tk.kind = TOKEN_ARROW_TO_RETURN;
    //     strcpy(tk.text,"->");
    //     break;
    case '!':
        tk.kind = match(L, '=') ? TOKEN_BANG_EQUAL : TOKEN_BANG;
        break;
    case '=':
        if (match(L, '='))
            tk.kind = TOKEN_EQUAL_EQUAL;
        else if (match(L, '>'))
            tk.kind = TOKEN_ARROW;
        else
            tk.kind = TOKEN_ASSIGN;
        break;
    case '<':
        tk.kind = match(L, '=') ? TOKEN_LESS_EQUAL : TOKEN_LESS;
        break;
    case '>':
        tk.kind = match(L, '=') ? TOKEN_GREATER_EQUAL : TOKEN_GREATER;
        break;
    case '|':
        if (match(L, '|'))
            tk.kind = TOKEN_PIPE_PIPE;
        else if (match(L,'>'))
            tk.kind = TOKEN_PIPELINE;
        else
            tk.kind = TOKEN_INVALID;
        break;
    case '&':
        if (match(L, '&'))
            tk.kind = TOKEN_AND_AND;
        else
            tk.kind = TOKEN_INVALID;
        break;

    default:
        tk.kind = TOKEN_INVALID;
        snprintf(tk.text, sizeof(tk.text), "%c", c);
        break;
    }

    return tk;
}