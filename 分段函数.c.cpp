#include <stdio.h>
int main()
{
    double x;
    double result;
    scanf("%lf",&x);
    if(x!=0){
        
        result=1.0/x;
               

    }
    else{
        result=0;
       
    }
     printf("f(%.1f) = %.1f",x,result);
    return 0;
}
