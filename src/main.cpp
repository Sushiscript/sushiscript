//
// Created by Alvin Xue on 2018/4/18.
//
#include <iostream>
#include <cstdlib>
#include <string>

const std::string bash_str = 
    "#/bin/bash\n"
    "echo \"helloworld\"";

int main() {
    int result = system(bash_str.c_str());
    exit(result);
}
