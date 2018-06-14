#include <stdio.h>
#include <upc.h>
#define f(x) (1.0/(1.0 + x*x))

shared double pi = 0.0;
upc_lock_t *l;

int main( int argc, char* argv[] ) {
  int trials = 1000000;
  double local_pi = 0.0;
  l = upc_all_lock_alloc();
  if(MYTHREAD == 0)
  {
    if( argc < 2 )
  	{
      printf("Usage: %s <num_of_trials>\n", argv[0]);
      printf("Default number of trials has been used (%d)\n", trials);
  	} else {
      trials = (int)atoi( argv[ 1 ] );
    }
  }
  
  upc_forall(int i = 0; i < trials; ++i; i)
    local_pi += (double) f( (0.5 + i) / (trials) );
  local_pi *= (double)(4.0 / trials);
  
  upc_lock(l);
  pi += local_pi;
  upc_unlock(l);
  upc_barrier;
  
  if(MYTHREAD == 0){
    printf("PI is approx. %5.20lf\n", pi);
    upc_lock_free(l);
  }
}