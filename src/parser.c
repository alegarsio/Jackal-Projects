#include "parser.h"
#include <stdlib.h>
#include <string.h>

/**
 * @brief Parses annotation
 * @param P Pointer to the Parser.
 * @param N Node to the Node pointer
 */
static void parse_annotations(Parser* P, Node* n);


/**
 * @brief Parses unary break statement.
 * @param P Pointer to the Parser.
 */
static Node* parse_break_stmt(Parser* P);


/**
 * @brief Parses unary continue statement.
 * @param P Pointer to the Parser.
 */
static Node* parse_continue_stmt(Parser* P);

/**
 * @brief Parses unary ! operator statement.
 * @param P Pointer to the Parser.
 */

static Node* parse_unary(Parser* P);

/**
 * @brief Parses or || operator statement.
 * @param P Pointer to the Parser.
 */
static Node* parse_logical_or(Parser* P);

/**
 * @brief Parses a try statement.
 * @param P Pointer to the Parser.
 */
static Node* parse_try_stmt(Parser* P);

/**
 * @brief Parses a throw statement.
 * @param P Pointer to the Parser.
 */
static Node* parse_throw_stmt(Parser* P);

/**
 * @brief Parses a function expression.
 * @param P Pointer to the Parser.
 */

static Node* parse_func_expr(Parser* P);
/**
 * @brief Parses an enum definition.
 * @param P Pointer to the Parser.
 */
static Node* parse_enum_def(Parser* P);

/**
 * @brief Parses an array literal.
 * @param P Pointer to the Parser.
 */
static Node* parse_map_literal(Parser* P);

/**
 * @brief Parses an interface definition.
 * @param P Pointer to the Parser.
 */
static Node* parse_interface_def(Parser* P);
/**
 * @brief Parses an match statement
 * @param P Pointer to the Parser.
 */
static Node* parse_match_stmt(Parser* P);

/**
 * @brief Parses an import statement.
 * @param P Pointer to the Parser.
 */
static Node* parse_import_stmt(Parser* P);

/**
 * @brief Advances to the next token in the parser.
 * @param P Pointer to the Parser.
 */
static void next(Parser* P) { P->current = lexer_next(P->lexer); }

/**
 * @brief Initializes the parser with the given lexer.
 * @param P Pointer to the Parser to be initialized.
 * @param L Pointer to the Lexer to be used by the parser.
 */
void parser_init(Parser* P, Lexer* L) {
    P->lexer = L;
    next(P);
}

/**
 * @brief Creates a new AST node of the given kind.
 * @param kind The kind of the node to be created.
 * @return Pointer to the newly created Node.
 */
static Node* new_node(NodeKind kind) {
    Node* n = calloc(1, sizeof(Node));
    n->kind = kind;
    return n;
}

/**
 * @brief Parses the given source code and returns the root of the AST.
 * @param source The source code to be parsed.
 * @return Pointer to the root Node of the AST.
 */
Node *parse(const char *source) {
    (void)source;
    return NULL;
}

/**
 * @brief Parses a statement.
 * @param P Pointer to the Parser.
 * @return Pointer to the parsed Node.
 */
static Node* parse_expr(Parser* P);

/**
 * @brief Parses a statement.
 * @param P Pointer to the Parser.
 * @return Pointer to the parsed Node.
 */
Node* parse_stmt(Parser* P);

/**
 * @brief Parses an expression.
 * @param P Pointer to the Parser.
 * @return Pointer to the parsed Node.
 */
static Node* parse_primary(Parser* P);

/**
 * @brief Parses a postfix expression.
 * @param P Pointer to the Parser.
 * @return Pointer to the parsed Node.
 */
static Node* parse_multiplication(Parser* P);

/**
 * @brief Parses an addition expression.
 * @param P Pointer to the Parser.
 * @return Pointer to the parsed Node.
 */
static Node* parse_addition(Parser* P);

/**
 * @brief Parses a comparison expression.
 * @param P Pointer to the Parser.
 * @return Pointer to the parsed Node.
 */
static Node* parse_comparison(Parser* P);

/**
 * @brief Parses an equality expression.
 * @param P Pointer to the Parser.
 * @return Pointer to the parsed Node.
 */
