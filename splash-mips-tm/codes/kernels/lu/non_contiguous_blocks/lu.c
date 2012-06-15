
/*************************************************************************/
/*                                                                       */
/*  Copyright (c) 1994 Stanford University                               */
/*                                                                       */
/*  All rights reserved.                                                 */
/*                                                                       */
/*  Permission is given to use, copy, and modify this software for any   */
/*  non-commercial purpose as long as this copyright notice is not       */
/*  removed.  All other uses, including redistribution in whole or in    */
/*  part, are forbidden without prior written permission.                */
/*                                                                       */
/*  This software is provided with absolutely no warranty and no         */
/*  support.                                                             */
/*                                                                       */
/*************************************************************************/

/*************************************************************************/
/*                                                                       */
/*  Parallel dense blocked LU factorization (no pivoting)                */
/*                                                                       */
/*  This version contains one dimensional arrays in which the matrix     */
/*  to be factored is stored.                                            */
/*                                                                       */
/*  Command line options:                                                */
/*                                                                       */
/*  -nN : Decompose NxN matrix.                                          */
/*  -pP : P = number of processors.                                      */
/*  -bB : Use a block size of B. BxB elements should fit in cache for    */
/*        good performance. Small block sizes (B=8, B=16) work well.     */
/*  -s  : Print individual processor timing statistics.                  */
/*  -t  : Test output.                                                   */
/*  -o  : Print out matrix values.                                       */
/*  -h  : Print out command line options.                                */
/*                                                                       */
/*  Note: This version works under both the FORK and SPROC models        */
/*                                                                       */
/*************************************************************************/

#include <stdio.h>
#include <math.h>
#include <stdlib.h>
 
	#include <fcntl.h>
	#include <sys/time.h>

	#include "sescapi.h"

        struct timeval sc_tp;


#define MAXRAND                         32767.0
#define DEFAULT_N                         128
#define DEFAULT_P                           1
#define DEFAULT_B                          16
#define min(a,b) ((a) < (b) ? (a) : (b))

struct GlobalMemory {
  double *t_in_fac;
  double *t_in_solve;
  double *t_in_mod;
  double *t_in_bar;
  double *completion;
  unsigned int starttime;
  unsigned int rf;
  unsigned int rs;
  unsigned int done;
  int id;
  sbarrier_t (start);
  slock_t (idlock);
} *Global;

struct LocalCopies {
  double t_in_fac;
  double t_in_solve;
  double t_in_mod;
  double t_in_bar;
};

int n = DEFAULT_N;          /* The size of the matrix */
int P = DEFAULT_P;          /* Number of processors */
int block_size = DEFAULT_B; /* Block dimension */
int nblocks;                /* Number of blocks in each dimension */
int num_rows;               /* Number of processors per row of processor grid */
int num_cols;               /* Number of processors per col of processor grid */
double *a;                  /* a = lu; l and u both placed back in a */
double *rhs;
int *proc_bytes;            /* Bytes to malloc per processor to hold blocks
                               of A*/
int test_result = 0;        /* Test result of factorization? */
int doprint = 0;            /* Print out matrix values? */
int dostats = 0;            /* Print out individual processor statistics? */

void SlaveStart();
void OneSolve(int, int, double *, int, int);
void lu0(double *,int, int);
void bdiv(double *, double *, int, int, int, int);
void bmodd(double *, double*, int, int, int, int);
void bmod(double *, double *, double *, int, int, int, int);
void daxpy(double *, double *, int, double);
int BlockOwner(int, int);
void lu(int, int, int, struct LocalCopies *, int);
void InitA(double *);
double TouchA(int, int);
void PrintA();
void CheckResult(int, double *, double *);
void printerr(char *);


main(argc, argv)

int argc;
char *argv[];

