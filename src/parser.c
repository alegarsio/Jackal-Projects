#include "parser.h"
#include <stdlib.h>
#include <string.h>

static void next(Parser* P) { P->current = lexer_next(P->lexer); }

void parser_init(Parser* P, Lexer* L) {
    P->lexer = L;
    next(P);
}

static Node* new_node(NodeKind kind) {
    Node* n = calloc(1, sizeof(Node));
    n->kind = kind;
    return n;
}

Node *parse(const char *source) {
    (void)source;
    return NULL;
}

static Node* parse_expr(Parser* P);
Node* parse_stmt(Parser* P);
static Node* parse_primary(Parser* P);
static Node* parse_multiplication(Parser* P);
static Node* parse_addition(Parser* P);
static Node* parse_comparison(Parser* P);
static Node* parse_equality(Parser* P);
static Node* parse_logical_and(Parser* P);
static Node* parse_assignment(Parser* P); 
static Node* parse_block(Parser* P);
static Node* parse_postfix(Parser* P);
static Node* parse_func_def(Parser* P);


static Node* parse_func_call(Parser* P, Node* callee) {
    Node* n = new_node(NODE_FUNC_CALL);
    n->left = callee;
    
    Node* args_head = NULL;
    Node* args_current = NULL;
    n->arity = 0;

    if (P->current.kind != TOKEN_RPAREN) {
        do {
            Node* arg_expr = parse_expr(P);
            n->arity++;
            
            if (args_head == NULL) {
                args_head = arg_expr;
                args_current = arg_expr;
            } else {
                // GUNAKAN 'next'
                args_current->next = arg_expr;
                args_current = arg_expr;
            }
        } while (P->current.kind == TOKEN_COMMA && (next(P), 1));
    }
    
    if (P->current.kind != TOKEN_RPAREN) print_error("Expected ')' after function arguments.");
    next(P);
    
    n->right = args_head;
    return n;
}

static Node* parse_array_literal(Parser* P) {
    Node* n = new_node(NODE_ARRAY_LITERAL);
    
    Node* items_head = NULL;
    Node* items_current = NULL;
    n->arity = 0;

    if (P->current.kind != TOKEN_RBRACKET) {
        do {
            Node* item_expr = parse_expr(P);
            n->arity++;
            
            if (items_head == NULL) {
                items_head = item_expr;
                items_current = item_expr;
            } else {
                // GUNAKAN 'next'
                items_current->next = item_expr;
                items_current = item_expr;
            }
        } while (P->current.kind == TOKEN_COMMA && (next(P), 1));
    }
    
    if (P->current.kind != TOKEN_RBRACKET) print_error("Expected ']' after array items.");
    next(P);
    
    n->left = items_head;
    return n;
}


static Node* parse_primary(Parser* P) {
    if (P->current.kind == TOKEN_NUMBER) {
        Node* n = new_node(NODE_NUMBER);
        n->value = P->current.number;
        next(P);
        return n;
    }
    
    if (P->current.kind == TOKEN_STRING) {
        Node* n = new_node(NODE_STRING);
        strcpy(n->name, P->current.text);
        next(P);
        return n;
    }

    if (P->current.kind == TOKEN_LBRACKET) {
        next(P);
        return parse_array_literal(P);
    }
    
    if (P->current.kind == TOKEN_THIS) {
        Node* n = new_node(NODE_THIS);
        strcpy(n->name, "this");
        next(P);
        return n;
    }

    if (P->current.kind == TOKEN_IDENT) {
        Node* n = new_node(NODE_IDENT);
        strcpy(n->name, P->current.text);
        next(P);
        return n;
    }
    if (P->current.kind == TOKEN_LPAREN) {
        next(P);
        Node* n = parse_expr(P);
        if (P->current.kind == TOKEN_RPAREN) next(P);
        return n;
    }
    return NULL;
}

