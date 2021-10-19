//This is Steven's attempt to get the lib vision library to work with the Person_Detect SDK example.
//It should be noted that steven does not know what he is doing.

#ifndef Lib_vision_task_H
#define Lib_vision_task_H

//System Headers
#include <platform.h>
#include "rtos/drivers/gpio/api/rtos_gpio.h"


#if ON_TILE(0)

//void lib_vision_task();
void lib_vision_task_create(
        unsigned priority,
        QueueHandle_t input_queueA,
        QueueHandle_t input_queueB);

#endif //On_Tile(0)

#endif  // Vision_Api_H