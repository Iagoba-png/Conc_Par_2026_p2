#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "options.h"


int main (int argc, char **argv)
{
    struct options opt;

    // Default values for the options
    opt.barbers = 5;
    opt.customers = 1000;
    opt.cut_time  = 3000;

    read_options(argc, argv, &opt);

    exit (0);
}