static Node* parse_equality(Parser* P);

/**
 * @brief Parses a logical AND expression.
 * @param P Pointer to the Parser.
 * @return Pointer to the parsed Node.
 */
static Node* parse_logical_and(Parser* P);

/**
 * @brief Parses an assignment expression.
 * @param P Pointer to the Parser.
 * @return Pointer to the parsed Node.
 */
static Node* parse_assignment(Parser* P);

/**
 * @brief Parses a block of statements.
 * @param P Pointer to the Parser.
 * @return Pointer to the parsed Node.
 */
static Node* parse_block(Parser* P);

/**
 * @brief Parses an if statement.
 * @param P Pointer to the Parser.
 * @return Pointer to the parsed Node.
 */
static Node* parse_postfix(Parser* P);

/**
 * @brief Parses a function definition.
 * @param P Pointer to the Parser.
 * @return Pointer to the parsed Node.
 */
static Node* parse_func_def(Parser* P);

/**
 * @brief Parses a function call.
 * @param P Pointer to the Parser.
 * @param callee The callee node.
 * @return Pointer to the parsed Node.
 */
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
                args_current->next = arg_expr;
                args_current = arg_expr;
            }
        } while (P->current.kind == TOKEN_COMMA && (next(P), 1));
    }
    
    if (P->current.kind != TOKEN_RPAREN) print_error("Expected ')' after function arguments.");
    next(P);
    
    n->right = args_head; 
    return n;}

/**
 * @brief Parses an array literal.
 * @param P Pointer to the Parser.
 * @return Pointer to the parsed Node.
 */
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

/**
 * @brief Parses a primary expression.
 * @param P Pointer to the Parser.
 * @return Pointer to the parsed Node.
 */
