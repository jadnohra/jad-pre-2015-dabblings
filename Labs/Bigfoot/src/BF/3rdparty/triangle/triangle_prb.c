/* 
  If SINGLE is defined when triangle.o is compiled, it should also be
  defined here.  If not, it should not be defined here.
*/

/* #define SINGLE */

#ifdef SINGLE
#define REAL float
#else /* not SINGLE */
#define REAL double
#endif /* not SINGLE */

#include <stdio.h>
#include "triangle.h"

#ifndef _STDLIB_H_
extern void *malloc();
extern void free();
#endif /* _STDLIB_H_ */

int main ( void );

void report ( struct triangulateio *io, int markers, int reporttriangles, 
  int reportneighbors, int reportsegments, int reportedges, int reportnorms );

/*****************************************************************************/

int main ( void )

/*****************************************************************************/
/*
  Purpose:

    MAIN creates and refines a mesh.
*/
{
  struct triangulateio in;
  struct triangulateio mid;
  struct triangulateio out;
  struct triangulateio vorout;
/* 
  Define the input points. 
*/

  in.numberofpoints = 4;
  in.numberofpointattributes = 1;
  in.pointlist = (REAL *) malloc(in.numberofpoints * 2 * sizeof(REAL));
  in.pointlist[0] = 0.0;
  in.pointlist[1] = 0.0;
  in.pointlist[2] = 1.0;
  in.pointlist[3] = 0.0;
  in.pointlist[4] = 1.0;
  in.pointlist[5] = 10.0;
  in.pointlist[6] = 0.0;
  in.pointlist[7] = 10.0;
  in.pointattributelist = (REAL *) malloc(in.numberofpoints *
                                          in.numberofpointattributes *
                                          sizeof(REAL));
  in.pointattributelist[0] = 0.0;
  in.pointattributelist[1] = 1.0;
  in.pointattributelist[2] = 11.0;
  in.pointattributelist[3] = 10.0;
  in.pointmarkerlist = (int *) malloc(in.numberofpoints * sizeof(int));
  in.pointmarkerlist[0] = 0;
  in.pointmarkerlist[1] = 2;
  in.pointmarkerlist[2] = 0;
  in.pointmarkerlist[3] = 0;

  in.numberofsegments = 0;
  in.numberofholes = 0;
  in.numberofregions = 1;
  in.regionlist = (REAL *) malloc(in.numberofregions * 4 * sizeof(REAL));
  in.regionlist[0] = 0.5;
  in.regionlist[1] = 5.0;
/* 
  Regional attribute (for whole mesh). 
*/
  in.regionlist[2] = 7.0;
/* 
  Area constraint that will not be used. 
*/            
  in.regionlist[3] = 0.1;          

  printf("Input point set:\n\n");

  report ( &in, 1, 0, 0, 0, 0, 0 );
/* 
  Make necessary initializations so that Triangle can return a 
  triangulation in `mid' and a Voronoi diagram in `vorout'.  
*/
  mid.pointlist = (REAL *) NULL;            
  mid.pointattributelist = (REAL *) NULL;
  mid.pointmarkerlist = (int *) NULL; 
  mid.trianglelist = (int *) NULL;
  mid.triangleattributelist = (REAL *) NULL;
  mid.neighborlist = (int *) NULL;
  mid.segmentlist = (int *) NULL;
  mid.segmentmarkerlist = (int *) NULL;
  mid.edgelist = (int *) NULL;
  mid.edgemarkerlist = (int *) NULL;

  vorout.pointlist = (REAL *) NULL;
  vorout.pointattributelist = (REAL *) NULL;
  vorout.edgelist = (int *) NULL;
  vorout.normlist = (REAL *) NULL;
/* 
  Triangulate the points.  Switches are chosen to 
    read and write a PSLG (p), 
    preserve the convex hull (c), 
    number everything from zero (z), 
    assign a regional attribute to each element (A),
    produce an edge list (e), 
    produce a Voronoi diagram (v),
    produce a triangle neighbor list (n).
*/
  triangulate ( "pczAevn", &in, &mid, &vorout );

  printf("Initial triangulation:\n\n");

  report ( &mid, 1, 1, 1, 1, 1, 0 );

  printf("Initial Voronoi diagram:\n\n");

  report ( &vorout, 0, 0, 0, 0, 1, 1 );
/* 
  Attach area constraints to the triangles in preparation for
  refining the triangulation.
*/
  mid.trianglearealist = (REAL *) malloc(mid.numberoftriangles * sizeof(REAL) );
  mid.trianglearealist[0] = 3.0;
  mid.trianglearealist[1] = 1.0;
/* 
  Make necessary initializations so that Triangle can return a
  triangulation in `out'.                                    
*/
  out.pointlist = (REAL *) NULL;
  out.pointattributelist = (REAL *) NULL;
  out.trianglelist = (int *) NULL;          
  out.triangleattributelist = (REAL *) NULL;
/* 
  Refine the triangulation according to the attached 
  triangle area constraints.                       
*/
  triangulate ( "prazBP", &mid, &out, (struct triangulateio *) NULL );

  printf("Refined triangulation:\n\n");

  report ( &out, 0, 1, 0, 0, 0, 0 );
/* 
  Free all allocated arrays, including those allocated by Triangle. 
*/
  free(in.pointlist);
  free(in.pointattributelist);
  free(in.pointmarkerlist);
  free(in.regionlist);

  free(mid.pointlist);
  free(mid.pointattributelist);
  free(mid.pointmarkerlist);
  free(mid.trianglelist);
  free(mid.triangleattributelist);
  free(mid.trianglearealist);
  free(mid.neighborlist);
  free(mid.segmentlist);
  free(mid.segmentmarkerlist);
  free(mid.edgelist);
  free(mid.edgemarkerlist);

  free(vorout.pointlist);
  free(vorout.pointattributelist);
  free(vorout.edgelist);
  free(vorout.normlist);

  free(out.pointlist);
  free(out.pointattributelist);
  free(out.trianglelist);
  free(out.triangleattributelist);

  return 0;
}
/*****************************************************************************/

