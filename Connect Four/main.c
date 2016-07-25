/*************************************************************************
 *************************************************************************
 *********************** Connect Four_UART/USB_UART **********************
 *************************************************************************
 *************************************************************************/

#include <project.h>
#include <stdio.h>
#include <stdlib.h>
#include "FS.h"
#include <device.h>

#if defined (__GNUC__)
    /* Add an explicit reference to the floating point printf library */
    /* to allow the usage of floating point conversion specifiers. */
    /* This is not linked in by default with the newlib-nano library. */
    asm (".global _printf_float");
#endif

#define OFF         0   
#define ON          1   
#define SIZE        16
#define TRUE        1
#define FALSE       0

uint8 row = 0b000; 
int col;
int discRow;
uint8 tx_buffer[11];
uint8 rx_buffer[11] = {0};
uint8 validRx[11] = {0};
int rxCount = 0;
char buffer[256];
uint16 curHorizontalPos, oldHorizontalPos, capsenseCol, capsenseRow;
int  t = 0, r = 0;
int rx_flag = 0;
int myTurn = 1; // Turn flag 

int ValidPacket(uint8 byte);

/* Global variables for functions for the SD Card */
FS_FILE *pFile;
char gameData[120];

/* Initializing the arrays where [][] = [row][col] */
uint8 red[SIZE][SIZE], blue[SIZE][SIZE], green[SIZE][SIZE], gameBoard[SIZE][SIZE];
int g, h;

/* Interrupt subroutine for LED Panel */
CY_ISR(ISR_LED) {  
    OE_Write(1);
    ControlReg_ABC_Write(row);
    
    for (col = 0; col < 32; col++) {
        ControlReg_Red_Write(0);
        ControlReg_Green_Write(0);
        ControlReg_Blue_Write(0);
        ControlReg_Red2_Write(0);
        ControlReg_Green2_Write(0);
        ControlReg_Blue2_Write(0);
        
        if (col < SIZE) {
            ControlReg_Red_Write(red[row][col]);
            ControlReg_Green_Write(green[row][col]);
            ControlReg_Blue_Write(blue[row][col]);
            ControlReg_Red2_Write(red[row+8][col]);
            ControlReg_Green2_Write(green[row+8][col]);
            ControlReg_Blue2_Write(blue[row+8][col]);
        }        
        Clk_Write(1);
        Clk_Write(0);
    }
    row++;
    
    if (row >= 8) {
        row = 0b000;    
    }
    
    // Latch pulse that you put after the LEDs have been written to
    LAT_Write(1);
    LAT_Write(0);
    OE_Write(0);
}

/* TX Interrupt - sending the tx_buffer all at once */
CY_ISR(ISR_TX) { 
    
    /* Transmitting my packet, which has been initialized */
    UART_WriteTxData(tx_buffer[t]);
    t++;
    
    /* Resets my packets count */
    if (t == 11) {
        t = 0;
    }
}

/* RX Interrupt - constantly receiving the rx_buffer packets */
CY_ISR(ISR_RX) {  
    
    /* Receiving packets from opponent */
    uint8 receivingPacket = UART_GetChar();

	/* Checks each byte from opponent */
    ValidPacket(receivingPacket);    

	/* Turns on opponents turn flag and says that 
	   the packet they just sent me is valid */
    rx_flag = 1;   
}

/* Draws an 16x16 to test the LED panel for debugging purposes */
void Square(uint8 grid[SIZE][SIZE]);

/* Prints out 'I WON'/'YOU WON' when a player wins the game */
void I_WON(uint8 grid[SIZE][SIZE]);
void YOU_WON(uint8 grid[SIZE][SIZE]);

/* Clears the LED Panel and setting all of the LED colors off */
void Clear_LED_Display();

/* This function drops the player's piece in 
   which ever row they decide to play in and 
   also makes sure that the discs stack on top
   of on another.
   
   Returns 0 if a disc hasn't been placed on the board 
   Returns 1 if a disc has been placed on the board 
*/
int Drop_Disc(int discCol, uint8 player[SIZE][SIZE]);

/* Check the game status 
	If return value is 0, game is over 
	If return value is 1, game is still active 
*/
int Game_Status(uint8 joueur[SIZE][SIZE]);

