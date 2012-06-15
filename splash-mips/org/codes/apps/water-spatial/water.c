
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

 
	#include <fcntl.h>
	#include <sys/time.h>

	#include "sescapi.h"

        struct timeval sc_tp;


/*  Usage:   water < infile,
    where infile has 10 fields which can be described in order as 
    follows:
    
    TSTEP:   the physical time interval (in sec) between timesteps.  
    Good default is 1e-15.
    NMOL:    the number of molecules to be simulated.
    NSTEP:   the number of timesteps to be simulated.
    NORDER:  the order of the predictor-corrector method to be used.
    set this to 6.
    NSAVE:   the frequency with which to save data in data collection.
    Set to 0 always.
    NRST:    the frequency with which to write RST file: set to 0 always (not used). 
    NPRINT:  the frequency with which to compute potential energy. 
    i.e. the routine POTENG is called every NPRINT timesteps.
    It also computes intermolecular as well as intramolecular
    interactions, and hence is very expensive.
    NFMC:    Not used (historical artifact).  Set to anything, say 0.
    NumProcs: the number of processors to be used.
    CUTOFF:  the cutoff radius to be used (in Angstrom, 
    floating-point).  In a real simulation, this
    will be set to 0 here in which case the program will 
    compute it itself (and set it to about 11 Angstrom.   
    It can be set by the user if they want
    to use an artificially small cutoff radius, for example
    to control the number of boxes created for small problems 
    (and not have fewer boxes than processors).  
    */

#include <stdio.h>
#include <math.h>

/*  include files for declarations  */
#define extern
#include "cnst.h"
#include "fileio.h"
#include "frcnst.h"
#include "global.h"
#include "mdvar.h"
#include "parameters.h"
#include "randno.h"
#include "split.h"
#include "water.h"
#include "wwpot.h"
#include "mddata.h"
#undef extern

struct GlobalMemory *gl;        /* pointer to the Global Memory
                                   structure, which contains the lock,
                                   barrier, and some scalar variables */

int NSTEP, NSAVE, NRST, NPRINT,NFMC;
int NORD1;
int II;                         /*  variables explained in common.h */
int i;
int NDATA;
int NFRST=11;
int NFSV=10;
int LKT=0;

first_last_array **start_end; /* ptr to array of start/end box #s */
int NumProcs;                 /* number of processors being used; 
                                 run-time input           */

void WorkStart();

main(argc, argv)
  char **argv;
  
