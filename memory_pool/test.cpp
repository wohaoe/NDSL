#include "Pool.h"

typedef struct student_st
{
   char name[10];
   int age;
}CStudent;

int main()
{
   dushuang::Pool<> student_pool(sizeof(CStudent));
   CStudent * const obj=(CStudent *)student_pool.malloc();
   student_pool.free(obj);
   return 0;
}