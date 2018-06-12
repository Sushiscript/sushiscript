#ifndef SUSHI_BUILTIN_BASH_H_
#define SUSHI_BUILTIN_BASH_H_

namespace sushi {

// Variable Definitions

// _sushi_unit_
constexpr char kSushiUnitDef[] = R"(declare -r _sushi_unit_=0)";
// _sushi_func_ret_
constexpr char kSushiFuncRetDef[] = R"(declare _sushi_func_ret_=0)";


// Function Definitions

// _sushi_extract_map_
constexpr char kSushiExtractMapFuncDef[] =
R"foo(_sushi_extract_map_() {
    local args=($@)
    local args_len=$#
    local ret=""
    local half=$(($args_len / 2))
    local i=0;
    for (( ; i < half; i++ )); do
        ret="${ret}[${args[i]}]=${args[$((i + $half))]} "
    done
    echo -ne $ret
})foo";

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

};

#endif