static Node* parse_postfix(Parser* P) {
    Node* node = parse_primary(P);

    while(1) {
        if (P->current.kind == TOKEN_LPAREN) {
            next(P);
            node = parse_func_call(P, node);
        } else if (P->current.kind == TOKEN_LBRACKET) {
            next(P);
            Node* access_node = new_node(NODE_ARRAY_ACCESS);
            access_node->left = node;
            access_node->right = parse_expr(P);
            if (P->current.kind != TOKEN_RBRACKET) print_error("Expected ']' after index.");
            next(P);
            node = access_node;
        } else if (P->current.kind == TOKEN_DOT) {
            next(P);
            if (P->current.kind != TOKEN_IDENT) print_error("Expected property name after '.'.");
            Node* get_node = new_node(NODE_GET);
            get_node->left = node;
            strcpy(get_node->name, P->current.text);
            next(P);
            node = get_node;
        } else if (P->current.kind == TOKEN_PLUS_PLUS) {
            next(P);
            Node* inc = new_node(NODE_POST_INC);
            inc->left = node;
            node = inc;
        } else if (P->current.kind == TOKEN_MINUS_MINUS) {
            next(P);
            Node* dec = new_node(NODE_POST_DEC);
            dec->left = node;
            node = dec;
        } else {
            break;
        }
    }
    return node;
}

static Node* parse_multiplication(Parser* P) {
    Node* left = parse_postfix(P);
    while (P->current.kind == TOKEN_STAR || P->current.kind == TOKEN_SLASH) {
        Node* n = new_node(NODE_BINOP);
        n->op = P->current.kind;
        next(P);
        n->left = left;
        n->right = parse_postfix(P);
        left = n;
    }
    return left;
}

static Node* parse_addition(Parser* P) {
    Node* left = parse_multiplication(P);
    while (P->current.kind == TOKEN_PLUS || P->current.kind == TOKEN_MINUS) {
        Node* n = new_node(NODE_BINOP);
        n->op = P->current.kind;
        next(P);
        n->left = left;
        n->right = parse_multiplication(P);
        left = n;
    }
    return left;
}

static Node* parse_comparison(Parser* P) {
    Node* left = parse_addition(P);
    while (P->current.kind == TOKEN_GREATER || P->current.kind == TOKEN_GREATER_EQUAL ||
           P->current.kind == TOKEN_LESS || P->current.kind == TOKEN_LESS_EQUAL) {
        Node* n = new_node(NODE_BINOP);
        n->op = P->current.kind;
        next(P);
        n->left = left;
        n->right = parse_addition(P);
        left = n;
    }
    return left;
}

static Node* parse_equality(Parser* P) {
    Node* left = parse_comparison(P);
    while (P->current.kind == TOKEN_EQUAL_EQUAL || P->current.kind == TOKEN_BANG_EQUAL) {
        Node* n = new_node(NODE_BINOP);
        n->op = P->current.kind;
        next(P);
        n->left = left;
        n->right = parse_comparison(P);
        left = n;
    }
    return left;
}

static Node* parse_logical_and(Parser* P) {
    Node* left = parse_equality(P);
    while (P->current.kind == TOKEN_AND_AND) {
        Node* n = new_node(NODE_BINOP);
        n->op = P->current.kind;
        next(P);
        n->left = left;
        n->right = parse_equality(P);
        left = n;
    }
    return left;
}


static Node* parse_assignment(Parser* P) {
    Node* node = parse_logical_and(P);

    if (P->current.kind == TOKEN_ASSIGN) {
        next(P);
        Node* value = parse_assignment(P); 

        if (node->kind == NODE_IDENT) {
            Node* assign_node = new_node(NODE_ASSIGN);
            strcpy(assign_node->name, node->name);
            assign_node->left = value; 
            free_node(node);
            return assign_node;
        }
        
        if (node->kind == NODE_ARRAY_ACCESS) {
            Node* assign_node = new_node(NODE_ARRAY_ASSIGN);
            assign_node->left = node;
            assign_node->right = value;
            return assign_node;
        }

        if (node->kind == NODE_GET) {
            Node* set_node = new_node(NODE_SET);
            set_node->left = node;
            set_node->right = value;
            return set_node;
        }
        
        print_error("Invalid assignment target.");
        free_node(value);
    }
    return node;
}


