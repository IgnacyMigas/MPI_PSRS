#include <stdio.h>
#include <xmp.h>

#define f(x) (1.0/(1.0 + x*x))

#pragma xmp nodes p[*]

int server = 0;

int main(int argc, char *argv[]) {
    double pi = 0.0;
    int trials = 1000000;

    int n;
    double h, x, sum;
    n = xmp_num_nodes();
    h = 1.0 / (double)n;


#pragma xmp task on p[server]
    {
        printf("Number of nodes %d\n", n);

        if (argc < 2) {
            printf("Usage: %s <num_of_trials>\n", argv[0]);
            printf("Default number of trials has been used (%d)\n\n", trials);
        } else {
            trials = (int) atoi(argv[1]);
        }
    }

    for (int i = 0; i < trials; i++) {
        x = h * ((double) i - 0.5);
        sum += (4.0 / (1.0 + x * x));
    }
    pi = h * sum;

    printf("[%d] local PI is %5.20lf\n", xmpc_node_num(), pi);

#pragma xmp reduction(+:pi)

#pragma xmp task on p[server]
    {
        printf("\nPI is approx. %5.20lf\n", pi);
    }

    return 0;
}