{
    int func, dir, atom;
    double XTT, MDMAIN();
    struct link *curr_ptr;
    unsigned ProcID;
    
    /* default values for the control parameters of the driver */
    /* are in parameters.h */
    
    if ((argc == 2) && ((strncmp(argv[1],"-h",strlen("-h")) == 0) || (strncmp(argv[1],"-H",strlen("-H")) == 0))) {
        printf("Usage:  WATER-SPATIAL < infile, where the contents of infile can be\nobtained from the comments at the top of water.C and the first scanf \nin main() in water.C\n\n");
        exit(0);
    }
    
        /*  POSSIBLE ENHANCEMENT:  One might bind the first process to a processor
            here, even before the other (child) processes are bound later in mdmain().
            */

    six = stdout;

    TEMP  =298.0;
    RHO   =0.9980;
    
    /* read input */
    
    if (scanf("%lf%d%d%d%d%d%d%d%d%lf",&TSTEP, &NMOL, &NSTEP, &NORDER, 
              &NSAVE, &NRST, &NPRINT, &NFMC,&NumProcs, &CUTOFF) != 10)
        fprintf(stderr,"ERROR: Usage: water < infile, which must have 10 fields, see SPLASH documentation or comment at top of water.C\n");
    
    printf("Using %d procs on %d steps of %d mols\n", NumProcs, NSTEP, NMOL);
    printf("Other parameters:\n\tTSTEP = %8.2e\n\tNORDER = %d\n\tNSAVE = %d\n",TSTEP,NORDER,NSAVE);
    printf("\tNRST = %d\n\tNPRINT = %d\n\tNFMC = %d\n\tCUTOFF = %lf\n\n",NRST,NPRINT,NFMC,CUTOFF);
    
    /* set up scaling factors and constants */
    
    NORD1=NORDER+1;
    
    CNSTNT(NORD1,TLC);  /* sub. call to set up constants */
    
    SYSCNS();    /* sub. call to initialize system constants  */
    
    printf("%d boxes with %d processors\n\n",
           BOX_PER_SIDE * BOX_PER_SIDE * BOX_PER_SIDE, NumProcs);
    
    if (NumProcs > (BOX_PER_SIDE * BOX_PER_SIDE * BOX_PER_SIDE)) {
        fprintf(stderr,"ERROR: less boxes (%d) than processors (%d)\n",
                BOX_PER_SIDE * BOX_PER_SIDE * BOX_PER_SIDE, NumProcs);
        fflush(stderr);
        exit(-1);
    }
    
    fprintf(six,"\nTEMPERATURE                = %8.2f K\n",TEMP);
    fprintf(six,"DENSITY                    = %8.5f G/C.C.\n",RHO);
    fprintf(six,"NUMBER OF MOLECULES        = %8d\n",NMOL);
    fprintf(six,"NUMBER OF PROCESSORS       = %8d\n",NumProcs);
    fprintf(six,"TIME STEP                  = %8.2e SEC\n",TSTEP);
    fprintf(six,"ORDER USED TO SOLVE F=MA   = %8d \n",NORDER);
    fprintf(six,"NO. OF TIME STEPS          = %8d \n",NSTEP);
    fprintf(six,"FREQUENCY OF DATA SAVING   = %8d \n",NSAVE);
    fprintf(six,"FREQUENCY TO WRITE RST FILE= %8d \n",NRST);
    fflush(six);
    
    { /* do memory initializations */
        
        int pid, procnum, i, j, k, l;
        struct list_of_boxes *new_box;
        struct list_of_boxes *temp_box;
        int xprocs, yprocs, zprocs;
        int x_inc, y_inc, z_inc;
        int x_ct, y_ct, z_ct;
        int x_left, y_left, z_left;
        int x_first, y_first, z_first;
        int x_last, y_last, z_last;
        double proccbrt;
        unsigned gmem_size = sizeof(struct GlobalMemory);
        
        {
	sesc_init();
};  /* macro call to initialize
                                      shared memory etc. */
        
        
        /* Allocate space for main (BOX) data structure as well as
         * synchronization variables 
         */
        
        start_end = (first_last_array **) 
            malloc(sizeof(first_last_array *) * NumProcs);;
        for (i=0; i < NumProcs; i++) {
            start_end[i] = (first_last_array *)
                malloc(sizeof(first_last_array));;
        }
        
        /* Calculate start and finish box numbers for processors */
        
        xprocs = 0;
        yprocs = 0;
        proccbrt = (double) pow((double) NumProcs, 1.0/3.0) + 0.00000000000001;
        j = (int) proccbrt;
        if (j<1) j = 1;
        while ((xprocs == 0) && (j>0)) {
            k = (int) sqrt((double) (NumProcs / j));
            if (k<1) k=1;
            while ((yprocs == 0) && (k>0)) {
                l = NumProcs/(j*k);
                if ((j*k*l) == NumProcs) {
                    xprocs = j;
                    yprocs = k;
                    zprocs = l;
                } /* if */
                k--;
            } /* while yprocs && k */
            j--;
        } /* while xprocs && j */
        
        printf("xprocs = %d\typrocs = %d\tzprocs = %d\n",
               xprocs, yprocs, zprocs);
        fflush(stdout);
        
        /* Fill in start_end array values */
        
        procnum = 0;
        x_inc = BOX_PER_SIDE/xprocs;
        y_inc = BOX_PER_SIDE/yprocs;
        z_inc = BOX_PER_SIDE/zprocs;
        
        x_left = BOX_PER_SIDE - (xprocs*x_inc);
        y_left = BOX_PER_SIDE - (yprocs*y_inc);
        z_left = BOX_PER_SIDE - (zprocs*z_inc);
        printf("x_inc = %d\t y_inc = %d\t z_inc = %d\n",x_inc,y_inc,z_inc);
        printf("x_left = %d\t y_left = %d\t z_left = %d\n",x_left,y_left,z_left);
        fflush(stdout);
        
        
        x_first = 0;
        x_ct = x_left;
        x_last = -1;
        x_inc++;
        for (i=0; i<xprocs; i++) {
            y_ct = y_left;
            if (x_ct == 0) x_inc--;
            x_last += x_inc;
            y_first = 0;
            y_last = -1;
            y_inc++;
            for (j=0; j<yprocs; j++) {
                z_ct = z_left;
                if (y_ct == 0) y_inc--;
                y_last += y_inc;
                z_first = 0;
                z_last = -1;
                z_inc++;
                for (k=0; k<zprocs; k++) {
                    if (z_ct == 0) z_inc--;
                    z_last += z_inc;
                    start_end[procnum]->box[XDIR][FIRST] = x_first;
                    start_end[procnum]->box[XDIR][LAST] = 
                        min(x_last, BOX_PER_SIDE - 1);
                    start_end[procnum]->box[YDIR][FIRST] = y_first;
                    start_end[procnum]->box[YDIR][LAST] = 
                        min(y_last, BOX_PER_SIDE - 1);
                    start_end[procnum]->box[ZDIR][FIRST] = z_first;
                    start_end[procnum]->box[ZDIR][LAST] = 
                        min(z_last, BOX_PER_SIDE - 1);
                    z_first = z_last + 1;
                    z_ct--;
                    procnum++;
                }
                y_first = y_last + 1;
                y_ct--;
            }
            x_first = x_last + 1;
            x_ct--;
        }
        
        /* Allocate space for my_boxes array */
        
        my_boxes = (box_list **) malloc(NumProcs * sizeof(box_list *));;
        
        /* Set all box ptrs to null */
        
        for (i=0; i<NumProcs; i++) my_boxes[i] = NULL;
        
        /* Set up links for all boxes for initial interf and intraf */
        
        temp_box = my_boxes[0];
        while (temp_box) {
            temp_box = temp_box->next_box;
        }
        
        /* Allocate space for BOX array */
        
        BOX = (box_type ***) malloc(BOX_PER_SIDE * sizeof(box_type **));;
        for (i=0; i < BOX_PER_SIDE; i++) {
            BOX[i] = (box_type **) malloc(BOX_PER_SIDE * sizeof(box_type *));;
            for (j=0; j < BOX_PER_SIDE; j++) {
                BOX[i][j] = (box_type *) malloc(BOX_PER_SIDE * sizeof(box_type));;
                for (k=0; k < BOX_PER_SIDE; k++) {
                    BOX[i][j][k].list = NULL;
                    sesc_lock_init(&(BOX[i][j][k].boxlock));
;
                }
            }
        } /* for i */
        
        gl = (struct GlobalMemory *) malloc(gmem_size);;
        
        /* macro calls to initialize synch variables  */
        
        sesc_barrier_init(&(gl->start));
;
        sesc_barrier_init(&(gl->InterfBar));
;
        sesc_barrier_init(&(gl->PotengBar));
;
        sesc_lock_init(&(gl->IOLock));
;
        sesc_lock_init(&(gl->IndexLock));
;
        sesc_lock_init(&(gl->IntrafVirLock));
;
        sesc_lock_init(&(gl->InterfVirLock));
;
        sesc_lock_init(&(gl->KinetiSumLock));
;
        sesc_lock_init(&(gl->PotengSumLock));
;
    }
    
    fprintf(six,"SPHERICAL CUTOFF RADIUS    = %8.4f ANGSTROM\n",CUTOFF);
    fflush(six);
    
    IRST=0;
    
    /* call initialization routine */
    
    INITIA();
    
    gl->tracktime = 0;
    gl->intratime = 0;
    gl->intertime = 0;
    
    /* initialize Index to 1 so that the first created child gets 
       id 1, not 0 */
    
    gl->Index = 1;
    
    /* spawn helper processes */
    
    {
  if (gettimeofday (&sc_tp, (struct timezone *)0))
	{
    perror (""); 
		fprintf (stderr, "error in clock macro\n"); 
		exit (-1);
	}
  gl->createstart = (sc_tp.tv_sec & 0x7ff)*1000000 + sc_tp.tv_usec;
};
    for (ProcID = 1; ProcID < NumProcs; ProcID++) {
        
	sesc_spawn(WorkStart, NULL, 0);
;
    }
    
    {
  if (gettimeofday (&sc_tp, (struct timezone *)0))
	{
    perror (""); 
		fprintf (stderr, "error in clock macro\n"); 
		exit (-1);
	}
  gl->createend = (sc_tp.tv_sec & 0x7ff)*1000000 + sc_tp.tv_usec;
};
    
    {
#ifdef ATOMIC
        sesc_start_transaction();
#else
#ifdef SIMICS
       SIMICS_START_IGNORE();
       SIMICS_BEGIN_LOCK(&(gl->IOLock));
#endif
	sesc_lock(&(gl->IOLock));

#ifdef SIMICS
       SIMICS_LOCKED(&(gl->IOLock));
       SIMICS_STOP_IGNORE();
#endif
#endif
};
    printf("CREATETIME = %d\n",gl->createend-gl->createstart);
    {
#ifdef ATOMIC
        sesc_commit_transaction();
#else
#ifdef SIMICS
       SIMICS_START_IGNORE();
#endif
	sesc_unlock(&(gl->IOLock));

#ifdef SIMICS
       SIMICS_UNLOCK(&(gl->IOLock));
       SIMICS_STOP_IGNORE();
#endif

#endif
};
    
    if (NSAVE > 0) {  /* not true for input decks provided */
        {
#ifdef ATOMIC
        sesc_start_transaction();
#else
#ifdef SIMICS
       SIMICS_START_IGNORE();
       SIMICS_BEGIN_LOCK(&(gl->IOLock));
#endif
	sesc_lock(&(gl->IOLock));

#ifdef SIMICS
       SIMICS_LOCKED(&(gl->IOLock));
       SIMICS_STOP_IGNORE();
#endif
#endif
};
        fprintf(six,"COLLECTING X AND V DATA AT EVERY %4d TIME STEPS \n",NSAVE);
        {
#ifdef ATOMIC
        sesc_commit_transaction();
#else
#ifdef SIMICS
       SIMICS_START_IGNORE();
#endif
	sesc_unlock(&(gl->IOLock));

#ifdef SIMICS
       SIMICS_UNLOCK(&(gl->IOLock));
       SIMICS_STOP_IGNORE();
#endif

#endif
};
    }
    
    
    /* CALL ROUTINE TO DO THE TIMESTEPS, with own id passed as 0 */
    
    {
  if (gettimeofday (&sc_tp, (struct timezone *)0))
	{
    perror (""); 
		fprintf (stderr, "error in clock macro\n"); 
		exit (-1);
	}
  gl->computestart = (sc_tp.tv_sec & 0x7ff)*1000000 + sc_tp.tv_usec;
};
    
    XTT = MDMAIN(NFSV,NFRST,NSTEP,NRST,NPRINT,NSAVE,LKT,NORD1,0); 
    
    /* macro to make main process wait for all others to finish */
    {
	int i;
	for (i = 0; i < NumProcs-1; i++)
		sesc_wait();
};
    {
  if (gettimeofday (&sc_tp, (struct timezone *)0))
	{
    perror (""); 
		fprintf (stderr, "error in clock macro\n"); 
		exit (-1);
	}
  gl->computeend = (sc_tp.tv_sec & 0x7ff)*1000000 + sc_tp.tv_usec;
};
    
    printf("COMPUTESTART (after initialization) = %u\n",gl->computestart);
    printf("COMPUTEEND = %u\n",gl->computeend);
    printf("COMPUTETIME (after initialization) = %u\n",gl->computeend-gl->computestart);
    printf("Measured Time (2nd timestep onward) = %u\n",gl->tracktime);
    printf("Intramolecular time only (2nd timestep onward) = %u\n",gl->intratime);
    printf("Intermolecular time only (2nd timestep onward) = %u\n",gl->intertime);
    printf("Other time (2nd timestep onward) = %u\n",gl->tracktime - gl->intratime - gl->intertime);
    
    printf("\nExited Happily with XTT = %g (note: XTT value is garbage if NPRINT > NSTEP)\n", XTT);
    
    {exit(0);};
} /* main.c */

