#include "parser.h"
#include <stdlib.h>
#include <string.h>

static Node *parse_extract_stmt(Parser *P);

/**
 *
 */
static Node *parse_observe_stmt(Parser *P);
/**
 *
 */
static Node *parse_every_loop(Parser *P);
/**
 * @brief parse when expresssion
 * @param P Pointer to the parser
 */
static Node *parse_when_expr(Parser *P);

/**
 * @brief Parses annotation
 * @param P Pointer to the Parser.
 * @param N Node to the Node pointer
 */
static void parse_annotations(Parser *P, Node *n);

/**
 * @brief Parses unary break statement.
 * @param P Pointer to the Parser.
 */
static Node *parse_break_stmt(Parser *P);

/**
 * @brief Parses unary continue statement.
 * @param P Pointer to the Parser.
 */
static Node *parse_continue_stmt(Parser *P);

/**
 * @brief Parses unary ! operator statement.
 * @param P Pointer to the Parser.
 */

static Node *parse_unary(Parser *P);

/**
 * @brief Parses or || operator statement.
 * @param P Pointer to the Parser.
 */
static Node *parse_logical_or(Parser *P);

/**
 * @brief Parses a try statement.
 * @param P Pointer to the Parser.
 */
static Node *parse_try_stmt(Parser *P);

/**
 * @brief Parses a throw statement.
 * @param P Pointer to the Parser.
 */
static Node *parse_throw_stmt(Parser *P);

/**
 * @brief Parses a function expression.
 * @param P Pointer to the Parser.
 */

static Node *parse_func_expr(Parser *P);
/**
 * @brief Parses an enum definition.
 * @param P Pointer to the Parser.
 */
static Node *parse_enum_def(Parser *P);

/**
 * @brief Parses an array literal.
 * @param P Pointer to the Parser.
 */
static Node *parse_map_literal(Parser *P);

/**
 * @brief Parses an interface definition.
 * @param P Pointer to the Parser.
 */
static Node *parse_interface_def(Parser *P);
/**
 * @brief Parses an match statement
 * @param P Pointer to the Parser.
 */
static Node *parse_match_stmt(Parser *P);

/**
 * @brief Parses an import statement.
 * @param P Pointer to the Parser.
 */
static Node *parse_import_stmt(Parser *P);

/**
 * @brief Advances to the next token in the parser.
 * @param P Pointer to the Parser.
 */
static void next(Parser *P) { P->current = lexer_next(P->lexer); }

/**
 * @brief Initializes the parser with the given lexer.
 * @param P Pointer to the Parser to be initialized.
 * @param L Pointer to the Lexer to be used by the parser.
 */
void parser_init(Parser *P, Lexer *L)
{
    P->lexer = L;
    next(P);
}

/**
 * @brief Creates a new AST node of the given kind.
 * @param kind The kind of the node to be created.
 * @return Pointer to the newly created Node.
 */
Node *new_node(NodeKind kind)
{
    Node *n = calloc(1, sizeof(Node));
    n->kind = kind;
    return n;
}

/**
 * @brief Parses the given source code and returns the root of the AST.
 * @param source The source code to be parsed.
 * @return Pointer to the root Node of the AST.
 */
Node *parse(const char *source)
{
    (void)source;
    return NULL;
}

/**
 * @brief Parses a statement.
 * @param P Pointer to the Parser.
 * @return Pointer to the parsed Node.
 */
static Node *parse_expr(Parser *P);

/**
 * @brief Parses a statement.
 * @param P Pointer to the Parser.
 * @return Pointer to the parsed Node.
 */
Node *parse_stmt(Parser *P);

/**
 * @brief Parses an expression.
 * @param P Pointer to the Parser.
 * @return Pointer to the parsed Node.
 */
static Node *parse_primary(Parser *P);

/**
 * @brief Parses a postfix expression.
 * @param P Pointer to the Parser.
 * @return Pointer to the parsed Node.
 */
static Node *parse_multiplication(Parser *P);

/**
 * @brief Parses an addition expression.
 * @param P Pointer to the Parser.
 * @return Pointer to the parsed Node.
 */
static Node *parse_addition(Parser *P);

/**
 * @brief Parses a comparison expression.
 * @param P Pointer to the Parser.
 * @return Pointer to the parsed Node.
 */
static Node *parse_comparison(Parser *P);

/**
 * @brief Parses an equality expression.
 * @param P Pointer to the Parser.
 * @return Pointer to the parsed Node.
 */
static Node *parse_equality(Parser *P);

/**
 * @brief Parses a logical AND expression.
 * @param P Pointer to the Parser.
 * @return Pointer to the parsed Node.
 */
static Node *parse_logical_and(Parser *P);

/**
 * @brief Parses an assignment expression.
 * @param P Pointer to the Parser.
 * @return Pointer to the parsed Node.
 */
static Node *parse_assignment(Parser *P);

/**
 * @brief Parses a block of statements.
 * @param P Pointer to the Parser.
 * @return Pointer to the parsed Node.
 */
static Node *parse_block(Parser *P);

/**
 * @brief Parses an if statement.
 * @param P Pointer to the Parser.
 * @return Pointer to the parsed Node.
 */
static Node *parse_postfix(Parser *P);

/**
 * @brief Parses a function definition.
 * @param P Pointer to the Parser.
 * @return Pointer to the parsed Node.
 */
static Node *parse_func_def(Parser *P);

/**
 * @brief Parses a function call.
 * @param P Pointer to the Parser.
 * @param callee The callee node.
 * @return Pointer to the parsed Node.
 */
static Node *parse_func_call(Parser *P, Node *callee)
{
    Node *n = new_node(NODE_FUNC_CALL);
    n->left = callee;

    Node *args_head = NULL;
    Node *args_current = NULL;
    n->arity = 0;

    if (P->current.kind != TOKEN_RPAREN)
    {
        do
        {
            Node *arg_expr = parse_expr(P);
            n->arity++;

            if (args_head == NULL)
            {
                args_head = arg_expr;
                args_current = arg_expr;
            }
            else
            {
                args_current->next = arg_expr;
                args_current = arg_expr;
            }
        } while (P->current.kind == TOKEN_COMMA && (next(P), 1));
    }

    if (P->current.kind != TOKEN_RPAREN)
        print_error("Expected ')' after function arguments.");
    next(P);

    n->right = args_head;
    return n;
}

/**
 * @brief Parses an array literal.
 * @param P Pointer to the Parser.
 * @return Pointer to the parsed Node.
 */
