#include <iostream>
#include <iomanip>
#include <cstdlib>
#include <random>
#include <upcxx/upcxx.hpp>
using namespace std;

#define XSIZE 20
#define YSIZE 20

int is_border(int x, int y)
{
    if (x == 0 || x == XSIZE || y == YSIZE)
        return 0;
    else if (y == 0)
        return 100;
    else
        return -1;
}

int main(int argc, char **argv)
{
    double s = 0.01;
    int x_init = 10, y_init = 10;
    upcxx::init();
    srand(upcxx::rank_me());
    if (argc == 4)
    {
        s = atof(argv[1]);
        x_init = atoi(argv[2]);
        y_init = atoi(argv[3]);
    }
    else
    {
        if(upcxx::rank_me() == 0)
            cout<<"Pass the following values: \nStability sensitivity, x coordinate of measured point, y coordinate of measured point"<<endl;
        return 0;
    }

    upcxx::global_ptr<bool> finalize = upcxx::new_<bool>(false);
    int64_t i = 0;
    int64_t consec_conv = 0;
    int64_t sum_main = 0;
    int64_t sum = 0;
    double ref = 0.0;
    while (1)
    {
        i++;
        int x = x_init;
        int y = y_init;
        while (1)
        {
            double rn = static_cast<double>(rand()) / RAND_MAX;
            int dir = static_cast<int>(rn * 4);
            switch (dir)
            {
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
            if (b != -1)
            {
                sum += b;
                break;
            }
        }
        sum_main = upcxx::reduce_all(sum, upcxx::op_fast_add).wait();
        if (upcxx::rank_me() == 0)
        {
            if (fabs((1.0 * sum_main) / (i * upcxx::rank_n()) - ref) < s && sum_main != 0)
            {
                if (consec_conv == 0)
                    ref = (1.0 * sum_main) / (i * upcxx::rank_n());
                else if (consec_conv == 20000)
                {
                    ref = (1.0 * sum_main) / (i * upcxx::rank_n());
                    upcxx::rput(true, finalize).wait();
                }
                consec_conv++;
            }
            else
            {
                consec_conv = 0;
                ref = (1.0 * sum_main) / (i * upcxx::rank_n());
            }
        }
        finalize = upcxx::broadcast(finalize, 0).wait();
        if (upcxx::rget(finalize).wait())
            break;
    }

    if (upcxx::rank_me() == 0)
    {
        printf("\nTemperature at point is %.16f.\n", ref);
        printf("Calculated after %d iterations\n", i * upcxx::rank_n());
    }

    upcxx::finalize();
    return 0;
}
