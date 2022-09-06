#pragma once

#include <dparse.h>

int wacc_scan(d_loc_t* loc, unsigned short* symbol, int* term_priority, unsigned char* op_assoc, int* op_priority);
