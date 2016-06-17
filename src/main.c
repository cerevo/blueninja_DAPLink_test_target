/**
 * @file   main.c
 * @brief  Application main.
 *
 * @author Cerevo Inc.
 */

/*
Copyright 2015 Cerevo Inc.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>

#include "TZ10xx.h"
#include "PMU_TZ10xx.h"
#include "Driver_UART.h"

#include "TZ01_system.h"

#include "utils.h"

extern TZ10XX_DRIVER_PMU  Driver_PMU;
extern ARM_DRIVER_UART Driver_UART1;

uint8_t buff[256];
uint32_t idx = 0;

int main(void)
{
    uint8_t b;
    uint32_t baud;
    ARM_UART_STATUS stat;
    /* Initialize */
    TZ01_system_init();
    
    Driver_PMU.SelectClockSource(PMU_CSM_UART1, PMU_CLOCK_SOURCE_OSC12M);
    Driver_PMU.SetPrescaler(PMU_CD_UART1, 1);
    
    Driver_UART1.Initialize(0, 0);
    Driver_UART1.Configure(TZ01_CONSOLE_BAUD, 8, ARM_UART_PARITY_NONE, ARM_UART_STOP_BITS_1, ARM_UART_FLOW_CONTROL_NONE);
    Driver_UART1.PowerControl(ARM_POWER_FULL);
    
    Driver_UART1.WriteData("{init}", 6);
    for (;;) {
        if (Driver_UART1.DataAvailable() == true) {
            Driver_UART1.ReadData(&b, 1);
            Driver_UART1.WriteData(&b, 1);
            
            switch (b) {
            case '{':
                idx = 0;
                buff[idx++] = b;
                break;
            case '}':
                buff[idx++] = b;
                buff[idx++] = '\0';
                idx = 0;
                
                if (sscanf((char *)buff, "{baud:%i}", &baud) == 1) {
                    Usleep(10000);  //10ms
                    stat = Driver_UART1.Configure(baud, 8, ARM_UART_PARITY_NONE, ARM_UART_STOP_BITS_1, ARM_UART_FLOW_CONTROL_NONE);
                    switch (stat) {
                    case ARM_UART_OK:
                        Usleep(100000); //100ms
                        Driver_UART1.WriteData("{change}", 8);
                        break;
                    case ARM_UART_ERROR_BAUDRATE:
                        Driver_UART1.WriteData("{unsupported}", 14);
                        break;
                    default:
                        Driver_UART1.WriteData("{failed}", 8);
                        break;
                    }
                }
                break;
            default:
                buff[idx++] = b;
                if (idx >= sizeof(buff)) {
                    idx = 0;
                }
                break;
            }
        }
    }

    return 0;
}
