
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

/*************************************************************************
*                                                                        *
*     adaptive.c:  Render dataset via raytracing. 					  	 *
*															             *
*************************************************************************/

#include "incl.h"

float invjacobian[NM][NM];	/* Jacobian matrix showing object space      */
				/*   d{x,y,z} per image space d{x,y,z}       */
				/*   [0][0] is dx(object)/dx(image)          */
				/*   [0][2] is dz(object)/dx(image)          */
				/*   [2][0] is dx(object)/dz(image)          */
float invinvjacobian[NM][NM];	/*   [i][j] = 1.0 / invjacobian[i][j]        */
                                /* For gathering statistics:                 */
int num_rays_traced;            /*   number of calls to Trace_Ray            */
int num_traced_rays_hit_volume; /*   number of traced rays that hit volume   */
int num_samples_trilirped;      /*   number of samples trilirped             */
int itest;

#define	RAY_TRACED	((MAX_PIXEL+1)/2)	/* Ray traced at this pixel  */
#define START_RAY       1
#define	INTERPOLATED	((MAX_PIXEL+1)/32)	/* This pixel interpolated   */

 
	#include <fcntl.h>
	#include <sys/time.h>

	#include "sescapi.h"

  extern struct timeval sc_tp;


#include "anl.h"

Ray_Trace(int my_node)
{
  int outx,outy,outz;
  int i,j;
  unsigned int starttime,stoptime,exectime,exectime1;
  int pid;
  char cmd[FILENAME_STRING_SIZE];

  
  /* Assumptions made by ray tracer:                                   */
  /*   o Frustrum clipping is performed.                               */
  /*     All viewing frustums will be handled correctly.               */
  /*   o Contributions are obtained only from nearest 8 neighbors.     */
  /*     If downsizing was specified, some input voxels will be        */
  /*     unsampled, but upsizing may be specified and will be          */
  /*     handled correctly.                                            */
  
  /* Compute inverse Jacobian matrix from                              */
  /* coordinates of output map unit voxel in object space,             */
  /* then make a copy of object space d{x,y,z} per image space dz,     */
  /* which controls number of ray samples per object space voxel       */
  /* (i.e. Z-sampling rate), to allow recomputation per region.        */
  for (i=0; i<NM; i++) {
    invjacobian[X][i] = uout_invvertex[0][0][1][i] -
      uout_invvertex[0][0][0][i];
    invjacobian[Y][i] = uout_invvertex[0][1][0][i] -
      uout_invvertex[0][0][0][i];
    invjacobian[Z][i] = uout_invvertex[1][0][0][i] -
      uout_invvertex[0][0][0][i];
  }

  /* Compute multiplicative inverse of inverse Jacobian matrix.        */
  /* If any Jacobian is zero, compute no inverse for that element.     */
  /* This test must be repeated before access to any inverse element.  */
  for (i=0; i<NM; i++) {
    for (j=0; j<NM; j++) {
      if (ABS(invjacobian[i][j]) > SMALL)
	invinvjacobian[i][j] = 1.0 / invjacobian[i][j];
    }
  }

  num_rays_traced = 0;
  num_traced_rays_hit_volume = 0;
  num_samples_trilirped = 0;

  /* Invoke adaptive or non-adaptive ray tracer                          */
  if (adaptive) {

    {
#ifdef SIMICS
       SIMICS_ENTER_BARRIER(&(Global->TimeBarrier), num_nodes);
#endif
	sesc_barrier(&(Global->TimeBarrier), num_nodes);

#ifdef SIMICS
       SIMICS_LEAVE_BARRIER(&(Global->TimeBarrier));
#endif
};

    {
  if (gettimeofday (&sc_tp, (struct timezone *)0))
	{
    perror (""); 
		fprintf (stderr, "error in clock macro\n"); 
		exit (-1);
	}
  starttime = (sc_tp.tv_sec & 0x7ff)*1000000 + sc_tp.tv_usec;
};
    Pre_Shade(my_node);

    {
#ifdef ATOMIC
        sesc_start_transaction();
#else
#ifdef SIMICS
       SIMICS_START_IGNORE();
       SIMICS_BEGIN_LOCK(&(Global->CountLock));
#endif
	sesc_lock(&(Global->CountLock));

#ifdef SIMICS
       SIMICS_LOCKED(&(Global->CountLock));
       SIMICS_STOP_IGNORE();
#endif
#endif
};
    Global->Counter--;
    {
#ifdef ATOMIC
        sesc_commit_transaction();
#else
#ifdef SIMICS
       SIMICS_START_IGNORE();
#endif
	sesc_unlock(&(Global->CountLock));

#ifdef SIMICS
       SIMICS_UNLOCK(&(Global->CountLock));
       SIMICS_STOP_IGNORE();
#endif

#endif
};
    while (Global->Counter);

    Ray_Trace_Adaptively(my_node);

    {
  if (gettimeofday (&sc_tp, (struct timezone *)0))
	{
    perror (""); 
		fprintf (stderr, "error in clock macro\n"); 
		exit (-1);
	}
  stoptime = (sc_tp.tv_sec & 0x7ff)*1000000 + sc_tp.tv_usec;
};

    {
#ifdef SIMICS
       SIMICS_ENTER_BARRIER(&(Global->TimeBarrier), num_nodes);
#endif
	sesc_barrier(&(Global->TimeBarrier), num_nodes);

#ifdef SIMICS
       SIMICS_LEAVE_BARRIER(&(Global->TimeBarrier));
#endif
};

    mclock(stoptime,starttime,&exectime);
    
    /* If adaptively ray tracing and highest sampling size is greater    */
    /* than lowest size for volume data if polygon list exists or        */
    /* display pixel size if it does not, recursively interpolate to     */
    /* fill in any missing samples down to lowest size for volume data.  */

    if (highest_sampling_boxlen > 1) {

      {
#ifdef SIMICS
       SIMICS_ENTER_BARRIER(&(Global->TimeBarrier), num_nodes);
#endif
	sesc_barrier(&(Global->TimeBarrier), num_nodes);

#ifdef SIMICS
       SIMICS_LEAVE_BARRIER(&(Global->TimeBarrier));
#endif
};

      {
  if (gettimeofday (&sc_tp, (struct timezone *)0))
	{
    perror (""); 
		fprintf (stderr, "error in clock macro\n"); 
		exit (-1);
	}
  starttime = (sc_tp.tv_sec & 0x7ff)*1000000 + sc_tp.tv_usec;
};
      Interpolate_Recursively(my_node);

      {
  if (gettimeofday (&sc_tp, (struct timezone *)0))
	{
    perror (""); 
		fprintf (stderr, "error in clock macro\n"); 
		exit (-1);
	}
  stoptime = (sc_tp.tv_sec & 0x7ff)*1000000 + sc_tp.tv_usec;
};

      {
#ifdef SIMICS
       SIMICS_ENTER_BARRIER(&(Global->TimeBarrier), num_nodes);
#endif
	sesc_barrier(&(Global->TimeBarrier), num_nodes);

#ifdef SIMICS
       SIMICS_LEAVE_BARRIER(&(Global->TimeBarrier));
#endif
};

      mclock(stoptime,starttime,&exectime1);
    }
  }
  else {

    {
#ifdef SIMICS
       SIMICS_ENTER_BARRIER(&(Global->TimeBarrier), num_nodes);
#endif
	sesc_barrier(&(Global->TimeBarrier), num_nodes);

#ifdef SIMICS
       SIMICS_LEAVE_BARRIER(&(Global->TimeBarrier));
#endif
};

    {
  if (gettimeofday (&sc_tp, (struct timezone *)0))
	{
    perror (""); 
		fprintf (stderr, "error in clock macro\n"); 
		exit (-1);
	}
  starttime = (sc_tp.tv_sec & 0x7ff)*1000000 + sc_tp.tv_usec;
};

    Pre_Shade(my_node);

    {
#ifdef ATOMIC
        sesc_start_transaction();
#else
#ifdef SIMICS
       SIMICS_START_IGNORE();
       SIMICS_BEGIN_LOCK(&(Global->CountLock));
#endif
	sesc_lock(&(Global->CountLock));

#ifdef SIMICS
       SIMICS_LOCKED(&(Global->CountLock));
       SIMICS_STOP_IGNORE();
#endif
#endif
};
    Global->Counter--;
    {
#ifdef ATOMIC
        sesc_commit_transaction();
#else
#ifdef SIMICS
       SIMICS_START_IGNORE();
#endif
	sesc_unlock(&(Global->CountLock));

#ifdef SIMICS
       SIMICS_UNLOCK(&(Global->CountLock));
       SIMICS_STOP_IGNORE();
#endif

#endif
};
    while (Global->Counter);

    Ray_Trace_Non_Adaptively(my_node);

    {
  if (gettimeofday (&sc_tp, (struct timezone *)0))
	{
    perror (""); 
		fprintf (stderr, "error in clock macro\n"); 
		exit (-1);
	}
  stoptime = (sc_tp.tv_sec & 0x7ff)*1000000 + sc_tp.tv_usec;
};

    {
#ifdef SIMICS
       SIMICS_ENTER_BARRIER(&(Global->TimeBarrier), num_nodes);
#endif
	sesc_barrier(&(Global->TimeBarrier), num_nodes);

#ifdef SIMICS
       SIMICS_LEAVE_BARRIER(&(Global->TimeBarrier));
#endif
};

    mclock(stoptime,starttime,&exectime);
    exectime1 = 0;
  }

    {
#ifdef ATOMIC
        sesc_start_transaction();
#else
#ifdef SIMICS
       SIMICS_START_IGNORE();
       SIMICS_BEGIN_LOCK(&(Global->CountLock));
#endif
	sesc_lock(&(Global->CountLock));

#ifdef SIMICS
       SIMICS_LOCKED(&(Global->CountLock));
       SIMICS_STOP_IGNORE();
#endif
#endif
};
    printf("%3d\t%3d\t%6u\t%6u\t%6d\t%6d\t%8d\n",my_node,frame,exectime,
	   exectime1,num_rays_traced,num_traced_rays_hit_volume,
	   num_samples_trilirped);

    {
#ifdef ATOMIC
        sesc_commit_transaction();
#else
#ifdef SIMICS
       SIMICS_START_IGNORE();
#endif
	sesc_unlock(&(Global->CountLock));

#ifdef SIMICS
       SIMICS_UNLOCK(&(Global->CountLock));
       SIMICS_STOP_IGNORE();
#endif

#endif
};

  {
#ifdef SIMICS
       SIMICS_ENTER_BARRIER(&(Global->TimeBarrier), num_nodes);
#endif
	sesc_barrier(&(Global->TimeBarrier), num_nodes);

#ifdef SIMICS
       SIMICS_LEAVE_BARRIER(&(Global->TimeBarrier));
#endif
};
}


