#pragma once
#include "parser.h"
#include "common.h"
#include <stdbool.h>
#include<cjson/cJSON.h>


typedef struct {
    double* data;      
    int* shape;      
    int* strides;  
    int ndim;        
    int size;         
} Tensor;

typedef struct {
    int k;
    int n_features;
    double** centroids;
} KMeansModel;


typedef struct {
    double mean;
    double variance;
} FeatureStats;


typedef struct {
    int class_label;
    int sample_count;
    FeatureStats* stats;
} ClassModel;

/**
 * @typedef @struct ENTRY
 * Represents a key-value pair in the HashMap.
 */
typedef struct {
    char* key;
    Value value;
} Entry;

/**
 * @typedef @struct HASHMAP
 * Represents a hash map (dictionary) in the Jackal programming language.
 */
typedef struct HashMap {
    int count;
    int capacity;
    Entry* entries;
} HashMap;


/**
 * @typedef @struct VALUEARRAY
 * Represents a dynamic array of Values in the Jackal programming language.
 */
ValueArray* array_new(void);

/**
 * Appends a Value to a ValueArray.
 * @param arr The ValueArray to append to.
 * @param val The Value to append.
 */
void array_append(ValueArray* arr, Value val);

/**
 * Frees the memory associated with a ValueArray.
 * @param arr The ValueArray to be freed.
 */
void array_free(ValueArray* arr);

/**
 * Prints a Value to stdout.
 * @param value The Value to be printed.
 */
void print_value(Value value);

/**
 * Frees the memory associated with a Value.
 * @param value The Value to be freed.
 */
void free_value(Value value);

/**
 * Creates a copy of a Value.
 * @param value The Value to be copied.
 * @return A new Value that is a copy of the input.
 */
Value copy_value(Value value);

/**
 * Determines if a Value is truthy.
 * @param value The Value to be evaluated.
 * @return true if the Value is truthy, false otherwise.
 */
bool is_value_truthy(Value value);

/**
 * Evaluates equality between two Values.
 * @param a The first Value.
 * @param b The second Value.
 * @return A Value of type VAL_NUMBER with 1.0 if equal, 0.0 otherwise.
 */
Value eval_equals(Value a, Value b);

/**
 * Deletes a Value at a specific index from a ValueArray.
 * @param arr The ValueArray to delete from.
 * @param index The index of the Value to delete.
 */
void array_delete(ValueArray* arr, int index);

/**
 * Pops the last Value from a ValueArray.
 * @param arr The ValueArray to pop from.
 * @return The popped Value.
 */
Value array_pop(ValueArray* arr);

/**
 * @typedef @struct HASHMAP
 * Represents a hash map (dictionary) in the Jackal programming language.
 */
HashMap* map_new(void);

/**
 * Frees the memory associated with a HashMap.
 * @param map The HashMap to be freed.
 */
void map_free(HashMap* map);

/**
 * Retrieves a Value from the HashMap by key.
 * @param map The HashMap to retrieve from.
 * @param key The key of the Value to retrieve.
 * @param out_val Pointer to store the retrieved Value.
 * @return true if the key exists and out_val is set, false otherwise.
 */
bool map_get(HashMap* map, const char* key, Value* out_val);

/**
 * Sets a key-value pair in the HashMap.
 * @param map The HashMap to set the value in.
 * @param key The key of the Value to set.
 * @param val The Value to set.
 */
void map_set(HashMap* map, const char* key, Value val);

/**
 * Jackal_value_to_cjson
 * @brief parse jackal std value to json format
 * @param jackal_val
 */
cJSON *jackal_value_to_cjson(Value jackal_val);

/**
 * builin_read_line
 * @brief represents the input function for jackal programming language
 */
Value builtin_read_line(int arity, Value* args);

Value builtin_read_array(int arity, Value* args);

Value builtin_print_table(int argCount, Value* args);

Value builtin_print_json(int argCount, Value *args);

