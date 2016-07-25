#include <device.h>
#include "stdio.h"

#if defined (__GNUC__)
    /* Add an explicit reference to the floating point printf library */
    /* to allow the usage of floating point conversion specifiers. */
    /* This is not linked in by default with the newlib-nano library. */
    asm (".global _printf_float");
#endif

/* The RX_LENGTH of the buffer is equal to maximum packet RX_LENGTH of the 
*  IN and OUT bulk endpoints. 
*/
#define BUFFER_LEN  64u

/* Status' of the RX buffer arrays */
#define RX_ON  1
#define RX_OFF 0

char8 *parity[] = { "None", "Odd", "Even", "Mark", "Space" };
char8 *stop[] = { "1", "1.5", "2" };

void main()
{
    uint16 count;
	int flag = 0;
	int bufferFlag = 0;
    
    /* Receive buffers */
    uint8 rx0[BUFFER_LEN];
    uint8 rx1[BUFFER_LEN];
    
    /* Indexes to track their status */
    uint16 rx0_index = 0; // Rx0 index
    uint16 rx1_index = 0; // Rx1 index
    uint16 buf_index = 0; // buffer index
    
    /* Status of the rx0 and rx 1 buffer */
    int statusRx0 = RX_ON; // Rx0 starts to get filled first
    int statusRx1 = RX_OFF; // Rx1 waits until the Rx0 is full
    
    /* Enable Global Interrupts */
    CyGlobalIntEnable;                        

    /* Start USBFS Operation with 3V operation */
    USBUART_Start(0u, USBUART_3V_OPERATION);

    /* LCD */
    LCD_Start();
    //LCD_PrintString("Test 1");
    
    /* Wait for Device to enumerate */
    while(!USBUART_GetConfiguration());

    /* Enumeration is done, enable OUT endpoint for receive data from Host */
    /* Initializes the CDC interface to be ready for the receive data from the PC */
    USBUART_CDC_Init();

    /* Main Loop: */
    for(;;)
    {
		if(USBUART_DataIsReady() != 0u) {
			if (statusRx0 == RX_ON) {
				count = USBUART_GetAll(rx0);
				
				
				
				USBUART_PutData(rx0, RX_LENGTH);
				statusRx0 = RX_OFF;
				statusRx1 = RX_ON;
				rx0_index = 0; 
				flag = 1;
			}

			else if (statusRx1 == RX_ON) {
				count = USBUART_GetAll(rx1);
			}
				
			if (count == BUFFER_LEN) {
				//flag to print buffer
			}
		}
		
		if okay to print buffer (buffer 1 or buffer 2)
			print buffer;
			flag clear buffer
			
		if flag clear buffer && LCD ready
			print null
			
		
	
	
	
	
	
	
	
		
		/* Print buffer, flag clear buffer */
		if ((flag == 1) && (statusRx0 == RX_ON)) {
			USBUART_PutData(rx0, RX_LENGTH);
			bufferFlag = 1;
			flag = 0;
		}
		
		if ((flag == 1) && (statusRx1 == RX_ON)) {
			USBUART_PutData(rx1, RX_LENGTH);
			bufferFlag = 1;	
			flag = 0;
		}
		
		/* Buffer flag and print null character */
		if (bufferFlag == 1) {
			if (USBUART_CDCIsReady() == 1) {
				USBUART_PutData(NULL, 0u);
			}
			bufferFlag = 0;
		}
    }   
}
/* [] END OF FILE */

