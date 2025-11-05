#include "eval.h"
#include <string.h>
#include <stdbool.h>

Env* env_new(Env* outer) {
    Env* env = calloc(1, sizeof(Env));
    env->outer = outer;
    return env;
}

static Var* find_var(Env* env, const char* name) {
    for (Var* v = env->vars; v; v = v->next) {
        if (strcmp(v->name, name) == 0) return v;
    }
    if (env->outer) {
        return find_var(env->outer, name);
    }
    return NULL;
}
static void set_var(Env* env, const char* name, Value value);
Value copy_value(Value value) {
    if (value.type == VAL_STRING) {
        char* new_string = malloc(strlen(value.as.string) + 1);
        if (new_string) { 
            strcpy(new_string, value.as.string);
        }
        return (Value){VAL_STRING, {.string = new_string}};
    }
    if (value.type == VAL_FUNCTION) {
        Func* old_func = value.as.function;
        Func* new_func = malloc(sizeof(Func));
        memcpy(new_func, old_func, sizeof(Func));
        return (Value){VAL_FUNCTION, {.function = new_func}};
    }
    if (value.type == VAL_ARRAY) {
        ValueArray* old_arr = value.as.array;
        ValueArray* new_arr = array_new();
        
        for (int i = 0; i < old_arr->count; i++) {
            array_append(new_arr, copy_value(old_arr->values[i]));
        }
        return (Value){VAL_ARRAY, {.array = new_arr}};
    }
    if (value.type == VAL_CLASS) {
        return value;
    }
    
    if (value.type == VAL_INSTANCE) {
        Instance* old_inst = value.as.instance;
        Instance* new_inst = malloc(sizeof(Instance));
        
        new_inst->class_val = malloc(sizeof(Value));
        *new_inst->class_val = *old_inst->class_val;
        
        new_inst->fields = env_new(NULL);
        if (old_inst->fields) {
            for (Var* v = old_inst->fields->vars; v; v = v->next) {
                set_var(new_inst->fields, v->name, v->value);
            }
        }
        return (Value){VAL_INSTANCE, {.instance = new_inst}};
    }
    
    return value; 
}
static void set_var(Env* env, const char* name, Value value) {
    Var* n = malloc(sizeof(Var));
    if (!n) return; 
    strcpy(n->name, name);
    n->value = copy_value(value); 
    n->next = env->vars;
    env->vars = n;
}

static bool is_value_truthy(Value value) {
    switch (value.type) {
        case VAL_NIL:    return false;
        case VAL_NUMBER: return value.as.number != 0;
        case VAL_STRING: return strlen(value.as.string) > 0;
        case VAL_FUNCTION: return true;
        case VAL_ARRAY: return value.as.array->count > 0;
        case VAL_CLASS:    return true;
        case VAL_INSTANCE: return true;
        case VAL_RETURN: return is_value_truthy(*value.as.return_val);
    }
    return false;
}

bool is_number(Value a, Value b) { return a.type == VAL_NUMBER && b.type == VAL_NUMBER; }
bool is_string(Value a, Value b) { return a.type == VAL_STRING && b.type == VAL_STRING; }

Value eval_equals(Value a, Value b) {
    double result = 0.0;
    if (a.type != b.type) {
        result = 0.0;
    } else {
        switch(a.type) {
            case VAL_NIL: result = 1.0; break;
            case VAL_NUMBER: result = (a.as.number == b.as.number); break;
            case VAL_STRING: result = (strcmp(a.as.string, b.as.string) == 0); break;
            case VAL_FUNCTION: result = (a.as.function == b.as.function); break;
            case VAL_ARRAY: result = (a.as.array == b.as.array); break;
            case VAL_CLASS: result = (a.as.class_obj == b.as.class_obj); break;
            case VAL_INSTANCE: result = (a.as.instance == b.as.instance); break;
            default: result = 0.0; break;
        }
    }
    return (Value){VAL_NUMBER, {.number = result}};
}

Value eval_node(Env* env, Node* n);

Value eval_node(Env* env, Node* n) {
    if (!n) return (Value){.type = VAL_NIL, .as = {0}};

    switch (n->kind) {
        case NODE_NUMBER:
            return (Value){VAL_NUMBER, {.number = n->value}};
        
        case NODE_STRING: {
            char* str_copy = malloc(strlen(n->name) + 1);
            if (str_copy) { 
                strcpy(str_copy, n->name);
            }
            return (Value){VAL_STRING, {.string = str_copy}};
        }

        case NODE_ARRAY_LITERAL: {
            ValueArray* arr = array_new();
            Node* item = n->left;
            for (int i = 0; i < n->arity; i++) {
                Value val = eval_node(env, item);
                array_append(arr, val);
                item = item->right;
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
                Value method_val = eval_node(class_obj->methods, method);
                free_value(method_val);
                method = method->right;
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
                    arg_node = arg_node->right;
                    param_node = param_node->right;
                }
                
                Value result = eval_node(call_env, func->body_head);
                env_free(call_env);
                
                // free_value(instance_val); // <-- BARIS INI DIHAPUS
                
                if (result.type == VAL_RETURN) {
                    Value actual_return = *result.as.return_val;
                    free(result.as.return_val);
                    return actual_return;
                }
                return (Value){.type = VAL_NIL, .as = {0}};
            }

            Value callee = eval_node(env, n->left);
            
            if (callee.type == VAL_CLASS) {
                Instance* inst = malloc(sizeof(Instance));
                inst->class_val = malloc(sizeof(Value));
                *inst->class_val = callee;
                inst->fields = env_new(NULL);
                return (Value){VAL_INSTANCE, {.instance = inst}};
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
                
                param_node = param_node->right;
                arg_node = arg_node->right;
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
                current = current->right;
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

void env_free(Env* env) {
    if (env == NULL) return;
    Var* v = env->vars;
    while (v) {
        Var* next = v->next;
        free_value(v->value); 
        free(v);
        v = next;
    }
    free(env);
}