/* Writes the move into the SD Card
   moveRow == players disc row position
   moveCol == players disc column position */
void Document_Move(int pid_4, int moveRow, int moveCol);

/* This function controls where the disc is being placed on the board, 
   whether it is my placing a disc or my opponent. Additionally, this 
   function switches back and forth between the two players. */
void CapSense_DiscDisplacement();

/* Verifying that the packet received from opponent is valid */

/* Prints out data on the USB_UART for debugging purposes */
void USB_UART_PrintData();

int main()
{ 
    /* Initializing the OUTPUT ENABLE for the LED Panel */
    OE_Write(0);
    
    /* Initialization for the USB_UART */
    USBUART_Start(0u, USBUART_3V_OPERATION);
    while(!USBUART_GetConfiguration());
    USBUART_CDC_Init();
    
    /* Initial arrays */
    for (g = 0; g <SIZE ; g++ ) {
        for (h = 0; h <SIZE ; h++ ) {
            red[g][h] = 0;
            blue[g][h] = 0;
            green[g][h] = 0;
            gameBoard[g][h] = 0;
        }
    }
    
    /* Enable all global interrupts */
    CyGlobalIntEnable;
    
    /* Initialization functions that initialize different 
       components for the game such as the Wifi module, UART, 
       CapSense, LCD Screen, USB-UART, Interrupts, and SD Card 
    */
    /* Initializing the CapSense component */
    CapSense_Start();
    CapSense_InitializeAllBaselines();
    
    /* Starting the Timer and LED interrupt */
    Timer_Start();
    isr_Start();
    isr_StartEx(ISR_LED);
	
	/* Initializing the LCD Screen */
    LCD_Start();
    LCD_Position(0,2);
    LCD_PrintString("CONNECT FOUR");
	
	/* Starting the UART */
	UART_Start();
	
    /* Initializing tx_buffer array */
    tx_buffer[0] = 0x55;  // Start of packet
	tx_buffer[1] = 0xAA;  // Start of packet
    tx_buffer[2] = 0x08;  // Length 
                          // My IP address : 192.168.1.101
    tx_buffer[3] = 0xc0;  // Player ID_Byte 3 (192)
	tx_buffer[4] = 0xA8;  // Player ID_Byte 2 (168)
	tx_buffer[5] = 0x01;  // Player ID_Byte 1 (001)
	tx_buffer[6] = 0x65;  // Player ID_Byte 0 (101)
    tx_buffer[7] = 0;     // Player row move
    tx_buffer[8] = 0;     // Player column move
	tx_buffer[9] = 0x99;  // End of packet
	tx_buffer[10] = 0x99; // End of packet
    
	/* Initializing the TX & RX interrupts */
    tx_StartEx(ISR_TX);
    rx_StartEx(ISR_RX);
	
	/* Initializing SD Card */
    FS_Init();
	pFile = FS_FOpen("Game_Data.txt", "w");
    sprintf(gameData, "\tConnect Four Player Log \n\n" );
    if (pFile != 0) {
        FS_FWrite(gameData, 1, strlen(gameData), pFile);
        FS_FClose(pFile);
    }
    
    /* Main for(;;) loop for the program */
    for(;;) {
        
        /* Update all  baselines*/
        CapSense_UpdateEnabledBaselines();
        
        /* Start scanning all enable sensors */
        CapSense_ScanEnabledWidgets();

        /* Wait for scanning to complete */
        while(CapSense_IsBusy() != 0);
        
        /* Display disc on LED panel */
        CapSense_DiscDisplacement();
        
        /* Disc cursor to left and right on top row of game board */
        for (col = 0; col < SIZE; col++) {
            if (col == capsenseCol) {
                red[0][capsenseCol] = ON;
            } else {
                red[0][col] = OFF;
            }     
        }
    }  
    
} // End of int main()

