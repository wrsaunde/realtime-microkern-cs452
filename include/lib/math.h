#ifndef __LIB_MATH_H__
#define __LIB_MATH_H__

//library functions for math

//absolute value
#define ABS(num) (((num) < 0) ? -(num) : (num))

//square
#define SQUARE(num) (num * num)

//max and min
#define MAX(num1,num2) ((num1 < num2) ?  (num2) : (num1))
#define MIN(num1,num2) ((num1 < num2) ?  (num1) : (num2))

//square root
#define SQRT(num) math_sqrt(num)
int math_sqrt(int x);

#endif