{
  int i, j;
  int ch;
  extern char *optarg;
  int MyNum=0;
  double mint, maxt, avgt;
  double min_fac, min_solve, min_mod, min_bar;
  double max_fac, max_solve, max_mod, max_bar;
  double avg_fac, avg_solve, avg_mod, avg_bar;
  int proc_num;
  unsigned int start;

  {
  if (gettimeofday (&sc_tp, (struct timezone *)0))
	{
    perror (""); 
		fprintf (stderr, "error in clock macro\n"); 
		exit (-1);
	}
  start = (sc_tp.tv_sec & 0x7ff)*1000000 + sc_tp.tv_usec;
};

  while ((ch = getopt(argc, argv, "n:p:b:cstoh")) != -1) {
    switch(ch) {
    case 'n': n = atoi(optarg); break;
    case 'p': P = atoi(optarg); break;
    case 'b': block_size = atoi(optarg); break;
    case 's': dostats = 1; break;
    case 't': test_result = !test_result; break;
    case 'o': doprint = !doprint; break;
    case 'h': printf("Usage: LU <options>\n\n");
              printf("options:\n");
              printf("  -nN : Decompose NxN matrix.\n");
              printf("  -pP : P = number of processors.\n");
              printf("  -bB : Use a block size of B. BxB elements should fit in cache for \n");
              printf("        good performance. Small block sizes (B=8, B=16) work well.\n");
              printf("  -c  : Copy non-locally allocated blocks to local memory before use.\n");
              printf("  -s  : Print individual processor timing statistics.\n");
              printf("  -t  : Test output.\n");
              printf("  -o  : Print out matrix values.\n");
              printf("  -h  : Print out command line options.\n\n");
              printf("Default: LU -n%1d -p%1d -b%1d\n",
                     DEFAULT_N,DEFAULT_P,DEFAULT_B);
              exit(0);
              break;
    }
  }

  {
	sesc_init();
}

  printf("\n");
  printf("Blocked Dense LU Factorization\n");
  printf("     %d by %d Matrix\n",n,n);
  printf("     %d Processors\n",P);
  printf("     %d by %d Element Blocks\n",block_size,block_size);
  printf("\n");
  printf("\n");

  num_rows = (int) sqrt((double) P);
  for (;;) {
    num_cols = P/num_rows;
    if (num_rows*num_cols == P)
      break;
    num_rows--;
  }
  nblocks = n/block_size;
  if (block_size * nblocks != n) {
    nblocks++;
  }

  a = (double *) malloc(n*n*sizeof(double));;
  rhs = (double *) malloc(n*sizeof(double));;

  Global = (struct GlobalMemory *) malloc(sizeof(struct GlobalMemory));;
  Global->t_in_fac = (double *) malloc(P*sizeof(double));;
  Global->t_in_mod = (double *) malloc(P*sizeof(double));;
  Global->t_in_solve = (double *) malloc(P*sizeof(double));;
  Global->t_in_bar = (double *) malloc(P*sizeof(double));;
  Global->completion = (double *) malloc(P*sizeof(double));;

  if (Global == NULL) {
    printerr("Could not malloc memory for Global\n");
    exit(-1);
  } else if (Global->t_in_fac == NULL) {
    printerr("Could not malloc memory for Global->t_in_fac\n");
    exit(-1);
  } else if (Global->t_in_mod == NULL) {
    printerr("Could not malloc memory for Global->t_in_mod\n");
    exit(-1);
  } else if (Global->t_in_solve == NULL) {
    printerr("Could not malloc memory for Global->t_in_solve\n");
    exit(-1);
  } else if (Global->t_in_bar == NULL) {
    printerr("Could not malloc memory for Global->t_in_bar\n");
    exit(-1);
  } else if (Global->completion == NULL) {
    printerr("Could not malloc memory for Global->completion\n");
    exit(-1);
  }

/* POSSIBLE ENHANCEMENT:  Here is where one might distribute the a
   matrix data across physically distributed memories in a 
   round-robin fashion as desired. */

  sesc_barrier_init(&(Global->start));
;
  sesc_lock_init(&(Global->idlock));
;
  Global->id = 0;

  for (i=1; i<P; i++) {
    
	sesc_spawn(SlaveStart, NULL, 0);

  }

  InitA(rhs);
  if (doprint) {
    printf("Matrix before decomposition:\n");
    PrintA();
  }


  SlaveStart(MyNum);

  {
	int i;
	for (i = 0; i < P-1; i++)
		sesc_wait();
}

  if (doprint) {
    printf("\nMatrix after decomposition:\n");
    PrintA();
  }

  if (dostats) {
    maxt = avgt = mint = Global->completion[0];
    for (i=1; i<P; i++) {
      if (Global->completion[i] > maxt) {
        maxt = Global->completion[i];
      }
      if (Global->completion[i] < mint) {
        mint = Global->completion[i];
      }
      avgt += Global->completion[i];
    }
    avgt = avgt / P;

    min_fac = max_fac = avg_fac = Global->t_in_fac[0];
    min_solve = max_solve = avg_solve = Global->t_in_solve[0];
    min_mod = max_mod = avg_mod = Global->t_in_mod[0];
    min_bar = max_bar = avg_bar = Global->t_in_bar[0];

    for (i=1; i<P; i++) {
      if (Global->t_in_fac[i] > max_fac) {
        max_fac = Global->t_in_fac[i];
      }
      if (Global->t_in_fac[i] < min_fac) {
        min_fac = Global->t_in_fac[i];
      }
      if (Global->t_in_solve[i] > max_solve) {
        max_solve = Global->t_in_solve[i];
      }
      if (Global->t_in_solve[i] < min_solve) {
        min_solve = Global->t_in_solve[i];
      }
      if (Global->t_in_mod[i] > max_mod) {
        max_mod = Global->t_in_mod[i];
      }
      if (Global->t_in_mod[i] < min_mod) {
        min_mod = Global->t_in_mod[i];
      }
      if (Global->t_in_bar[i] > max_bar) {
        max_bar = Global->t_in_bar[i];
      }
      if (Global->t_in_bar[i] < min_bar) {
        min_bar = Global->t_in_bar[i];
      }
      avg_fac += Global->t_in_fac[i];
      avg_solve += Global->t_in_solve[i];
      avg_mod += Global->t_in_mod[i];
      avg_bar += Global->t_in_bar[i];
    }
    avg_fac = avg_fac/P;
    avg_solve = avg_solve/P;
    avg_mod = avg_mod/P;
    avg_bar = avg_bar/P;
  }
  printf("                            PROCESS STATISTICS\n");
  printf("              Total      Diagonal     Perimeter      Interior       Barrier\n");
  printf(" Proc         Time         Time         Time           Time          Time\n");
  printf("    0    %10.0f    %10.0f    %10.0f    %10.0f    %10.0f\n",
          Global->completion[0],Global->t_in_fac[0],
          Global->t_in_solve[0],Global->t_in_mod[0],
          Global->t_in_bar[0]);
  if (dostats) {
    for (i=1; i<P; i++) {
      printf("  %3d    %10.0f    %10.0f    %10.0f    %10.0f    %10.0f\n",
              i,Global->completion[i],Global->t_in_fac[i],
              Global->t_in_solve[i],Global->t_in_mod[i],
              Global->t_in_bar[i]);
    }
    printf("  Avg    %10.0f    %10.0f    %10.0f    %10.0f    %10.0f\n",
           avgt,avg_fac,avg_solve,avg_mod,avg_bar);
    printf("  Min    %10.0f    %10.0f    %10.0f    %10.0f    %10.0f\n",
           mint,min_fac,min_solve,min_mod,min_bar);
    printf("  Max    %10.0f    %10.0f    %10.0f    %10.0f    %10.0f\n",
           maxt,max_fac,max_solve,max_mod,max_bar);
  }
  printf("\n");
  Global->starttime = start;
  printf("                            TIMING INFORMATION\n");
  printf("Start time                        : %16d\n",
          Global->starttime);
  printf("Initialization finish time        : %16d\n",
          Global->rs);
  printf("Overall finish time               : %16d\n",
          Global->rf);
  printf("Total time with initialization    : %16d\n",
          Global->rf-Global->starttime);
  printf("Total time without initialization : %16d\n",
          Global->rf-Global->rs);
  printf("\n");

  if (test_result) {
    printf("                             TESTING RESULTS\n");
    CheckResult(n, a, rhs);
  }

  {exit(0);};
}

