//
// Created by Alvin Xue on 2018/4/18.
//

#include "sushi/pipeline.h"
#include <iostream>
using namespace sushi::pipeline;

int main(int argc, const char *argv[]) {
    Pipeline pipeline(argc, argv);
    pipeline.Main();
}
