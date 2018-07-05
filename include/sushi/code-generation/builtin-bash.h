#ifndef SUSHI_BUILTIN_BASH_H_
#define SUSHI_BUILTIN_BASH_H_

namespace sushi {
namespace code_generation {

// Variable Definitions

// _sushi_unit_
constexpr char kSushiUnitDef[] = R"(declare -r _sushi_unit_='')";
// _sushi_func_ret_
constexpr char kSushiFuncRetDef[] = R"(declare _sushi_func_ret_=0)";
// _sushi_func_map_ret_
constexpr char kSushiFuncMapRetDef[] = "declare -A _sushi_func_map_ret_=()";

// Function Definitions

// _sushi_abs_
constexpr char kSushiAbsFuncDef[] =
    R"foo(_sushi_abs_() {
    local arg=$1
    local ret=0
    if [[ $arg -ge 0 ]]; then
        ret=$(($arg))
    else
        ret=$((-$arg))
    fi
    echo -ne $ret
})foo";

// _sushi_dup_str_
constexpr char kSushiDupStrFuncDef[] =
    R"foo(_sushi_dup_str_() {
    local str=$1
    local times=$2
    local i=0
    local ret=""
    for (( ; i < times; i++ )); do
        ret="${ret}${str}"
    done
    echo -ne $ret
})foo";

// _sushi_path_concat_
constexpr char kSushiPathConcatFuncDef[] =
    R"foo(_sushi_path_concat_() {
    echo -ne $1/$2
})foo";

// _sushi_file_eq_
constexpr char kSushiFileEqFuncDef[] =
    R"foo(_sushi_file_eq_() {
    if [[ $1 -ef $2 ]]; then
        echo -n '1'
        return 0
    else
        echo -n '0'
        return 1
    fi
})foo";

// _sushi_comp_array_
constexpr char kSushiCompArrayFuncDef[] =
    R"foo(_sushi_compare_array_() {
    local -n arr0=$1
    local -n arr1=$2
    if [[ ${#arr0[@]} -ne ${#arr1[@]} ]]; then
        echo -n 0
        return 1
    fi
    local n=${#arr0[@]}
    for ((i=0; i < n; i++)); do
        if [[ ${arr0[i]} -ne ${arr1[i]} ]]; then
        echo -n 0
        return 1
        fi
    done
    echo -n 1
    return 0
}
)foo";

} // namespace code_generation
} // namespace sushi

#endif
