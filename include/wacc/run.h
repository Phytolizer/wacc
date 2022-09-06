#pragma once

#include <buf/buf.h>
#include <stdio.h>

typedef BUF(char*) Argv;

int run(Argv argv, FILE* out, FILE* err);
