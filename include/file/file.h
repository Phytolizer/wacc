#pragma once

#include <str/str.h>
#include <sum/sum.h>

typedef RESULT(str, str) SlurpFileResult;

SlurpFileResult slurp_file(str filename);