static Node *parse_array_literal(Parser *P)
{
    Node *n = new_node(NODE_ARRAY_LITERAL); //

    Node *items_head = NULL;
    Node *items_current = NULL;
    n->arity = 0;

    if (P->current.kind != TOKEN_RBRACKET) //
    {
        do
        {
            if (P->current.kind == TOKEN_RBRACKET)
                break;

            Node *item_expr = parse_expr(P); //

            if (item_expr == NULL)
                break;

            n->arity++;

            if (items_head == NULL)
            {
                items_head = item_expr;
                items_current = item_expr;
            }
            else
            {
                items_current->next = item_expr;
                items_current = item_expr;
            }
        } while (P->current.kind == TOKEN_COMMA && (next(P), 1)); //
    }

    if (P->current.kind != TOKEN_RBRACKET)
    {
        print_error("Expected ']' after array items."); //
    }

    next(P); // Konsumsi TOKEN_RBRACKET

    n->left = items_head;
    return n;
}

/**
 * @brief Parses a primary expression.
 * @param P Pointer to the Parser.
 * @return Pointer to the parsed Node.
 */
static Node *parse_primary(Parser *P)
{

    if (P->current.kind == TOKEN_NUMBER)
    {
        Node *n = new_node(NODE_NUMBER);
        n->value = P->current.number;
        next(P);
        return n;
    }

    /**
     * True Token
     */
    if (P->current.kind == TOKEN_TRUE)
    {
        next(P);
        Node *n = new_node(NODE_NUMBER);
        n->value = 1.0;
        return n;
    }

    /**
     * False Token
     */

    if (P->current.kind == TOKEN_FALSE)
    {
        next(P);
        Node *n = new_node(NODE_NUMBER);
        n->value = 0.0;
        return n;
    }

    if (P->current.kind == TOKEN_STRING)
    {
        Node *n = new_node(NODE_STRING);
        strcpy(n->name, P->current.text);
        next(P);
        return n;
    }

    if (P->current.kind == TOKEN_FUNCTION)
    {
        return parse_func_expr(P);
    }

    if (P->current.kind == TOKEN_WHEN)
    {
        return parse_when_expr(P);
    }

    if (P->current.kind == TOKEN_LBRACKET)
    {
        next(P);
        return parse_array_literal(P);
    }

    if (P->current.kind == TOKEN_LBRACE)
    {
        return parse_map_literal(P);
    }

    if (P->current.kind == TOKEN_THIS)
    {
        Node *n = new_node(NODE_THIS);
        strcpy(n->name, "this");
        next(P);
        return n;
    }

    if (P->current.kind == TOKEN_IDENT)
    {
        Node *n = new_node(NODE_IDENT);
        strcpy(n->name, P->current.text);
        next(P);
        return n;
    }
    if (P->current.kind == TOKEN_LPAREN)
    {
        next(P);
        Node *n = parse_expr(P);
        if (P->current.kind == TOKEN_RPAREN)
            next(P);
        return n;
    }
    return NULL;
}
static Node *parse_template_declaration(Parser *P)
{
    if (P->current.kind != TOKEN_LESS)
        return NULL;

    size_t saved_pos = P->lexer->pos;
    Token saved_token = P->current;

    next(P);

    Node *head = NULL;
    Node *current = NULL;

    if (P->current.kind != TOKEN_GREATER)
    {
        do
        {
            if (P->current.kind != TOKEN_IDENT)
            {
                free_node(head);
                goto fail;
            }

            Node *type_param = new_node(NODE_IDENT);
            strcpy(type_param->name, P->current.text);
            next(P);

            if (head == NULL)
            {
                head = type_param;
                current = type_param;
            }
            else
            {
                current->next = type_param;
                current = type_param;
            }
        } while (P->current.kind == TOKEN_COMMA && (next(P), 1));
    }

    if (P->current.kind != TOKEN_GREATER)
    {
        free_node(head);
        goto fail;
    }

    next(P);
    return head;

fail:
    P->lexer->pos = saved_pos;
    P->current = saved_token;
    return NULL;
}

/**
 * @brief Parses a postfix expression.
 * @param P Pointer to the Parser.
 * @return Pointer to the parsed Node.
 */
