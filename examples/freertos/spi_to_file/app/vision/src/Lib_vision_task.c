

#include <platform.h>
#include "rtos/drivers/intertile/api/rtos_intertile.h"

#if ON_TILE(0)

#include "Lib_vision_task.h"
#include "Vision_api.h"

typedef struct Lib_vision_args_t{
        QueueHandle_t input_queue;
        rtos_intertile_address_t intertile_addr;
        rtos_gpio_t gpio_ctx;
}lib_vision_args_t;

//This file is a lib_vision copy of vision_api

void lib_vision_task(){
/*
In a loop, wait for communciations.
Format communcation into an array.
Define all constants into correct format for the api call
make call
repeat loop
*/
    //char Filepath[] = {'o','u','t','p','u','t','_','i','m','a','g','e','.','b','m','p'};
    rtos_printf("\nInitialized Lib_vision Variables");
    while(1){
        rtos_printf("\n Vision waiting for image...");
        //Get Communications here, uh somehow.
        rtos_printf("\n Vision recieved image.");
    }
  
  rtos_printf("\nCalling Get Image Pointer Here:");
  //writeImage(ai_img_buf,96,96,1, Filepath);
  rtos_printf("\nEnd Call to Get Image Pointer:");
  rtos_printf("\n");

}

void lib_vision_task_create(rtos_intertile_address_t *intertile_addr,unsigned priority){

    rtos_printf("lib_vision_task_create reached\n");
    lib_vision_args_t *args = pvPortMalloc(sizeof(lib_vision_args_t));

    //configASSERT(args);

    args->intertile_addr = *intertile_addr;

    xTaskCreate((TaskFunction_t)lib_vision_task, "lib_vision_task",
            RTOS_THREAD_STACK_SIZE(lib_vision_task), args, priority,
            NULL);
}

#endif //On_Tile(0)