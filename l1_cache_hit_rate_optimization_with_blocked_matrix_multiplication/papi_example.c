#include<stdio.h>
#include<stdlib.h>
#include<papi.h>
#include<string.h>

void handle_error(int errcode);

int main(int argc, char **argv)
{
  double *a,*b,*c;                /* daxpy arrays */
  double alpha = 1.2;             /* daxpy const */
  int  i, count, n, stride;       /* looping vars */
  int ret,iter,niter;
  double d = 0.0;
  int j=0;
  if (argc != 4){
    printf("usage: ./daxpy n stride niter\n");
    abort();
  }

  n = atoi(argv[1]);                        /* vector length */
  stride = atoi(argv[2]);                   /* loop stride */
  niter = atoi(argv[3]);
  
  a = (double *) malloc(n*sizeof(double));  /* daxpy vectors */
  b = (double *) malloc(n*sizeof(double));
  c = (double *) malloc(n*sizeof(double));
 
  for (i=0;i<n;++i){
    a[i] = -1;
    b[i] = 1;
  }


  /* Start counting events */
  ret = PAPI_hl_region_begin("daxpy");
  if ( ret != PAPI_OK )
    handle_error(1);


  /* daxpy with arbitrary stride */
  for (iter=0;iter<niter;++iter){
    for (count=0,i=0;i<n;i+=stride,++count){
      c[i] = a[i] + alpha*b[i];
    }
  }


  ret = PAPI_hl_region_end("daxpy");
  if ( ret != PAPI_OK )
    handle_error(ret);


  double sum = 0;
  for (i=0;i<n;++i)
    sum += c[i];

  printf("%f\n", sum);
  return 0;
}


void handle_error (int retval){
     printf("PAPI error %d: %s\n", retval, PAPI_strerror(retval));
     exit(1);
}
