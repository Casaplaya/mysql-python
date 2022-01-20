#include <pybind11/embed.h>
#include <pybind11/eval.h>
#include <string.h>
#include "udf_registration_types.h"

namespace py = pybind11;

py::scoped_interpreter guard{};

/***************************************************************************
** UDF py_eval function.
** Arguments:
** initid	Structure filled by xxx_init
** args		The same structure as to xxx_init. This structure
**		contains values for all parameters.
**		Note that the functions MUST check and convert all
**		to the type it wants!  Null values are represented by
**		a NULL pointer
** is_null	If the result is null, one should store 1 here.
** error	If something goes fatally wrong one should store 1 here.
**
** This function should return the result.
***************************************************************************/

// py_eval evaluates arbitrary python statements - VERY DANGEROUS!
extern "C" bool py_eval_init(UDF_INIT *initid, UDF_ARGS *args,
                             char *message)
{
    if (args->arg_count != 1)
    {
        strcpy(message, "py_eval must have one argument");
        return true;
    }
    args->arg_type[0] = STRING_RESULT;
    initid->maybe_null = true; /* The result may be null */
    return false;
}

extern "C" char *py_eval(UDF_INIT *, UDF_ARGS *args, char *result,
                         unsigned long *res_length, unsigned char *null_value,
                         unsigned char *)
{
    if (args->args[0] == nullptr) {
        *null_value = 1;
        return result;
    }
    auto obj = py::eval(args->args[0]);
    std::string res_str = std::string(py::str(obj));
    strcpy(result, res_str.c_str());
    *res_length = res_str.length();
    return result;
}

// Convert hex strings to decimal strings
extern "C" bool py_hex_to_dec_init(UDF_INIT *initid, UDF_ARGS *args,
                             char *message)
{
    if (args->arg_count != 1) {
        strcpy(message, "py_hex_to_dec requires exactly one string arg");
        return 1;
    }
    args->arg_type[0] = STRING_RESULT;
    initid->maybe_null = true; /* The result may be null */
    return 0;
}

extern "C" char *py_hex_to_dec(UDF_INIT *, UDF_ARGS *args, char *result,
                         unsigned long *res_length, unsigned char *null_value,
                         unsigned char *)
{
    if (args->args[0] == nullptr) {
        *null_value = 1;
        return result;
    }
    auto obj = py::eval(std::string("int('") + std::string(args->args[0]) + std::string("', 16)"));
    std::string res_str = std::string(py::str(obj));
    strcpy(result, res_str.c_str());
    *res_length = res_str.length();
    return result;
}

// Convert hex string to a decimal string, divided by a given number of decimals
extern "C" bool py_hex_with_dec_init(UDF_INIT *initid, UDF_ARGS *args,
                             char *message)
{
    if (args->arg_count != 2) {
        strcpy(message, "py_hex_with_dec requires one string arg and an int arg");
        return 1;
    }
    args->arg_type[0] = STRING_RESULT;
    initid->maybe_null = true; /* The result may be null */
    return 0;
}

extern "C" char *py_hex_with_dec(UDF_INIT *, UDF_ARGS *args, char *result,
                         unsigned long *res_length, unsigned char *null_value,
                         unsigned char *)
{
    if (args->args[0] == nullptr || args->args[1] == nullptr) {
        *null_value = 1;
        return result;
    }
    auto obj = py::eval(
        std::string("int('") +
        std::string(args->args[0]) +
        std::string("', 16) / 10**") +
        std::to_string(*((long long*)args->args[1]))
    );
    std::string res_str = std::string(py::str(obj));
    strcpy(result, res_str.c_str());
    *res_length = res_str.length();
    return result;
}

// Aggregated sums of large BINARY numbers as hex strings
struct sum_data {
    std::string running_total;
};

extern "C" bool py_hex_sum_init(UDF_INIT *initid, UDF_ARGS *args,
                             char *message)
{
    struct sum_data* data;
    if (args->arg_count != 1) {
        strcpy(message, "py_hex_sum requires one string arg");
        return 1;
    }
    args->arg_type[0] = STRING_RESULT;
    initid->maybe_null = true; /* The result may be null */
    data = new struct sum_data;
    data->running_total = std::to_string(0);
    initid->ptr = (char*)data;
    return 0;
}

extern "C" void py_hex_sum_deinit(UDF_INIT* initid) {
    delete initid->ptr;
}

extern "C" void py_hex_sum_add(UDF_INIT* initid, UDF_ARGS* args, char* is_null,
                                char* message) {
    if (args->args[0] == nullptr)
        return;

    struct sum_data* data = (struct sum_data*)initid->ptr;
    data->running_total = std::string(py::str(py::eval(
        data->running_total + std::string(" + int('") +
        std::string(args->args[0]) + std::string("', 16)")
    )));
}

extern "C" void py_hex_sum_clear(UDF_INIT* initid, char* is_null, char* message) {
    struct sum_data* data = (struct sum_data*)initid->ptr;
    data->running_total = std::to_string(0);
}

extern "C" char *py_hex_sum(UDF_INIT *initid, UDF_ARGS *, char *result,
                         unsigned long *res_length, unsigned char *null_value,
                         unsigned char *)
{
    struct sum_data* data = (struct sum_data*)initid->ptr;
    strcpy(result, data->running_total.c_str());
    *res_length = strlen(data->running_total.c_str());
    return result;
}

// Aggregated sums of large BINARY numbers as hex strings, divided by decimals
struct sum_data_dec {
    std::string running_total;
    int decimals;
};

extern "C" bool py_hex_sum_with_dec_init(UDF_INIT *initid, UDF_ARGS *args,
                             char *message)
{
    struct sum_data_dec* data;
    if (args->arg_count != 2) {
        strcpy(message, "py_hex_sum_with_dec requires one string arg and an integer arg");
        return 1;
    }
    args->arg_type[0] = STRING_RESULT;
    initid->maybe_null = true; /* The result may be null */
    data = new struct sum_data_dec;
    data->running_total = std::to_string(0);
    initid->ptr = (char*)data;
    return 0;
}

extern "C" void py_hex_sum_with_dec_deinit(UDF_INIT* initid) {
    delete initid->ptr;
}

extern "C" void py_hex_sum_with_dec_add(UDF_INIT* initid, UDF_ARGS* args,
                                        char* is_null, char* message) {
    if (args->args[0] == nullptr || args->args[1] == nullptr)
        return;

    struct sum_data_dec* data = (struct sum_data_dec*)initid->ptr;
    data->decimals = *((long long*)args->args[1]);
    data->running_total = std::string(py::str(py::eval(
        data->running_total + std::string(" + int('") +
        std::string(args->args[0]) + std::string("', 16)")
    )));
}

extern "C" void py_hex_sum_with_dec_clear(UDF_INIT* initid, char* is_null,
                                            char* message) {
    struct sum_data_dec* data = (struct sum_data_dec*)initid->ptr;
    data->running_total = std::to_string(0);
}

extern "C" char *py_hex_sum_with_dec(UDF_INIT *initid, UDF_ARGS *, char *result,
                         unsigned long *res_length, unsigned char *null_value,
                         unsigned char *)
{
    struct sum_data_dec* data = (struct sum_data_dec*)initid->ptr;
    auto obj = py::eval(
        std::string("int('") +
        data->running_total +
        std::string("') / 10**") +
        std::to_string(data->decimals)
    );
    std::string res_str = std::string(py::str(obj));
    strcpy(result, res_str.c_str());
    *res_length = res_str.length();
    return result;
}
