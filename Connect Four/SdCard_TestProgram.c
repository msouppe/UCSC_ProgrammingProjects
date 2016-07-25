/* ========================================
 *
 * Copyright YOUR COMPANY, THE YEAR
 * All Rights Reserved
 * UNPUBLISHED, LICENSED SOFTWARE.
 *
 * CONFIDENTIAL AND PROPRIETARY INFORMATION
 * WHICH IS THE PROPERTY OF your company.
 *
 * ========================================
*/
#include <project.h>
#include <stdio.h>
#include "FS.h"
char acText[50];

int main()
{
    FS_Init();
    //FS_Mount("");
    
    FS_FILE *pFile;
    sprintf(acText, "Connect Four Game Data\n");
    pFile = FS_FOpen("Sprintf.txt", "w");
    int i = 0;
    if (pFile != 0) {
        i++;
        FS_FWrite(acText, 1, strlen(acText), pFile);
        FS_FClose(pFile);
        //FS_Unmount("");
    }
}

/* [] END OF FILE */