void SlaveStart()

{
  int i;
  int j;
  int cluster;
  int max_block;
  int MyNum;

  {
#ifdef ATOMIC
        sesc_start_transaction();
#else
#ifdef SIMICS
       SIMICS_START_IGNORE();
       SIMICS_BEGIN_LOCK(&(Global->idlock));
#endif
	sesc_lock(&(Global->idlock));

#ifdef SIMICS
       SIMICS_LOCKED(&(Global->idlock));
       SIMICS_STOP_IGNORE();
#endif
#endif
}
    MyNum = Global->id;
    Global->id ++;
  {
#ifdef ATOMIC
        sesc_commit_transaction();
#else
#ifdef SIMICS
       SIMICS_START_IGNORE();
#endif
	sesc_unlock(&(Global->idlock));

#ifdef SIMICS
       SIMICS_UNLOCK(&(Global->idlock));
       SIMICS_STOP_IGNORE();
#endif

#endif
}

/* POSSIBLE ENHANCEMENT:  Here is where one might pin processes to
   processors to avoid migration */

  OneSolve(n, block_size, a, MyNum, dostats);
}


void OneSolve(n, block_size, a, MyNum, dostats)

double *a;
int n;
int block_size;
int MyNum;
int dostats;

{
  unsigned int i;
  unsigned int myrs;
  unsigned int myrf;
  unsigned int mydone;
  struct LocalCopies *lc;

  lc = (struct LocalCopies *) malloc(sizeof(struct LocalCopies));
  if (lc == NULL) {
    fprintf(stderr,"Proc %d could not malloc memory for lc\n",MyNum);
    exit(-1);
  }
  lc->t_in_fac = 0.0;
  lc->t_in_solve = 0.0;
  lc->t_in_mod = 0.0;
  lc->t_in_bar = 0.0;

  /* barrier to ensure all initialization is done */
  {
#ifdef SIMICS
       SIMICS_ENTER_BARRIER(&(Global->start), P);
#endif
	sesc_barrier(&(Global->start), P);

#ifdef SIMICS
       SIMICS_LEAVE_BARRIER(&(Global->start));
#endif
};

  /* to remove cold-start misses, all processors begin by touching a[] */
  TouchA(block_size, MyNum);

  {
#ifdef SIMICS
       SIMICS_ENTER_BARRIER(&(Global->start), P);
#endif
	sesc_barrier(&(Global->start), P);

#ifdef SIMICS
       SIMICS_LEAVE_BARRIER(&(Global->start));
#endif
};

/* POSSIBLE ENHANCEMENT:  Here is where one might reset the
   statistics that one is measuring about the parallel execution */

  if ((MyNum == 0) || (dostats)) {
    {
  if (gettimeofday (&sc_tp, (struct timezone *)0))
	{
    perror (""); 
		fprintf (stderr, "error in clock macro\n"); 
		exit (-1);
	}
  myrs = (sc_tp.tv_sec & 0x7ff)*1000000 + sc_tp.tv_usec;
};
  }

  lu(n, block_size, MyNum, lc, dostats);

  if ((MyNum == 0) || (dostats)) {
    {
  if (gettimeofday (&sc_tp, (struct timezone *)0))
	{
    perror (""); 
		fprintf (stderr, "error in clock macro\n"); 
		exit (-1);
	}
  mydone = (sc_tp.tv_sec & 0x7ff)*1000000 + sc_tp.tv_usec;
};
  }

  {
#ifdef SIMICS
       SIMICS_ENTER_BARRIER(&(Global->start), P);
#endif
	sesc_barrier(&(Global->start), P);

#ifdef SIMICS
       SIMICS_LEAVE_BARRIER(&(Global->start));
#endif
};

  if ((MyNum == 0) || (dostats)) {
    {
  if (gettimeofday (&sc_tp, (struct timezone *)0))
	{
    perror (""); 
		fprintf (stderr, "error in clock macro\n"); 
		exit (-1);
	}
  myrf = (sc_tp.tv_sec & 0x7ff)*1000000 + sc_tp.tv_usec;
};
    Global->t_in_fac[MyNum] = lc->t_in_fac;
    Global->t_in_solve[MyNum] = lc->t_in_solve;
    Global->t_in_mod[MyNum] = lc->t_in_mod;
    Global->t_in_bar[MyNum] = lc->t_in_bar;
    Global->completion[MyNum] = mydone-myrs;
  }
  if (MyNum == 0) {
    Global->rs = myrs;
    Global->done = mydone;
    Global->rf = myrf;
  }
}