Ray_Trace_Adaptively(int my_node)
{
  int i,outx,outy,yindex,xindex;

  int num_xqueue,num_yqueue,num_queue,lnum_xblocks,lnum_yblocks,lnum_blocks;
  int xstart,xstop,ystart,ystop,local_node,work;

  itest = 0;

  num_xqueue = ROUNDUP((float)image_len[X]/(float)image_section[X]);
  num_yqueue = ROUNDUP((float)image_len[Y]/(float)image_section[Y]);
  num_queue = num_xqueue * num_yqueue;
  lnum_xblocks = ROUNDUP((float)num_xqueue/(float)block_xlen);
  lnum_yblocks = ROUNDUP((float)num_yqueue/(float)block_ylen);
  lnum_blocks = lnum_xblocks * lnum_yblocks;
  local_node = my_node;
  Global->Queue[local_node][0] = 0;
  while (Global->Queue[num_nodes][0] > 0) {
    xstart = (local_node % image_section[X]) * num_xqueue;
    xstart = ROUNDUP((float)xstart/(float)highest_sampling_boxlen);
    xstart = xstart * highest_sampling_boxlen;
    xstop = MIN(xstart+num_xqueue,image_len[X]);
    ystart = (local_node / image_section[X]) * num_yqueue;
    ystart = ROUNDUP((float)ystart/(float)highest_sampling_boxlen);
    ystart = ystart * highest_sampling_boxlen;
    ystop = MIN(ystart+num_yqueue,image_len[Y]);
    {
#ifdef ATOMIC
        sesc_start_transaction();
#else
#ifdef SIMICS
       SIMICS_START_IGNORE();
       SIMICS_BEGIN_LOCK(&(Global->QLock[local_node]));
#endif
	sesc_lock(&(Global->QLock[local_node]));

#ifdef SIMICS
       SIMICS_LOCKED(&(Global->QLock[local_node]));
       SIMICS_STOP_IGNORE();
#endif
#endif
};
    work = Global->Queue[local_node][0];
    Global->Queue[local_node][0] += 1;
    {
#ifdef ATOMIC
        sesc_commit_transaction();
#else
#ifdef SIMICS
       SIMICS_START_IGNORE();
#endif
	sesc_unlock(&(Global->QLock[local_node]));

#ifdef SIMICS
       SIMICS_UNLOCK(&(Global->QLock[local_node]));
       SIMICS_STOP_IGNORE();
#endif

#endif
};
    while (work < lnum_blocks) {
      xindex = xstart + (work%lnum_xblocks)*block_xlen;
      yindex = ystart + (work/lnum_xblocks)*block_ylen;
      for (outy=yindex; outy<yindex+block_ylen && outy<ystop;
	   outy+=highest_sampling_boxlen) {
	for (outx=xindex; outx<xindex+block_xlen && outx<xstop;
	     outx+=highest_sampling_boxlen) {
	  
	  /* Trace rays within square box of highest sampling size     */
	  /* whose lower-left corner is current image space location.  */
	  Ray_Trace_Adaptive_Box(outx,outy,highest_sampling_boxlen);
	}
      }
      {
#ifdef ATOMIC
        sesc_start_transaction();
#else
#ifdef SIMICS
       SIMICS_START_IGNORE();
       SIMICS_BEGIN_LOCK(&(Global->QLock[local_node]));
#endif
	sesc_lock(&(Global->QLock[local_node]));

#ifdef SIMICS
       SIMICS_LOCKED(&(Global->QLock[local_node]));
       SIMICS_STOP_IGNORE();
#endif
#endif
};
      work = Global->Queue[local_node][0];
      Global->Queue[local_node][0] += 1;
      {
#ifdef ATOMIC
        sesc_commit_transaction();
#else
#ifdef SIMICS
       SIMICS_START_IGNORE();
#endif
	sesc_unlock(&(Global->QLock[local_node]));

#ifdef SIMICS
       SIMICS_UNLOCK(&(Global->QLock[local_node]));
       SIMICS_STOP_IGNORE();
#endif

#endif
};
    }
    if (my_node == local_node) {
      {
#ifdef ATOMIC
        sesc_start_transaction();
#else
#ifdef SIMICS
       SIMICS_START_IGNORE();
       SIMICS_BEGIN_LOCK(&(Global->QLock[num_nodes]));
#endif
	sesc_lock(&(Global->QLock[num_nodes]));

#ifdef SIMICS
       SIMICS_LOCKED(&(Global->QLock[num_nodes]));
       SIMICS_STOP_IGNORE();
#endif
#endif
};
      Global->Queue[num_nodes][0]--;
      {
#ifdef ATOMIC
        sesc_commit_transaction();
#else
#ifdef SIMICS
       SIMICS_START_IGNORE();
#endif
	sesc_unlock(&(Global->QLock[num_nodes]));

#ifdef SIMICS
       SIMICS_UNLOCK(&(Global->QLock[num_nodes]));
       SIMICS_STOP_IGNORE();
#endif

#endif
};
    }
    local_node = (local_node+1)%num_nodes;
    while (Global->Queue[local_node][0] >= lnum_blocks &&
	   Global->Queue[num_nodes][0] > 0)
      local_node = (local_node+1)%num_nodes;
  }

}


