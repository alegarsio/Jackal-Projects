
#include "eval.h"
#include "value.h"
#include "env.h"

#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>

/**
 * @include collections DSA stl
 */
#include "collections/linkedlist.h"
/**
 * Global exception state for the interpreter.
 */

ExceptionState global_ex_state = {.active = 0};

/**
 * @brief Recursively searches for a method in a class and its superclasses.
 * @param klass The class to search in.
 * @param name The name of the method to find.
 * @return Pointer to the Var representing the method, or NULL if not found.
 */
static Var *find_method(Class *klass, const char *name);

/**
 * @brief Checks if two values are both numbers.
 */
static bool is_number(Value a, Value b)
{
    return a.type == VAL_NUMBER && b.type == VAL_NUMBER;
}

/**
 * @brief Checks if two values are both strings.
 */

static bool is_string(Value a, Value b) { return a.type == VAL_STRING && b.type == VAL_STRING; }

/**
 * @brief Reads the content of a file into a string.
 * @param filename The name of the file to read.
 * @return A pointer to the file content string, or NULL on failure.
 */
static char *read_file(const char *filename)
{
    FILE *f = fopen(filename, "r");
    if (!f)
        return NULL;
    fseek(f, 0, SEEK_END);
    long len = ftell(f);
    rewind(f);
    char *source = malloc(len + 1);
    if (source)
    {
        fread(source, 1, len, f);
        source[len] = '\0';
    }
    fclose(f);
    return source;
}

static bool is_integer_value(Value val)
{
    if (val.type != VAL_NUMBER)
        return false;
    return val.as.number == floor(val.as.number);
}




/**
 * @brief get value for return type
 */
static const char *get_value_type_name(Value val)
{
    switch (val.type)
    {
    case VAL_NIL:
        return "nil";
    case VAL_NUMBER:
        if (is_integer_value(val)) {
            return "Int";
        } else {
            return "Float";
        }
    case VAL_STRING:
        return "String";
    case VAL_ARRAY:
        return "Array";
    case VAL_MAP:
        return "Map";
    case VAL_FUNCTION:
        return "Function";
    case VAL_NATIVE:
        return "Function";
    case VAL_CLASS:
        return val.as.class_obj->name;
    case VAL_INSTANCE:
        return val.as.instance->class_val->as.class_obj->name;
    case VAL_INTERFACE:
        return val.as.interface_obj->name;
    case VAL_ENUM:
        return val.as.enum_obj->name;
    case VAL_FILE:
        return "File";
    default:
        return "unknown";
    }
}


static bool is_type_alias_match(const char* expected_type, Value actual_val) {
    if (actual_val.type == VAL_NUMBER) {
        if (strcmp(expected_type, "Int") == 0) {
            return is_integer_value(actual_val);
        }
        if (strcmp(expected_type, "Float") == 0) {
            return !is_integer_value(actual_val);
        }
        if (strcmp(expected_type, "Number") == 0) {
            return true;
        }
    }
    
    const char *actual_type_name = get_value_type_name(actual_val);
    if (strcmp(expected_type, actual_type_name) == 0) {
        return true;
    }

    return false;
}

/**
 * @brief call stream built in method
 *
 */
Value call_jackal_function(Env *env, Value func_val, int arg_count, Value *args)
{
    if (func_val.type != VAL_FUNCTION)
    {
        print_error("Invalid callback: not a function.");
        return (Value){VAL_NIL, {0}};
    }

    Func *func = func_val.as.function;

    if (arg_count != func->arity)
    {
        char buffer[128];
        snprintf(buffer, sizeof(buffer), "Callback function expected %d arguments but got %d.", func->arity, arg_count);
        print_error(buffer);
        return (Value){VAL_NIL, {0}};
    }

    Env *call_env = env_new(func->env);
    Node *param = func->params_head;

    for (int i = 0; i < arg_count; i++)
    {
        set_var(call_env, param->name, args[i], false);
        param = param->next;
    }

    Value result = eval_node(call_env, func->body_head);
    env_free(call_env);

    if (result.type == VAL_RETURN)
    {
        Value actual_return = *result.as.return_val;
        free(result.as.return_val);
        return actual_return;
    }

    return (Value){VAL_NIL, {0}};
}

/**
 * @brief Recursively searches for a method in a class and its superclasses.
 * @param klass The class to search in.
 * @param name The name of the method to find.
 * @return Pointer to the Var representing the method, or NULL if not found.
 */
static Var *find_method(Class *klass, const char *name)
{

    Var *method = find_var(klass->methods, name);
    if (method)
        return method;

    if (klass->superclass)
    {
        return find_method(klass->superclass, name);
    }

    return NULL;
}

/**
 * @brief Main evaluation function.
 * Recursively evaluates an AST node in a given environment.
 * @param env The current variable environment.
 * @param n The AST node to evaluate.
 * @return The result of the evaluation as a Value.
 */

