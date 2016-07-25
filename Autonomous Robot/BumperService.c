
/*
 * File: TemplateService.h
 * Author: J. Edward Carryer
 * Modified: Gabriel H Elkaim
 *
 * Template file to set up a simple service to work with the Events and Services
 * Framework (ES_Framework) on the Uno32 for the CMPE-118/L class. Note that this file
 * will need to be modified to fit your exact needs, and most of the names will have
 * to be changed to match your code.
 *
 * This is provided as an example and a good place to start.
 *
 * Created on 23/Oct/2011
 * Updated on 13/Nov/2013
 */

/*******************************************************************************
 * MODULE #INCLUDE                                                             *
 ******************************************************************************/

//#include "ES_Configure.h"
//#include "ES_Framework.h"
//#include "BOARD.h"
//#include "Ewok.h"
//#include "stdio.h"
//#include "EwokHSM.h"

#include <stdio.h>
#include "ES_Configure.h"
#include "ES_Framework.h"
#include "BOARD.h"
#include "EwokHSM.h"
#include "AD.h"
#include "IO_Ports.h"
#include "LED.h"
#include "BumperService.h"

/*******************************************************************************
 * MODULE #DEFINES                                                             *
 ******************************************************************************/
#define HIT 5
#define NO_HIT 9
#define CHECK_BUMPER_RATE 10
#define BUMPER_PORT PORTZ
#define FRONT_LEFT_BUMPER PIN5
#define FRONT_RIGHT_BUMPER PIN7
/*******************************************************************************
 * PRIVATE FUNCTION PROTOTYPES                                                 *
 ******************************************************************************/
/* Prototypes for private functions for this machine. They should be functions
   relevant to the behavior of this state machine */

/*******************************************************************************
 * PRIVATE MODULE VARIABLES                                                    *
 ******************************************************************************/
/* You will need MyPriority and maybe a state variable; you may need others
 * as well. */


static uint8_t MyPriority;


/*******************************************************************************
 * PUBLIC FUNCTIONS                                                            *
 ******************************************************************************/

/**
 * @Function InitTemplateService(uint8_t Priority)
 * @param Priority - internal variable to track which event queue to use
 * @return TRUE or FALSE
 * @brief This will get called by the framework at the beginning of the code
 *        execution. It will post an ES_INIT event to the appropriate event
 *        queue, which will be handled inside RunTemplateService function. Remember
 *        to rename this to something appropriate.
 *        Returns TRUE if successful, FALSE otherwise
 * @author J. Edward Carryer, 2011.10.23 19:25 */
uint8_t InitBumperService(uint8_t Priority) {
    ES_Event ThisEvent;
    MyPriority = Priority;

    // in here you write your initialization code
    // this includes all hardware and software initialization
    // that needs to occur.

    //ES_Timer_Init();
    
    LED_Init();
    LED_AddBanks(LED_BANK3);
    LED_SetBank(LED_BANK3, 0x00);
    //IO_PortsSetPortInputs(BUMPER_PORT, FRONT_LEFT_BUMPER | FRONT_RIGHT_BUMPER);
    ES_Timer_InitTimer(BUMPER_SERVICE_TIMER, CHECK_BUMPER_RATE);
    // post the initial transition event
    ThisEvent.EventType = ES_INIT;
    if (ES_PostToService(MyPriority, ThisEvent) == TRUE) {
        return TRUE;
    } else {
        return FALSE;
    }
}

/**
 * @Function PostTemplateService(ES_Event ThisEvent)
 * @param ThisEvent - the event (type and param) to be posted to queue
 * @return TRUE or FALSE
 * @brief This function is a wrapper to the queue posting function, and its name
 *        will be used inside ES_Configure to point to which queue events should
 *        be posted to. Remember to rename to something appropriate.
 *        Returns TRUE if successful, FALSE otherwise
 * @author J. Edward Carryer, 2011.10.23 19:25 */
uint8_t PostBumperService(ES_Event ThisEvent) {
    return ES_PostToService(MyPriority, ThisEvent);
}

/**
 * @Function RunTemplateService(ES_Event ThisEvent)
 * @param ThisEvent - the event (type and param) to be responded.
 * @return Event - return event (type and param), in general should be ES_NO_EVENT
 * @brief This function is where you implement the whole of the service,
 *        as this is called any time a new event is passed to the event queue.
 * @note Remember to rename to something appropriate.
 *       Returns ES_NO_EVENT if the event have been "consumed."
 * @author J. Edward Carryer, 2011.10.23 19:25 */
ES_Event RunBumperService(ES_Event ThisEvent) {
    ES_Event ReturnEvent;
    ES_Event thisEvent;
    static long bumpValue = 0;
    static unsigned char prevBumpState = 0;
    unsigned char currentBumpState;
   

    

    ReturnEvent.EventType = ES_NO_EVENT; // assume no errors



    if(ThisEvent.EventType != ES_TIMEOUT) return ReturnEvent;
    //printf("In bumper service\n");

    if (ThisEvent.EventType == ES_INIT)// only respond to ES_Init
    {
        // No hardware initialization or single time setups, those
        // go in the init function above.
        // This section is used to reset service for some reason
    }
    

        currentBumpState = Ewok_ReadBumpers();
        bumpValue = bumpValue << 2;
        bumpValue |= currentBumpState;
        if (bumpValue == 0x55555555) { /*FRONT LEFT TRIGGERED*/
            if (currentBumpState != prevBumpState) {
                printf("LEFT BUMP -> ");
                thisEvent.EventType = BUMP_CHANGE;
                LED_SetBank(LED_BANK3, 0x01);
                thisEvent.EventParam = (uint16_t) currentBumpState;
                PostEwokHSM(thisEvent);
                prevBumpState = currentBumpState;
            }
        }
        if (bumpValue == 0xAAAAAAAA) { /*FRONT RIGHT TRIGGERED*/
            if (currentBumpState != prevBumpState) {
                printf("RIGHT BUMP -> ");
                thisEvent.EventType = BUMP_CHANGE;
                LED_SetBank(LED_BANK3, 0x08);
                thisEvent.EventParam = (uint16_t) currentBumpState;
                PostEwokHSM(thisEvent);
                prevBumpState = currentBumpState;
            }
        }

        if (bumpValue == 0xFFFFFFFF) { /*BOTH TRIGGERED*/
            if (currentBumpState != prevBumpState) {
                printf("DOUBLE BUMP -> ");
                thisEvent.EventType = BUMP_CHANGE;
                LED_SetBank(LED_BANK3, 0x04);
                thisEvent.EventParam = (uint16_t) currentBumpState;
                PostEwokHSM(thisEvent);
                prevBumpState = currentBumpState;
            }
        }

            if (bumpValue == 0) { /* Release the BUMP */
                if (currentBumpState != prevBumpState) {
                    //ReturnEvent.EventType = UNBUMP;
                    LED_SetBank(LED_BANK3, 0x00);
                    //printf("Release\n");
                }
                prevBumpState = currentBumpState;
            }

        //printf("%d", ES_TIMEOUT);
//        if (ThisEvent.EventType == ES_TIMEOUT){
            //ES_Timer_InitTimer(BUMPER_SERVICE_TIMER, CHECK_BUMPER_RATE); //restart the timer
//        }
    
    ES_Timer_InitTimer(BUMPER_SERVICE_TIMER, CHECK_BUMPER_RATE);
    return ReturnEvent;
}














/*******************************************************************************
 * PRIVATE FUNCTIONs                                                           *
 ******************************************************************************/


/*******************************************************************************
 * TEST HARNESS                                                                *
 ******************************************************************************/

