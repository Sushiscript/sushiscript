#ifndef SUSHI_BUILTIN_BASH_H_
#define SUSHI_BUILTIN_BASH_H_

namespace sushi {
namespace code_generation {

// Variable Definitions

// _sushi_unit_
constexpr char kSushiUnitDef[] = R"(declare -r _sushi_unit_=0)";
// _sushi_func_ret_
constexpr char kSushiFuncRetDef[] = R"(declare _sushi_func_ret_=0)";


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

} // namespace code_generation
} // namespace sushi

#endif