void lu0(a, n, stride)

double *a;
int n;
int stride;

{
  int j; 
  int k;
  int length;
  double alpha;

  for (k=0; k<n; k++) {
    /* modify subsequent columns */
    for (j=k+1; j<n; j++) {
      a[k+j*stride] /= a[k+k*stride];
      alpha = -a[k+j*stride];
      length = n-k-1;
      daxpy(&a[k+1+j*stride], &a[k+1+k*stride], n-k-1, alpha);
    }
  }
}


void bdiv(a, diag, stride_a, stride_diag, dimi, dimk)

double *a;
double *diag;
int stride_a;
int stride_diag;
int dimi;
int dimk;

{
  int j;
  int k;
  double alpha;

  for (k=0; k<dimk; k++) {
    for (j=k+1; j<dimk; j++) {
      alpha = -diag[k+j*stride_diag];
      daxpy(&a[j*stride_a], &a[k*stride_a], dimi, alpha);
    }
  }
}


void bmodd(a, c, dimi, dimj, stride_a, stride_c)

double *a;
double *c;
int dimi;
int dimj;
int stride_a;
int stride_c;

{
  int i;
  int j;
  int k;
  int length;
  double alpha;

  for (k=0; k<dimi; k++)
    for (j=0; j<dimj; j++) {
      c[k+j*stride_c] /= a[k+k*stride_a];
      alpha = -c[k+j*stride_c];
      length = dimi - k - 1;
      daxpy(&c[k+1+j*stride_c], &a[k+1+k*stride_a], dimi-k-1, alpha);
    }
}


