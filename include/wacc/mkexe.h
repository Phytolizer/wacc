#pragma once

#include "wacc/ast.h"

#include <stdio.h>
#include <str/str.h>

void mkexe(WaccProgram program, str exe_output, FILE* err);