void YOU_WON(uint8 grid[SIZE][SIZE]) {
	// Y
	grid[1][0] = ON;
	grid[2][0] = ON;
	grid[3][0] = ON;
	grid[4][1] = ON;
	grid[5][2] = ON;
	grid[6][2] = ON;
	grid[7][2] = ON;
	grid[4][3] = ON;
	grid[1][4] = ON;
	grid[2][4] = ON;
	grid[3][4] = ON;
	
	// O
	grid[2][6] = ON;
	grid[3][6] = ON;
	grid[4][6] = ON;
	grid[5][6] = ON;
	grid[6][6] = ON;
	grid[1][7] = ON;
	grid[7][7] = ON;
	grid[1][8] = ON;
	grid[7][8] = ON;
	grid[2][9] = ON;
	grid[3][9] = ON;
	grid[4][9] = ON;
	grid[5][9] = ON;
	grid[6][9] = ON;
	
	// U
	grid[1][11] = ON;
	grid[2][11] = ON;
	grid[3][11] = ON;
	grid[4][11] = ON;
	grid[5][11] = ON;
	grid[6][11] = ON;
	grid[7][12] = ON;
	grid[7][13] = ON;
	grid[7][14] = ON;
	grid[1][15] = ON;
	grid[2][15] = ON;
	grid[3][15] = ON;
	grid[4][15] = ON;
	grid[5][15] = ON;
	grid[6][15] = ON;
	
	// W
	grid[9][0] = ON;
	grid[10][0] = ON;
	grid[11][0] = ON;
	grid[12][0] = ON;
	grid[13][0] = ON;
	grid[14][0] = ON;
	grid[15][0] = ON;
	grid[14][1] = ON;
	grid[13][2] = ON;
	grid[14][3] = ON;
	grid[9][4] = ON;
	grid[10][4] = ON;
	grid[11][4] = ON;
	grid[12][4] = ON;
	grid[13][4] = ON;
	grid[14][4] = ON;
	grid[15][4] = ON;
	
	// O
	grid[10][6] = ON;
	grid[11][6] = ON;
	grid[12][6] = ON;
	grid[13][6] = ON;
	grid[14][6] = ON;
	grid[9][7] = ON;
	grid[15][7] = ON;
	grid[9][8] = ON;
	grid[15][8] = ON;
	grid[10][9] = ON;
	grid[11][9] = ON;
	grid[12][9] = ON;
	grid[13][9] = ON;
	grid[14][9] = ON;
	
	// N
	grid[9][11] = ON;
	grid[10][11] = ON;
	grid[11][11] = ON;
	grid[12][11] = ON;
	grid[13][11] = ON;
	grid[14][11] = ON;
	grid[15][11] = ON;
	grid[11][12] = ON;
	grid[12][13] = ON;
	grid[13][14] = ON;
	grid[9][15] = ON;
	grid[10][15] = ON;
	grid[11][15] = ON;
	grid[12][15] = ON;
	grid[13][15] = ON;
	grid[14][15] = ON;
	grid[15][15] = ON;
}

void I_WON(uint8 grid[SIZE][SIZE]) {
    // I
    grid[1][6] = ON;
    grid[1][7] = ON;
    grid[1][8] = ON;
    grid[1][9] = ON;
    grid[1][10] = ON;
    grid[2][8] = ON;
    grid[3][8] = ON;
    grid[4][8] = ON;
    grid[5][8] = ON;
    grid[6][8] = ON;
    grid[7][8] = ON;
    grid[7][9] = ON;
    grid[7][10] = ON;
    grid[7][7] = ON;
    grid[7][6] = ON;
    
    // W
	grid[9][0] = ON;
	grid[10][0] = ON;
	grid[11][0] = ON;
	grid[12][0] = ON;
	grid[13][0] = ON;
	grid[14][0] = ON;
	grid[15][0] = ON;
	grid[14][1] = ON;
	grid[13][2] = ON;
	grid[14][3] = ON;
	grid[9][4] = ON;
	grid[10][4] = ON;
	grid[11][4] = ON;
	grid[12][4] = ON;
	grid[13][4] = ON;
	grid[14][4] = ON;
	grid[15][4] = ON;
	
	// O
	grid[10][6] = ON;
	grid[11][6] = ON;
	grid[12][6] = ON;
	grid[13][6] = ON;
	grid[14][6] = ON;
	grid[9][7] = ON;
	grid[15][7] = ON;
	grid[9][8] = ON;
	grid[15][8] = ON;
	grid[10][9] = ON;
	grid[11][9] = ON;
	grid[12][9] = ON;
	grid[13][9] = ON;
	grid[14][9] = ON;
	
	// N
	grid[9][11] = ON;
	grid[10][11] = ON;
	grid[11][11] = ON;
	grid[12][11] = ON;
	grid[13][11] = ON;
	grid[14][11] = ON;
	grid[15][11] = ON;
	grid[11][12] = ON;
	grid[12][13] = ON;
	grid[13][14] = ON;
	grid[9][15] = ON;
	grid[10][15] = ON;
	grid[11][15] = ON;
	grid[12][15] = ON;
	grid[13][15] = ON;
	grid[14][15] = ON;
	grid[15][15] = ON;
    
}

