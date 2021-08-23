//This is Steven's attempt to get the lib vision library to work with the Person_Detect SDK example.
//It should be noted that steven does not know what he is doing.
//https://isocpp.org/wiki/faq/mixing-c-and-cpp#cpp-higher-than-c

#ifndef Vision_Api_H
#define Vision_Api_H

void* CreateImage(int*, int,int,int);

void* GetImagePointer(int* DataPtr, int width, int height, int channels){
    return(CreateImage(DataPtr, width, height, channels));
}

#endif  // Vision_Api_H