static Node* parse_expr(Parser* P) {
    return parse_assignment(P);
}


static Node* parse_block(Parser* P) {
    Node* n = new_node(NODE_BLOCK);
    
    if (P->current.kind != TOKEN_LBRACE) {
        print_error("Expected '{' to start a block.");
        return n;
    }
    next(P);

    Node* head = NULL;
    Node* current = NULL;

    while (P->current.kind != TOKEN_RBRACE && P->current.kind != TOKEN_END) {
        Node* stmt = parse_stmt(P);
        if (!stmt) continue;

        if (head == NULL) {
            head = stmt;
            current = stmt;
        } else {
            // GUNAKAN 'next'
            current->next = stmt; 
            current = stmt;
        }
    }

    if (P->current.kind != TOKEN_RBRACE) {
         print_error("Expected '}' to end a block.");
    } else {
        next(P);
    }
    
    n->left = head;
    return n;
}

static Node* parse_if_stmt(Parser *P) {
    Node* n = new_node(NODE_IF_STMT);
    next(P);

    if (P->current.kind != TOKEN_LPAREN) print_error("Expected '(' after 'if'.");
    next(P);
    
    n->left = parse_expr(P);
    
    if (P->current.kind != TOKEN_RPAREN) print_error("Expected ')' after if condition.");
    next(P);

    n->right = new_node(NODE_IDENT);
    
    n->right->left = parse_block(P);
    n->right->right = NULL; 

    if (P->current.kind == TOKEN_ELSE) {
        next(P);
        
        if (P->current.kind == TOKEN_IF) {
            n->right->right = parse_if_stmt(P);
        } else {
            n->right->right = parse_block(P);
        }
    }

    return n;
}

static Node* parse_func_def(Parser* P) {
    Node* n = new_node(NODE_FUNC_DEF);

    if (P->current.kind != TOKEN_IDENT) print_error("Expected function name.");
    strcpy(n->name, P->current.text);
    next(P);

    if (P->current.kind != TOKEN_LPAREN) print_error("Expected '(' after function name.");
    next(P);

    Node* params_head = NULL;
    Node* params_current = NULL;
    n->arity = 0;

    if (P->current.kind != TOKEN_RPAREN) {
        do {
            if (P->current.kind != TOKEN_IDENT) print_error("Expected parameter name.");
            Node* param_node = new_node(NODE_IDENT);
            strcpy(param_node->name, P->current.text);
            next(P);
            n->arity++;
            
            if (params_head == NULL) {
                params_head = param_node;
                params_current = param_node;
            } else {
                // GUNAKAN 'next'
                params_current->next = param_node;
                params_current = param_node;
            }
        } while (P->current.kind == TOKEN_COMMA && (next(P), 1));
    }
    
    if (P->current.kind != TOKEN_RPAREN) print_error("Expected ')' after parameters.");
    next(P);

    n->left = params_head;
    n->right = parse_block(P);

    return n;
}

static Node* parse_class_def(Parser* P) {
    Node* n = new_node(NODE_CLASS_DEF);
    next(P);

    if (P->current.kind != TOKEN_IDENT) print_error("Expected class name.");
    strcpy(n->name, P->current.text);
    next(P);

    if (P->current.kind != TOKEN_LBRACE) print_error("Expected '{' before class body.");
    next(P);

    Node* methods_head = NULL;
    Node* methods_current = NULL;

    while (P->current.kind != TOKEN_RBRACE && P->current.kind != TOKEN_END) {
        if (P->current.kind != TOKEN_FUNCTION) {
            print_error("Expected 'function' keyword for method.");
            break;
        }
        next(P); 

        Node* method = parse_func_def(P);
        if (methods_head == NULL) {
            methods_head = method;
            methods_current = method;
        } else {
            // GUNAKAN 'next'
            methods_current->next = method;
            methods_current = method;
        }
    }

    if (P->current.kind != TOKEN_RBRACE) print_error("Expected '}' after class body.");
    next(P);
    
    n->left = methods_head;
    return n;
}


