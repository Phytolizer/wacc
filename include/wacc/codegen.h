#pragma once

#include "wacc/ast.h"

#include <stdio.h>

void codegen_program(WaccProgram program, FILE* fp);
