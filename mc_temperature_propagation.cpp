#include "mpi.h" 
#include <stdio.h> 
#include <stdlib.h>
#include <math.h>
#include "sprng_cpp.h"

#define SEED 985456376

#define XSIZE 20
#define YSIZE 20

int is_border(int x, int y){
    if(x==0 || x==XSIZE || y==YSIZE)
        return 0;
    else if(y==0)
        return 100;
    else
        return -1;
}

int main( int argc, char *argv[] ) 
{ 
    int n = 0, myid = 0, numprocs = 1; 
    int x_init = 10, y_init = 10;
    MPI_Init(&argc,&argv); 
    MPI_Comm_size(MPI_COMM_WORLD,&numprocs); 
    MPI_Comm_rank(MPI_COMM_WORLD,&myid);

    // SPRNG initialization

    Sprng *stream;
    int streamnum = myid;	
    int nstreams = numprocs;
    int gtype = 3;
    MPI_Bcast(&gtype,1,MPI_INT,0,MPI_COMM_WORLD);
    stream = SelectType(gtype);
    stream->init_sprng(streamnum,nstreams,SEED,SPRNG_DEFAULT);

    // Choosing amount of times that we generate
    // a path to the sides of the plate. 
    if (argc >= 4) {  
        n       = atoi(argv[1]);
        x_init  = atoi(argv[2]);
        y_init  = atoi(argv[3]);
    }
    else
    {
        if (myid == 0) { 
            printf("Enter the number o iterations: "); 
            scanf("%d",&n);
            printf("Enter x coordinate of point[1-19]: "); 
            scanf("%d",&x_init);
            printf("Enter y coordinate of point[1-19]: "); 
            scanf("%d",&y_init);
        } 

        // Broadcasting the number of iterations
        MPI_Bcast(&n, 1, MPI_INT, 0, MPI_COMM_WORLD); 
        MPI_Bcast(&x_init, 1, MPI_INT, 0, MPI_COMM_WORLD); 
        MPI_Bcast(&y_init, 1, MPI_INT, 0, MPI_COMM_WORLD); 
    } 

    // Generating the appropriate amount of paths
    // and accumulating the values a the end.
    int sum = 0; 
    for (int i = 0; i <= n/numprocs; i++) {
        int x = x_init;
        int y = y_init; 
        while(1)
        {
            double rn = stream->sprng();
            int dir = static_cast<int>(rn*4);
            switch(dir){
                case 0:
                    x++;
                    break;
                case 1:
                    x--;
                    break;
                case 2:
                    y++;
                    break;
                case 3:
                    y--;
                    break;
            }
            int b = is_border(x, y);
            if(b!=-1)
            {
                sum += b;
                break;
            }
        }
    }
    int sum_main = sum;
    MPI_Reduce(&sum, &sum_main, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD); 

    if (myid == 0)  
        printf("Temperature at point is %.16f.\n", (1.0*sum_main)/((n/numprocs)*numprocs)); 

    stream->free_sprng();
    MPI_Finalize(); 
    return 0; 
} 