Ray_Trace_Adaptive_Box(outx, outy, boxlen)
     int outx, outy, boxlen;
{
  int i,j;
  int half_boxlen;
  int min_volume_color,max_volume_color;
  float foutx,fouty;
  volatile int imask;
  
  PIXEL *pixel_address;
  
  /* Trace rays from all four corners of the box into the map,         */
  /* being careful not to exceed the boundaries of the output image,   */
  /* and using a flag array to avoid retracing any rays.               */
  /* For diagnostic display, flag is set to a light gray.              */
  /* If mipmapping, flag is light gray minus current mipmap level.     */
  /* If mipmapping and ray has already been traced,                    */
  /* retrace it if current mipmap level is lower than                  */
  /* mipmap level in effect when ray was last traced,                  */
  /* thus replacing crude approximation with better one.               */
  /* Meanwhile, compute minimum and maximum geometry/volume colors.    */
  /* If polygon list exists, compute geometry-only colors              */
  /* and volume-attenuated geometry-only colors as well.               */
  /* If stochastic sampling and box is smaller than a display pixel,   */
  /* distribute the rays uniformly across a square centered on the     */
  /* nominal ray location and of size equal to the image array spacing.*/
  /* This scheme interpolates the jitter size / sample spacing ratio   */
  /* from zero at one sample per display pixel, avoiding jitter noise, */
  /* to one at the maximum sampling rate, insuring complete coverage,  */
  /* all the while building on previously traced rays where possible.  */
  /* The constant radius also prevents overlap of jitter squares from  */
  /* successive subdivision levels, preventing sample clumping noise.  */

  min_volume_color = MAX_PIXEL;
  max_volume_color = MIN_PIXEL;

  for (i=0; i<=boxlen && outy+i<image_len[Y]; i+=boxlen) {
    for (j=0; j<=boxlen && outx+j<image_len[X]; j+=boxlen) {

/*reschedule processes here if rescheduling only at synch points on simulator*/

      if (MASK_IMAGE(outy+i,outx+j) == 0) {

/*reschedule processes here if rescheduling only at synch points on simulator*/

	MASK_IMAGE(outy+i,outx+j) = START_RAY;

/*reschedule processes here if rescheduling only at synch points on simulator*/

	foutx = (float)(outx+j);
	fouty = (float)(outy+i);
	pixel_address = IMAGE_ADDRESS(outy+i,outx+j);

/*reschedule processes here if rescheduling only at synch points on simulator*/

	Trace_Ray(outx+j,outy+i,foutx,fouty,pixel_address);

/*reschedule processes here if rescheduling only at synch points on simulator*/

	MASK_IMAGE(outy+i,outx+j) = RAY_TRACED;
      }
    }
  }
  for (i=0; i<=boxlen && outy+i<image_len[Y]; i+=boxlen) {
    for (j=0; j<=boxlen && outx+j<image_len[X]; j+=boxlen) {
      imask = MASK_IMAGE(outy+i,outx+j);

/*reschedule processes here if rescheduling only at synch points on simulator*/

      while (imask == START_RAY) {

/*reschedule processes here if rescheduling only at synch points on simulator*/

	imask = MASK_IMAGE(outy+i,outx+j);

/*reschedule processes here if rescheduling only at synch points on simulator*/

      }
      min_volume_color = MIN(IMAGE(outy+i,outx+j),min_volume_color);
      max_volume_color = MAX(IMAGE(outy+i,outx+j),max_volume_color);
    }
  }

  /* If size of current box is above lowest size for volume data and   */
  /* magnitude of geometry/volume color difference is significant, or  */
  /* size of current box is above lowest size for geometric data and   */
  /* magnitudes of geometry-only and volume-attenuated geometry-only   */
  /* are both significant, thus detecting only visible geometry events,*/
  /* invoke this function recursively to trace rays within the         */
  /* four equal-sized square sub-boxes enclosed by the current box,    */
  /* being careful not to exceed the boundaries of the output image.   */
  /* Use of geometry-only color difference suppressed in accordance    */
  /* with hybrid.trf as published in IEEE CG&A, March, 1990.           */
  if (boxlen > lowest_volume_boxlen && 
      max_volume_color - min_volume_color >=
      volume_color_difference) {
    half_boxlen = boxlen >> 1;
    for (i=0; i<boxlen && outy+i<image_len[Y]; i+=half_boxlen) {
      for (j=0; j<boxlen && outx+j<image_len[X]; j+=half_boxlen) {
	Ray_Trace_Adaptive_Box(outx+j,outy+i,half_boxlen);
      }
    }
  }
}


