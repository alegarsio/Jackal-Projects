#include "eval.h"
#include "value.h"
#include "env.h"
#include <string.h>
#include <stdbool.h>

/**
 * @brief Checks if two values are both numbers.
 */
static bool is_number(Value a, Value b) { return a.type == VAL_NUMBER && b.type == VAL_NUMBER; }

/**
 * @brief Checks if two values are both strings.
 */

static bool is_string(Value a, Value b) { return a.type == VAL_STRING && b.type == VAL_STRING; }


/**
 * @brief Main evaluation function.
 * Recursively evaluates an AST node in a given environment.
 * @param env The current variable environment.
 * @param n The AST node to evaluate.
 * @return The result of the evaluation as a Value.
 */


Value eval_node(Env* env, Node* n) {
    if (!n) return (Value){.type = VAL_NIL, .as = {0}};

    switch (n->kind) {

        /**
         * literal Values
         */
        case NODE_NUMBER:
            return (Value){VAL_NUMBER, {.number = n->value}};
        
        /**
         * Strings must be heap-allocated because they are passed around by pointer.
         */
        case NODE_STRING: {
            char* str_copy = malloc(strlen(n->name) + 1);
            if (str_copy) { 
                strcpy(str_copy, n->name);
            }
            return (Value){VAL_STRING, {.string = str_copy}};
        }

        // Create a new dynamic array and populate it by evaluating each item.
        case NODE_ARRAY_LITERAL: {
            ValueArray* arr = array_new();
            Node* item = n->left;
            for (int i = 0; i < n->arity; i++) {
                Value val = eval_node(env, item);
                array_append(arr, val);
                item = item->next;
            }
            return (Value){VAL_ARRAY, {.array = arr}};
        }
        
        case NODE_ARRAY_ACCESS: {
            Value arr_val = eval_node(env, n->left);
            Value idx_val = eval_node(env, n->right);
            
            if (arr_val.type != VAL_ARRAY) {
                print_error("Value is not an array.");
                return (Value){.type = VAL_NIL, .as = {0}};
            }
            if (idx_val.type != VAL_NUMBER) {
                print_error("Index must be a number.");
                return (Value){.type = VAL_NIL, .as = {0}};
            }
            
            int idx = (int)idx_val.as.number;
            if (idx < 0 || idx >= arr_val.as.array->count) {
                print_error("Array index out of bounds.");
                return (Value){.type = VAL_NIL, .as = {0}};
            }
            
            free_value(idx_val);
            Value result = copy_value(arr_val.as.array->values[idx]);
            free_value(arr_val);
            return result;
        }
        
        case NODE_ARRAY_ASSIGN: {
            Value new_val = eval_node(env, n->right);
            
            Node* access_node = n->left;
            Value arr_val = eval_node(env, access_node->left);
            Value idx_val = eval_node(env, access_node->right);
            
            if (arr_val.type != VAL_ARRAY) {
                print_error("Value is not an array.");
                return (Value){.type = VAL_NIL, .as = {0}};
            }
            if (idx_val.type != VAL_NUMBER) {
                print_error("Index must be a number.");
                return (Value){.type = VAL_NIL, .as = {0}};
            }
            
            int idx = (int)idx_val.as.number;
            if (idx < 0 || idx >= arr_val.as.array->count) {
                print_error("Array index out of bounds.");
                return (Value){.type = VAL_NIL, .as = {0}};
            }
            
            free_value(arr_val.as.array->values[idx]);
            arr_val.as.array->values[idx] = copy_value(new_val);
            
            free_value(arr_val);
            free_value(idx_val);
            return new_val;
        }

        case NODE_IDENT: {
            Var* v = find_var(env, n->name);
            if (!v) { 
                print_error("Undefined variable."); 
                return (Value){.type = VAL_NIL, .as = {0}}; 
            }
            return copy_value(v->value);
        }
        
        case NODE_THIS: {
            Var* v = find_var(env, n->name);
            if (!v) { 
                print_error("'this' is not defined."); 
                return (Value){.type = VAL_NIL, .as = {0}}; 
            }
            return copy_value(v->value); 
        }
        
        case NODE_ASSIGN: {
            Value val = eval_node(env, n->left);
            Var* v = find_var(env, n->name);
            if (!v) {
                print_error("Undefined variable.");
                free_value(val);
                return (Value){.type = VAL_NIL, .as = {0}};
            }
            free_value(v->value);
            v->value = copy_value(val);
            return val;
        }
        
        case NODE_GET: {
            Value obj = eval_node(env, n->left);
            if (obj.type != VAL_INSTANCE) {
                print_error("Only instances have properties.");
                return (Value){.type = VAL_NIL, .as = {0}};
            }
            
            Var* field = find_var(obj.as.instance->fields, n->name);
            if (field) {
                Value result = copy_value(field->value);
                free_value(obj);
                return result;
            }
            
            Var* method = find_var(obj.as.instance->class_val->as.class_obj->methods, n->name);
            if (method) {
                Value result = copy_value(method->value);
                free_value(obj);
                return result;
            }
            
            print_error("Undefined property.");
            free_value(obj);
            return (Value){.type = VAL_NIL, .as = {0}};
        }
        
        case NODE_SET: {
            Value obj = eval_node(env, n->left->left);
            if (obj.type != VAL_INSTANCE) {
                print_error("Only instances have fields.");
                return (Value){.type = VAL_NIL, .as = {0}};
            }
            
            Value val = eval_node(env, n->right);
            // Karena obj sekarang adalah referensi, set_var akan mengubah objek asli
            set_var(obj.as.instance->fields, n->left->name, val);
            free_value(obj);
            return val;
        }
        
        case NODE_POST_INC: {
            if (n->left->kind != NODE_IDENT) {
                print_error("Invalid l-value for '++'.");
                return (Value){.type = VAL_NIL, .as = {0}};
            }
            Var* v = find_var(env, n->left->name);
            if (v->value.type != VAL_NUMBER) {
                print_error("Operand for '++' must be a number.");
                return (Value){.type = VAL_NIL, .as = {0}};
            }
            double old_val = v->value.as.number;
            free_value(v->value);
            v->value = (Value){VAL_NUMBER, {.number = old_val + 1}};
            return (Value){VAL_NUMBER, {.number = old_val}};
        }
        
        case NODE_POST_DEC: {
            if (n->left->kind != NODE_IDENT) {
                print_error("Invalid l-value for '--'.");
                return (Value){.type = VAL_NIL, .as = {0}};
            }
            Var* v = find_var(env, n->left->name);
            if (v->value.type != VAL_NUMBER) {
                print_error("Operand for '--' must be a number.");
                return (Value){.type = VAL_NIL, .as = {0}};
            }
            double old_val = v->value.as.number;
            free_value(v->value);
            v->value = (Value){VAL_NUMBER, {.number = old_val - 1}};
            return (Value){VAL_NUMBER, {.number = old_val}};
        }

        case NODE_BINOP: {
            if (n->op == TOKEN_AND_AND) {
                Value left = eval_node(env, n->left);
                if (!is_value_truthy(left)) {
                    return left;
                }
                free_value(left);
                return eval_node(env, n->right);
            }
            
            Value left = eval_node(env, n->left);
            Value right = eval_node(env, n->right);

            switch (n->op) {
                case TOKEN_PLUS:
                    if (is_number(left, right)) {
                        return (Value){VAL_NUMBER, {.number = left.as.number + right.as.number}};
                    }
                    if (is_string(left, right)) {
                        size_t len_left = strlen(left.as.string);
                        size_t len_right = strlen(right.as.string);
                        char* new_string = malloc(len_left + len_right + 1);
                        if (new_string) { 
                            strcpy(new_string, left.as.string);
                            strcat(new_string, right.as.string);
                        }
                        free_value(left); free_value(right);
                        return (Value){VAL_STRING, {.string = new_string}};
                    }
                    print_error("Operands must be two numbers or two strings for '+'.");
                    break;
                case TOKEN_MINUS:
                    if (is_number(left, right)) {
                        return (Value){VAL_NUMBER, {.number = left.as.number - right.as.number}};
                    }
                    print_error("Operands must be numbers for '-'.");
                    break;
                case TOKEN_STAR:
                    if (is_number(left, right)) {
                        return (Value){VAL_NUMBER, {.number = left.as.number * right.as.number}};
                    }
                    print_error("Operands must be numbers for '*'.");
                    break;
                case TOKEN_SLASH:
                    if (is_number(left, right)) {
                        return (Value){VAL_NUMBER, {.number = left.as.number / right.as.number}};
                    }
                    print_error("Operands must be numbers for '/'.");
                    break;
                case TOKEN_GREATER:
                    if (is_number(left, right)) {
                        return (Value){VAL_NUMBER, {.number = (left.as.number > right.as.number) ? 1.0 : 0.0}};
                    }
                    print_error("Operands must be numbers for '>'.");
                    break;
                case TOKEN_GREATER_EQUAL:
                    if (is_number(left, right)) {
                        return (Value){VAL_NUMBER, {.number = (left.as.number >= right.as.number) ? 1.0 : 0.0}};
                    }
                    print_error("Operands must be numbers for '>='.");
                    break;
                case TOKEN_LESS:
                    if (is_number(left, right)) {
                        return (Value){VAL_NUMBER, {.number = (left.as.number < right.as.number) ? 1.0 : 0.0}};
                    }
                    print_error("Operands must be numbers for '<'.");
                    break;
                case TOKEN_LESS_EQUAL:
                    if (is_number(left, right)) {
                        return (Value){VAL_NUMBER, {.number = (left.as.number <= right.as.number) ? 1.0 : 0.0}};
                    }
                    print_error("Operands must be numbers for '<='.");
                    break;
                case TOKEN_EQUAL_EQUAL: {
                    Value result = eval_equals(left, right);
                    free_value(left); free_value(right);
                    return result;
                }
                case TOKEN_BANG_EQUAL: {
                    Value result = eval_equals(left, right);
                    result.as.number = (result.as.number == 0.0) ? 1.0 : 0.0;
                    free_value(left); free_value(right);
                    return result;
                }
                default: 
                    print_error("Unknown binary operator.");
            }
            
            free_value(left); free_value(right);
            return (Value){.type = VAL_NIL, .as = {0}};
        }
        
        case NODE_VARDECL: {
            Value val = eval_node(env, n->right);
            set_var(env, n->name, val);
            free_value(val); 
            return (Value){.type = VAL_NIL, .as = {0}}; 
        }

        case NODE_PRINT: {
            Value val = eval_node(env, n->right);
            print_value(val); 
            printf("\n");
            free_value(val); 
            return (Value){.type = VAL_NIL, .as = {0}}; 
        }

        case NODE_IF_STMT: {
            Value cond = eval_node(env, n->left);
            bool is_true = is_value_truthy(cond);
            free_value(cond); 

            Node* then_branch = n->right->left;
            Node* else_branch = n->right->right;

            Value result = (Value){.type = VAL_NIL, .as = {0}};
            if (is_true) {
                result = eval_node(env, then_branch);
            } else if (else_branch != NULL) {
                result = eval_node(env, else_branch);
            }
            
            return result;
        }
        
        case NODE_CLASS_DEF: {
            Class* class_obj = malloc(sizeof(Class));
            strcpy(class_obj->name, n->name);
            class_obj->methods = env_new(env);
            
            Node* method = n->left;
            while(method) {
                Node* next_method = method->next;
                Value method_val = eval_node(class_obj->methods, method);
                free_value(method_val);
                method = next_method;
            }

            Value class_val = (Value){VAL_CLASS, {.class_obj = class_obj}};
            set_var(env, n->name, class_val);
            return (Value){.type = VAL_NIL, .as = {0}};
        }
        
        case NODE_FUNC_DEF: {
            Func* func = malloc(sizeof(Func));
            func->params_head = n->left;
            func->body_head = n->right;
            func->env = env;
            func->arity = n->arity;

            n->left = NULL;
            n->right = NULL;

            Value func_val = (Value){VAL_FUNCTION, {.function = func}};
            set_var(env, n->name, func_val);
            
            return (Value){.type = VAL_NIL, .as = {0}};
        }
        
        case NODE_FUNC_CALL: {
            if (n->left->kind == NODE_GET) {
                Node* get_node = n->left;
                Value instance_val = eval_node(env, get_node->left);
                if (instance_val.type != VAL_INSTANCE) {
                    print_error("Can only call methods on instances.");
                    return (Value){.type = VAL_NIL, .as = {0}};
                }
                
                Var* method_var = find_var(instance_val.as.instance->class_val->as.class_obj->methods, get_node->name);
                if (!method_var) {
                    print_error("Undefined method.");
                    return (Value){.type = VAL_NIL, .as = {0}};
                }
                
                Func* func = method_var->value.as.function;
                Env* call_env = env_new(func->env);
                set_var(call_env, "this", instance_val);
                
                Node* arg_node = n->right;
                Node* param_node = func->params_head;
                for (int i = 0; i < n->arity; i++) {
                    Value arg_val = eval_node(env, arg_node);
                    set_var(call_env, param_node->name, arg_val);
                    free_value(arg_val);
                    arg_node = arg_node->next;
                    param_node = param_node->next;
                }
                
                Value result = eval_node(call_env, func->body_head);
                env_free(call_env);
                free_value(instance_val);
                
                if (result.type == VAL_RETURN) {
                    Value actual_return = *result.as.return_val;
                    free(result.as.return_val);
                    return actual_return;
                }
                return (Value){.type = VAL_NIL, .as = {0}};
            }

            if (n->left->kind == NODE_IDENT && strcmp(n->left->name, "read") == 0) {
                 char buffer[1024];
                 if (fgets(buffer, sizeof(buffer), stdin)) {
                     size_t len = strlen(buffer);
                     if (len > 0 && buffer[len - 1] == '\n') buffer[len - 1] = '\0';
                     char* str = malloc(len + 1);
                     strcpy(str, buffer);
                     return (Value){VAL_STRING, {.string = str}};
                 }
                 return (Value){VAL_NIL, .as = {0}};
            }

            Value callee = eval_node(env, n->left);
            
            if (callee.type == VAL_CLASS) {
                Instance* inst = malloc(sizeof(Instance));
                inst->class_val = malloc(sizeof(Value));
                *inst->class_val = callee;
                inst->fields = env_new(NULL);
                
                Value instance_val = (Value){VAL_INSTANCE, {.instance = inst}};
                
                Var* init_method = find_var(callee.as.class_obj->methods, "init");
                if (init_method) {
                    Func* func = init_method->value.as.function;
                    
                    if (n->arity != func->arity) {
                        print_error("Incorrect number of arguments for init().");
                        return (Value){.type = VAL_NIL, .as = {0}};
                    }

                    Env* call_env = env_new(func->env);
                    set_var(call_env, "this", instance_val);
                    
                    Node* arg_node = n->right;
                    Node* param_node = func->params_head;
                    for (int i = 0; i < n->arity; i++) {
                        Value arg_val = eval_node(env, arg_node);
                        set_var(call_env, param_node->name, arg_val);
                        free_value(arg_val);
                        arg_node = arg_node->next;
                        param_node = param_node->next;
                    }
                    
                    Value init_result = eval_node(call_env, func->body_head);
                    free_value(init_result);
                    env_free(call_env);
                }

                return instance_val;
            }
            
            if (callee.type != VAL_FUNCTION) {
                print_error("Can only call functions and classes.");
                return (Value){.type = VAL_NIL, .as = {0}};
            }
            
            Func* func = callee.as.function;
            
            if (n->arity != func->arity) {
                print_error("Incorrect number of arguments.");
                return (Value){.type = VAL_NIL, .as = {0}};
            }
            
            Env* call_env = env_new(func->env);
            
            Node* param_node = func->params_head;
            Node* arg_node = n->right;
            
            for(int i = 0; i < func->arity; i++) {
                Value arg_val = eval_node(env, arg_node);
                set_var(call_env, param_node->name, arg_val);
                free_value(arg_val);
                arg_node = arg_node->next;
                param_node = param_node->next;
            }
            
            Value result = eval_node(call_env, func->body_head);
            
            env_free(call_env);
            
            if (result.type == VAL_RETURN) {
                Value actual_return = *result.as.return_val;
                free(result.as.return_val);
                return actual_return;
            }

            
            
            return (Value){.type = VAL_NIL, .as = {0}};
        }
        
        case NODE_RETURN_STMT: {
            Value* ret = malloc(sizeof(Value));
            if (n->left) {
                *ret = eval_node(env, n->left);
            } else {
                *ret = (Value){.type = VAL_NIL, .as = {0}};
            }
            return (Value){VAL_RETURN, {.return_val = ret}};
        }
        
        case NODE_BLOCK: {
            Env* block_env = env_new(env);
            
            Node* current = n->left;
            Value result = (Value){.type = VAL_NIL, .as = {0}};
            
            while (current) {
                free_value(result);
                result = eval_node(block_env, current);
                if (result.type == VAL_RETURN) {
                    break;
                }
                current = current->next;
            }
            
            env_free(block_env);
            return result;
        }

        case NODE_WHILE_STMT: {
            Value cond = eval_node(env, n->left);
            
            while (is_value_truthy(cond)) {
                free_value(cond);
                
                Value result = eval_node(env, n->right);
                if (result.type == VAL_RETURN) {
                    return result;
                }
                free_value(result);
                
                cond = eval_node(env, n->left);
            }
            
            free_value(cond);
            return (Value){.type = VAL_NIL, .as = {0}};
        }

        case NODE_FOR_STMT: {
            Env* for_env = env_new(env);
            
            Value init_res = eval_node(for_env, n->left);
            free_value(init_res);
            
            Node* condition = n->right->left;
            Node* increment = n->right->right->left;
            Node* body = n->right->right->right;
            
            while(1) {
                Value cond_res = eval_node(for_env, condition);
                if (!is_value_truthy(cond_res)) {
                    free_value(cond_res);
                    break;
                }
                free_value(cond_res);
                
                Value body_res = eval_node(for_env, body);
                if (body_res.type == VAL_RETURN) {
                    env_free(for_env);
                    return body_res;
                }
                free_value(body_res);
                
                Value inc_res = eval_node(for_env, increment);
                free_value(inc_res);
            }
            
            env_free(for_env);
            return (Value){.type = VAL_NIL, .as = {0}};
        }

        default: 
            return (Value){.type = VAL_NIL, .as = {0}};
    }
}