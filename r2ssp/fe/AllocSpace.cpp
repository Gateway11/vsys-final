//
#include "AllocSpace.h"
#include <string.h>

/*
 * Matrix allocation routines.  
 * To allocate a 10 by 10 matrix of floats use:
 *
 *	char **alloc2d();
 *	float **x;
 *
 *	x = (float **) alloc2d(10, 10, sizeof(float));
 *
 * To free this matrix use:
 *
 *	free2d(x);
 */
	
char **Alloc2d(int dim1, int dim2,int size)
{
	int		i;
	unsigned	nelem;
	char		*p, **pp;

	nelem = (unsigned) dim1*dim2;

	p = (char *)calloc(nelem, (unsigned) size);
    
	if( p == NULL ) 
	{
		return(NULL);
	}
    memset(p,0,nelem);
	pp = (char **)calloc((unsigned) dim1, (unsigned) sizeof(char *));

	if (pp == NULL)
	{
		free(p);
		return( NULL );
	}

	for(i=0; i<dim1; i++)
		pp[i] = p + i*dim2*size; 

	return(pp);	
}


int Free2d(char **mat )

{
	if (mat != NULL && *mat != NULL)
		free((char *) *mat);
	if (mat != NULL)
		free((char *) mat); 
	return(0);
}


char ***Alloc3d(int dim1,int dim2, int dim3, int size )
{
	int	i;
	char	**pp, ***ppp;
	pp = (char **) Alloc2d(dim1*dim2,dim3, size);

	if(pp == NULL) {
	
		return(NULL);
	}

	ppp = (char ***) calloc((unsigned) dim1, (unsigned) sizeof(char **));
	if(ppp == NULL)
	{
		Free2d(pp);
		return(NULL);
	}

	for(i=0; i< dim1; i++)
		ppp[i] = pp + i*dim2 ;
	return(ppp);
}


int Free3d(char ***mat)
{
	Free2d( *mat );
	if (mat != NULL)
	   free((char *) mat);
	return(0);
}