Ray_Trace_Non_Adaptively(int my_node)
{
  int i,outx,outy,xindex,yindex;
  float foutx,fouty;
  PIXEL *pixel_address;

  int num_xqueue,num_yqueue,num_queue,lnum_xblocks,lnum_yblocks,lnum_blocks;
  int xstart,xstop,ystart,ystop,local_node,work;

  num_xqueue = ROUNDUP((float)image_len[X]/(float)image_section[X]);
  num_yqueue = ROUNDUP((float)image_len[Y]/(float)image_section[Y]);
  num_queue = num_xqueue * num_yqueue;
  lnum_xblocks = ROUNDUP((float)num_xqueue/(float)block_xlen);
  lnum_yblocks = ROUNDUP((float)num_yqueue/(float)block_ylen);
  lnum_blocks = lnum_xblocks * lnum_yblocks;
  local_node = my_node;
  Global->Queue[local_node][0] = 0;
  while (Global->Queue[num_nodes][0] > 0) {
    xstart = (local_node % image_section[X]) * num_xqueue;
    xstop = MIN(xstart+num_xqueue,image_len[X]);
    ystart = (local_node / image_section[X]) * num_yqueue;
    ystop = MIN(ystart+num_yqueue,image_len[Y]);
    {
#ifdef ATOMIC
        sesc_start_transaction();
#else
#ifdef SIMICS
       SIMICS_START_IGNORE();
       SIMICS_BEGIN_LOCK(&(Global->QLock[local_node]));
#endif
	sesc_lock(&(Global->QLock[local_node]));

#ifdef SIMICS
       SIMICS_LOCKED(&(Global->QLock[local_node]));
       SIMICS_STOP_IGNORE();
#endif
#endif
};
    work = Global->Queue[local_node][0]++;
    {
#ifdef ATOMIC
        sesc_commit_transaction();
#else
#ifdef SIMICS
       SIMICS_START_IGNORE();
#endif
	sesc_unlock(&(Global->QLock[local_node]));

#ifdef SIMICS
       SIMICS_UNLOCK(&(Global->QLock[local_node]));
       SIMICS_STOP_IGNORE();
#endif

#endif
};
    while (work < lnum_blocks) {
      xindex = xstart + (work%lnum_xblocks)*block_xlen;
      yindex = ystart + (work/lnum_xblocks)*block_ylen;
      for (outy=yindex; outy<yindex+block_ylen && outy<ystop; outy++) {
	for (outx=xindex; outx<xindex+block_xlen && outx<xstop; outx++) {
	  
	  /* Trace ray from specified image space location into map.   */
	  /* Stochastic sampling is as described in adaptive code.     */
	  foutx = (float)(outx);
	  fouty = (float)(outy);
	  pixel_address = IMAGE_ADDRESS(outy,outx);
	  Trace_Ray(outx,outy,foutx,fouty,pixel_address);
	}
      }
      {
#ifdef ATOMIC
        sesc_start_transaction();
#else
#ifdef SIMICS
       SIMICS_START_IGNORE();
       SIMICS_BEGIN_LOCK(&(Global->QLock[local_node]));
#endif
	sesc_lock(&(Global->QLock[local_node]));

#ifdef SIMICS
       SIMICS_LOCKED(&(Global->QLock[local_node]));
       SIMICS_STOP_IGNORE();
#endif
#endif
};
      work = Global->Queue[local_node][0]++;
      {
#ifdef ATOMIC
        sesc_commit_transaction();
#else
#ifdef SIMICS
       SIMICS_START_IGNORE();
#endif
	sesc_unlock(&(Global->QLock[local_node]));

#ifdef SIMICS
       SIMICS_UNLOCK(&(Global->QLock[local_node]));
       SIMICS_STOP_IGNORE();
#endif

#endif
};
    }
    if (my_node == local_node) {
      {
#ifdef ATOMIC
        sesc_start_transaction();
#else
#ifdef SIMICS
       SIMICS_START_IGNORE();
       SIMICS_BEGIN_LOCK(&(Global->QLock[num_nodes]));
#endif
	sesc_lock(&(Global->QLock[num_nodes]));

#ifdef SIMICS
       SIMICS_LOCKED(&(Global->QLock[num_nodes]));
       SIMICS_STOP_IGNORE();
#endif
#endif
};
      Global->Queue[num_nodes][0]--;
      {
#ifdef ATOMIC
        sesc_commit_transaction();
#else
#ifdef SIMICS
       SIMICS_START_IGNORE();
#endif
	sesc_unlock(&(Global->QLock[num_nodes]));

#ifdef SIMICS
       SIMICS_UNLOCK(&(Global->QLock[num_nodes]));
       SIMICS_STOP_IGNORE();
#endif

#endif
};
    }
    local_node = (local_node+1)%num_nodes;
    while (Global->Queue[local_node][0] >= lnum_blocks &&
	   Global->Queue[num_nodes][0] > 0)
      local_node = (local_node+1)%num_nodes;
  }
}


