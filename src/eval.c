#include "eval.h"

static Var* find_var(Env* env, const char* name) {
    for (Var* v = env->vars; v; v = v->next)
        if (strcmp(v->name, name) == 0) return v;
    return NULL;
}

static void set_var(Env* env, const char* name, double value) {
    Var* v = find_var(env, name);
    if (v) { v->value = value; return; }
    Var* n = malloc(sizeof(Var));
    strcpy(n->name, name);
    n->value = value;
    n->next = env->vars;
    env->vars = n;
}

double eval_node(Env* env, Node* n) {
    if (!n) return 0;

    double result = 0; 

    switch (n->kind) {
        case NODE_NUMBER: result = n->value; break; 
        case NODE_IDENT: {
            Var* v = find_var(env, n->name);
            if (!v) { printf("Undefined variable: %s\n", n->name); return 0; }
            result = v->value; // beri 'result ='
            break;
        }
        case NODE_BINOP: {
            double l = eval_node(env, n->left);
            double r = eval_node(env, n->right);
            switch (n->name[0]) {
                case '+': result = l + r; break; 
                case '-': result = l - r; break; 
                case '*': result = l * r; break;
                case '/': result = l / r; break; 
                default: result = 0; break;
            }
            return result; 
        }
        case NODE_VARDECL: {
            double val = eval_node(env, n->right);
            set_var(env, n->name, val);
            result = val; 
            return result; 
        }
        case NODE_PRINT: {
            double val = eval_node(env, n->right);
            printf("%g\n", val);
            result = val; 
            
            return result; 
        }
        default: result = 0; break;
    }

    if (n->right) {
        eval_node(env, n->right);
    }
    
    return result;
}

void env_free(Env* env) {
    Var* v = env->vars;
    while (v) {
        Var* next = v->next;
        free(v);
        v = next;
    }
}