static Node* parse_return_stmt(Parser* P) {
    Node* n = new_node(NODE_RETURN_STMT);
    next(P);

    if (P->current.kind != TOKEN_SEMI) {
        n->left = parse_expr(P);
    } else {
        n->left = NULL;
    }

    if (P->current.kind != TOKEN_SEMI) print_error("Expected ';' after return value.");
    if (P->current.kind == TOKEN_SEMI) next(P);
    return n;
}

static Node* parse_while_stmt(Parser* P) {
    Node* n = new_node(NODE_WHILE_STMT);
    next(P);

    if (P->current.kind != TOKEN_LPAREN) print_error("Expected '(' after 'while'.");
    next(P);

    n->left = parse_expr(P);

    if (P->current.kind != TOKEN_RPAREN) print_error("Expected ')' after while condition.");
    next(P);

    n->right = parse_block(P);

    return n;
}

static Node* parse_for_stmt(Parser* P) {
    Node* n = new_node(NODE_FOR_STMT);
    
    next(P);
    if (P->current.kind != TOKEN_LPAREN) print_error("Expected '(' after 'for'.");
    next(P);

    if (P->current.kind == TOKEN_LET) {
        n->left = parse_stmt(P);
    } else {
        n->left = parse_expr(P);
        if (P->current.kind != TOKEN_SEMI) print_error("Expected ';' after initializer.");
        if (P->current.kind == TOKEN_SEMI) next(P);
    }

    n->right = new_node(NODE_IDENT);

    n->right->left = parse_expr(P);
    if (P->current.kind != TOKEN_SEMI) print_error("Expected ';' after condition.");
    if (P->current.kind == TOKEN_SEMI) next(P);

    n->right->right = new_node(NODE_IDENT);

    n->right->right->left = parse_expr(P);
    if (P->current.kind != TOKEN_RPAREN) print_error("Expected ')' after for clauses.");
    if (P->current.kind == TOKEN_RPAREN) next(P);

    n->right->right->right = parse_block(P);

    return n;
}

Node* parse_stmt(Parser* P) {
    if (P->current.kind == TOKEN_IF) {
        return parse_if_stmt(P);
    }
    
    if (P->current.kind == TOKEN_CLASS) {
        return parse_class_def(P);
    }
    
    if (P->current.kind == TOKEN_FUNCTION) {
        next(P);
        return parse_func_def(P);
    }
    
    if (P->current.kind == TOKEN_RETURN) {
        return parse_return_stmt(P);
    }

    if (P->current.kind == TOKEN_WHILE) {
        return parse_while_stmt(P);
    }

    if (P->current.kind == TOKEN_FOR) {
        return parse_for_stmt(P);
    }
    
    if (P->current.kind == TOKEN_LBRACE) {
        return parse_block(P);
    }

    if (P->current.kind == TOKEN_LET) {
        next(P);
        Node* n = new_node(NODE_VARDECL);
        if (P->current.kind != TOKEN_IDENT) print_error("Expected identifier after 'let'.");
        strcpy(n->name, P->current.text);
        next(P);
        
        if (P->current.kind != TOKEN_ASSIGN) print_error("Expected '=' after variable name.");
        next(P);
        
        n->right = parse_expr(P);
        
        if (P->current.kind != TOKEN_SEMI) print_error("Expected ';' after statement.");
        if (P->current.kind == TOKEN_SEMI) next(P);
        return n;
    }

    if (P->current.kind == TOKEN_PRINT) {
        next(P);
        Node* n = new_node(NODE_PRINT);
        n->right = parse_expr(P);
        
        if (P->current.kind != TOKEN_SEMI) print_error("Expected ';' after statement.");
        if (P->current.kind == TOKEN_SEMI) next(P);
        return n;
    }

    Node* expr = parse_expr(P);
    if (P->current.kind != TOKEN_SEMI) print_error("Expected ';' after statement.");
    if (P->current.kind == TOKEN_SEMI) next(P);
    return expr;
}

// UPDATE free_node untuk membersihkan 'next'
void free_node(Node* n) {
    if (!n) return;
    free_node(n->left);
    free_node(n->right);
    free_node(n->next); // <-- TAMBAHAN PENTING
    free(n);
}