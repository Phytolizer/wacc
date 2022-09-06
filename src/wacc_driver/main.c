#include "wacc/run.h"

int main(int argc, char** argv)
{
    return run((Argv)BUF_REF(argv, argc), stdout, stderr);
}
