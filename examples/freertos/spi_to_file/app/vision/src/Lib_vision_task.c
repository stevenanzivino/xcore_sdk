#include <platform.h>
#include "rtos/drivers/intertile/api/rtos_intertile.h"

#if ON_TILE(0)

#include "Lib_vision_task.h"
#include "Vision_api.h"

#define IMAGE_SIZE (96 * 96) //This is also defined in person_detect_task, and must be equivalent.

typedef struct Lib_vision_args_t{
        QueueHandle_t input_queueA;
        QueueHandle_t input_queueB;
}lib_vision_args_t;

void lib_vision_task(void *args){
  vTaskCoreAffinitySet(NULL, 0x01);
  lib_vision_args_t *targs = (lib_vision_args_t *)args;
  QueueHandle_t q = targs->input_queueB;
  uint8_t *img_buf = NULL;
  uint8_t final_img_buf[IMAGE_SIZE];

  //Image writing variables:
  const int width = 96;
  const int height = 96;
  const int channels = 1;
  const char Filepath[] = "./images/output_image.bmp";
  const char Filepath1[] = "./images/output_image1.bmp";
  const char Filepath2[] = "./images/output_image2.bmp";
  const char Filepath3[] = "./images/output_image3.bmp";
  const char Filepath4[] = "./images/output_image4.bmp";
  const char Filepath5[] = "./images/output_image5.bmp";
  /*const char Filepath6[] = "./images/output_image6.bmp";
  const char Filepath7[] = "./images/output_image7.bmp";
  const char Filepath8[] = "./images/output_image8.bmp";
  const char Filepath9[] = "./images/output_image9.bmp";
  const char Filepath10[] = "./images/output_image10.bmp";
  const char Filepath11[] = "./images/output_image11.bmp";
  const char Filepath12[] = "./images/output_image12.bmp";
  const char Filepath13[] = "./images/output_image13.bmp";
  const char Filepath14[] = "./images/output_image14.bmp";//*/

  while(1){
    xQueueReceive(q, &img_buf, portMAX_DELAY);
    /* img_buf[i%2] contains the values we want to print over the api */
    for (int i = 0; i < (IMAGE_SIZE * 2); i++) {
      if ((i % 2)) {
        final_img_buf[i >> 1] = img_buf[i];
      }
    }
    vPortFree(img_buf);
    //debug_printf("\n--------------------A");
    //debug_printf("\nlib vision task is on rtos core %d",rtos_core_id_get());
    //debug_printf("\nlib vision task is on rtos logical core %d",rtos_logical_core_id_get(rtos_core_id_get()));
    ArrayToFile(final_img_buf,width,height,channels,Filepath);
    ArrayToFile(final_img_buf,width,height,channels,Filepath1);
    ArrayToFile(final_img_buf,width,height,channels,Filepath2);
    ArrayToFile(final_img_buf,width,height,channels,Filepath3);
    ArrayToFile(final_img_buf,width,height,channels,Filepath4);
    ArrayToFile(final_img_buf,width,height,channels,Filepath5);
    /*ArrayToFile(final_img_buf,width,height,channels,Filepath6);
    ArrayToFile(final_img_buf,width,height,channels,Filepath7);
    ArrayToFile(final_img_buf,width,height,channels,Filepath8);
    ArrayToFile(final_img_buf,width,height,channels,Filepath9);
    ArrayToFile(final_img_buf,width,height,channels,Filepath10);
    ArrayToFile(final_img_buf,width,height,channels,Filepath11);
    ArrayToFile(final_img_buf,width,height,channels,Filepath12);
    ArrayToFile(final_img_buf,width,height,channels,Filepath13);
    ArrayToFile(final_img_buf,width,height,channels,Filepath14);//*/
  }
}

static void lib_vision_runner_rx(void *args) {
  
  lib_vision_args_t *targs = (lib_vision_args_t *)args;
  QueueHandle_t q_A = targs->input_queueA;
  QueueHandle_t q_B = targs->input_queueB;
  uint8_t *input_tensor;
  int input_tensor_len;

  rtos_printf("\nV_rx <While>");

  while (1) {
    xQueueReceive(q_A, &input_tensor, portMAX_DELAY);

    //rtos_printf("REMOVE THIS PRINT\n"); 
    if( xQueueSend( q_B, &input_tensor, pdMS_TO_TICKS( 1 ) ) == errQUEUE_FULL ){     //FreeRTOS memory management?
        debug_printf( "Camera frame lost\n" );
        vPortFree( input_tensor );
      }
  }
}

void lib_vision_task_create(unsigned priority,QueueHandle_t input_queueA,QueueHandle_t input_queueB){
  lib_vision_args_t *args = pvPortMalloc(sizeof(lib_vision_args_t));
  configASSERT(args);

  args->input_queueA = input_queueA;
  args->input_queueB = input_queueB;

  xTaskCreate((TaskFunction_t)lib_vision_task, "lib_vision_task",
          RTOS_THREAD_STACK_SIZE(lib_vision_task), args, priority,
          NULL);
  xTaskCreate((TaskFunction_t)lib_vision_runner_rx, "lib_vision_runner_rx",
          RTOS_THREAD_STACK_SIZE(lib_vision_runner_rx), args, priority,
          NULL);
}

#endif //On_Tile(0)