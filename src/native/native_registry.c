#include "native/native_registry.h"

#define REGISTER(env, name, func) set_var(env, name, (Value){VAL_NATIVE, {.native = func}}, true, "")

#define DEFINE_NATIVE(env, name_str, func_ptr) set_var(env, name_str, (Value){VAL_NATIVE, {.native = func_ptr}}, true, "")

void register_all_natives(Env* env) {
    set_var(env, "nil", (Value){VAL_NIL, {0}}, true, "");
    set_var(env, "typeof", (Value){VAL_NATIVE, {.native = builtin_typeof}}, true, "");
    set_var(env, "__math_PI", (Value){VAL_NUMBER, {.number = 3.1415926535}}, true, "");

    REGISTER(env, "__math_sqrt", builtin_math_sqrt);
    REGISTER(env, "__math_pow", builtin_math_pow);
    REGISTER(env, "fmod", builtin_math_fmod);
    REGISTER(env, "achos", native_math_acos);

    REGISTER(env, "__io_open", builtin_io_open);
    REGISTER(env, "__io_readAll", builtin_io_readAll);
    REGISTER(env, "__io_write", builtin_io_write);
    REGISTER(env, "__io_close", builtin_io_close);
    REGISTER(env, "println", builtin_writeline);
    REGISTER(env, "print", builtin_write);
    REGISTER(env, "__io_read_line", builtin_read_line);
    REGISTER(env, "__io_read_array", builtin_read_array);
    REGISTER(env, "__io_table_stream", builtin_print_table);
    REGISTER(env, "__io_json", builtin_print_json);
    REGISTER(env, "_io_plot", builtin_plot);

    REGISTER(env, "__array_distinct", builtin_array_distinct);
    REGISTER(env, "__array_anyMatch", builtin_array_anyMatch);
    REGISTER(env, "__array_map", builtin_array_map);
    REGISTER(env, "__array_filter", builtin_array_filter);
    REGISTER(env, "__array_reduce", builtin_array_reduce);
    REGISTER(env, "__array_sort", builtin_array_sort);
    REGISTER(env, "__array_statistics", builtin_array_statistics);
    REGISTER(env, "__array_mean", builtin_array_mean);
    REGISTER(env, "__array_max", builtin_array_max);
    REGISTER(env, "__array_to_tree", builtin_array_to_tree);
    REGISTER(env, "__array_limit", builtin_array_limit);

    REGISTER(env, "__knn", native_knn_nd);
    REGISTER(env, "__accuracy", native_accuracy);
    REGISTER(env, "__normalize", native_normalize_nd);
    REGISTER(env, "__knn_predictprob", native_knn_prob);
    REGISTER(env, "__confusion_matrix", native_confusion_matrix);
    REGISTER(env, "__split", native_split);
    REGISTER(env, "__sync_shuffle", native_sync_shuffle);
    REGISTER(env, "__logistic_predict", native_logistic_predict);
    REGISTER(env, "__logistic_fit", native_logistic_fit);
    REGISTER(env, "__nb_fit", native_nb_fit);
    REGISTER(env, "__nb_predict", native_nb_predict);
    REGISTER(env, "__kmeans_fit", native_kmeans_fit);
    REGISTER(env, "__kmeans_predict", native_kmeans_predict);
    REGISTER(env, "__kmeans_loss", native_kmeans_loss);

    REGISTER(env, "__matrix_dot", native_matrix_dot);
    REGISTER(env, "__matrix_add", native_matrix_add);
    REGISTER(env, "__matrix_sub", native_matrix_sub);
    REGISTER(env, "__matrix_det", native_matrix_det);
    REGISTER(env, "__matrix_scalar", native_matrix_scalar_mul);
    REGISTER(env, "native_transpose", native_transpose);

    REGISTER(env, "__tensor_add", native_tensor_add);
    REGISTER(env, "__tensor_mul", native_tensor_mul);
    REGISTER(env, "__tensor_sub", native_tensor_sub);
    REGISTER(env, "__tensor_shape", native_tensor_check_shape);
    REGISTER(env, "__tensor_sum", native_tensor_sum);
    REGISTER(env, "__tenso_mean", native_tensor_mean);
    REGISTER(env, "__tensor_dot", native_tensor_dot);
    REGISTER(env, "__vector_dot", native_vector_dot);
    REGISTER(env, "__vector_num", native_vector_norm);

    REGISTER(env, "__read_csv", native_read_csv);
    REGISTER(env, "__read_csv_advance", native_load_csv_smart);
    REGISTER(env, "http_request", builtin_http_request);
    REGISTER(env, "http_serve_internal", builtin_http_serve);
    REGISTER(env, "__json_parse", builtin_json_parse);
    REGISTER(env, "__json_encode", builtin_json_encode);
    REGISTER(env, "__json_string", builtin_json_stringify);
    REGISTER(env, "web_show_internal", builtin_web_show);
    REGISTER(env, "web_sync_internal", builtin_web_sync);
    REGISTER(env, "__save_jml", native_save_jml);
    REGISTER(env, "__load_jml", native_load_jml);
    REGISTER(env, "__jackal_sleep", builtin_jackal_sleep);
    REGISTER(env, "plot", native_plot);
    REGISTER(env, "_native_linear_predicts", native_linear_regression);
    REGISTER(env, "native_standardnize", native_standardize);
    REGISTER(env, "native_smooth", native_smooth);
    REGISTER(env, "native_correlate", native_correlate);
    REGISTER(env, "__zip", native_zip);
    REGISTER(env, "__time_now", builtin_time_now);
    REGISTER(env, "__get_local_hour", builtin_time_get_local_hour);
    REGISTER(env, "systems", builtin_system);
    REGISTER(env, "__typeof", builtin_type);

    REGISTER(env, "__mapstream_stream", builtin_map_forEach);
    REGISTER(env, "__mapstream_keys", builtin_map_keys);
    REGISTER(env, "__mapstream_values", builtin_map_values);
    REGISTER(env, "__map_get", builtin_map_get);

    DEFINE_NATIVE(env, "len", builtin_len);
    DEFINE_NATIVE(env, "push", builtin_push);
    DEFINE_NATIVE(env, "pop", builtin_pop);
    DEFINE_NATIVE(env, "remove", builtin_remove);
    DEFINE_NATIVE(env, "File", builtin_file_open);

}