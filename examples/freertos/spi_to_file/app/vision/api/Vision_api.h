#ifndef Vision_Api_H
#define Vision_Api_H

//System Headers
#include <platform.h>

#include <stdint.h>

#if ON_TILE(0)

void ArrayToFile(uint8_t*, int, int, int, char*);

#endif //On_Tile(0)

#endif // Vision_Api_H