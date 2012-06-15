
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

  extern struct timeval sc_tp;

#include "math.h"
#include "mdvar.h"
#include "parameters.h"
#include "mddata.h"
#include "split.h"
#include "global.h"

KINETI(NMOL,SUM,HMAS,OMAS,ProcID)
  int NMOL;
  double HMAS,OMAS;
  double SUM[];
  unsigned ProcID;
  
  /* this routine computes kinetic energy in each of the three
     spatial dimensions, and puts the computed values in the
     SUM array */ 
{
    int dir, mol;
    double S;
    
    /* loop over the three directions */
    for (dir = XDIR; dir <= ZDIR; dir++) {
        S=0.0;
        /* loop over the molecules */
        for (mol = StartMol[ProcID]; mol < StartMol[ProcID+1]; mol++) {
            double *tempptr = VAR[mol].F[VEL][dir]; 
            S += ( tempptr[H1] * tempptr[H1] +
                  tempptr[H2] * tempptr[H2] ) * HMAS
                      + (tempptr[O] * tempptr[O]) * OMAS;
        }
        {
#ifdef ATOMIC
        sesc_start_transaction();
#else
#ifdef SIMICS
       SIMICS_START_IGNORE();
       SIMICS_BEGIN_LOCK(&(gl->KinetiSumLock));
#endif
	sesc_lock(&(gl->KinetiSumLock));

#ifdef SIMICS
       SIMICS_LOCKED(&(gl->KinetiSumLock));
       SIMICS_STOP_IGNORE();
#endif
#endif
};
        SUM[dir]+=S;
        {
#ifdef ATOMIC
        sesc_commit_transaction();
#else
#ifdef SIMICS
       SIMICS_START_IGNORE();
#endif
	sesc_unlock(&(gl->KinetiSumLock));

#ifdef SIMICS
       SIMICS_UNLOCK(&(gl->KinetiSumLock));
       SIMICS_STOP_IGNORE();
#endif

#endif
};
    } /* for */
} /* end of subroutine KINETI */

