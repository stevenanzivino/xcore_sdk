// Copyright 2020-2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#define DEBUG_UNIT SPI_CAMERA

/* FreeRTOS headers */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

/* Library headers */
#include "rtos/drivers/i2c/api/rtos_i2c_master.h"
#include "rtos/drivers/spi/api/rtos_spi_master.h"

/* App headers */
#include "spi_camera.h"
#include "ov2640.h"
#include "app_conf.h"

typedef struct Output_queues_t{
        QueueHandle_t person_detect_output_q;
        QueueHandle_t vision_output_q;
}output_queues_t;

static void camera_task( void *args )
{
    
    output_queues_t *targs = (output_queues_t *)args;
    QueueHandle_t pd_q = targs->person_detect_output_q;
    QueueHandle_t v_q = targs->vision_output_q;

    //QueueHandle_t output_queue = ( QueueHandle_t ) args;
    uint8_t* img_buf = NULL;
    uint8_t* img_buf2 = NULL;
    uint8_t tx_buf = ARDUCAM_BURST_FIFO_READ;

    /* Setup and start first capture */
    ov2640_flush_fifo();
    ov2640_clear_fifo_flag();
    ov2640_start_capture();
    rtos_printf("\nCamera Task <While>");
    while( 1 )
    {
        img_buf = pvPortMalloc( sizeof( uint8_t ) * IMAGE_BUF_SIZE );                       //Allocate memory equal to image buffer size, give pointer to img_buf
        img_buf2 = pvPortMalloc( sizeof( uint8_t ) * IMAGE_BUF_SIZE );    
        configASSERT( img_buf != NULL );                                                    //Assert allocation success
        configASSERT( img_buf2 != NULL );

        /* Poll until capture is completed */
        while( !ov2640_capture_done() )                                                     //While the capture is not yet completed
        {
            vTaskDelay( pdMS_TO_TICKS( 1 ) );                                               //Wait one ms and check again.
        }

        debug_printf("arducam buffer has %d bytes ready\n", ov2640_read_fifo_length());
        
        ov2640_spi_read_buf( img_buf, IMAGE_BUF_SIZE, &tx_buf, 1 );                         //Reads a buffer from a location???
        //Check the readme, OV2640 is the camera != ARDUCAM.
        //OV camera module, 

        for(int i = 0; i < IMAGE_BUF_SIZE; i++){                                            //Copy data between buffers
            img_buf2[i] = 0;//img_buf[i];
        }

        debug_printf("Arducam -> FreeRTOS done\n");
        rtos_printf("\nCamera task is sending: %d,%d,%d",img_buf[0],img_buf[1],img_buf[2]);

        if( xQueueSend( pd_q, &img_buf, pdMS_TO_TICKS( 1 ) ) == errQUEUE_FULL )     //FreeRTOS memory management?
        {
            debug_printf( "Camera frame lost\n" );
            vPortFree( img_buf );
        }
//*
        if( xQueueSend( v_q, &img_buf2, pdMS_TO_TICKS( 1 ) ) == errQUEUE_FULL )     //FreeRTOS memory management?
        {
            debug_printf( "Camera frame lost\n" );
            vPortFree( img_buf2 );
        }//*/

        ov2640_clear_fifo_flag();
        ov2640_start_capture();
    }
}

static int32_t setup(rtos_spi_master_device_t* spi_dev, rtos_i2c_master_t* i2c_dev)
{
    int32_t retval = pdFALSE;

    retval = ov2640_init( spi_dev, i2c_dev );

    if( retval == pdTRUE )
    {
        retval = ov2640_configure();
    }

    return retval;
}

int32_t create_spi_camera_to_queue( rtos_spi_master_device_t* spi_dev, rtos_i2c_master_t* i2c_dev, UBaseType_t priority, QueueHandle_t pd_q_output, QueueHandle_t vision_q_output )
{
    int32_t retval = pdFALSE;

    output_queues_t *args = pvPortMalloc(sizeof(output_queues_t));

    //configASSERT(args);

    args->person_detect_output_q = pd_q_output;
    args->vision_output_q = vision_q_output;

    /* Setup camera */
    if( setup(spi_dev, i2c_dev) == pdTRUE )
    {
        /* Create camera task to take images and feed them to queue */
        xTaskCreate( camera_task, "camera", portTASK_STACK_DEPTH( camera_task ), args , priority, NULL );
        retval = pdTRUE;
    }

    return retval;
}
