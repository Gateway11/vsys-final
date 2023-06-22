void transpose(float * x, int row, int col)
{
	int i,j,k;
	float * y;
	
	y = (float*) malloc(sizeof(float)*row*col);
	for(i=0;i<row;i++){
		for(j=0;j<col;j++){
			y[j*row+i] = x[i*col+j];
		}
	}
	for(k=0;k<row*col;k++){
		x[k] = y[k];
	}
	free(y);
}

void scopy(const float * x,int n, float * y)
{
	int i;
	for(i=0;i<n;i++)
		y[i] = x[i];
}