Value builtin_map_forEach(int argCount, Value* args);

Value builtin_map_keys(int argCount, Value* args);

Value builtin_map_values(int argCount, Value* args);

Value builtin_array_distinct(int argCount, Value* args);

Value builtin_array_anyMatch(int argCount, Value* args);

Value builtin_array_map(int argCount, Value* args);

Value builtin_array_filter(int argCount, Value* args);

Value builtin_array_reduce(int argCount, Value* args);

Value builtin_array_sort(int argCount, Value* args);

Value builtin_array_limit(int argCount, Value* args);

Value builtin_array_statistics(int argCount, Value *args);

Value builtin_map_get(int argCount, Value *args);

Value builtin_array_mean(int argCount, Value *args);

Value builtin_array_max(int argCount, Value *args);

Value builtin_json_parse(int argCount, Value *args);

Value builtin_json_stringify(int argCount, Value *args);

Value builtin_type(int argCount, Value *args);

Value builtin_json_stringify(int argCount, Value *args) ;

Value builtin_plot(int argCount, Value *args);

Value builtin_web_show(int argCount, Value *args);

Value builtin_web_sync(int argCount, Value *args);

Value builtin_system(int argCount, Value *args) ;

Value builtin_http_serve(int argCount, Value *args);

Value builtin_array_to_tree(int argCount, Value* args);

void inorder_traverse_logic(ValueArray* source, ValueArray* dest, int index);

void preorder_traverse_logic(ValueArray* source, ValueArray* dest, int index);

void postorder_traverse_logic(ValueArray* source, ValueArray* dest, int index);

Value native_plot(int arg_count, Value* args);

Value native_transpose(int arg_count, Value* args);

Value native_linear_regression(int arg_count, Value* args);

Value native_standardize(int arg_count, Value* args);

Value native_smooth(int arg_count, Value* args);

Value native_correlate(int arg_count, Value* args);

// Value native_knn(int arg_count, Value* args);

Value native_accuracy(int arg_count, Value* args);

Value native_knn_nd(int arg_count, Value* args);

Value native_normalize_nd(int arg_count, Value* args) ;

Value native_knn_prob(int arg_count, Value* args);

Value native_confusion_matrix(int arg_count, Value *args);

Value native_split(int arg_count, Value* args) ;

Value native_sync_shuffle(int arg_count, Value* args);

Value native_zip(int arg_count, Value* args);

Value native_logistic_predict(int arg_count, Value* args);

Value native_logistic_fit(int arg_count, Value* args);

Value native_nb_predict(int arg_count, Value* args);

Value native_nb_fit(int arg_count, Value* args);

Value native_kmeans_fit(int arg_count, Value* args);

Value native_kmeans_predict(int arg_count, Value* args);

Value native_kmeans_loss(int arg_count, Value* args);

Value native_matrix_dot(int arg_count, Value* args);

Value native_matrix_add(int arg_count, Value* args);

Value native_matrix_sub(int arg_count, Value* args);

Value native_matrix_det(int arg_count, Value* args);

Value native_matrix_scalar_mul(int arg_count, Value* args);

Value native_read_csv(int arg_count, Value* args);

Value native_tensor_add(int arg_count, Value* args);

Value native_tensor_sub(int arg_count, Value* args);

Value native_tensor_mul(int arg_count, Value* args);

Value native_tensor_check_shape(int arg_count, Value* args);

Value native_save_jml(int arg_count, Value* args);

Value native_load_jml(int arg_count, Value* args);

Value native_tensor_sum(int arg_count, Value* args);

Value native_tensor_mean(int arg_count, Value* args);

Value native_tensor_dot(int arg_count, Value* args);

Value native_vector_dot(int arg_count, Value* args);

Value native_vector_norm(int arg_count, Value* args);

Value native_math_acos(int arg_count, Value* args);

Value native_load_csv_smart(int arg_count, Value* args);