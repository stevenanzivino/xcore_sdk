//This is Steven's attempt to get the lib vision library to work with the Person_Detect SDK example.
//It should be noted that steven does not know what he is doing.
//https://isocpp.org/wiki/faq/mixing-c-and-cpp#cpp-higher-than-c

#ifndef Vision_Api_H
#define Vision_Api_H

//System Headers
#include <platform.h>

#include <stdint.h>

#if ON_TILE(0)

void ArrayToFile(uint8_t*, int, int, int, char*);


void Vision_API_Void(){

}

#endif //On_Tile(0)

#endif  // Vision_Api_H