void bmod(a, b, c, dimi, dimj, dimk, stride)

double *a;
double *b;
double *c;
int dimi;
int dimj;
int dimk;
int stride;

{
  int i;
  int j;
  int k;
  double alpha;

  for (k=0; k<dimk; k++) {
    for (j=0; j<dimj; j++) {
      alpha = -b[k+j*stride];
      daxpy(&c[j*stride], &a[k*stride], dimi, alpha);
    }
  }
}


void daxpy(a, b, n, alpha)

double *a;
double *b;
double alpha;
int n;

{
  int i;

  for (i=0; i<n; i++) {
    a[i] += alpha*b[i];
  }
}


int BlockOwner(I, J)

int I;
int J;

{
  return((I%num_cols) + (J%num_rows)*num_cols);
}


void lu(n, bs, MyNum, lc, dostats)

int n;
int bs;
int MyNum;
struct LocalCopies *lc;
int dostats;

{
  int i, il, j, jl, k, kl;
  int I, J, K;
  double *A, *B, *C, *D;
  int dimI, dimJ, dimK;
  int strI;
  unsigned int t1, t2, t3, t4, t11, t22;
  int diagowner;
  int colowner;

  strI = n;
  for (k=0, K=0; k<n; k+=bs, K++) {
    kl = k+bs; 
    if (kl>n) {
      kl = n;
    }

    if ((MyNum == 0) || (dostats)) {
      {
  if (gettimeofday (&sc_tp, (struct timezone *)0))
	{
    perror (""); 
		fprintf (stderr, "error in clock macro\n"); 
		exit (-1);
	}
  t1 = (sc_tp.tv_sec & 0x7ff)*1000000 + sc_tp.tv_usec;
};
    }

    /* factor diagonal block */
    diagowner = BlockOwner(K, K);
    if (diagowner == MyNum) {
      A = &(a[k+k*n]); 
      lu0(A, kl-k, strI);
    }

    if ((MyNum == 0) || (dostats)) {
      {
  if (gettimeofday (&sc_tp, (struct timezone *)0))
	{
    perror (""); 
		fprintf (stderr, "error in clock macro\n"); 
		exit (-1);
	}
  t11 = (sc_tp.tv_sec & 0x7ff)*1000000 + sc_tp.tv_usec;
};
    }

    {
#ifdef SIMICS
       SIMICS_ENTER_BARRIER(&(Global->start), P);
#endif
	sesc_barrier(&(Global->start), P);

#ifdef SIMICS
       SIMICS_LEAVE_BARRIER(&(Global->start));
#endif
};

    if ((MyNum == 0) || (dostats)) {
      {
  if (gettimeofday (&sc_tp, (struct timezone *)0))
	{
    perror (""); 
		fprintf (stderr, "error in clock macro\n"); 
		exit (-1);
	}
  t2 = (sc_tp.tv_sec & 0x7ff)*1000000 + sc_tp.tv_usec;
};
    }

    /* divide column k by diagonal block */
    D = &(a[k+k*n]);
    for (i=kl, I=K+1; i<n; i+=bs, I++) {
      if (BlockOwner(I, K) == MyNum) {  /* parcel out blocks */
        il = i + bs;
        if (il > n) {
          il = n;
        }
        A = &(a[i+k*n]);
        bdiv(A, D, strI, n, il-i, kl-k);
      }
    }
    /* modify row k by diagonal block */
    for (j=kl, J=K+1; j<n; j+=bs, J++) {
      if (BlockOwner(K, J) == MyNum) {  /* parcel out blocks */
        jl = j+bs;
        if (jl > n) {
          jl = n;
        }
        A = &(a[k+j*n]);
        bmodd(D, A, kl-k, jl-j, n, strI);
      }
    }

    if ((MyNum == 0) || (dostats)) {
      {
  if (gettimeofday (&sc_tp, (struct timezone *)0))
	{
    perror (""); 
		fprintf (stderr, "error in clock macro\n"); 
		exit (-1);
	}
  t22 = (sc_tp.tv_sec & 0x7ff)*1000000 + sc_tp.tv_usec;
};
    }

    {
#ifdef SIMICS
       SIMICS_ENTER_BARRIER(&(Global->start), P);
#endif
	sesc_barrier(&(Global->start), P);

#ifdef SIMICS
       SIMICS_LEAVE_BARRIER(&(Global->start));
#endif
};

    if ((MyNum == 0) || (dostats)) {
      {
  if (gettimeofday (&sc_tp, (struct timezone *)0))
	{
    perror (""); 
		fprintf (stderr, "error in clock macro\n"); 
		exit (-1);
	}
  t3 = (sc_tp.tv_sec & 0x7ff)*1000000 + sc_tp.tv_usec;
};
    }

    /* modify subsequent block columns */
    for (i=kl, I=K+1; i<n; i+=bs, I++) {
      il = i+bs;
      if (il > n) {
        il = n;
      }
      colowner = BlockOwner(I,K);
      A = &(a[i+k*n]);
      for (j=kl, J=K+1; j<n; j+=bs, J++) {
        jl = j + bs;
        if (jl > n) {
          jl = n;
        }
        if (BlockOwner(I, J) == MyNum) {  /* parcel out blocks */
          B = &(a[k+j*n]);
          C = &(a[i+j*n]);
          bmod(A, B, C, il-i, jl-j, kl-k, n);
        }
      }
    }
    if ((MyNum == 0) || (dostats)) {
      {
  if (gettimeofday (&sc_tp, (struct timezone *)0))
	{
    perror (""); 
		fprintf (stderr, "error in clock macro\n"); 
		exit (-1);
	}
  t4 = (sc_tp.tv_sec & 0x7ff)*1000000 + sc_tp.tv_usec;
};
      lc->t_in_fac += (t11-t1);
      lc->t_in_solve += (t22-t2);
      lc->t_in_mod += (t4-t3);
      lc->t_in_bar += (t2-t11) + (t3-t22);
    }
  }
}


