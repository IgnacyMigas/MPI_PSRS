/*
 * XcalableMP parallel directives test
   - modified for WFiIS Stud Lab by PG 01/06/2017
   
   - config path:
     $ source /opt/nfs/config/source_omni130.sh

   - compilation example (requires CUDA SDK):
     $ xmpcc -I/opt/nfs/cuda/include -L/opt/nfs/cuda/lib64 \
       xmp-hello.c -o xmp-hello -lcudart

   - execution example (requires cluster with CUDA-enabled nodes):
     $ mpiexec -n 4 -f nodes -errfile-pattern /dev/null ./xmp-hello \
       | egrep -v '(context|handle)'
       
 *
 */
#include <stdio.h>
#include <xmp.h>
#pragma xmp nodes p[*]

int main()
{
  printf("node num = %d, Hello World\n", xmpc_node_num());
  return 0;
}