void WorkStart() /* routine that each created process starts at;
                    it simply calls the timestep routine */
{
    unsigned ProcID;
    double MDMAIN();
    
    {
#ifdef ATOMIC
        sesc_start_transaction();
#else
#ifdef SIMICS
       SIMICS_START_IGNORE();
       SIMICS_BEGIN_LOCK(&(gl->IndexLock));
#endif
	sesc_lock(&(gl->IndexLock));

#ifdef SIMICS
       SIMICS_LOCKED(&(gl->IndexLock));
       SIMICS_STOP_IGNORE();
#endif
#endif
};
    ProcID = gl->Index++;
    {
#ifdef ATOMIC
        sesc_commit_transaction();
#else
#ifdef SIMICS
       SIMICS_START_IGNORE();
#endif
	sesc_unlock(&(gl->IndexLock));

#ifdef SIMICS
       SIMICS_UNLOCK(&(gl->IndexLock));
       SIMICS_STOP_IGNORE();
#endif

#endif
};
    
    ProcID = ProcID % NumProcs; 
    
    /*  POSSIBLE ENHANCEMENT:  Here's where one might bind processes to processors
        if one wanted to.
        */
    
    MDMAIN(NFSV,NFRST,NSTEP,NRST,NPRINT,NSAVE,LKT,NORD1,ProcID);
}

