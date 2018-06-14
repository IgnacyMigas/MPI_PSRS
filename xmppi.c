#include <stdio.h>
#include <xmp.h>

#define f(x) (1.0/(1.0 + x*x))

#pragma xmp nodes p[*]

int server = 0;

int main(int argc, char *argv[]) {
    double local_pi = 0.0;

    int trials = 1000000

#pragma xmp task on p[server]
    {
        if (argc < 2) {
            printf("Usage: %s <num_of_trials>\n", argv[0]);
            printf("Default number of trials has been used (%d)\n", trials);
        } else {
            trials = (int) atoi(argv[1]);
        }
    }

    for (int i = 0; i < trials; i++) {
        local_pi += (double) f((0.5 + i) / (trials));
    }
    local_pi *= (double) (4.0 / trials);

#pragma xmp reduction(+:local_pi)

#pragma xmp barrier

#pragma xmp task on p[server]
    {
        printf("PI is approx. %5.20lf\n", pi);
        upc_lock_free(l);
    }
}