Ray_Trace_Fast_Non_Adaptively(int my_node)
{
  int i,outx,outy,xindex,yindex;
  float foutx,fouty;
  PIXEL *pixel_address;

  for (i=0; i<num_blocks; i+=num_nodes) {
    yindex = ((my_node+i)/num_xblocks)*block_ylen;
    xindex = ((my_node+i)%num_xblocks)*block_xlen;

    for (outy=yindex; outy<yindex+block_ylen &&
         outy<image_len[Y]; outy+=lowest_volume_boxlen) {
      for (outx=xindex; outx<xindex+block_xlen &&
           outx<image_len[X]; outx+=lowest_volume_boxlen) {
      
	/* Trace ray from specified image space location into map.   */
	/* Stochastic sampling is as described in adaptive code.     */
	MASK_IMAGE(outy,outx) += RAY_TRACED;
	foutx = (float)(outx);
	fouty = (float)(outy);
	pixel_address = IMAGE_ADDRESS(outy,outx);
	Trace_Ray(outx,outy,foutx,fouty,pixel_address);
	num_rays_traced++;
      }
    }
  }
}


Interpolate_Recursively(int my_node)
{
  int i,outx,outy,xindex,yindex;
  
  for (i=0; i<num_blocks; i+=num_nodes) {
    yindex = ((my_node+i)/num_xblocks)*block_ylen;
    xindex = ((my_node+i)%num_xblocks)*block_xlen;

    for (outy=yindex; outy<yindex+block_ylen &&
         outy<image_len[Y]; outy+=highest_sampling_boxlen) {
      for (outx=xindex; outx<xindex+block_xlen &&
           outx<image_len[X]; outx+=highest_sampling_boxlen) {

	/* Fill in image within square box of highest sampling size  */
	/* whose lower-left corner is current image space location.  */
	Interpolate_Recursive_Box(outx,outy,highest_sampling_boxlen);
      }
    }
  }
}


