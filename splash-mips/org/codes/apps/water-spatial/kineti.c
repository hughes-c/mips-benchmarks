
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


#include <math.h>
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
    int dir, i, j, k;
    double S;
    struct link *curr_ptr;
    struct list_of_boxes *curr_box;
    double *tempptr;
    
    /* Loop over three directions */
    
    for (dir = XDIR; dir <= ZDIR; dir++) {
        S=0.0;
        
        /* loop over processor's boxes */
        curr_box = my_boxes[ProcID];
        
        while (curr_box) {
            
            i = curr_box->coord[XDIR];  /* X coordinate of box */
            j = curr_box->coord[YDIR];  /* Y coordinate of box */
            k = curr_box->coord[ZDIR];  /* Z coordinate of box */
            
            /* loop over the molecules */
            
            curr_ptr = BOX[i][j][k].list;
            while (curr_ptr) {
                tempptr = curr_ptr->mol.F[VEL][dir];
                S += (tempptr[H1] * tempptr[H1] +
                      tempptr[H2] * tempptr[H2] ) * HMAS +
                          (tempptr[O] * tempptr[O]) * OMAS;
                curr_ptr = curr_ptr->next_mol;
            } /* while curr_ptr */
            
            curr_box = curr_box->next_box;
            
        } /* while curr_box */ 
        
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
        
    } /* for dir */
    
} /* end of subroutine KINETI */
