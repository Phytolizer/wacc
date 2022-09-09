#include <wacc/run.h>

int main(int argc, char** argv)
{
    return run((WaccArgBuf)BUF_REF(argv, argc), stdout, stderr);
}
