#include "mpi.h" 
#include <stdio.h> 
#include <stdlib.h>
#include <math.h>
#include "sprng_cpp.h"

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
    int myid = 0, numprocs = 1;
    double s = 0.01; 
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
    stream->init_sprng(streamnum,nstreams,make_sprng_seed(),SPRNG_DEFAULT);

    // Choosing amount of times that we generate
    // a path to the sides of the plate. 
    if (argc >= 4) {  
        s       = atof(argv[1]);
        x_init  = atoi(argv[2]);
        y_init  = atoi(argv[3]);
    }
    else
    {
        if (myid == 0) { 
            printf("Enter sensitivity for end condition (ex. 0.01): "); 
            scanf("%f",&s);
            printf("Enter x coordinate of point[1-19]: "); 
            scanf("%d",&x_init);
            printf("Enter y coordinate of point[1-19]: "); 
            scanf("%d",&y_init);
        } 

        // Broadcasting the number of iterations
        // MPI_Bcast(&s, 1, MPI_INT, 0, MPI_COMM_WORLD); 
        MPI_Bcast(&x_init, 1, MPI_INT, 0, MPI_COMM_WORLD); 
        MPI_Bcast(&y_init, 1, MPI_INT, 0, MPI_COMM_WORLD); 
    } 

    if (myid == 0) { 
        printf("Iterations:\n");
    } 

    // Generating the appropriate amount of paths
    // and accumulating the values a the end.
    bool finalize = false;
    int i = 0;
    int consec_conv = 0;
    int sum_main = 0;
    int sum = 0; 
    double ref = 0.0;
    while(1) {
        i++;
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
        MPI_Reduce(&sum, &sum_main, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);    
        if (myid == 0)
        {
            if(fabs((1.0*sum_main)/(i*numprocs) - ref) < s && sum_main!=0)
            {
                if(consec_conv==0)
                    ref = (1.0*sum_main)/(i*numprocs);
                else if(consec_conv==20000)
                {
                    ref = (1.0*sum_main)/(i*numprocs);
                    finalize = true; 
                }
                consec_conv++;
            }
            else
            {
                consec_conv = 0;
                ref = (1.0*sum_main)/(i*numprocs);
            }
            if(i%(10000)==0)
                printf("%dx%d ", i, numprocs);
        }
        MPI_Bcast(&finalize, 1, MPI_C_BOOL, 0, MPI_COMM_WORLD);
        if(finalize)
            break;
    }
    
    if (myid == 0)
    {  
        printf("\nTemperature at point is %.16f.\n", ref);
        printf("Calculated after %d iterations\n", i*numprocs); 
    }

    stream->free_sprng();
    MPI_Finalize(); 
    return 0; 
} 

