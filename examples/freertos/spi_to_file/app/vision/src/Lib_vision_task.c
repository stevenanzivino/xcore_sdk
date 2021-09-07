

#include <platform.h>
#include "Lib_vision_task.h"
#include "Vision_api.h"

#if ON_TILE(0)

//This file is a lib_vision copy of vision_api

void lib_vision_task(){
/*
In a loop, wait for communciations.
Format communcation into an array.
Define all constants into correct format for the api call
make call
repeat loop
*/
    char Filepath[] = {'o','u','t','p','u','t','_','i','m','a','g','e','.','b','m','p'};
    rtos_printf("\nInitialized Lib_vision Variables");
    
    
    while(1){
        rtos_printf("\n Vision waiting for image...")
        //Get Communications here, uh somehow.
        rtos_printf("\n Vision recieved image.")


    }
  
  rtos_printf("\nCalling Get Image Pointer Here:");
  writeImage(ai_img_buf,96,96,1, Filepath);
  rtos_printf("\nEnd Call to Get Image Pointer:");
  rtos_printf("\n");

}

void lib_vision_task_create(){


    xTaskCreate((TaskFunction_t)lib_vision_task, "lib_vision_task",
            RTOS_THREAD_STACK_SIZE(lib_vision_task), args, priority,
            NULL);
}

#endif //On_Tile(0)