void Square(uint8 grid[SIZE][SIZE]) {
    int a, b;
    for (a = 0; a < 16; a++) {
        for (b = 0; b < SIZE; b++) {
            grid[a][b] = ON;
        }
    }
}

void Clear_LED_Display() {
    int x, y;
    for (x = 0; x < SIZE; x++) {
        for (y = 0; y < SIZE; y++) {
            red[x][y] = OFF;
            blue[x][y] = OFF;
            green[x][y] = OFF;
        }
    }    
}

int Drop_Disc(int discCol, uint8 player[SIZE][SIZE]) {
    int i; // dummy variable for for loop
    int rowCount = 0; //expected count
    int actCount = 0; // actual count
    for( i = 15 ; i >= 1 ; i--) {
        rowCount++;
        actCount = actCount + gameBoard[i][discCol];
        if (actCount != rowCount) {break;}
    }
    if (actCount == 15) {
        return 0;
    } else {
        discRow = 15-actCount;
        gameBoard[discRow][discCol] = 1; // Keeps track of all of the game pieces on the board and makes sure that the pieces don't overlap
        player[discRow][discCol] = 1;
    }
    return 1;
}

int Game_Status(uint8 joueur[SIZE][SIZE]) {
	int r, c; // dummie variables for loop

    /* Check ROW win */
    for (c = 0; c < 12; c++) {
		for (r = 15; r >= 1; r--) {
			if (joueur[r][c] == 1 && joueur[r][c+1] == 1 && joueur[r][c+2] == 1 && joueur[r][c+3] == 1)
			{
				return 0;
			} 
		}
	}
	
	/* Check COL win */
	for (c = 0; c < 16; c++) {
		for (r = 15; r >= 4; r--) {
			if (joueur[r][c] == 1 && joueur[r-1][c] == 1 && joueur[r-2][c] == 1 && joueur[r-3][c] == 1)
			{
				return 0;
			} 
		}
	}
		
	/* Check DOWN DIAGONAL win */
    for (c = 3; c < 16; c++) {
		for (r = 15; r >= 4; r--) {
			if (joueur[r][c] == 1 && joueur[r-1][c-1] == 1 && joueur[r-2][c-2] == 1 && joueur[r-3][c-3] == 1)
			{
				return 0;
			} 
		}
	}
	
	/* Check UP DIAGONAL win */
    for (c = 0; c < 12; c++) {
		for (r = 15; r >= 4; r--) {
			if (joueur[r][c] == 1 && joueur[r-1][c+1] == 1 && joueur[r-2][c+2] == 1 && joueur[r-3][c+3] == 1)
			{
				return 0;
			} 
		}
	}
	
    return 1;
}

void Document_Move(int pid_4, int moveRow, int moveCol) {
    sprintf(gameData, "Player IP Address: %3d.%3d.%3d.%3d \t Row: %2d \t Column: %2d \n", 
											192, 168, 1, pid_4, moveRow ,moveCol);
    pFile = FS_FOpen("Game_Data.txt", "a");
    if (pFile != 0) {
		FS_FWrite(gameData, 1, strlen(gameData), pFile);
		FS_FClose(pFile);
    }
}

