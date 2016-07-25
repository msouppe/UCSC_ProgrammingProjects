#include <project.h>

#define OFF     0   
#define ON      1   
#define SIZE    4

uint8 row = 0b000; 
int col; 

/* Initializing the arrays where [][] = [row][col] */
uint8 red[SIZE][SIZE], blue[SIZE][SIZE], green[SIZE][SIZE];

CY_ISR(MyISR) {     
    ControlReg_ABC_Write(row);
    
    for (col = 0; col < 32; col++) {
        if ( col < SIZE) {
            ControlReg_Red_Write(red[row][col]);
            ControlReg_Green_Write(green[row][col]);
            ControlReg_Blue_Write(blue[row][col]);
        } else {
            ControlReg_Red_Write(0);
            ControlReg_Green_Write(0);
            ControlReg_Blue_Write(0);
        }        
        Clk_Write(1);
        Clk_Write(0);
    }
    
    // Latch pulse that you put after the LEDs have been written to
    LAT_Write(1);
    LAT_Write(0);
    OE_Write(0);
    OE_Write(1);
    
    row++;

	/* Make sure staying between row boundaries */
	if (row >= 4) {
		row = 0b000;
	}
}

void FourByFour() {
    int a, b;
    for (a = 0; a < SIZE; a++) {
        for (b = 0; b < SIZE; b++) {
            red[a][b] = ON;
            green[a][b] = ON;
            blue[a][b] = ON;
        }
    }
}

void SingleLEDs() {
    // Cyan [blue and green]
    red[0][0] = OFF;
    green[0][0] = ON;
    blue[0][0] = ON;
    
    // Red 
    red[2][0] = ON;
    green[2][0] = OFF;
    blue[2][0] = OFF;
    
    // Blue 
    red[0][1] = OFF;
    green[0][1] = OFF;
    blue[0][1] = ON;
    
    // Purple [red and blue]
    red[3][2] = ON;
    green[3][2] = OFF;
    blue[3][2] = ON;
    
    // Green
    red[2][2] = OFF;
    green[2][2] = ON;
    blue[2][2] = OFF;
    
    // White [all three]
    red[1][3] = ON;
    green[1][3] = ON;
    blue[1][3] = ON;
    
    // Yellow [red and green]
    red[2][3] = ON;
    green[2][3] = ON;
    blue[2][3] = OFF;
}

int main()
{ 
    OE_Write(0);
    
    CyGlobalIntEnable; /* Uncomment this line to enable global interrupts. */
    Timer_Start();
    isr_Start();
    isr_StartEx(MyISR);
	
    /* Functions to turn on the LEDS */
    //SingleLEDs();
    FourByFour();
}

/* [] END OF FILE */