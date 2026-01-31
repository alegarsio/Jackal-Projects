#include "native/native_registry.h"
#include "socket/socket_native.h"
#include "String/string_native.h"
#include "System/system_native.h"
#include "math/native_math.h"
#include "Io/io_native.h"
#include "File/native_file.h"
#include"http/native_http.h"
#include"json/native_json.h"

/**
 * Register DEFAULT is now DEPRECATED
 */

#define SAFE_REGISTER(env, name, func) \
    do { \
        if (func != NULL) { \
            set_var(env, name, (Value){VAL_NATIVE, {.native = func}}, true, ""); \
        } else { \
            print_error("Native function '%s' is not implemented!", name); \
        } \
    } while (0)

#define REGISTER_CONST(env, name, val) set_var(env, name, val, true, "")

void register_all_natives(Env* env) {
    if (env == NULL) return;

    REGISTER_CONST(env, "nil", ((Value){VAL_NIL, {0}}));
    REGISTER_CONST(env, "__math_PI", ((Value){VAL_NUMBER, {.number = 3.1415926535}}));

    SAFE_REGISTER(env, "typeof", builtin_typeof);
    SAFE_REGISTER(env, "__typeof", builtin_type);
    SAFE_REGISTER(env, "systems", builtin_system);

    SAFE_REGISTER(env, "__math_sqrt", builtin_math_sqrt);
    SAFE_REGISTER(env, "__math_pow", builtin_math_pow);
    SAFE_REGISTER(env, "fmod", builtin_math_fmod);
    SAFE_REGISTER(env, "achos", native_math_acos);

    SAFE_REGISTER(env, "println", builtin_writeline);
    SAFE_REGISTER(env, "print", builtin_write);
    SAFE_REGISTER(env, "__io_open", builtin_io_open);
    SAFE_REGISTER(env, "__io_readAll", builtin_io_readAll);
    SAFE_REGISTER(env, "__io_write", builtin_io_write);
    SAFE_REGISTER(env, "__io_close", builtin_io_close);
    // // safe register now in src/Io/io_native.c
    // SAFE_REGISTER(env, "__io_autostream", builtin_read_line);
    SAFE_REGISTER(env, "__io_read_array", builtin_read_array);
    SAFE_REGISTER(env, "__io_table_stream", builtin_print_table);
    SAFE_REGISTER(env, "__io_json", builtin_print_json);
    SAFE_REGISTER(env, "_io_plot", builtin_plot);

    SAFE_REGISTER(env, "__array_distinct", builtin_array_distinct);
    SAFE_REGISTER(env, "__array_anyMatch", builtin_array_anyMatch);
    SAFE_REGISTER(env, "__array_map", builtin_array_map);
    SAFE_REGISTER(env, "__array_filter", builtin_array_filter);
    SAFE_REGISTER(env, "__array_reduce", builtin_array_reduce);
    SAFE_REGISTER(env, "__array_sort", builtin_array_sort);
    SAFE_REGISTER(env, "__array_statistics", builtin_array_statistics);
    SAFE_REGISTER(env, "__array_mean", builtin_array_mean);
    SAFE_REGISTER(env, "__array_max", builtin_array_max);
    SAFE_REGISTER(env, "__array_to_tree", builtin_array_to_tree);
    SAFE_REGISTER(env, "__array_limit", builtin_array_limit);

    SAFE_REGISTER(env, "__knn", native_knn_nd);
    SAFE_REGISTER(env, "__accuracy", native_accuracy);
    SAFE_REGISTER(env, "__normalize", native_normalize_nd);
    SAFE_REGISTER(env, "__knn_predictprob", native_knn_prob);
    SAFE_REGISTER(env, "__confusion_matrix", native_confusion_matrix);
    SAFE_REGISTER(env, "__split", native_split);
    SAFE_REGISTER(env, "__sync_shuffle", native_sync_shuffle);
    SAFE_REGISTER(env, "__logistic_predict", native_logistic_predict);
    SAFE_REGISTER(env, "__logistic_fit", native_logistic_fit);
    SAFE_REGISTER(env, "__nb_fit", native_nb_fit);
    SAFE_REGISTER(env, "__nb_predict", native_nb_predict);
    SAFE_REGISTER(env, "__kmeans_fit", native_kmeans_fit);
    SAFE_REGISTER(env, "__kmeans_predict", native_kmeans_predict);
    SAFE_REGISTER(env, "__kmeans_loss", native_kmeans_loss);

    SAFE_REGISTER(env, "__matrix_dot", native_matrix_dot);
    SAFE_REGISTER(env, "__matrix_add", native_matrix_add);
    SAFE_REGISTER(env, "__matrix_sub", native_matrix_sub);
    SAFE_REGISTER(env, "__matrix_det", native_matrix_det);
    SAFE_REGISTER(env, "__matrix_scalar", native_matrix_scalar_mul);
    SAFE_REGISTER(env, "native_transpose", native_transpose);

    SAFE_REGISTER(env, "__tensor_add", native_tensor_add);
    SAFE_REGISTER(env, "__tensor_mul", native_tensor_mul);
    SAFE_REGISTER(env, "__tensor_sub", native_tensor_sub);
    SAFE_REGISTER(env, "__tensor_shape", native_tensor_check_shape);
    SAFE_REGISTER(env, "__tensor_sum", native_tensor_sum);
    SAFE_REGISTER(env, "__tenso_mean", native_tensor_mean);
    SAFE_REGISTER(env, "__tensor_dot", native_tensor_dot);
    SAFE_REGISTER(env, "__vector_dot", native_vector_dot);
    SAFE_REGISTER(env, "__vector_num", native_vector_norm);

    SAFE_REGISTER(env, "__read_csv", native_read_csv);
    SAFE_REGISTER(env, "__read_csv_advance", native_load_csv_smart);
    SAFE_REGISTER(env, "__json_parse", builtin_json_parse);
    SAFE_REGISTER(env, "__json_encode", builtin_json_encode);
    SAFE_REGISTER(env, "__json_string", builtin_json_stringify);
    SAFE_REGISTER(env, "__save_jml", native_save_jml);
    SAFE_REGISTER(env, "__load_jml", native_load_jml);
    SAFE_REGISTER(env, "__jackal_sleep", builtin_jackal_sleep);
    SAFE_REGISTER(env, "plot", native_plot);
    SAFE_REGISTER(env, "_native_linear_predicts", native_linear_regression);
    SAFE_REGISTER(env, "native_standardnize", native_standardize);
    SAFE_REGISTER(env, "native_smooth", native_smooth);
    SAFE_REGISTER(env, "native_correlate", native_correlate);
    SAFE_REGISTER(env, "__zip", native_zip);
    SAFE_REGISTER(env, "__time_now", builtin_time_now);
    SAFE_REGISTER(env, "__get_local_hour", builtin_time_get_local_hour);

    SAFE_REGISTER(env, "__mapstream_stream", builtin_map_forEach);
    SAFE_REGISTER(env, "__mapstream_keys", builtin_map_keys);
    SAFE_REGISTER(env, "__mapstream_values", builtin_map_values);
    SAFE_REGISTER(env, "__map_get", builtin_map_get);

    SAFE_REGISTER(env, "len", builtin_len);
    SAFE_REGISTER(env, "push", builtin_push);
    SAFE_REGISTER(env, "pop", builtin_pop);
    SAFE_REGISTER(env, "remove", builtin_remove);
    SAFE_REGISTER(env, "File", builtin_file_open);

    register_socket_natives(env);
    register_string_natives(env);
    register_sys_natives(env);
    register_math_natives(env);
    register_io_natives(env);
    register_file_natives(env);
    register_http_natives(env);
    register_json_natives(env);
}