static Node* parse_primary(Parser* P) {
    if (P->current.kind == TOKEN_NUMBER) {
        Node* n = new_node(NODE_NUMBER);
        n->value = P->current.number;
        next(P);
        return n;
    }

    /**
     * True Token 
     */
    if (P->current.kind == TOKEN_TRUE) {
        next(P);
        Node* n = new_node(NODE_NUMBER);
        n->value = 1.0; 
        return n;
    }

    /**
     * False Token
     */

    if (P->current.kind == TOKEN_FALSE) {
        next(P); 
        Node* n = new_node(NODE_NUMBER);
        n->value = 0.0; 
        return n;
    }
    
    if (P->current.kind == TOKEN_STRING) {
        Node* n = new_node(NODE_STRING);
        strcpy(n->name, P->current.text);
        next(P);
        return n;
    }

    if (P->current.kind == TOKEN_FUNCTION) {
        return parse_func_expr(P);
    }

    if (P->current.kind == TOKEN_LBRACKET) {
        next(P);
        return parse_array_literal(P);
    }

    if (P->current.kind == TOKEN_LBRACE) {
        return parse_map_literal(P);
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


/**
 * @brief Parses a postfix expression.
 * @param P Pointer to the Parser.
 * @return Pointer to the parsed Node.
 */
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

/**
 * @brief Parses a multiplication expression.
 * @param P Pointer to the Parser.
 * @return Pointer to the parsed Node.
 */
static Node* parse_multiplication(Parser* P) {

    Node* left = parse_unary(P);
    
    while (P->current.kind == TOKEN_STAR || P->current.kind == TOKEN_SLASH || P->current.kind == TOKEN_PERCENT) {
   
        Node* n = new_node(NODE_BINOP);
        n->op = P->current.kind;
        next(P);
        n->left = left;
        n->right = parse_unary(P);
        left = n;
    }
    return left;
}

/**
 * @brief Parses an addition expression.
 * @param P Pointer to the Parser.
 * @return Pointer to the parsed Node.
 */
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

/**
 * @brief Parses a comparison expression.
 * @param P Pointer to the Parser.
 * @return Pointer to the parsed Node.
 */
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

/**
 * @brief Parses an equality expression.
 * @param P Pointer to the Parser.
 * @return Pointer to the parsed Node.
 */
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

/**
 * @brief Parses a logical AND expression.
 * @param P Pointer to the Parser.
 * @return Pointer to the parsed Node.
 */
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

/**
 * @brief Parses an assignment expression.
 * @param P Pointer to the Parser.
 * @return Pointer to the parsed Node.
 */
static Node* parse_assignment(Parser* P) {

  

    /**
     * Logical (||) or operator
     */
    Node* node = parse_logical_or(P);

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

/**
 * @brief Parses an expression.
 * @param P Pointer to the Parser.
 * @return Pointer to the parsed Node.
 */

static Node* parse_expr(Parser* P) {
    return parse_assignment(P);
}

/**
 * @brief Parses a block of statements.
 * @param P Pointer to the Parser.
 * @return Pointer to the parsed Node.
 */

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

/**
 * @brief Parses an if statement.
 * @param P Pointer to the Parser.
 * @return Pointer to the parsed Node.
 */
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

/**
 * @brief Parses a class definition.
 * @param P Pointer to the Parser.
 * @return Pointer to the parsed Node.
 */
static Node* parse_class_def(Parser* P) {
    Node* n = new_node(NODE_CLASS_DEF);
    next(P);

    if (P->current.kind != TOKEN_IDENT) print_error("Expected class name.");
    strcpy(n->name, P->current.text);
    next(P);

    /**
     * @if condition for inheritance
     * keyword extends -> TOKEN_EXTENDS
     * expect IDENT as superclass nameß
     */
    if (P->current.kind == TOKEN_EXTENDS) {
        next(P); 
        if (P->current.kind != TOKEN_IDENT) {
            print_error("Expected superclass name after 'extends'.");
        }
        strcpy(n->super_name, P->current.text); 
        next(P); 
    } else {
        n->super_name[0] = '\0'; 
    }

    if (P->current.kind == TOKEN_IMPLEMENTS) {
        next(P);
        if (P->current.kind != TOKEN_IDENT) {
            print_error("Expected interface name after 'implements'.");
        }
        strcpy(n->interface_name, P->current.text); // Simpan nama interface
        next(P);
    } else {
        n->interface_name[0] = '\0';
    }

    if (P->current.kind != TOKEN_LBRACE) print_error("Expected '{' before class body.");
    next(P);

    Node* methods_head = NULL;
    Node* methods_current = NULL;

    while (P->current.kind != TOKEN_RBRACE && P->current.kind != TOKEN_END) {

        Node meta_node = {0}; 
        parse_annotations(P, &meta_node);

        if (P->current.kind != TOKEN_FUNCTION) {
            print_error("Expected 'function' keyword for method.");
            break;
        }
        next(P); 

        Node* method = parse_func_def(P);

        method->is_override = meta_node.is_override;
        method->is_deprecated = meta_node.is_deprecated;

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

/**
 * @brief Parses a return statement.
 * @param P Pointer to the Parser.
 * @return Pointer to the parsed Node.
 */
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

/**
 * @brief Parses a while statement.
 * @param P Pointer to the Parser.
 * @return Pointer to the parsed Node.
 */
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
/**
 * @brief Parses a for statement.
 * @param P Pointer to the Parser.
 * @return Pointer to the parsed Node.
 */
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

/**
 * @brief Parses a statement.
 * @param P Pointer to the Parser.
 * @return Pointer to the parsed Node.
 */
Node* parse_stmt(Parser* P) {

    Node meta_node = {0};

    if (P->current.kind == TOKEN_AT) {
        parse_annotations(P, &meta_node);
    }

    if (P->current.kind == TOKEN_IF) {
        return parse_if_stmt(P);
    }

    if (P->current.kind == TOKEN_MATCH) { 
        return parse_match_stmt(P);
    }

    if (P->current.kind == TOKEN_BREAK) {
        return parse_break_stmt(P);
    }
    
    if (P->current.kind == TOKEN_CONTINUE) {
        return parse_continue_stmt(P);
    }

    if (P->current.kind == TOKEN_INTERFACE) {
        return parse_interface_def(P);
    }

    if (P->current.kind == TOKEN_TRY) {
        return parse_try_stmt(P);
    }
    
    if (P->current.kind == TOKEN_THROW) {
        return parse_throw_stmt(P);
    }

    if (P->current.kind == TOKEN_IMPORT) { 
        return parse_import_stmt(P);
    }
    
    if (P->current.kind == TOKEN_CLASS) {
        return parse_class_def(P);
    }
    
    if (P->current.kind == TOKEN_FUNCTION) {
        next(P);
        Node* n = parse_func_def(P);
       
        n->is_override = meta_node.is_override;
        n->is_deprecated = meta_node.is_deprecated;
        return n;
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

    if (P->current.kind == TOKEN_ENUM) {
        return parse_enum_def(P);
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


    if (P->current.kind == TOKEN_CONST) {
        next(P);
        Node* n = new_node(NODE_CONSTDECL); 
        if (P->current.kind != TOKEN_IDENT) print_error("Expected identifier after 'const'.");
        strcpy(n->name, P->current.text);
        next(P);
        
        if (P->current.kind != TOKEN_ASSIGN) print_error("Expected '=' after constant name.");
        next(P);
        
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

/**
 * @brief Parses an import statement.
 * @param P Pointer to the Parser.
 * @return Pointer to the parsed Node.
 */
static Node* parse_import_stmt(Parser* P) {
    Node* n = new_node(NODE_IMPORT);
    next(P); 

    char path[256] = {0}; 

    if (P->current.kind != TOKEN_IDENT) {
        print_error("Expected library name after 'import'.");
    }
    strcat(path, P->current.text);
    next(P);

    while (P->current.kind == TOKEN_DOT) {
        strcat(path, "/"); 
        next(P); 
        
        if (P->current.kind != TOKEN_IDENT) {
             print_error("Expected library name after '.' in import.");
             break;
        }
        strcat(path, P->current.text);
        next(P); 
    }

    strcat(path, ".jackal");

    strcpy(n->name, path);

    if (P->current.kind != TOKEN_SEMI) print_error("Expected ';' after import.");
    if (P->current.kind == TOKEN_SEMI) next(P);
    
    return n;
}

/**
 * @brief Parses an match statement.
 * @param P Pointer to the Parser.
 * @return Pointer to the parsed Node.
 */
static Node* parse_match_stmt(Parser* P) {
    Node* n = new_node(NODE_MATCH_STMT);
    next(P);

    if (P->current.kind != TOKEN_LPAREN) print_error("Expected '(' after 'match'.");
    next(P);
    n->left = parse_expr(P); 
    if (P->current.kind != TOKEN_RPAREN) print_error("Expected ')' after match expression.");
    next(P);

    if (P->current.kind != TOKEN_LBRACE) print_error("Expected '{' before match cases.");
    next(P);

    Node* cases_head = NULL;
    Node* cases_current = NULL;

    while (P->current.kind != TOKEN_RBRACE && P->current.kind != TOKEN_END) {
        Node* case_node = new_node(NODE_MATCH_CASE);

        if (P->current.kind == TOKEN_DEFAULT) {
            next(P);
            case_node->left = NULL; 
        } else {
            case_node->left = parse_expr(P); 
        }

        if (P->current.kind != TOKEN_ARROW) print_error("Expected '=>' after match case.");
        next(P);

        if (P->current.kind == TOKEN_LBRACE) {
            case_node->right = parse_block(P);
        } else {
            case_node->right = parse_stmt(P);
        }

        if (cases_head == NULL) {
            cases_head = case_node;
            cases_current = case_node;
        } else {
            cases_current->next = case_node;
            cases_current = case_node;
        }
    }

    if (P->current.kind != TOKEN_RBRACE) print_error("Expected '}' after match cases.");
    next(P);

    n->right = cases_head;
    return n;
}

static Node* parse_interface_def(Parser* P) {
    Node* n = new_node(NODE_INTERFACE_DEF);
    next(P); 

    if (P->current.kind != TOKEN_IDENT) print_error("Expected interface name.");
    strcpy(n->name, P->current.text);
    next(P);

    if (P->current.kind != TOKEN_LBRACE) print_error("Expected '{' before interface body.");
    next(P);

    Node* methods_head = NULL;
    Node* methods_current = NULL;

    while (P->current.kind != TOKEN_RBRACE && P->current.kind != TOKEN_END) {
        if (P->current.kind != TOKEN_FUNCTION) {
            print_error("Expected 'function' keyword in interface.");
            break;
        }
        next(P); 

        Node* method = new_node(NODE_FUNC_DEF);
        if (P->current.kind != TOKEN_IDENT) print_error("Expected function name.");
        strcpy(method->name, P->current.text);
        next(P);

        if (P->current.kind != TOKEN_LPAREN) print_error("Expected '(' after function name.");
        next(P);

        method->arity = 0;
        if (P->current.kind != TOKEN_RPAREN) {
            do {
                if (P->current.kind != TOKEN_IDENT) print_error("Expected parameter name.");
                next(P);
                method->arity++;
            } while (P->current.kind == TOKEN_COMMA && (next(P), 1));
        }
        if (P->current.kind != TOKEN_RPAREN) print_error("Expected ')' after parameters.");
        next(P);

        if (P->current.kind != TOKEN_SEMI) print_error("Expected ';' after interface method declaration.");
        next(P);

        method->left = NULL; 
        method->right = NULL;

        if (methods_head == NULL) {
            methods_head = method;
            methods_current = method;
        } else {
            methods_current->next = method;
            methods_current = method;
        }
    }

    if (P->current.kind != TOKEN_RBRACE) print_error("Expected '}' after interface body.");
    next(P);
    
    n->left = methods_head;
    return n;
}

/**
 * @brief Parses a map literal.
 * @param P Pointer to the Parser.
 * @return Pointer to the parsed Node.
 */

static Node* parse_map_literal(Parser* P) {
    Node* n = new_node(NODE_MAP_LITERAL);
    next(P); 
    
    Node* head = NULL; 
    Node* current = NULL;

    while (P->current.kind != TOKEN_RBRACE && P->current.kind != TOKEN_END) {
        if (P->current.kind != TOKEN_STRING) {
             print_error("Expected string key in map literal.");
        }
        char key_str[64];
        strncpy(key_str, P->current.text, 63);
        key_str[63] = '\0';
        next(P); 

        if (P->current.kind != TOKEN_COLON) print_error("Expected ':' after map key.");
        next(P); 

        Node* value_node = parse_expr(P); 

        Node* entry = new_node(NODE_MAP_LITERAL);
        strcpy(entry->name, key_str);
        entry->left = value_node;

        if (head == NULL) { head = entry; current = entry; }
        else { current->next = entry; current = entry; }

        if (P->current.kind == TOKEN_COMMA) next(P);
    }
    if (P->current.kind != TOKEN_RBRACE) print_error("Expected '}' after map.");
    next(P); 
    
    n->left = head;
    return n;
}

/**
 * @brief Parses an enum definition.
 * @param P Pointer to the Parser.
 * @return Pointer to the parsed Node.
 */
static Node* parse_enum_def(Parser* P) {
    Node* n = new_node(NODE_ENUM_DEF);
    next(P); 

    if (P->current.kind != TOKEN_IDENT) print_error("Expected enum name.");
    strcpy(n->name, P->current.text);
    next(P);

    if (P->current.kind != TOKEN_LBRACE) print_error("Expected '{' before enum body.");
    next(P);

    Node* head = NULL;
    Node* current = NULL;
    int currentValue = 0; 

    while (P->current.kind != TOKEN_RBRACE && P->current.kind != TOKEN_END) {
        if (P->current.kind != TOKEN_IDENT) {
            print_error("Expected enum constant name.");
        }
        Node* entry = new_node(NODE_ENUM_DEF); 
        strcpy(entry->name, P->current.text);
        next(P);

        if (P->current.kind == TOKEN_ASSIGN) {
            next(P);
            if (P->current.kind != TOKEN_NUMBER) print_error("Enum value must be a number.");
            entry->value = P->current.number;
            currentValue = (int)P->current.number + 1; // Update counter
            next(P);
        } else {
            entry->value = (double)currentValue++;
        }

        if (head == NULL) { head = entry; current = entry; }
        else { current->next = entry; current = entry; }

        if (P->current.kind == TOKEN_COMMA) next(P);
    }

    if (P->current.kind != TOKEN_RBRACE) print_error("Expected '}' after enum body.");
    next(P);
    
    n->left = head; 
    return n;
}


static Node* parse_func_expr(Parser* P) {
    Node* n = new_node(NODE_FUNC_EXPR);
    next(P);

    
    if (P->current.kind != TOKEN_LPAREN) print_error("Expected '(' after 'function'.");
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

/**
 * @brief Parses a throw statement.
 * @param P Pointer to the Parser.
 * @return Pointer to the parsed Node.
 */
static Node* parse_throw_stmt(Parser* P) {
    Node* n = new_node(NODE_THROW_STMT);
    next(P); 
    n->left = parse_expr(P); 
    if (P->current.kind != TOKEN_SEMI) print_error("Expected ';' after throw.");
    if (P->current.kind == TOKEN_SEMI) next(P);
    return n;
}

/**
 * @brief Parses a break statement.
 * @param P Pointer to the Parser.
 * @return Pointer to the parsed Node.
 */
static Node* parse_break_stmt(Parser* P) {
    Node* n = new_node(NODE_BREAK_STMT);
    next(P); 
    if (P->current.kind != TOKEN_SEMI) print_error("Expected ';' after 'break'.");
    if (P->current.kind == TOKEN_SEMI) next(P);
    return n;
}

static Node* parse_continue_stmt(Parser* P) {
    Node* n = new_node(NODE_CONTINUE_STMT);
    next(P); // Lewati 'continue'
    if (P->current.kind != TOKEN_SEMI) print_error("Expected ';' after 'continue'.");
    if (P->current.kind == TOKEN_SEMI) next(P);
    return n;
}

/**
 * @brief Parses a try statement.
 * @param P Pointer to the Parser.
 * @return Pointer to the parsed Node.
 */
static Node* parse_try_stmt(Parser* P) {
    Node* n = new_node(NODE_TRY_STMT);
    next(P); 
    n->left = parse_block(P);

    if (P->current.kind != TOKEN_CATCH) {
        print_error("Expected 'catch' after try block.");
        return n;
    }
    next(P); 
    if (P->current.kind != TOKEN_LPAREN) print_error("Expected '(' after catch.");
    next(P);
    if (P->current.kind != TOKEN_IDENT) print_error("Expected error variable name.");
    strcpy(n->name, P->current.text); 
    next(P);
    if (P->current.kind != TOKEN_RPAREN) print_error("Expected ')' after error variable.");
    next(P);

    n->right = parse_block(P);

    return n;
}
/**
 * @brief Parses a logical operator.
 * @param P Pointer to the Parser.
 * @return Pointer to the parsed Node.
 */
static Node* parse_logical_or(Parser* P) {
    Node* left = parse_logical_and(P); 

    while (P->current.kind == TOKEN_PIPE_PIPE) {
        Node* n = new_node(NODE_BINOP);
        n->op = P->current.kind;
        next(P);
        n->left = left;
        n->right = parse_logical_and(P); 
        left = n;
    }
    return left;
}

/**
 * @brief Parses a unary operator.
 * @param P Pointer to the Parser.
 * @return Pointer to the parsed Node.
 */
static Node* parse_unary(Parser* P) {

    if (P->current.kind == TOKEN_BANG || P->current.kind == TOKEN_MINUS) {
        Node* n = new_node(NODE_UNARY);
        n->op = P->current.kind;
        next(P);

        n->right = parse_unary(P); 
        return n;
    }

    return parse_postfix(P);
}



static void parse_annotations(Parser* P, Node* n) {
   
    n->is_override = false;
    n->is_deprecated = false;

    while (P->current.kind == TOKEN_AT) {
        next(P);
        
        if (P->current.kind != TOKEN_IDENT) {
            print_error("Expected annotation name after '@'.");
            return;
        }

        if (strcmp(P->current.text, "override") == 0) {
            n->is_override = true;
        } else if (strcmp(P->current.text, "deprecated") == 0) {
            n->is_deprecated = true;
        } else {
            print_error("Unknown annotation '@%s'.", P->current.text);
        }
        next(P); 
    }
}
/**
 * @brief Frees the memory allocated for the given AST node and its children.
 * @param n Pointer to the Node to be freed.
 */
void free_node(Node* n) {
    if (!n) return;
    free_node(n->left);
    free_node(n->right);
    free_node(n->next); 
    free(n);
}
