#include<stdio.h>
#include<stdlib.h>
#include<papi.h>
#include<sys/time.h>

#define N 1000

double x[N][N];
double y[N][N];
double z[N][N];

double timer(void){
  struct timeval time;
  gettimeofday(&time, 0);
  return time.tv_sec + time.tv_usec/1000000.0;
}

void handle_error(int errcode);

int main(int argc, char **argv){
  int i,j,k;
  double r;
  int ret;
  double ts,tf;


  
  /* initialize y, z */
  for (i=0;i<N;++i){
    for (j=0;j<N;++j){
      y[i][j] = 1;
      z[i][j] = 1;
    }
  }

  ret = PAPI_library_init(PAPI_VER_CURRENT);
  if (ret < 0)
      handle_error(ret);

  /* Start counting events */
  ret = PAPI_hl_region_begin("mxm");
  if ( ret != PAPI_OK )
      handle_error(1);

  ts = timer();
  for (i=0;i<N;++i){
    for (j=0;j<N;++j){
      r = 0;
      for (k=0;k<N;++k)
	r += y[i][k]*z[k][j];
      x[i][j] = r;
    }
  }
  tf = timer();

  ret = PAPI_hl_region_end("mxm");
  if ( ret != PAPI_OK )
    handle_error(ret);

  printf("%f %f(s)\n", x[0][0],tf-ts);
}

void handle_error (int retval){
     printf("PAPI error %d: %s\n", retval, PAPI_strerror(retval));
     exit(1);
}
