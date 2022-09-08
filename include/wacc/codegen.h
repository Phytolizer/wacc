#pragma once

#include "wacc/ast.h"

#include <hedley.h>
#include <stdio.h>
#include <sum/sum.h>

typedef MAYBE(str) CodegenError;

HEDLEY_WARN_UNUSED_RESULT CodegenError codegen_program(WaccProgram program, FILE* fp);
