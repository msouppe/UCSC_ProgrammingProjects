
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
#include "Ewok.h"
#include "EwokHSM.h"
#include "AD.h"
#include "IO_Ports.h"
#include "LED.h"
#include "BeaconService.h"

/*******************************************************************************
 * MODULE #DEFINES                                                             *
 ******************************************************************************/
#define BEACON_SAMPLE_RATE 10
#define B_ON  1
#define B_OFF 0
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
uint8_t InitBeaconService(uint8_t Priority) {
    ES_Event ThisEvent;
    MyPriority = Priority;

    // in here you write your initialization code
    // this includes all hardware and software initialization
    // that needs to occur.

    //ES_Timer_Init();

    ES_Timer_InitTimer(BEACON_SERVICE_TIMER, BEACON_SAMPLE_RATE);

    //IO_PortsSetPortInputs(BUMPER_PORT, FRONT_LEFT_BUMPER | FRONT_RIGHT_BUMPER);
    //ES_Timer_InitTimer(BUMPER_SERVICE_TIMER, CHECK_BUMPER_RATE);
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
uint8_t PostBeaconService(ES_Event ThisEvent) {
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
ES_Event RunBeaconService(ES_Event ThisEvent) {
    ES_Event ReturnEvent;
    ES_Event thisEvent;
    //static uint16_t BeaconValue = 0;
    //static uint16_t prevBeaconValue = 0;
    //uint16_t currentBeaconValue;
    static uint16_t state = B_OFF;
    static uint8_t beaconValues = 0;

    // Debouncing mechanism

//    if (ThisEvent.EventType != ES_TIMEOUT) return ReturnEvent;


    beaconValues = beaconValues << 1;
    beaconValues |= Ewok_ReadBeacon();


    ReturnEvent.EventType = ES_NO_EVENT; // assume no errors

    if(ThisEvent.EventType != ES_TIMEOUT) return ReturnEvent;

    if (ThisEvent.EventType == ES_INIT) { }// only respond to ES_Init

    // If there is a constant HIGH value from the beacon detector
    if ((beaconValues == 0xFF) && (state == B_OFF))
    {
        thisEvent.EventType = BEACON_ON;
//        thisEvent.EventParam = (uint16_t) BEACON_ON;
        PostEwokHSM(thisEvent);
        state = B_ON;
        printf("BEACON ON -> ");
    } 
    else if ((beaconValues == 0x00) && (state == B_ON))
    {
        thisEvent.EventType = BEACON_OFF;
//        thisEvent.EventParam = (uint16_t) BEACON_OFF;
        PostEwokHSM(thisEvent);
        state = B_OFF;

        printf("BEACON OFF -> ");
    }
    

    ES_Timer_InitTimer(BEACON_SERVICE_TIMER, BEACON_SAMPLE_RATE);
    return ReturnEvent;
}