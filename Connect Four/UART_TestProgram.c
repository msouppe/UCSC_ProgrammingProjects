/* ========================================
 *
 *  Name: Souppe, Mariette
 *  Class: CMPE 121/L
 *  Project: Exercise 3, Part 2
 *  Date: Oct 28, 2014
 *
 * ========================================
*/
#include <project.h>

/* Global variables throughout the program*/
int flag = 0;
int t = 0, r = 0, c, dataCheck = 0;

/* Keeps track of how many times entered in each ISR */
int rCount = 0, tCount = 0;

/* Define array size */
#define size 4095 

uint8 transmit[size];
uint8 receive[size] = {0};

/* Transmit or TX Interrupt */
CY_ISR(MyISR1) {
    while (UART_GetTxBufferSize() != 4) {
        UART_WriteTxData(transmit[t]);
        t++;
        if ( t == size ){
            tx_Stop();
        }
    }
    tCount++;
}

/* Receiver or RX Interrupt */
CY_ISR(MyISR2) { 
    receive[r] = UART_ReadRxData(); 
    r++;
    if ( r == size ){
        flag = 1;
        rx_Stop();
    }
    rCount++;
}

int main()
{
    UART_Start();
    
    int i;
    for (i = 0; i < size; i++) {
        transmit[i] = i % 256;
    } 
    
    CyGlobalIntEnable; /* Uncomment this line to enable global interrupts. */
    
    tx_StartEx(MyISR1);
    rx_StartEx(MyISR2);
   
    for(;;)
    {
        /* Checking for the flag */
        if (flag == 1) {
            flag = 0;  
        }
        break;
    }   
    
    /* Checking if the transmit[] and receive[] are equal to one another */
    for (c = 0; c < size; c++) {
        if (transmit[c] != receive[c]) {
            dataCheck = 1;
        }
        else { 
            dataCheck = 0;
            break;
        }
    }    
    
    /* Printing onto the LCD Screen */
    LCD_ClearDisplay();
    for (;;) {
        if (dataCheck == 0) {
            LCD_Position(0,0);
            LCD_PrintString("TransferComplete");
            LCD_Position(1,0);
            
            /* Transmit ISR count and Receiver ISR count*/
            LCD_PrintString("TX: ");
            LCD_Position(1,3);
            LCD_PrintNumber(tCount);
            LCD_Position(1,8);
            LCD_PrintString("RX: ");
            LCD_Position(1,11);
            LCD_PrintNumber(rCount);
        }
        else {
            LCD_Position(0,0);
            LCD_PrintString("TransferFail");
        }
    }
    
    
}

/* [] END OF FILE */