void InitA(rhs)

double *rhs;

{
  int i, j;

  srand48((long) 1);
  for (j=0; j<n; j++) {
    for (i=0; i<n; i++) {
      a[i+j*n] = (double) lrand48()/MAXRAND;
      if (i == j) {
	a[i+j*n] *= 10;
      }
    }
  }

  for (j=0; j<n; j++) {
    rhs[j] = 0.0;
  }
  for (j=0; j<n; j++) {
    for (i=0; i<n; i++) {
      rhs[i] += a[i+j*n];
    }
  }
}


double TouchA(bs, MyNum)

int bs;
int MyNum;

{
  int i, j, I, J;
  double tot = 0.0;

  for (J=0; J*bs<n; J++) {
    for (I=0; I*bs<n; I++) {
      if (BlockOwner(I, J) == MyNum) {
        for (j=J*bs; j<(J+1)*bs && j<n; j++) {
          for (i=I*bs; i<(I+1)*bs && i<n; i++) {
            tot += a[i+j*n];
          }
        }
      }
    }
  }
  return(tot);
}


void PrintA()
{
  int i, j;

  for (i=0; i<n; i++) {
    for (j=0; j<n; j++) {
      printf("%8.1f ", a[i+j*n]);
    }
    printf("\n");
  }
}


void CheckResult(n, a, rhs)

int n;
double *a;
double  *rhs;

{
  int i, j, bogus = 0;
  double *y, diff, max_diff;

  y = (double *) malloc(n*sizeof(double));
  if (y == NULL) {
    printerr("Could not malloc memory for y\n");
    exit(-1);
  }
  for (j=0; j<n; j++) {
    y[j] = rhs[j];
  }
  for (j=0; j<n; j++) {
    y[j] = y[j]/a[j+j*n];
    for (i=j+1; i<n; i++) {
      y[i] -= a[i+j*n]*y[j];
    }
  }

  for (j=n-1; j>=0; j--) {
    for (i=0; i<j; i++) {
      y[i] -= a[i+j*n]*y[j];
    }
  }

  max_diff = 0.0;
  for (j=0; j<n; j++) {
    diff = y[j] - 1.0;
    if (fabs(diff) > 0.00001) {
      bogus = 1;
      max_diff = diff;
    }
  }
  if (bogus) {
    printf("TEST FAILED: (%.5f diff)\n", max_diff);
  } else {
    printf("TEST PASSED\n");
  }
  free(y);
}


void printerr(s)

char *s;

{
  fprintf(stderr,"ERROR: %s\n",s);
}
