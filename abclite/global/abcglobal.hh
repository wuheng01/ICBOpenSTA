
#pragma once



#define ABCLITE_CONST_64(number) number  ## ULL
#define ABCLITE_CONST_32(number) number  ## U
typedef unsigned word32 ;
typedef  unsigned long long word64 ;
#define ABCLITE_ALLOC(type,num) ((type *) malloc(sizeof(type) * (size_t)(num)))
#define ABCLITE_FREE(t) delete t 