Interpolate_Recursive_Box(outx, outy, boxlen)
     int outx, outy, boxlen;
{
  int i,j;
  int half_boxlen;
  int corner_color[2][2],color;
  int outx_plus_boxlen,outy_plus_boxlen;
  
  float one_over_boxlen;
  float xalpha,yalpha;
  float one_minus_xalpha,one_minus_yalpha;
  
  /* Fill in the four pixels at the midpoints of the sides and at      */
  /* the center of the box by bilirping between the four corners,      */
  /* being careful not to exceed the boundaries of the output image,   */
  /* and using a flag array to avoid recomputing any pixels.           */
  /* For diagnostic display, flag is set to a dark gray.               */
  /* By making interpolation follow ray tracing, no pixels along the   */
  /* perimeter are filled in using interpolation, just to be replaced  */
  /* by a more accurate color when adjacent boxes are ray traced.      */
  /* By making the interpolation recursive, it is able to fill in      */
  /* large boxes using all pixels available along their perimeters,    */
  /* rather than just at their four corners.  This prevents "creases", */
  /* or, stated another way, insures zero-order continuity everywhere. */
  half_boxlen = boxlen >> 1;
  one_over_boxlen = 1.0 / (float)boxlen;

  outx_plus_boxlen = outx+boxlen < image_len[X] ? outx+boxlen : outx;
  outy_plus_boxlen = outy+boxlen < image_len[Y] ? outy+boxlen : outy;

  corner_color[0][0] = IMAGE(outy,outx);
  corner_color[0][1] = IMAGE(outy,outx_plus_boxlen);
  corner_color[1][0] = IMAGE(outy_plus_boxlen,outx);
  corner_color[1][1] = IMAGE(outy_plus_boxlen,outx_plus_boxlen);
  for (i=0; i<=boxlen && outy+i<image_len[Y]; i+=half_boxlen) {
    yalpha = (float)i * one_over_boxlen;
    one_minus_yalpha = 1.0 - yalpha;
    for (j=0; j<=boxlen && outx+j<image_len[X]; j+=half_boxlen) {
      xalpha = (float)j * one_over_boxlen;
      one_minus_xalpha = 1.0 - xalpha;
      if (MASK_IMAGE(outy+i,outx+j) == 0) {
	color = corner_color[0][0]*one_minus_xalpha*one_minus_yalpha+
	  corner_color[0][1]*xalpha*one_minus_yalpha+
	    corner_color[1][0]*one_minus_xalpha*yalpha+
	      corner_color[1][1]*xalpha*yalpha;
	IMAGE(outy+i,outx+j) = color;
	MASK_IMAGE(outy+i,outx+j) += INTERPOLATED;
      }
    }
  }

  /* If size of sub-boxes is above lowest size for volume data         */
  /* if polygon list exists or display pixel size if it does not,      */
  /* invoke this function recursively to fill in image within the      */
  /* four equal-sized square sub-boxes enclosed by the current box,    */
  /* being careful not to exceed the boundaries of the output image.   */
  if (half_boxlen > 1) {
    for (i=0; i<boxlen && outy+i<image_len[Y]; i+=half_boxlen) {
      for (j=0; j<boxlen && outx+j<image_len[X]; j+=half_boxlen) {
	Interpolate_Recursive_Box(outx+j,outy+i,half_boxlen);
      }
    }
  }
}