Value eval_node(Env *env, Node *n)
{
    if (!n)
        return (Value){.type = VAL_NIL, .as = {0}};

    switch (n->kind)
    {

    case NODE_FUNC_EXPR:
    {
        Func *func = malloc(sizeof(Func));
        strcpy(func->return_type, n->return_type);
        func->params_head = n->left;
        func->body_head = n->right;
        func->env = env;
        func->arity = n->arity;

        return (Value){VAL_FUNCTION, {.function = func}};
    }

    case NODE_BREAK_STMT:
        return (Value){VAL_BREAK, {0}};

    case NODE_CONTINUE_STMT:
        return (Value){VAL_CONTINUE, {0}};

    case NODE_ENUM_DEF:
    {
        Enum *en = malloc(sizeof(Enum));
        strcpy(en->name, n->name);
        en->values = env_new(NULL);

        Node *entry = n->left;
        while (entry)
        {
            Value val = (Value){VAL_NUMBER, {.number = entry->value}};
            set_var(en->values, entry->name, val, true);
            entry = entry->next;
        }

        Value enum_val = (Value){VAL_ENUM, {.enum_obj = en}};
        set_var(env, n->name, enum_val, true);
        return (Value){VAL_NIL, {0}};
    }

    case NODE_THROW_STMT:
    {
        Value err_val = eval_node(env, n->left);

        if (global_ex_state.active)
        {
            global_ex_state.error_val = err_val;
            longjmp(global_ex_state.buf, 1);
        }
        else
        {
            printf("Uncaught Exception: ");
            print_value(err_val);
            printf("\n");
            exit(1);
        }
        return (Value){VAL_NIL, {0}};
    }

    case NODE_TRY_STMT:
    {
        jmp_buf old_buf;
        memcpy(old_buf, global_ex_state.buf, sizeof(jmp_buf));
        int was_active = global_ex_state.active;

        global_ex_state.active = 1;
        if (setjmp(global_ex_state.buf) == 0)
        {
            Value res = eval_node(env, n->left);

            memcpy(global_ex_state.buf, old_buf, sizeof(jmp_buf));
            global_ex_state.active = was_active;
            return res;
        }
        else
        {
            memcpy(global_ex_state.buf, old_buf, sizeof(jmp_buf));
            global_ex_state.active = was_active;

            Env *catch_env = env_new(env);
            set_var(catch_env, n->name, global_ex_state.error_val, false);

            Value catch_res = eval_node(catch_env, n->right);

            env_free(catch_env);

            return catch_res;
        }
    }

    case NODE_MAP_LITERAL:
    {
        HashMap *map = map_new();
        Node *entry = n->left;
        while (entry)
        {
            Value val = eval_node(env, entry->left);
            map_set(map, entry->name, val);
            free_value(val);
            entry = entry->next;
        }
        return (Value){VAL_MAP, {.map = map}};
    }

    case NODE_IMPORT:
    {
        char *source = read_file(n->name);
        if (!source)
        {
            printf("Runtime Error: Cannot open import file '%s'\n", n->name);
            return (Value){VAL_NIL, {0}};
        }

        Lexer imp_lex;
        Parser imp_parser;
        lexer_init(&imp_lex, source);
        parser_init(&imp_parser, &imp_lex);

        while (imp_parser.current.kind != TOKEN_END)
        {
            Node *stmt = parse_stmt(&imp_parser);
            if (stmt)
            {
                Value res = eval_node(env, stmt);
                free_value(res);
                free_node(stmt);
            }
        }

        free(source);
        return (Value){VAL_NIL, {0}};
    }

    case NODE_NUMBER:
        return (Value){VAL_NUMBER, {.number = n->value}};

    case NODE_UNARY:
    {
        Value right = eval_node(env, n->right);

        switch (n->op)
        {
        case TOKEN_MINUS:
        {
            if (right.type != VAL_NUMBER)
            {
                print_error("Operand for '-' must be a number.");
                free_value(right);
                return (Value){VAL_NIL, {0}};
            }
            double val = -right.as.number;
            free_value(right);
            return (Value){VAL_NUMBER, {.number = val}};
        }

        case TOKEN_BANG:
        {
            bool is_true = is_value_truthy(right);
            free_value(right);
            return (Value){VAL_NUMBER, {.number = is_true ? 0.0 : 1.0}};
        }

        default:
            break;
        }
        return (Value){VAL_NIL, {0}};
    }

    /**
     * Strings must be heap-allocated because they are passed around by pointer.
     */
    case NODE_STRING:
    {
        char *str_copy = malloc(strlen(n->name) + 1);
        if (str_copy)
        {
            strcpy(str_copy, n->name);
        }
        return (Value){VAL_STRING, {.string = str_copy}};
    }

    case NODE_WHEN_EXPR:
    {
        Node *case_node = n->left;
        Value default_result = (Value){VAL_NIL, {0}};
        bool found_default = false;

        while (case_node)
        {
            if (case_node->left == NULL)
            {
                default_result = eval_node(env, case_node->right);
                found_default = true;
            }
            else
            {
                Value cond = eval_node(env, case_node->left);
                bool is_true = is_value_truthy(cond);
                free_value(cond);

                if (is_true)
                {
                    Value result = eval_node(env, case_node->right);
                    if (found_default)
                        free_value(default_result);
                    return result;
                }
            }
            case_node = case_node->next;
        }

        return default_result;
    }

    case NODE_MATCH_STMT:
    {
        Value match_val = eval_node(env, n->left);
        bool matched = false;

        Node *case_node = n->right;
        while (case_node)
        {
            if (case_node->left != NULL)
            {
                Value case_val = eval_node(env, case_node->left);

                Value is_eq = eval_equals(match_val, case_val);

                if (is_eq.as.number == 1.0)
                {
                    matched = true;
                    free_value(case_val);
                    Value result = eval_node(env, case_node->right);
                    free_value(match_val);
                    return result;
                }
                free_value(case_val);
            }
            else
            {
                if (!matched)
                {
                    free_value(match_val);
                    return eval_node(env, case_node->right);
                }
            }
            case_node = case_node->next;
        }

        free_value(match_val);
        return (Value){.type = VAL_NIL, .as = {0}};
    }

    case NODE_ARRAY_LITERAL:
    {
        ValueArray *arr = array_new();
        Node *item = n->left;
        for (int i = 0; i < n->arity; i++)
        {
            Value val = eval_node(env, item);
            array_append(arr, val);
            item = item->next;
        }
        return (Value){VAL_ARRAY, {.array = arr}};
    }

    case NODE_ARRAY_ACCESS:
    {
        Value container = eval_node(env, n->left);
        Value index = eval_node(env, n->right);

        /**
         * Unused variables to hold evaluated values.
         */

        if (container.type == VAL_ARRAY)
        {
            if (index.type != VAL_NUMBER)
            {
                print_error("Array index must be a number.");
                free_value(container);
                free_value(index);
                return (Value){.type = VAL_NIL, .as = {0}};
            }
            int idx = (int)index.as.number;
            if (idx < 0 || idx >= container.as.array->count)
            {
                print_error("Array index out of bounds.");
                free_value(container);
                free_value(index);
                return (Value){.type = VAL_NIL, .as = {0}};
            }
            Value result = copy_value(container.as.array->values[idx]);
            free_value(container);
            free_value(index);
            return result;
        }
        else if (container.type == VAL_MAP)
        {
            if (index.type != VAL_STRING)
            {
                print_error("Map key must be a string.");
                free_value(container);
                free_value(index);
                return (Value){.type = VAL_NIL, .as = {0}};
            }
            Value result;
            if (map_get(container.as.map, index.as.string, &result))
            {
                free_value(container);
                free_value(index);
                return copy_value(result);
            }
            else
            {
                free_value(container);
                free_value(index);
                return (Value){.type = VAL_NIL, .as = {0}};
            }
        }

        print_error("Invalid access operation (not an array or map).");
        free_value(container);
        free_value(index);
        return (Value){.type = VAL_NIL, .as = {0}};
    }

    case NODE_ARRAY_ASSIGN:
    {
        Value new_val = eval_node(env, n->right);

        Node *access_node = n->left;

        Value container = eval_node(env, access_node->left);
        Value index = eval_node(env, access_node->right);

        if (container.type == VAL_ARRAY)
        {
            if (index.type != VAL_NUMBER)
            {
                print_error("Array index must be a number.");
                return (Value){.type = VAL_NIL, .as = {0}};
            }
            int idx = (int)index.as.number;
            if (idx < 0 || idx >= container.as.array->count)
            {
                print_error("Array index out of bounds.");
                return (Value){.type = VAL_NIL, .as = {0}};
            }
            free_value(container.as.array->values[idx]);
            container.as.array->values[idx] = copy_value(new_val);
        }
        else if (container.type == VAL_MAP)
        {
            if (index.type != VAL_STRING)
            {
                print_error("Map key must be a string.");
                return (Value){.type = VAL_NIL, .as = {0}};
            }
            map_set(container.as.map, index.as.string, new_val);
        }
        else
        {
            print_error("Invalid assignment target.");
        }

        free_value(container);
        free_value(index);
        return new_val;
    }

    case NODE_IDENT:
    {
        Var *v = find_var(env, n->name);
        if (!v)
        {
            print_error("Undefined identifier '%s'.", n->name);
            return (Value){.type = VAL_NIL, .as = {0}};
        }
        return copy_value(v->value);
    }

    case NODE_THIS:
    {
        Var *v = find_var(env, n->name);
        if (!v)
        {
            print_error("'this' is not defined.");
            return (Value){.type = VAL_NIL, .as = {0}};
        }
        return copy_value(v->value);
    }

    case NODE_ASSIGN:
    {
        Value val = eval_node(env, n->left);
        Var *v = find_var(env, n->name);
        if (!v)
        {
            print_error("Undefined variable.");
            free_value(val);
            return (Value){.type = VAL_NIL, .as = {0}};
        }

        if (v->is_const)
        {
            char buffer[128];
            snprintf(buffer, sizeof(buffer), "Cannot reassign to constant '%s'.", n->name);
            print_error(buffer);
            free_value(val);
            return (Value){.type = VAL_NIL, .as = {0}};
        }

        free_value(v->value);
        v->value = copy_value(val);
        return val;
    }

    case NODE_CONSTDECL:
    {
        Value val = eval_node(env, n->right);
        set_var(env, n->name, val, true);
        free_value(val);
        return (Value){VAL_NIL, {0}};
    }

    case NODE_GET:
    {
        Value obj = eval_node(env, n->left);

        if (obj.type == VAL_INSTANCE)
        {
            Var *field = find_var(obj.as.instance->fields, n->name);
            if (field)
            {

                return copy_value(field->value);
            }

            Var *method = find_method(obj.as.instance->class_val->as.class_obj, n->name);
            if (method)
            {
                return copy_value(method->value);
            }
        }

        else if (obj.type == VAL_MAP)
        {
            Value result;
            if (map_get(obj.as.map, n->name, &result))
            {
                return copy_value(result);
            }
        }

        if (obj.type == VAL_ENUM)
        {
            Var *constant = find_var(obj.as.enum_obj->values, n->name);
            if (constant)
            {
                return copy_value(constant->value);
            }
            print_error("Undefined enum constant.");
            return (Value){VAL_NIL, {0}};
        }

        Var *field = find_var(obj.as.instance->fields, n->name);
        if (field)
        {
            Value result = copy_value(field->value);
            free_value(obj);
            return result;
        }

        Var *method = find_method(obj.as.instance->class_val->as.class_obj, n->name);
        if (method)
        {
            Value result = copy_value(method->value);
            free_value(obj);
            return result;
        }

        print_error("Undefined property.");
        free_value(obj);
        return (Value){.type = VAL_NIL, .as = {0}};
    }

    case NODE_SET:
    {
        Value obj = eval_node(env, n->left->left);
        if (obj.type != VAL_INSTANCE)
        {
            print_error("Only instances have fields.");
            free_value(obj);
            return (Value){.type = VAL_NIL, .as = {0}};
        }

        if (obj.as.instance->class_val->as.class_obj->is_record)
        {
            print_error("Cannot modify field of immutable record instance.");
            free_value(obj);
            return (Value){.type = VAL_NIL, .as = {0}};
        }

        Value val = eval_node(env, n->right);

        set_var(obj.as.instance->fields, n->left->name, val, false);

        free_value(val);

        return (Value){.type = VAL_NIL, .as = {0}};
    }

    case NODE_POST_INC:
    {

        Node *lvalue = n->left;

        if (lvalue->kind == NODE_IDENT)
        {

            Var *v = find_var(env, lvalue->name);
            if (v == NULL || v->value.type != VAL_NUMBER)
            {
                print_error("Operand for '++' must be a number variable.");
                return (Value){.type = VAL_NIL, .as = {0}};
            }
            double old_val = v->value.as.number;
            v->value.as.number++;
            return (Value){VAL_NUMBER, {.number = old_val}};
        }
        else if (lvalue->kind == NODE_GET)
        {
            Value obj = eval_node(env, lvalue->left);
            if (obj.type != VAL_INSTANCE)
            {
                print_error("Invalid l-value for '++'.");
                return (Value){.type = VAL_NIL, .as = {0}};
            }

            Var *v = find_var(obj.as.instance->fields, lvalue->name);
            if (v == NULL || v->value.type != VAL_NUMBER)
            {
                print_error("Operand for '++' must be a number property.");
                free_value(obj);
                return (Value){.type = VAL_NIL, .as = {0}};
            }

            double old_val = v->value.as.number;
            v->value.as.number++;
            free_value(obj);
            return (Value){VAL_NUMBER, {.number = old_val}};
        }

        print_error("Invalid l-value for '++'.");
        return (Value){.type = VAL_NIL, .as = {0}};
    }

    case NODE_POST_DEC:
    {
        Node *lvalue = n->left;

        if (lvalue->kind == NODE_IDENT)
        {
            Var *v = find_var(env, lvalue->name);
            if (v == NULL || v->value.type != VAL_NUMBER)
            {
                print_error("Operand for '--' must be a number variable.");
                return (Value){.type = VAL_NIL, .as = {0}};
            }
            double old_val = v->value.as.number;
            v->value.as.number--;
            return (Value){VAL_NUMBER, {.number = old_val}};
        }
        else if (lvalue->kind == NODE_GET)
        {

            Value obj = eval_node(env, lvalue->left);
            if (obj.type != VAL_INSTANCE)
            {
                print_error("Invalid l-value for '--'.");
                return (Value){.type = VAL_NIL, .as = {0}};
            }

            Var *v = find_var(obj.as.instance->fields, lvalue->name);
            if (v == NULL || v->value.type != VAL_NUMBER)
            {
                print_error("Operand for '--' must be a number property.");
                free_value(obj);
                return (Value){.type = VAL_NIL, .as = {0}};
            }

            double old_val = v->value.as.number;
            v->value.as.number--;
            free_value(obj);
            return (Value){VAL_NUMBER, {.number = old_val}};
        }

        print_error("Invalid l-value for '--'.");
        return (Value){.type = VAL_NIL, .as = {0}};
    }

    case NODE_BINOP:
    {

        /**
         * Token (||)
         * Or
         */
        if (n->op == TOKEN_PIPE_PIPE)
        {
            Value left = eval_node(env, n->left);
            if (is_value_truthy(left))
            {
                return left;
            }
            free_value(left);
            return eval_node(env, n->right);
        }

        /**
         * Token (&&)
         * And
         */
        if (n->op == TOKEN_AND_AND)
        {
            Value left = eval_node(env, n->left);
            if (!is_value_truthy(left))
            {
                return left;
            }
            free_value(left);
            return eval_node(env, n->right);
        }

        Value left = eval_node(env, n->left);
        Value right = eval_node(env, n->right);

        switch (n->op)
        {
        case TOKEN_PLUS:
            if (is_number(left, right))
            {
                return (Value){VAL_NUMBER, {.number = left.as.number + right.as.number}};
            }
            if (is_string(left, right))
            {
                size_t len_left = strlen(left.as.string);
                size_t len_right = strlen(right.as.string);
                char *new_string = malloc(len_left + len_right + 1);
                if (new_string)
                {
                    strcpy(new_string, left.as.string);
                    strcat(new_string, right.as.string);
                }
                free_value(left);
                free_value(right);
                return (Value){VAL_STRING, {.string = new_string}};
            }
            print_error("Operands must be two numbers or two strings for '+'.");
            break;
        case TOKEN_MINUS:
            if (is_number(left, right))
            {
                return (Value){VAL_NUMBER, {.number = left.as.number - right.as.number}};
            }
            print_error("Operands must be numbers for '-'.");
            break;
        case TOKEN_STAR:
            if (is_number(left, right))
            {
                return (Value){VAL_NUMBER, {.number = left.as.number * right.as.number}};
            }
            print_error("Operands must be numbers for '*'.");
            break;
        case TOKEN_SLASH:
            if (is_number(left, right))
            {
                return (Value){VAL_NUMBER, {.number = left.as.number / right.as.number}};
            }
            print_error("Operands must be numbers for '/'.");
            break;

        case TOKEN_PERCENT:
            if (is_number(left, right))
            {

                return (Value){VAL_NUMBER, {.number = fmod(left.as.number, right.as.number)}};
            }
            print_error("Operands must be numbers for '%'.");
            break;

        case TOKEN_GREATER:
            if (is_number(left, right))
            {
                return (Value){VAL_NUMBER, {.number = (left.as.number > right.as.number) ? 1.0 : 0.0}};
            }
            print_error("Operands must be numbers for '>'.");
            break;
        case TOKEN_GREATER_EQUAL:
            if (is_number(left, right))
            {
                return (Value){VAL_NUMBER, {.number = (left.as.number >= right.as.number) ? 1.0 : 0.0}};
            }
            print_error("Operands must be numbers for '>='.");
            break;
        case TOKEN_LESS:
            if (is_number(left, right))
            {
                return (Value){VAL_NUMBER, {.number = (left.as.number < right.as.number) ? 1.0 : 0.0}};
            }
            print_error("Operands must be numbers for '<'.");
            break;
        case TOKEN_LESS_EQUAL:
            if (is_number(left, right))
            {
                return (Value){VAL_NUMBER, {.number = (left.as.number <= right.as.number) ? 1.0 : 0.0}};
            }
            print_error("Operands must be numbers for '<='.");
            break;
        case TOKEN_EQUAL_EQUAL:
        {
            Value result = eval_equals(left, right);
            free_value(left);
            free_value(right);
            return result;
        }
        case TOKEN_BANG_EQUAL:
        {
            Value result = eval_equals(left, right);
            result.as.number = (result.as.number == 0.0) ? 1.0 : 0.0;
            free_value(left);
            free_value(right);
            return result;
        }
        default:
            print_error("Unknown binary operator.");
        }

        free_value(left);
        free_value(right);
        return (Value){.type = VAL_NIL, .as = {0}};
    }

    case NODE_VARDECL:
    {
        Value val = eval_node(env, n->right);

        set_var(env, n->name, val, false);
        free_value(val);

        return (Value){.type = VAL_NIL, .as = {0}};
    }

        // case NODE_PRINT:
        // {
        //     Value val = eval_node(env, n->right);
        //     print_value(val);

        //     fflush(stdout);
        //     free_value(val);
        //     return (Value){.type = VAL_NIL, .as = {0}};
        // }

    case NODE_IF_STMT:
    {
        Value cond = eval_node(env, n->left);
        bool is_true = is_value_truthy(cond);
        free_value(cond);

        Node *then_branch = n->right->left;
        Node *else_branch = n->right->right;

        Value result = (Value){.type = VAL_NIL, .as = {0}};
        if (is_true)
        {
            result = eval_node(env, then_branch);
        }
        else if (else_branch != NULL)
        {
            result = eval_node(env, else_branch);
        }

        return result;
    }

    case NODE_INTERFACE_DEF:
    {
        Interface *iface = malloc(sizeof(Interface));
        strcpy(iface->name, n->name);
        iface->methods = env_new(NULL);

        Node *method = n->left;
        while (method)
        {

            Value arity_val = (Value){VAL_NUMBER, {.number = (double)method->arity}};
            set_var(iface->methods, method->name, arity_val, true);

            method = method->next;
        }

        Value iface_val = (Value){VAL_INTERFACE, {.interface_obj = iface}};
        set_var(env, n->name, iface_val, true);
        return (Value){VAL_NIL, {0}};
    }

    case NODE_CLASS_DEF:
    {
        Class *class_obj = malloc(sizeof(Class));
        strcpy(class_obj->name, n->name);
        class_obj->methods = env_new(env);
        class_obj->superclass = NULL;
        class_obj->interface = NULL;

        if (strlen(n->super_name) > 0)
        {
            Var *super_var = find_var(env, n->super_name);
            if (!super_var || super_var->value.type != VAL_CLASS)
            {
                print_error("Superclass '%s' not found or invalid.", n->super_name);
                env_free(class_obj->methods);
                free(class_obj);
                return (Value){.type = VAL_NIL, .as = {0}};
            }
            class_obj->superclass = super_var->value.as.class_obj;
        }

        if (strlen(n->interface_name) > 0)
        {
            Var *iface_var = find_var(env, n->interface_name);
            if (!iface_var || iface_var->value.type != VAL_INTERFACE)
            {
                print_error("Interface '%s' not found or invalid.", n->interface_name);
                env_free(class_obj->methods);
                free(class_obj);
                return (Value){.type = VAL_NIL, .as = {0}};
            }
            class_obj->interface = iface_var->value.as.interface_obj;
        }

        if (n->is_singleton)
        {
            Value class_val = (Value){VAL_CLASS, {.class_obj = class_obj}};

            Instance *inst = malloc(sizeof(Instance));
            inst->class_val = malloc(sizeof(Value));
            *inst->class_val = class_val;
            inst->fields = env_new(NULL);

            Node *method = n->left;
            while (method)
            {
                Value method_val = eval_node(class_obj->methods, method);
                free_value(method_val);
                method = method->next;
            }

            Node *param_node = n->right;
            while (param_node)
            {
                set_var(inst->fields, param_node->name, (Value){VAL_STRING, {.string = strdup("")}}, false);
                param_node = param_node->next;
            }

            Value instance_val = (Value){VAL_INSTANCE, {.instance = inst}};
            set_var(env, n->name, instance_val, true);

            return (Value){.type = VAL_NIL, .as = {0}};
        }

        Node *method = n->left;
        while (method)
        {
            Node *next_method = method->next;
            Value method_val = eval_node(class_obj->methods, method);
            free_value(method_val);
            method = next_method;
        }

        Node *check_node = n->left;
        while (check_node)
        {
            if (check_node->is_override)
            {
                bool found_in_parent = false;
                if (class_obj->superclass != NULL)
                {
                    if (find_method(class_obj->superclass, check_node->name) != NULL)
                    {
                        found_in_parent = true;
                    }
                }

                if (!found_in_parent && class_obj->interface != NULL)
                {
                    if (find_var(class_obj->interface->methods, check_node->name) != NULL)
                    {
                        found_in_parent = true;
                    }
                }

                if (!found_in_parent)
                {
                    print_error("Method '%s' marked @override but does not override or implement any method from a superclass or interface.",
                                check_node->name);
                }
            }
            check_node = check_node->next;
        }

        if (class_obj->interface)
        {
            Interface *iface = class_obj->interface;
            Var *required_method = iface->methods->vars;
            while (required_method)
            {

                Var *implemented = find_method(class_obj, required_method->name);
                if (!implemented)
                {
                    char buffer[128];
                    snprintf(buffer, sizeof(buffer), "Class '%s' must implement method '%s' from interface '%s'.",
                             class_obj->name, required_method->name, iface->name);
                    print_error(buffer);
                }
                required_method = required_method->next;
            }
        }
        Value class_val = (Value){VAL_CLASS, {.class_obj = class_obj}};
        set_var(env, n->name, class_val, true);
        return (Value){.type = VAL_NIL, .as = {0}};
    }

    case NODE_FUNC_DEF:
    {
        Func *func = malloc(sizeof(Func));
        strcpy(func->return_type, n->return_type);
        func->params_head = n->left;
        func->body_head = n->right;
        func->env = env;
        func->arity = n->arity;
        func->is_private = n->is_private;

        func->is_deprecated = n->is_deprecated;

        n->left = NULL;
        n->right = NULL;

        Value func_val = (Value){VAL_FUNCTION, {.function = func}};
        set_var(env, n->name, func_val, true);

        return (Value){.type = VAL_NIL, .as = {0}};
    }

    case NODE_FUNC_CALL:
    {

        if (n->left->kind == NODE_IDENT && strcmp(n->left->name, "read") == 0)
        {
            char buffer[1024];
            if (fgets(buffer, sizeof(buffer), stdin))
            {
                size_t len = strlen(buffer);
                if (len > 0 && buffer[len - 1] == '\n')
                    buffer[len - 1] = '\0';
                char *str = malloc(len + 1);
                strcpy(str, buffer);
                return (Value){VAL_STRING, {.string = str}};
            }
            return (Value){VAL_NIL, .as = {0}};
        }

        if (n->left->kind == NODE_GET)
        {
            Node *get_node = n->left;
            Value obj = eval_node(env, get_node->left);

            if (obj.type == VAL_NUMBER)
            {
                if (strcmp(get_node->name, "toString") == 0)
                {
                    char buffer[64];

                    sprintf(buffer, "%g", obj.as.number);

                    char *str_copy = malloc(strlen(buffer) + 1);
                    strcpy(str_copy, buffer);

                    return (Value){VAL_STRING, {.string = str_copy}};
                }

                print_error("Undefined method '%s' for Number.", get_node->name);
                return (Value){VAL_NIL, {0}};
            }

            if (obj.type == VAL_MAP)
            {
                Value method_val;
                if (!map_get(obj.as.map, get_node->name, &method_val))
                {
                    print_error("Undefined method for Map.");
                    return (Value){VAL_NIL, {0}};
                }

                if (method_val.type != VAL_NATIVE)
                {
                    print_error("Map value is not a callable function.");
                    return (Value){VAL_NIL, {0}};
                }

                NativeFn native = method_val.as.native;
                Value args[255];
                int arg_count = 0;
                Node *arg_node = n->right;
                while (arg_node)
                {
                    args[arg_count++] = eval_node(env, arg_node);
                    arg_node = arg_node->next;
                }

                Value res = native(arg_count, args);
                for (int i = 0; i < arg_count; i++)
                    free_value(args[i]);
                return res;
            }

            if (obj.type == VAL_ARRAY)
            {
                Value callback = (Value){VAL_NIL, {0}};

                if (n->right)
                {
                    callback = eval_node(env, n->right);
                }

                if (strcmp(get_node->name, "map") == 0)
                {
                    ValueArray *old_arr = obj.as.array;
                    ValueArray *new_arr = array_new();
                    Value args[1];

                    for (int i = 0; i < old_arr->count; i++)
                    {
                        args[0] = old_arr->values[i];
                        Value new_val = call_jackal_function(env, callback, 1, args);
                        array_append(new_arr, new_val);
                    }
                    free_value(callback);
                    return (Value){VAL_ARRAY, {.array = new_arr}};
                }

                if (strcmp(get_node->name, "filter") == 0)
                {
                    ValueArray *old_arr = obj.as.array;
                    ValueArray *new_arr = array_new();
                    Value args[1];

                    for (int i = 0; i < old_arr->count; i++)
                    {
                        args[0] = old_arr->values[i];
                        Value result = call_jackal_function(env, callback, 1, args);
                        if (is_value_truthy(result))
                        {
                            array_append(new_arr, copy_value(args[0])); // Salin nilai asli
                        }
                        free_value(result);
                    }
                    free_value(callback);
                    return (Value){VAL_ARRAY, {.array = new_arr}};
                }

                if (strcmp(get_node->name, "reduce") == 0)
                {
                    ValueArray *arr = obj.as.array;
                    Value accumulator;
                    int start_index = 0;

                    if (n->arity == 2)
                    {
                        accumulator = eval_node(env, n->right->next);
                    }
                    else if (n->arity == 1)
                    {
                        if (arr->count == 0)
                            return (Value){VAL_NIL, {0}};
                        accumulator = copy_value(arr->values[0]);
                        start_index = 1;
                    }
                    else
                    {
                        print_error("Error: .reduce() takes 1 or 2 arguments.");
                        return (Value){VAL_NIL, {0}};
                    }

                    Value args[2];
                    for (int i = start_index; i < arr->count; i++)
                    {
                        args[0] = accumulator;
                        args[1] = arr->values[i];

                        Value new_acc = call_jackal_function(env, callback, 2, args);

                        free_value(accumulator);
                        accumulator = new_acc;
                    }

                    free_value(callback);
                    return accumulator;
                }

                if (strcmp(get_node->name, "length") == 0)
                {
                    return (Value){VAL_NUMBER, {.number = (double)obj.as.array->count}};
                }

                if (strcmp(get_node->name, "push") == 0)
                {
                    Node *arg = n->right;
                    while (arg)
                    {
                        Value val = eval_node(env, arg);
                        array_append(obj.as.array, copy_value(val));
                        free_value(val);
                        arg = arg->next;
                    }
                    return (Value){VAL_NIL, {0}};
                }

                if (strcmp(get_node->name, "pop") == 0)
                {
                    return array_pop(obj.as.array);
                }

                if (strcmp(get_node->name, "remove") == 0)
                {

                    if (n->arity != 1)
                    {
                        print_error("Error: remove() takes exactly 1 argument (index).");
                        return (Value){VAL_NIL, {0}};
                    }

                    Value index_val = eval_node(env, n->right);
                    if (index_val.type != VAL_NUMBER)
                    {
                        print_error("Error: remove() argument must be a number.");
                        free_value(index_val);
                        return (Value){VAL_NIL, {0}};
                    }

                    int index = (int)index_val.as.number;
                    array_delete(obj.as.array, index);

                    free_value(index_val);
                    return (Value){VAL_NIL, {0}};
                }

                print_error("Undefined method for Array.");
                return (Value){VAL_NIL, {0}};
            }

            if (obj.type == VAL_STRING)
            {

                if (strcmp(get_node->name, "toNumber") == 0)
                {

                    double val = atof(obj.as.string);
                    free_value(obj);
                    return (Value){VAL_NUMBER, {.number = val}};
                }

                if (strcmp(get_node->name, "toString") == 0)
                {
                    return obj;
                }

                if (strcmp(get_node->name, "length") == 0)
                {
                    return (Value){VAL_NUMBER, {.number = (double)strlen(obj.as.string)}};
                }

                print_error("Undefined method for String.");
                return (Value){VAL_NIL, {0}};
            }

            if (obj.type == VAL_INSTANCE)
            {

                Var *method_var = find_method(obj.as.instance->class_val->as.class_obj, get_node->name);
                if (!method_var)
                {
                    print_error("Undefined method.");
                    return (Value){VAL_NIL, .as = {0}};
                }

                Func *func = method_var->value.as.function;

                if (func->is_private)
                {
                    if (get_node->left->kind != NODE_THIS)
                    {
                        print_error("Cannot access private method '%s' outside of class context.", get_node->name);
                        return (Value){VAL_NIL, .as = {0}};
                    }
                }
                if (func->is_deprecated)
                {
                    printf("Warning: Method '%s' is deprecated.\n", get_node->name);
                }
                Env *call_env = env_new(func->env);

                Var *this_var = malloc(sizeof(Var));
                strcpy(this_var->name, "this");
                this_var->value = obj;
                this_var->next = call_env->vars;
                call_env->vars = this_var;

                Node *arg = n->right;
                Node *param = func->params_head;
                for (int i = 0; i < n->arity; i++)
                {
                    Value v = eval_node(env, arg);
                    set_var(call_env, param->name, v, false);
                    free_value(v);
                    arg = arg->next;
                    param = param->next;
                }

                Value res = eval_node(call_env, func->body_head);

                call_env->vars = this_var->next;
                free(this_var);
                env_free(call_env);

                if (res.type == VAL_RETURN)
                {
                    Value ret = *res.as.return_val;
                    free(res.as.return_val);
                    return ret;
                }
                return (Value){VAL_NIL, .as = {0}};
            }

            print_error("Only instances, arrays, and strings have methods.");
            return (Value){.type = VAL_NIL, .as = {0}};
        }

        Value callee = eval_node(env, n->left);

        if (callee.type == VAL_CLASS)
        {
            Instance *inst = malloc(sizeof(Instance));
            inst->class_val = malloc(sizeof(Value));
            *inst->class_val = callee;
            inst->fields = env_new(NULL);

            Value instance_val = (Value){VAL_INSTANCE, {.instance = inst}};

            Var *init_method = find_method(callee.as.class_obj, "init");
            if (init_method)
            {
                Func *func = init_method->value.as.function;

                if (n->arity != func->arity)
                {
                    print_error("Incorrect number of arguments for init().");
                    return (Value){.type = VAL_NIL, .as = {0}};
                }

                Env *call_env = env_new(func->env);
                set_var(call_env, "this", instance_val, true);

                Node *arg_node = n->right;
                Node *param_node = func->params_head;
                for (int i = 0; i < n->arity; i++)
                {
                    Value arg_val = eval_node(env, arg_node);
                    set_var(call_env, param_node->name, arg_val, false);
                    free_value(arg_val);
                    arg_node = arg_node->next;
                    param_node = param_node->next;
                }

                Value init_result = eval_node(call_env, func->body_head);
                free_value(init_result);
                env_free(call_env);

                return instance_val;
            }
            return instance_val;
        }

        if (callee.type == VAL_FUNCTION)
        {

            Func *func = callee.as.function;
            Env *call_env = env_new(func->env);
            Node *arg = n->right;
            Node *param = func->params_head;

            const char *expected_type = NULL;
            const char *actual_type = NULL;
            Value v = (Value){VAL_NIL, {0}}; 

            for (int i = 0; i < n->arity; i++)
            {
                v = eval_node(env, arg);

                if (param->type_name[0] != '\0')
                {
                    expected_type = param->type_name;
                    actual_type = get_value_type_name(v);

                    if (strcmp(expected_type, actual_type) != 0)
                    {
                        if (v.type == VAL_INSTANCE)
                        {

                            Var *expected_class_var = find_var(env, expected_type);

                            if (expected_class_var && expected_class_var->value.type == VAL_CLASS)
                            {

                                Class *expected_class = expected_class_var->value.as.class_obj;
                                Class *actual_class = v.as.instance->class_val->as.class_obj;

                                bool is_match = false;
                                Class *current_class = actual_class;
                                while (current_class)
                                {
                                    if (strcmp(current_class->name, expected_class->name) == 0)
                                    {
                                        is_match = true;
                                        break;
                                    }
                                    current_class = current_class->superclass;
                                }

                                if (!is_match)
                                {
                                    goto type_mismatch_error;
                                }
                            }
                            else
                            {
                                goto type_mismatch_error;
                            }
                        }
                        else
                        {
                            goto type_mismatch_error;
                        }
                    }
                }

                set_var(call_env, param->name, v, false);
                free_value(v);
                arg = arg->next;
                param = param->next;
            }

            Value result = eval_node(call_env, func->body_head);
            env_free(call_env);

            Value final_result = (Value){VAL_NIL, {0}};
            if (result.type == VAL_RETURN)
            {
                final_result = *result.as.return_val;
                free(result.as.return_val);
            }

            if (func->return_type[0] != '\0')
            {
                expected_type = func->return_type;
                actual_type = get_value_type_name(final_result);

                if (strcmp(expected_type, actual_type) != 0)
                {
                    print_error("Type Mismatch: Function expected return type '%s' but got '%s'.",
                                expected_type, actual_type);
                    free_value(final_result);
                    return (Value){VAL_NIL, {0}};
                }
            }

            return final_result;

        type_mismatch_error:
            print_error("Type Mismatch for parameter '%s'. Expected '%s' but got '%s'.",
                        param->name, expected_type, actual_type);

            free_value(v);
            env_free(call_env);
            return (Value){VAL_NIL, {0}};
        }

        if (callee.type == VAL_NATIVE)
        {
            NativeFn native = callee.as.native;
            Value args[255];
            int arg_count = 0;
            Node *arg_node = n->right;
            while (arg_node)
            {
                args[arg_count++] = eval_node(env, arg_node);

                arg_node = arg_node->next;
            }
            Value res = native(arg_count, args);
            for (int i = 0; i < arg_count; i++)
                free_value(args[i]);
            return res;
        }

        return (Value){VAL_NIL, {0}};
    }

    case NODE_RETURN_STMT:
    {
        Value *ret = malloc(sizeof(Value));
        if (n->left)
        {
            *ret = eval_node(env, n->left);
        }
        else
        {
            *ret = (Value){.type = VAL_NIL, .as = {0}};
        }
        return (Value){VAL_RETURN, {.return_val = ret}};
    }

    case NODE_BLOCK:
    {
        Env *block_env = env_new(env);

        Node *current = n->left;
        Value result = (Value){.type = VAL_NIL, .as = {0}};

        while (current)
        {
            free_value(result);
            result = eval_node(block_env, current);
            if (result.type == VAL_RETURN)
            {
                break;
            }

            if (result.type == VAL_RETURN || result.type == VAL_BREAK || result.type == VAL_CONTINUE)
            {
                break;
            }
            current = current->next;
        }

        env_free(block_env);
        return result;
    }

    case NODE_WHILE_STMT:
    {
        Value cond = eval_node(env, n->left);

        while (is_value_truthy(cond))
        {
            free_value(cond);

            Value result = eval_node(env, n->right);
            if (result.type == VAL_RETURN)
            {
                return result;
            }

            if (result.type == VAL_BREAK)
            {
                free_value(result);
                break;
            }
            if (result.type == VAL_CONTINUE)
            {
                free_value(result);
            }
            else
            {
                free_value(result);
            }

            cond = eval_node(env, n->left);
        }

        free_value(cond);
        return (Value){.type = VAL_NIL, .as = {0}};
    }

    case NODE_FOR_STMT:
    {
        Env *for_env = env_new(env);

        Value init_res = eval_node(for_env, n->left);
        free_value(init_res);

        Node *condition = n->right->left;
        Node *increment = n->right->right->left;
        Node *body = n->right->right->right;

        while (1)
        {
            Value cond_res = eval_node(for_env, condition);
            if (!is_value_truthy(cond_res))
            {
                free_value(cond_res);
                break;
            }
            free_value(cond_res);

            Value body_res = eval_node(for_env, body);
            if (body_res.type == VAL_RETURN)
            {
                env_free(for_env);
                return body_res;
            }
            free_value(body_res);

            if (body_res.type == VAL_BREAK)
            {
                free_value(body_res);
                break;
            }

            if (body_res.type == VAL_CONTINUE)
            {
                free_value(body_res);
            }
            else
            {
                free_value(body_res);
            }

            Value inc_res = eval_node(for_env, increment);
            free_value(inc_res);
        }

        env_free(for_env);
        return (Value){.type = VAL_NIL, .as = {0}};
    }
    case NODE_EVERY_LOOP:
    {
        Env *every_env = env_new(env);

        Value time_val = eval_node(every_env, n->left);
        if (time_val.type != VAL_NUMBER || time_val.as.number < 0)
        {
            print_error("Every loop time must be a non-negative number (milliseconds).");
            free_value(time_val);
            env_free(every_env);
            return (Value){.type = VAL_NIL, .as = {0}};
        }

        Value sleep_time_ms = time_val;

        Var *sleep_var = find_var(every_env, "__jackal_sleep");
        if (!sleep_var || sleep_var->value.type != VAL_NATIVE)
        {
            print_error("Internal error: __jackal_sleep native function not found.");
            free_value(sleep_time_ms);
            env_free(every_env);
            return (Value){.type = VAL_NIL, .as = {0}};
        }
        NativeFn sleep_fn = sleep_var->value.as.native;

        Node *body = n->right->left;
        Node *until_condition = n->right->right;

        while (1)
        {
            Value until_cond_res = eval_node(every_env, until_condition);
            if (is_value_truthy(until_cond_res))
            {
                free_value(until_cond_res);
                break;
            }
            free_value(until_cond_res);

            Value sleep_res = sleep_fn(1, &sleep_time_ms);
            free_value(sleep_res);

            Value body_res = eval_node(every_env, body);

            if (body_res.type == VAL_RETURN || body_res.type == VAL_BREAK || body_res.type == VAL_CONTINUE)
            {
                free_value(body_res);
                if (body_res.type == VAL_RETURN || body_res.type == VAL_BREAK)
                {
                    break;
                }
            }

            free_value(body_res);
        }

        free_value(sleep_time_ms);
        env_free(every_env);
        return (Value){.type = VAL_NIL, .as = {0}};
    }

    case NODE_OBSERVE_STMT:
    {
        Node *case_node = n->right;

        while (1)
        {

            for (Node *current_case = case_node; current_case; current_case = current_case->next)
            {

                Value condition_result = eval_node(env, current_case->left);

                if (is_value_truthy(condition_result))
                {

                    Value action_result = eval_node(env, current_case->right);

                    if (action_result.type == VAL_BREAK || action_result.type == VAL_RETURN)
                    {
                        free_value(action_result);
                        free_value(condition_result);
                        return action_result;
                    }
                    free_value(action_result);
                }
                free_value(condition_result);
            }
            Var *sleep_var = find_var(env, "__jackal_sleep");
            if (sleep_var && sleep_var->value.type == VAL_NATIVE)
            {
                Value args[1] = {(Value){VAL_NUMBER, {.number = 1.0}}};
                Value sleep_res = sleep_var->value.as.native(1, args);
                free_value(sleep_res);
            }
        }

        return (Value){.type = VAL_NIL, .as = {0}};
    }

    default:
        return (Value){.type = VAL_NIL, .as = {0}};
    }
}
