#include <stdio.h>
#include <math.h>
#include <xmp.h>

#pragma xmp nodes p[*]

int main(int argc, char *argv[]) {
    double pi = 0.0;
    double PI25DT = 3.141592653589793238462643;
    long n = 1000000;
    double h, x, sum;
    int server = 0;

#pragma xmp task on p[server]
    {
        printf("Number of nodes %d\n", xmp_num_nodes());

        if (argc <= 1) {
            printf("Default number of n = %d\n\n", n);
        } else {
            n = (int) atoi(argv[1]);
            printf("Passed number of n = %d\n\n", n);
        }
    }
#pragma bcast (n) from p[server]
#pragma xmp barrier

    h = 1.0 / (double) n;

#pragma xmp template t[n]
#pragma xmp distribute t[cyclic] onto p


#pragma xmp loop on t[i]
    for (int i = 0; i < n; i++) {
        x = h * ((double) i - 0.5);
        sum += (4.0 / (1.0 + x * x));
    }
    pi = h * sum;

    printf("[%d] Local PI is %.16f\n", xmpc_node_num(), pi);

#pragma xmp reduction(+:pi)

#pragma xmp task on p[server]
    {
        printf("PI is approximately %.16f, Error is %.16f\n", pi, fabs(pi - PI25DT));
    }

    return 0;
}