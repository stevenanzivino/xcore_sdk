//This is Steven's attempt to get the lib vision library to work with the Person_Detect SDK example.
//It should be noted that steven does not know what he is doing.

#ifndef Lib_vision_task_H
#define Lib_vision_task_H

//System Headers
#include <platform.h>
#include "rtos/drivers/gpio/api/rtos_gpio.h"

//#include "rtos/drivers/intertile/api/rtos_intertile.h"    //Potential datatypes

// #ifdef vision_args
//     #define EXTERN
// #else
//     #define EXTERN extern
// #endif


// struct Books {
//    char  title[50];
//    char  author[50];
//    char  subject[100];
//    int   book_id;
// } book; 

#if ON_TILE(0)

//void lib_vision_task();
void lib_vision_task_create(
        //rtos_intertile_address_t *intertile_addr,
        //rtos_gpio_t *gpio_ctx,
        unsigned priority,
        QueueHandle_t input_queueA,
        QueueHandle_t input_queueB); //Intertile communcation information



#endif //On_Tile(0)

#endif  // Vision_Api_H