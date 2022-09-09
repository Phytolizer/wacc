#pragma once

#include <buf/buf.h>
#include <stdio.h>

typedef BUF(char*) WaccArgBuf;

int run(WaccArgBuf args, FILE* out, FILE* err);