int ValidPacket(uint8 byte) {   
    if ((byte != 0x55) && (r==0)) {
        r = 0;    
        //return 0;
    } else if ((byte != 0xAA) && (r==1)) {
        r = 0;   
        //return 0;
    } else if ((byte != 0x08) && (r==2)) {
        r = 0;   
        //return 0;
    } else if ((byte != 192) && (r==3)) {
        r = 0;
        //return 0;
    } else if ((byte != 168) && (r==4)) {
        r = 0;   
        //return 0;
    } else if ((byte != 1) && (r==5)) {
        r = 0;  
        //return 0;
    } else if ((byte != 146) && (r==6)) {
        r = 0; 
        //return 0;
    } else if (((byte < 1 ) && (byte > 16)) && (r==7)) { // Row
        r = 0; 
        //return 0;
    } else if (((byte < 1) && (byte > 16)) && (r==8)) { // Column
        r = 0;   
        //return 0;
    } else if ((byte != 0x00) && (r==9)) {
        r = 0;   
        //return 0;
    } else if ((byte != 0x00) && (r==10)) {
        r = 0;
        //return 0;
    } else{
        rx_buffer[r] = byte;    
        r++;
    }    
    if (r >= 11) {
        int p;
        for (p = 0; p < 11; p++) {
            validRx[p] = rx_buffer[p];   
        }
        r = 0;
        //return 1;
    }
    return 1;
}

void CapSense_DiscDisplacement() {
    
    /*********************************************************************
    *********************** Controls the disc vertically *****************
    **********************************************************************/
    /* Player 1 - blue LED */
    if (myTurn==1) {
    
         if (CapSense_CheckIsWidgetActive(CapSense_BUTTON0__BTN)) {
            /* Puts disc in corresponding spot on board */
            //if (gameBoard[discRow][capsenseCol] == 0) {
                if (Drop_Disc(capsenseCol, blue) == 1) {
                    
                    /* Updating packets after my move */
                    tx_buffer[7] = 16 - discRow; // transmitting row part of move
                    tx_buffer[8] = capsenseCol + 1; // transmitting column part of move
                    tx_buffer[9] = 0x00; // End of packet
                    tx_buffer[10] = 0x00; // End of packet
                    
                    /* Documenting my moves in SD Card */
                    Document_Move(tx_buffer[6],tx_buffer[7], tx_buffer[8]);
                    
                    /* Switching turns */
                    myTurn = 0;    
                }
            //}
    	}	
        /* Checks if the game is over or still active */
		if (!Game_Status(blue)) {
            Clear_LED_Display();
			I_WON(blue);
		}
        
    }
    
    /* Player 2 - green LED */
    else if ((myTurn==0) && (rx_flag==1)) {        
        
        /* Puts disc in corresponding spot on board */
        // 16-rx_buffer[7] >>> Row 
        // rx_buffer[8]-1  >>> Column 
        if ((validRx[0] == 0x55) && (validRx[1] == 0xAA) && (validRx[2] == 0x08) && (validRx[3] == 192) && 
			(validRx[4] == 168) && (validRx[5] == 1) && (validRx[6] == 146) && (validRx[9] == 0) && (validRx[10] == 0) && 
			(validRx[7] != 0) && (validRx[7] <= 15) && (validRx[8] != 0) && (validRx[8] <=16)) {
            char num[64];
            sprintf(num, "R: %d C: %d", validRx[7], validRx[8]);
            LCD_Position(1,0);
            LCD_PrintString(num);    
            if (gameBoard[15-validRx[7]][validRx[8]-1] == 0) {
                
                if(Drop_Disc(validRx[8]-1, green) == 1){
                    
                    /* Documenting opponent moves in SD Card */
                    Document_Move(validRx[6], validRx[7], validRx[8]);
                    
                    /* Swtiching turns */
                    myTurn = 1;
                    rx_flag = 0;
                }
            }
		}
        /* Checks if the game is over or still active */
		if (!Game_Status(green)) {
            Clear_LED_Display();
			YOU_WON(green);
		}
    }       
    
    /**********************************************************************
     ********************* Controls the disc horizontally *****************
     **********************************************************************/
    
    /* Find slider position */
    curHorizontalPos = CapSense_GetCentroidPos(CapSense_LINEARSLIDER0__LS);
    
    /* Reset position */
		if (curHorizontalPos == 0xFFFF) {
			curHorizontalPos = 0;    
		}
		
		/* Move the LED back and forth */
		if ((curHorizontalPos!= oldHorizontalPos) && (curHorizontalPos != 0)) {
			oldHorizontalPos = curHorizontalPos;
			
			/* Scales the slider position to stay between the range 0-15 */
			curHorizontalPos = (curHorizontalPos / 6); 
			
			if (curHorizontalPos > 15) {
				curHorizontalPos = 15;    
			}

			capsenseCol = curHorizontalPos;
		} 
	
}