void report ( struct triangulateio *io, int markers, int reporttriangles, 
  int reportneighbors, int reportsegments, int reportedges, int reportnorms )

/*****************************************************************************/
/* 
  Purpose:

    REPORT prints information about a triangulation.
*/ 
{
  int i;
  int j;

  for ( i = 0; i < io->numberofpoints; i++ ) 
  {
    printf("Point %4d:", i);
    for (j = 0; j < 2; j++)
    {
      printf("  %.6g", io->pointlist[i * 2 + j]);
    }
    if (io->numberofpointattributes > 0)
    {
      printf("   attributes");
    }
    for (j = 0; j < io->numberofpointattributes; j++)
    {
      printf("  %.6g",
             io->pointattributelist[i * io->numberofpointattributes + j]);
    }
    if (markers)
    {
      printf("   marker %d\n", io->pointmarkerlist[i]);
    }
    else
    {
      printf("\n");
    }
  }
  printf("\n");

  if ( reporttriangles || reportneighbors )
  {
    for ( i = 0; i < io->numberoftriangles; i++ )
    {
      if (reporttriangles)
      {
        printf("Triangle %4d points:", i);
        for (j = 0; j < io->numberofcorners; j++)
        {
          printf("  %4d", io->trianglelist[i * io->numberofcorners + j]);
        }
        if (io->numberoftriangleattributes > 0)
        {
          printf("   attributes");
        }
        for (j = 0; j < io->numberoftriangleattributes; j++)
        {
          printf("  %.6g", io->triangleattributelist[i *
                                         io->numberoftriangleattributes + j]);
        }
        printf("\n");
      }
      if (reportneighbors)
      {
        printf("Triangle %4d neighbors:", i);
        for (j = 0; j < 3; j++)
        {
          printf("  %4d", io->neighborlist[i * 3 + j]);
        }
        printf("\n");
      }
    }
    printf("\n");
  }

  if ( reportsegments )
  {
    for (i = 0; i < io->numberofsegments; i++)
    {
      printf("Segment %4d points:", i);
      for (j = 0; j < 2; j++)
      {
        printf("  %4d", io->segmentlist[i * 2 + j]);
      }
      if ( markers )
      {
        printf("   marker %d\n", io->segmentmarkerlist[i]);
      } 
      else
      {
        printf ( "\n" );
      }
    }
    printf ( "\n" );
  }

  if ( reportedges )
  {
    for ( i = 0; i < io->numberofedges; i++ )
    {
      printf("Edge %4d points:", i);
      for ( j = 0; j < 2; j++)
      {
        printf("  %4d", io->edgelist[i * 2 + j] );
      }
      if ( reportnorms && (io->edgelist[i * 2 + 1] == -1 ) )
      {
        for (j = 0; j < 2; j++)
        {
          printf("  %.6g", io->normlist[i * 2 + j] );
        }
      }
      if ( markers )
      {
        printf("   marker %d\n", io->edgemarkerlist[i] );
      }
      else
      {
        printf ( "\n" );
      }
    }
    printf ( "\n" );
  }
}