static Node *parse_postfix(Parser *P)
{
    Node *node = parse_primary(P);

    while (1)
    {

        if (node != NULL && node->kind == NODE_IDENT && P->current.kind == TOKEN_LESS)
        {
            Node *template_args = parse_template_declaration(P);

            if (template_args != NULL)
            {
                node->template_types = template_args;

                continue;
            }
        }

        else if (P->current.kind == TOKEN_LBRACKET)
        {
            next(P);
            Node *access_node = new_node(NODE_ARRAY_ACCESS);
            access_node->left = node;
            access_node->right = parse_expr(P);
            if (P->current.kind != TOKEN_RBRACKET)
                print_error("Expected ']' after index.");
            next(P);
            node = access_node;
        }

        if (P->current.kind == TOKEN_LPAREN)
        {
            next(P);
            node = parse_func_call(P, node);
        }

        else if (P->current.kind == TOKEN_DOT)
        {
            next(P);
            if (P->current.kind != TOKEN_IDENT)
                print_error("Expected property name after '.'.");
            Node *get_node = new_node(NODE_GET);
            get_node->left = node;
            strcpy(get_node->name, P->current.text);
            next(P);
            node = get_node;
        }
        else if (P->current.kind == TOKEN_PLUS_PLUS)
        {
            next(P);
            Node *inc = new_node(NODE_POST_INC);
            inc->left = node;
            node = inc;
        }
        else if (P->current.kind == TOKEN_MINUS_MINUS)
        {
            next(P);
            Node *dec = new_node(NODE_POST_DEC);
            dec->left = node;
            node = dec;
        }
        else
        {
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
static Node *parse_multiplication(Parser *P)
{

    Node *left = parse_unary(P);

    while (P->current.kind == TOKEN_STAR || P->current.kind == TOKEN_SLASH || P->current.kind == TOKEN_PERCENT)
    {

        Node *n = new_node(NODE_BINOP);
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
static Node *parse_addition(Parser *P)
{
    Node *left = parse_multiplication(P);
    while (P->current.kind == TOKEN_PLUS || P->current.kind == TOKEN_MINUS)
    {
        Node *n = new_node(NODE_BINOP);
        n->op = P->current.kind;
        next(P);
        n->left = left;
        n->right = parse_multiplication(P);
        left = n;
    }
    return left;
}

static Node *parse_range(Parser *P)
{
    Node *left = parse_addition(P);
    if (P->current.kind == TOKEN_TO)
    {
        next(P);
        Node *n = new_node(NODE_RANGE_EXPR);
        n->left = left;
        n->right = parse_addition(P);

        if (P->current.kind == TOKEN_STEP)
        {
            next(P);
            n->next = parse_addition(P);
        }
        else
        {
            n->next = NULL;
        }
        return n;
    }
    return left;
}
/**
 * @brief Parses a comparison expression.
 * @param P Pointer to the Parser.
 * @return Pointer to the parsed Node.
 */
static Node *parse_comparison(Parser *P)
{
    Node *left = parse_range(P);
    while (P->current.kind == TOKEN_GREATER || P->current.kind == TOKEN_GREATER_EQUAL ||
           P->current.kind == TOKEN_LESS || P->current.kind == TOKEN_LESS_EQUAL)
    {
        Node *n = new_node(NODE_BINOP);
        n->op = P->current.kind;
        next(P);
        n->left = left;
        n->right = parse_range(P);
        left = n;
    }
    return left;
}

/**
 * @brief Parses an equality expression.
 * @param P Pointer to the Parser.
 * @return Pointer to the parsed Node.
 */
static Node *parse_equality(Parser *P)
{
    Node *left = parse_comparison(P);
    while (P->current.kind == TOKEN_EQUAL_EQUAL || P->current.kind == TOKEN_BANG_EQUAL)
    {
        Node *n = new_node(NODE_BINOP);
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
static Node *parse_logical_and(Parser *P)
{
    Node *left = parse_equality(P);
    while (P->current.kind == TOKEN_AND_AND)
    {
        Node *n = new_node(NODE_BINOP);
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
static Node *parse_assignment(Parser *P)
{

    /**
     * Logical (||) or operator
     */
    Node *node = parse_logical_or(P);

    if (P->current.kind == TOKEN_ASSIGN)
    {
        next(P);
        Node *value = parse_assignment(P);

        if (node->kind == NODE_IDENT)
        {
            Node *assign_node = new_node(NODE_ASSIGN);
            strcpy(assign_node->name, node->name);
            assign_node->left = value;
            free_node(node);
            return assign_node;
        }

        if (node->kind == NODE_ARRAY_ACCESS)
        {
            Node *assign_node = new_node(NODE_ARRAY_ASSIGN);
            assign_node->left = node;
            assign_node->right = value;
            return assign_node;
        }

        if (node->kind == NODE_GET)
        {
            Node *set_node = new_node(NODE_SET);
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

static Node *parse_expr(Parser *P)
{
    return parse_assignment(P);
}

/**
 * @brief Parses a block of statements.
 * @param P Pointer to the Parser.
 * @return Pointer to the parsed Node.
 */

static Node *parse_block(Parser *P)
{
    Node *n = new_node(NODE_BLOCK);

    if (P->current.kind != TOKEN_LBRACE)
    {
        print_error("Expected '{' to start a block.");
        return n;
    }
    next(P);

    Node *head = NULL;
    Node *current = NULL;

    while (P->current.kind != TOKEN_RBRACE && P->current.kind != TOKEN_END)
    {
        Node *stmt = parse_stmt(P);
        if (!stmt)
            continue;

        if (head == NULL)
        {
            head = stmt;
            current = stmt;
        }
        else
        {
            // GUNAKAN 'next'
            current->next = stmt;
            current = stmt;
        }
    }

    if (P->current.kind != TOKEN_RBRACE)
    {
        print_error("Expected '}' to end a block.");
    }
    else
    {
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
static Node *parse_if_stmt(Parser *P)
{
    Node *n = new_node(NODE_IF_STMT);
    next(P);

    if (P->current.kind != TOKEN_LPAREN)
        print_error("Expected '(' after 'if'.");
    next(P);

    n->left = parse_expr(P);

    if (P->current.kind != TOKEN_RPAREN)
        print_error("Expected ')' after if condition.");
    next(P);

    n->right = new_node(NODE_IDENT);

    n->right->left = parse_block(P);
    n->right->right = NULL;

    if (P->current.kind == TOKEN_ELSE)
    {
        next(P);

        if (P->current.kind == TOKEN_IF)
        {
            n->right->right = parse_if_stmt(P);
        }
        else
        {
            n->right->right = parse_block(P);
        }
    }

    return n;
}

/**
 *
 */
static Node *parse_func_def(Parser *P)
{
    Node *n = new_node(NODE_FUNC_DEF);

    if (P->current.kind != TOKEN_IDENT)
        print_error("Expected function name.");
    strcpy(n->name, P->current.text);
    next(P);

    n->template_types = parse_template_declaration(P);

    if (P->current.kind != TOKEN_LPAREN)
        print_error("Expected '(' after function name.");
    next(P);

    Node *params_head = NULL;
    Node *params_current = NULL;
    n->arity = 0;

    if (P->current.kind != TOKEN_RPAREN)
    {
        do
        {
            if (P->current.kind != TOKEN_IDENT)
                print_error("Expected parameter name.");
            Node *param_node = new_node(NODE_IDENT);
            strcpy(param_node->name, P->current.text);
            next(P);
            n->arity++;

            if (P->current.kind == TOKEN_COLON)
            {
                next(P);
                if (P->current.kind != TOKEN_IDENT)
                {
                    print_error("Expected type name after ':'.");
                    param_node->type_name[0] = '\0';
                }
                else
                {
                    strcpy(param_node->type_name, P->current.text);
                    next(P);
                }
            }
            else
            {
                param_node->type_name[0] = '\0';
            }

            if (params_head == NULL)
            {
                params_head = param_node;
                params_current = param_node;
            }
            else
            {
                params_current->next = param_node;
                params_current = param_node;
            }
        } while (P->current.kind == TOKEN_COMMA && (next(P), 1));
    }

    if (P->current.kind != TOKEN_RPAREN)
        print_error("Expected ')' after parameters.");
    next(P);

    if (P->current.kind == TOKEN_ARROW_TO_RETURN)
    {
        next(P);
        if (P->current.kind != TOKEN_IDENT)
        {
            print_error("Expected type name after '->'.");
            n->return_type[0] = '\0';
        }
        else
        {
            strcpy(n->return_type, P->current.text);
            next(P);
        }
    }
    else
    {
        n->return_type[0] = '\0';
    }

    n->left = params_head;
    n->right = parse_block(P);

    return n;
}

/**
 * @brief Parses a class definition.
 * Supports: class Name extends Super implements Interface { ... }
 */
static Node *parse_class_def(Parser *P)
{
    Node *n = new_node(NODE_CLASS_DEF);
    Node *template_check = parse_template_declaration(P);

    if (P->current.kind == TOKEN_OBJECT)
    {
        n->is_singleton = true;
        n->is_record = false;
    }
    next(P);

    if (P->current.kind != TOKEN_IDENT)
        print_error("Expected class/record name.");

    strcpy(n->name, P->current.text);
    next(P);

    n->template_types = parse_template_declaration(P);
    n->super_name[0] = '\0';
    n->interface_name[0] = '\0';

    Node *interface_list_head = NULL;

    Node *fields_head = NULL;
    Node *fields_current = NULL;

    if (P->current.kind == TOKEN_LPAREN)
    {
        next(P);

        if (P->current.kind != TOKEN_RPAREN)
        {
            do
            {
                if (P->current.kind != TOKEN_IDENT)
                    print_error("Expected field name in definition.");

                Node *field_node = new_node(NODE_IDENT);
                strcpy(field_node->name, P->current.text);
                next(P);

                field_node->type_name[0] = '\0';
                if (P->current.kind == TOKEN_COLON)
                {
                    next(P);
                    if (P->current.kind != TOKEN_IDENT)
                        print_error("Expected type name after ':'.");
                    else
                    {
                        strcpy(field_node->type_name, P->current.text);
                        next(P);
                    }
                }

                if (fields_head == NULL)
                {
                    fields_head = field_node;
                    fields_current = field_node;
                }
                else
                {
                    fields_current->next = field_node;
                    fields_current = field_node;
                }
            } while (P->current.kind == TOKEN_COMMA && (next(P), 1));
        }

        if (P->current.kind != TOKEN_RPAREN)
            print_error("Expected ')' after parameters.");
        next(P);

        n->right = fields_head;
    }

    if (n->is_record)
    {
        if (P->current.kind == TOKEN_EXTENDS || P->current.kind == TOKEN_IMPLEMENTS)
        {
            print_error("Record cannot extend or implement other types.");
        }
        if (P->current.kind == TOKEN_LBRACE)
        {
            print_error("Record definition cannot have a body or methods.");

            while (P->current.kind != TOKEN_RBRACE && P->current.kind != TOKEN_END)
            {
                next(P);
            }
            if (P->current.kind == TOKEN_RBRACE)
                next(P);
        }

        n->left = NULL;
        return n;
    }

    if (n->is_singleton)
    {
        if (template_check != NULL)
        {
            print_error("Error: Object (Singleton) cannot have template type parameters.");

            free_node(template_check);
            return NULL;
        }

        if (P->current.kind == TOKEN_EXTENDS || P->current.kind == TOKEN_IMPLEMENTS)
        {
            print_error("Error: Object cannot extend or implement other types.");
        }
        n->super_name[0] = '\0';
        n->interface_name[0] = '\0';
    }

    else
    {
        n->template_types = template_check;
    }

    while (P->current.kind == TOKEN_EXTENDS || P->current.kind == TOKEN_IMPLEMENTS)
    {
        if (P->current.kind == TOKEN_EXTENDS)
        {
            if (n->super_name[0] != '\0')
            {
                print_error("Class can only extend one superclass.");
            }
            next(P);
            if (P->current.kind != TOKEN_IDENT)
            {
                print_error("Expected superclass name after 'extends'.");
            }
            strcpy(n->super_name, P->current.text);
            next(P);

            Node *super_template_args = parse_template_declaration(P);
            if (super_template_args != NULL)
            {
                n->super_template_types = super_template_args;
            }
        }
        else if (P->current.kind == TOKEN_IMPLEMENTS)
        {
            if (interface_list_head != NULL)
            {
                print_error("Interface implementation already defined.");
            }
            next(P);

            if (P->current.kind == TOKEN_LPAREN)
            {
                next(P);

                Node *current_list = NULL;

                do
                {
                    if (P->current.kind != TOKEN_IDENT)
                        print_error("Expected interface name after 'implements (' or comma.");

                    Node *interface_node = new_node(NODE_IDENT);
                    strcpy(interface_node->name, P->current.text);

                    interface_node->next = current_list;
                    current_list = interface_node;

                    next(P);

                } while (P->current.kind == TOKEN_COMMA && (next(P), 1));

                if (P->current.kind != TOKEN_RPAREN)
                    print_error("Expected ')' after multiple interface list.");
                next(P);

                interface_list_head = current_list;
            }
            else
            {
                if (P->current.kind != TOKEN_IDENT)
                    print_error("Expected interface name after 'implements'.");

                strcpy(n->interface_name, P->current.text);

                Node *interface_node = new_node(NODE_IDENT);
                strcpy(interface_node->name, P->current.text);
                interface_list_head = interface_node;

                next(P);
            }
        }
    }

    n->next = interface_list_head;

    if (P->current.kind != TOKEN_LBRACE)
        print_error("Expected '{' before class body.");
    next(P);

    Node *methods_head = NULL;
    Node *methods_current = NULL;

    while (P->current.kind != TOKEN_RBRACE && P->current.kind != TOKEN_END)
    {
        Node meta_node = {0};
        parse_annotations(P, &meta_node);
        bool is_private = false;

        if (P->current.kind == TOKEN_PRIVATE)
        {
            is_private = true;
            next(P);
        }
        bool is_constructor = false;
        if (P->current.kind == TOKEN_IDENT && strcmp(P->current.text, "init") == 0)
        {
            is_constructor = true;
        }

        if (n->is_singleton && is_constructor)
        {
            print_error("Object cannot define 'init()', use initialization parameters (or remove the parameters).");
            Node *temp_method = parse_func_def(P);
            free_node(temp_method);
            continue;
        }

        if (P->current.kind != TOKEN_FUNCTION && !is_constructor)
        {
            print_error("Expected 'function' keyword or constructor 'init' declaration.");
            next(P);
            continue;
        }

        if (!is_constructor)
        {
            next(P);
        }

        Node *method = parse_func_def(P);

        method->is_private = is_private;
        method->is_override = meta_node.is_override;
        method->is_deprecated = meta_node.is_deprecated;

        if (methods_head == NULL)
        {
            methods_head = method;
            methods_current = method;
        }
        else
        {
            methods_current->next = method;
            methods_current = method;
        }
    }

    if (P->current.kind != TOKEN_RBRACE)
        print_error("Expected '}' after class body.");
    next(P);

    n->left = methods_head;
    return n;
}
/**
 * @brief Parses a return statement.
 * @param P Pointer to the Parser.
 * @return Pointer to the parsed Node.
 */
static Node *parse_return_stmt(Parser *P)
{
    Node *n = new_node(NODE_RETURN_STMT);
    next(P);

    if (P->current.kind != TOKEN_SEMI)
    {
        n->left = parse_expr(P);
    }
    else
    {
        n->left = NULL;
    }

    if (P->current.kind == TOKEN_SEMI)
        next(P);
    return n;
}

/**
 * @brief Parses a while statement.
 * @param P Pointer to the Parser.
 * @return Pointer to the parsed Node.
 */
static Node *parse_while_stmt(Parser *P)
{
    Node *n = new_node(NODE_WHILE_STMT);
    next(P);

    if (P->current.kind != TOKEN_LPAREN)
        print_error("Expected '(' after 'while'.");
    next(P);

    n->left = parse_expr(P);

    if (P->current.kind != TOKEN_RPAREN)
        print_error("Expected ')' after while condition.");
    next(P);

    n->right = parse_block(P);

    return n;
}

static Node *parse_for_each_clauses(Parser *P)
{
    if (P->current.kind != TOKEN_IDENT)
        return NULL;
    Node *item_var = new_node(NODE_IDENT);
    strcpy(item_var->name, P->current.text);
    next(P);

    if (P->current.kind != TOKEN_IN)
    {
        free_node(item_var);
        return NULL;
    }
    next(P);

    Node *n = new_node(NODE_FOR_EACH);
    n->left = item_var;
    n->right = parse_expr(P);
    return n;
}
/**
 * @brief Parses a for statement.
 * @param P Pointer to the Parser.
 * @return Pointer to the parsed Node.
 */
static Node *parse_for_stmt(Parser *P)
{
    next(P);

    if (P->current.kind != TOKEN_LPAREN)
        print_error("Expected '(' after 'for'.");
    next(P);

    if (P->current.kind == TOKEN_IDENT)
    {
        Parser saved_P = *P;
        Node *iteration_node = parse_for_each_clauses(P);

        if (iteration_node != NULL)
        {
            if (P->current.kind != TOKEN_RPAREN)
                print_error("Expected ')' after for-each clauses.");
            next(P);

            iteration_node->super_template_types = parse_block(P);
            return iteration_node;
        }
        *P = saved_P;
    }

    Node *n = new_node(NODE_FOR_STMT);
    if (P->current.kind == TOKEN_LET)
    {
        n->left = parse_stmt(P);
    }
    else
    {
        n->left = parse_expr(P);
        if (P->current.kind == TOKEN_SEMI)
            next(P);
    }

    n->right = new_node(NODE_IDENT);
    n->right->left = parse_expr(P);
    if (P->current.kind == TOKEN_SEMI)
        next(P);

    n->right->right = new_node(NODE_IDENT);
    n->right->right->left = parse_expr(P);
    if (P->current.kind == TOKEN_RPAREN)
        next(P);

    n->right->right->right = parse_block(P);
    return n;
}
static Node *parse_for_in_stmt(Parser *P)
{
    Node *n = new_node(NODE_FOR_IN);

    if (P->current.kind != TOKEN_IDENT)
    {
        return NULL;
    }

    Node *key_var = new_node(NODE_IDENT);
    strcpy(key_var->name, P->current.text);
    next(P);

    if (P->current.kind != TOKEN_COMMA)
    {
        free_node(key_var);
        return NULL;
    }
    next(P);

    if (P->current.kind != TOKEN_IDENT)
    {
        print_error("Expected identifier for 'value' in for-in loop.");
        free_node(key_var);
        return NULL;
    }

    Node *value_var = new_node(NODE_IDENT);
    strcpy(value_var->name, P->current.text);
    next(P);

    n->left = key_var;
    n->right = value_var;

    if (P->current.kind != TOKEN_IN)
    {
        print_error("Expected 'in' in for-in loop.");
        free_node(key_var);
        free_node(value_var);
        return NULL;
    }
    next(P);

    Node *iteree = parse_expr(P);
    n->next = iteree;

    if (P->current.kind != TOKEN_LBRACE)
        print_error("Expected '{' for for-in loop body.");

    Node *body_node = parse_block(P);
    n->super_template_types = body_node;

    return n;
}

static Node *parse_observe_stmt(Parser *P)
{
    Node *n = new_node(NODE_OBSERVE_STMT);
    next(P);

    if (P->current.kind != TOKEN_IDENT)
        print_error("Expected identifier to observe.");
    strcpy(n->name, P->current.text);
    next(P);

    if (P->current.kind != TOKEN_LBRACE)
        print_error("Expected '{' after observe identifier.");
    next(P);

    Node *cases_head = NULL;
    Node *cases_current = NULL;

    while (P->current.kind != TOKEN_RBRACE && P->current.kind != TOKEN_END)
    {
        if (P->current.kind != TOKEN_ON)
            print_error("Expected 'on' keyword inside observe block.");
        next(P);

        Node *case_node = new_node(NODE_OBSERVE_CASE);

        if (P->current.kind != TOKEN_LPAREN)
            print_error("Expected '(' after 'on'.");
        next(P);
        case_node->left = parse_expr(P);
        if (P->current.kind != TOKEN_RPAREN)
            print_error("Expected ')' after observation condition.");
        next(P);

        case_node->right = parse_block(P);

        if (cases_head == NULL)
        {
            cases_head = case_node;
            cases_current = case_node;
        }
        else
        {
            cases_current->next = case_node;
            cases_current = case_node;
        }
    }

    if (P->current.kind != TOKEN_RBRACE)
        print_error("Expected '}' to end observe block.");
    next(P);

    n->right = cases_head;
    return n;
}

/**
 * @brief Parses a statement.
 * @param P Pointer to the Parser.
 * @return Pointer to the parsed Node.
 */
Node *parse_stmt(Parser *P)
{

    Node meta_node = {0};

    if (P->current.kind == TOKEN_AT)
    {
        parse_annotations(P, &meta_node);
    }

    if (P->current.kind == TOKEN_IF)
    {
        return parse_if_stmt(P);
    }
    if (P->current.kind == TOKEN_EVERY)
    {
        return parse_every_loop(P);
    }
    if (P->current.kind == TOKEN_OBSERVE)
    {
        return parse_observe_stmt(P);
    }

    if (P->current.kind == TOKEN_MATCH)
    {
        return parse_match_stmt(P);
    }

    if (P->current.kind == TOKEN_BREAK)
    {
        return parse_break_stmt(P);
    }

    if (P->current.kind == TOKEN_CONTINUE)
    {
        return parse_continue_stmt(P);
    }

    if (P->current.kind == TOKEN_INTERFACE)
    {
        return parse_interface_def(P);
    }

    if (P->current.kind == TOKEN_TRY)
    {
        return parse_try_stmt(P);
    }

    if (P->current.kind == TOKEN_THROW)
    {
        return parse_throw_stmt(P);
    }

    if (P->current.kind == TOKEN_IMPORT)
    {
        return parse_import_stmt(P);
    }

    if (P->current.kind == TOKEN_CLASS || P->current.kind == TOKEN_RECORD || P->current.kind == TOKEN_OBJECT)
    {
        return parse_class_def(P);
    }

    if (P->current.kind == TOKEN_FUNCTION)
    {
        next(P);
        Node *n = parse_func_def(P);

        n->is_platform_specific = meta_node.is_platform_specific;
        if (meta_node.is_platform_specific && meta_node.target_os != NULL)
        {
            n->target_os = strdup(meta_node.target_os);
        }
        else
        {
            n->target_os = NULL;
        }

        n->is_main = meta_node.is_main;
        n->is_paralel = meta_node.is_paralel;
        n->is_memoize = meta_node.is_memoize;
        n->is_override = meta_node.is_override;
        n->is_deprecated = meta_node.is_deprecated;
        n->is_platform_specific = meta_node.is_platform_specific;

        meta_node.is_platform_specific = false;

        if (meta_node.is_platform_specific && meta_node.target_os != NULL)
        {

            n->target_os = strdup(meta_node.target_os);
        }
        else
        {
            n->target_os = NULL;
        }

        return n;
    }

    if (P->current.kind == TOKEN_RETURN)
    {
        return parse_return_stmt(P);
    }

    if (P->current.kind == TOKEN_WHILE)
    {
        return parse_while_stmt(P);
    }

    if (P->current.kind == TOKEN_FOR)
    {
        return parse_for_stmt(P);
    }

    if (P->current.kind == TOKEN_LBRACE)
    {
        return parse_block(P);
    }

    if (P->current.kind == TOKEN_ENUM)
    {
        return parse_enum_def(P);
    }

    if (P->current.kind == TOKEN_LET || P->current.kind == TOKEN_CONST)
    {
        bool is_const = (P->current.kind == TOKEN_CONST);
        next(P);
        Node *n = new_node(is_const ? NODE_CONSTDECL : NODE_VARDECL);

        strcpy(n->name, P->current.text);
        next(P);

        n->type_name[0] = '\0';
        if (P->current.kind == TOKEN_COLON)
        {
            next(P);
            strcpy(n->type_name, P->current.text);
            next(P);
        }

        if (P->current.kind == TOKEN_ASSIGN)
            next(P);
        n->right = parse_expr(P);

        if (P->current.kind == TOKEN_SEMI)
            next(P);
        return n;
    }

    // if (P->current.kind == TOKEN_LET)
    // {
    //     next(P);
    //     Node *n = new_node(NODE_VARDECL);

    //     if (P->current.kind != TOKEN_IDENT)
    //         print_error("Expected identifier after 'let'.");
    //     strcpy(n->name, P->current.text);
    //     next(P);

    //     if (P->current.kind != TOKEN_ASSIGN)
    //         print_error("Expected '=' after variable name.");
    //     next(P);

    //     n->right = parse_expr(P);

    //     if (P->current.kind == TOKEN_SEMI)
    //     {
    //         next(P);
    //     }

    //     return n;
    // }

    if (P->current.kind == TOKEN_PRINT)
    {
        next(P);
        Node *n = new_node(NODE_PRINT);
        n->right = parse_expr(P);

        if (P->current.kind == TOKEN_SEMI)
            next(P);
        return n;
    }

    // if (P->current.kind == TOKEN_CONST)
    // {
    //     next(P);
    //     Node *n = new_node(NODE_CONSTDECL);
    //     if (P->current.kind != TOKEN_IDENT)
    //         print_error("Expected identifier after 'const'.");
    //     strcpy(n->name, P->current.text);
    //     next(P);

    //     if (P->current.kind != TOKEN_ASSIGN)
    //         print_error("Expected '=' after constant name.");
    //     next(P);

    //     n->right = parse_expr(P);

    //     if (P->current.kind == TOKEN_SEMI)
    //         next(P);
    //     return n;
    // }

    Node *expr = parse_expr(P);
    if (P->current.kind == TOKEN_SEMI)
        next(P);
    return expr;
}

/**
 * @brief Parses an import statement.
 * @param P Pointer to the Parser.
 * @return Pointer to the parsed Node.
 */
static Node *parse_import_stmt(Parser *P)
{
    Node *n = new_node(NODE_IMPORT);
    next(P);

    char path[256] = {0};

    if (P->current.kind != TOKEN_IDENT)
    {
        print_error("Expected library name after 'import'.");
    }
    strcat(path, P->current.text);
    next(P);

    while (P->current.kind == TOKEN_DOT)
    {
        strcat(path, "/");
        next(P);

        if (P->current.kind != TOKEN_IDENT)
        {
            print_error("Expected library name after '.' in import.");
            break;
        }
        strcat(path, P->current.text);
        next(P);
    }

    strcat(path, ".jackal");

    strcpy(n->name, path);

    if (P->current.kind == TOKEN_SEMI)
        next(P);

    return n;
}

/**
 * @brief Parses an match statement.
 * @param P Pointer to the Parser.
 * @return Pointer to the parsed Node.
 */
static Node *parse_match_stmt(Parser *P)
{
    Node *n = new_node(NODE_MATCH_STMT);
    next(P);

    if (P->current.kind != TOKEN_LPAREN)
        print_error("Expected '(' after 'match'.");
    next(P);
    n->left = parse_expr(P);
    if (P->current.kind != TOKEN_RPAREN)
        print_error("Expected ')' after match expression.");
    next(P);

    if (P->current.kind != TOKEN_LBRACE)
        print_error("Expected '{' before match cases.");
    next(P);

    Node *cases_head = NULL;
    Node *cases_current = NULL;

    while (P->current.kind != TOKEN_RBRACE && P->current.kind != TOKEN_END)
    {
        Node *case_node = new_node(NODE_MATCH_CASE);

        if (P->current.kind == TOKEN_DEFAULT)
        {
            next(P);
            case_node->left = NULL;
        }
        else
        {
            case_node->left = parse_expr(P);
        }

        if (P->current.kind != TOKEN_ARROW)
            print_error("Expected '=>' after match case.");
        next(P);

        if (P->current.kind == TOKEN_LBRACE)
        {
            case_node->right = parse_block(P);
        }
        else
        {
            case_node->right = parse_stmt(P);
        }

        if (cases_head == NULL)
        {
            cases_head = case_node;
            cases_current = case_node;
        }
        else
        {
            cases_current->next = case_node;
            cases_current = case_node;
        }
    }

    if (P->current.kind != TOKEN_RBRACE)
        print_error("Expected '}' after match cases.");
    next(P);

    n->right = cases_head;
    return n;
}

static Node *parse_interface_def(Parser *P)
{
    Node *n = new_node(NODE_INTERFACE_DEF);
    next(P);

    if (P->current.kind != TOKEN_IDENT)
        print_error("Expected interface name.");
    strcpy(n->name, P->current.text);
    next(P);

    if (P->current.kind != TOKEN_LBRACE)
        print_error("Expected '{' before interface body.");
    next(P);

    Node *methods_head = NULL;
    Node *methods_current = NULL;

    while (P->current.kind != TOKEN_RBRACE && P->current.kind != TOKEN_END)
    {
        if (P->current.kind != TOKEN_FUNCTION)
        {
            print_error("Expected 'function' keyword in interface.");
            break;
        }
        next(P);

        Node *method = new_node(NODE_FUNC_DEF);
        if (P->current.kind != TOKEN_IDENT)
            print_error("Expected function name.");
        strcpy(method->name, P->current.text);
        next(P);

        if (P->current.kind != TOKEN_LPAREN)
            print_error("Expected '(' after function name.");
        next(P);

        method->arity = 0;
        if (P->current.kind != TOKEN_RPAREN)
        {
            do
            {
                if (P->current.kind != TOKEN_IDENT)
                    print_error("Expected parameter name.");
                next(P);
                method->arity++;
            } while (P->current.kind == TOKEN_COMMA && (next(P), 1));
        }
        if (P->current.kind != TOKEN_RPAREN)
            print_error("Expected ')' after parameters.");
        next(P);

        next(P);

        method->left = NULL;
        method->right = NULL;

        if (methods_head == NULL)
        {
            methods_head = method;
            methods_current = method;
        }
        else
        {
            methods_current->next = method;
            methods_current = method;
        }
    }

    if (P->current.kind != TOKEN_RBRACE)
        print_error("Expected '}' after interface body.");
    next(P);

    n->left = methods_head;
    return n;
}

/**
 * @brief Parses a map literal.
 * @param P Pointer to the Parser.
 * @return Pointer to the parsed Node.
 */

static Node *parse_map_literal(Parser *P)
{
    Node *n = new_node(NODE_MAP_LITERAL);
    next(P);

    Node *head = NULL;
    Node *current = NULL;

    while (P->current.kind != TOKEN_RBRACE && P->current.kind != TOKEN_END)
    {
        if (P->current.kind != TOKEN_STRING)
        {
            print_error("Expected string key in map literal.");
        }
        char key_str[64];
        strncpy(key_str, P->current.text, 63);
        key_str[63] = '\0';
        next(P);

        if (P->current.kind != TOKEN_COLON)
            print_error("Expected ':' after map key.");
        next(P);

        Node *value_node = parse_expr(P);

        Node *entry = new_node(NODE_MAP_LITERAL);
        strcpy(entry->name, key_str);
        entry->left = value_node;

        if (head == NULL)
        {
            head = entry;
            current = entry;
        }
        else
        {
            current->next = entry;
            current = entry;
        }

        if (P->current.kind == TOKEN_COMMA)
            next(P);
    }
    if (P->current.kind != TOKEN_RBRACE)
        print_error("Expected '}' after map.");
    next(P);

    n->left = head;
    return n;
}

/**
 * @brief Parses an enum definition.
 * @param P Pointer to the Parser.
 * @return Pointer to the parsed Node.
 */
static Node *parse_enum_def(Parser *P)
{
    Node *n = new_node(NODE_ENUM_DEF);
    next(P);

    if (P->current.kind != TOKEN_IDENT)
        print_error("Expected enum name.");
    strcpy(n->name, P->current.text);
    next(P);

    if (P->current.kind != TOKEN_LBRACE)
        print_error("Expected '{' before enum body.");
    next(P);

    Node *head = NULL;
    Node *current = NULL;
    int currentValue = 0;

    while (P->current.kind != TOKEN_RBRACE && P->current.kind != TOKEN_END)
    {
        if (P->current.kind != TOKEN_IDENT)
        {
            print_error("Expected enum constant name.");
        }
        Node *entry = new_node(NODE_ENUM_DEF);
        strcpy(entry->name, P->current.text);
        next(P);

        if (P->current.kind == TOKEN_ASSIGN)
        {
            next(P);
            if (P->current.kind != TOKEN_NUMBER)
                print_error("Enum value must be a number.");
            entry->value = P->current.number;
            currentValue = (int)P->current.number + 1; // Update counter
            next(P);
        }
        else
        {
            entry->value = (double)currentValue++;
        }

        if (head == NULL)
        {
            head = entry;
            current = entry;
        }
        else
        {
            current->next = entry;
            current = entry;
        }

        if (P->current.kind == TOKEN_COMMA)
            next(P);
    }

    if (P->current.kind != TOKEN_RBRACE)
        print_error("Expected '}' after enum body.");
    next(P);

    n->left = head;
    return n;
}

static Node *parse_func_expr(Parser *P)
{
    Node *n = new_node(NODE_FUNC_EXPR);
    next(P);

    if (P->current.kind != TOKEN_LPAREN)
        print_error("Expected '(' after 'function'.");
    next(P);

    Node *params_head = NULL;
    Node *params_current = NULL;
    n->arity = 0;

    if (P->current.kind != TOKEN_RPAREN)
    {
        do
        {
            if (P->current.kind != TOKEN_IDENT)
                print_error("Expected parameter name.");
            Node *param_node = new_node(NODE_IDENT);
            strcpy(param_node->name, P->current.text);
            next(P);
            n->arity++;

            if (params_head == NULL)
            {
                params_head = param_node;
                params_current = param_node;
            }
            else
            {
                params_current->next = param_node;
                params_current = param_node;
            }
        } while (P->current.kind == TOKEN_COMMA && (next(P), 1));
    }

    if (P->current.kind != TOKEN_RPAREN)
        print_error("Expected ')' after parameters.");
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
static Node *parse_throw_stmt(Parser *P)
{
    Node *n = new_node(NODE_THROW_STMT);
    next(P);
    n->left = parse_expr(P);
    if (P->current.kind == TOKEN_SEMI)
        next(P);
    return n;
}

/**
 * @brief Parses a break statement.
 * @param P Pointer to the Parser.
 * @return Pointer to the parsed Node.
 */
static Node *parse_break_stmt(Parser *P)
{
    Node *n = new_node(NODE_BREAK_STMT);
    next(P);
    if (P->current.kind == TOKEN_SEMI)
        next(P);
    return n;
}

static Node *parse_continue_stmt(Parser *P)
{
    Node *n = new_node(NODE_CONTINUE_STMT);
    next(P);
    if (P->current.kind == TOKEN_SEMI)
        next(P);
    return n;
}

/**
 * @brief Parses a try statement.
 * @param P Pointer to the Parser.
 * @return Pointer to the parsed Node.
 */
static Node *parse_try_stmt(Parser *P)
{
    Node *n = new_node(NODE_TRY_STMT);
    next(P);
    n->left = parse_block(P);

    if (P->current.kind != TOKEN_CATCH)
    {
        print_error("Expected 'catch' after try block.");
        return n;
    }
    next(P);
    if (P->current.kind != TOKEN_LPAREN)
        print_error("Expected '(' after catch.");
    next(P);
    if (P->current.kind != TOKEN_IDENT)
        print_error("Expected error variable name.");
    strcpy(n->name, P->current.text);
    next(P);
    if (P->current.kind != TOKEN_RPAREN)
        print_error("Expected ')' after error variable.");
    next(P);

    n->right = parse_block(P);

    return n;
}
/**
 * @brief Parses a logical operator.
 * @param P Pointer to the Parser.
 * @return Pointer to the parsed Node.
 */
static Node *parse_logical_or(Parser *P)
{
    Node *left = parse_logical_and(P);

    while (P->current.kind == TOKEN_PIPE_PIPE)
    {
        Node *n = new_node(NODE_BINOP);
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
static Node *parse_unary(Parser *P)
{

    if (P->current.kind == TOKEN_BANG || P->current.kind == TOKEN_MINUS)
    {
        Node *n = new_node(NODE_UNARY);
        n->op = P->current.kind;
        next(P);

        n->right = parse_unary(P);
        return n;
    }

    return parse_postfix(P);
}

static inline bool is_ident(Parser *P, const char *s)
{
    return P->current.kind == TOKEN_IDENT &&
           P->current.text &&
           strcmp(P->current.text, s) == 0;
}

static void parse_annotations(Parser *P, Node *n)
{

    n->is_override = false;
    n->is_deprecated = false;
    n->is_main = false;
    n->is_memoize = false;
    n->is_paralel = false;
    n->is_static = false;
    n->is_platform_specific = false;
    n->is_main = false;

    while (P->current.kind == TOKEN_AT)
    {
        next(P);

        if (P->current.kind != TOKEN_IDENT)
        {
            print_error("Expected annotation name after '@'.");
            return;
        }

        if (strcmp(P->current.text, "os") == 0)
        {
            n->is_platform_specific = true;
            next(P);

            if (P->current.kind == TOKEN_LPAREN)
            {
                next(P);
                if (P->current.kind == TOKEN_STRING && P->current.text != NULL)
                {
                    char *raw = P->current.text;
                    size_t len = strlen(raw);

                    if (len >= 2 && raw[0] == '"' && raw[len - 1] == '"')
                    {
                        n->target_os = strndup(raw + 1, len - 2);
                    }
                    else
                    {
                        n->target_os = strdup(raw);
                    }

                    next(P); 
                }
                else
                {
                    print_error("Expected platform string inside @os(...).");
                }

                if (P->current.kind == TOKEN_RPAREN)
                {
                    next(P);
                }
                else
                {
                    print_error("Expected ')' after platform name.");
                }
            }
            else
            {
                print_error("Expected '(' after @os.");
            }

            continue;
        }
        if (strcmp(P->current.text, "main") == 0)
        {
            n->is_main = true;
        }
        
        else if (strcmp(P->current.text, "async") == 0){
            n -> is_async = true;
        }
        else if (strcmp(P->current.text, "static") == 0)
        {
            n->is_static = true;
        }
        else if (strcmp(P->current.text, "parallel") == 0)
        {
            n->is_paralel = true;
        }

        else if (strcmp(P->current.text, "memoize") == 0)
        {
            n->is_memoize = true;
        }
        else if (strcmp(P->current.text, "override") == 0)
        {
            n->is_override = true;
        }
        else if (strcmp(P->current.text, "deprecated") == 0)
        {
            n->is_deprecated = true;
        }
        else
        {

            print_error("Unknown annotation '@%s'.", P->current.text);
        }
        next(P);
    }
}

/**
 * @brief parse when expression
 * @param P to parser pointer
 */
static Node *parse_when_expr(Parser *P)
{
    Node *n = new_node(NODE_WHEN_EXPR);
    next(P);

    if (P->current.kind != TOKEN_LBRACE)
    {
        print_error("Expected '{' after 'when'.");
        return n;
    }
    next(P);

    Node *cases_head = NULL;
    Node *cases_current = NULL;

    while (P->current.kind != TOKEN_RBRACE && P->current.kind != TOKEN_END)
    {
        Node *case_node = new_node(NODE_WHEN_CASE);

        if (P->current.kind == TOKEN_DEFAULT)
        {
            next(P);
            case_node->left = NULL;
        }
        else
        {
            case_node->left = parse_expr(P);
        }

        if (P->current.kind != TOKEN_ARROW)
        {
            print_error("Expected '=>' after when condition.");
        }
        next(P);

        case_node->right = parse_expr(P);

        if (P->current.kind == TOKEN_COMMA)
        {
            next(P);
        }

        if (cases_head == NULL)
        {
            cases_head = case_node;
            cases_current = case_node;
        }
        else
        {
            cases_current->next = case_node;
            cases_current = case_node;
        }
    }

    if (P->current.kind != TOKEN_RBRACE)
        print_error("Expected '}' after when cases.");
    next(P);

    n->left = cases_head;
    return n;
}

static Node *parse_every_loop(Parser *P)
{
    Node *n = new_node(NODE_EVERY_LOOP);
    next(P);

    if (P->current.kind != TOKEN_LPAREN)
        print_error("Expected '(' after 'every'.");
    next(P);

    n->left = parse_expr(P);

    if (P->current.kind != TOKEN_RPAREN)
        print_error("Expected ')' after every count.");
    next(P);

    n->right = new_node(NODE_IDENT);

    n->right->left = parse_block(P);

    if (P->current.kind != TOKEN_UNTIL)
        print_error("Expected 'until' after every block.");
    next(P);

    if (P->current.kind != TOKEN_LPAREN)
        print_error("Expected '(' after 'until'.");
    next(P);

    n->right->right = parse_expr(P);

    if (P->current.kind != TOKEN_RPAREN)
        print_error("Expected ')' after until condition.");
    next(P);

    return n;
}
/**
 * @brief Frees the memory allocated for the given AST node and its children.
 * @param n Pointer to the Node to be freed.
 */
void free_node(Node *n)
{
    if (!n)
        return;
    free_node(n->left);
    free_node(n->right);
    free_node(n->next);
    free(n);
}
