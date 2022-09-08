#pragma once

#include "wacc/ast.h"

#include <stdio.h>
#include <str/str.h>

int mkexe(WaccProgram program, str exe_output, FILE* err);
