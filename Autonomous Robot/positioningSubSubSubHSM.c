/*
 * File: TemplateSubHSM.c
 * Author: J. Edward Carryer
 * Modified: Gabriel H Elkaim
 *
 * Template file to set up a Heirarchical State Machine to work with the Events and
 * Services Framework (ES_Framework) on the Uno32 for the CMPE-118/L class. Note that
 * this file will need to be modified to fit your exact needs, and most of the names
 * will have to be changed to match your code.
 *
 * There is for a substate machine. Make sure it has a unique name
 *
 * This is provided as an example and a good place to start.
 *
 * History
 * When           Who     What/Why
 * -------------- ---     --------
 * 09/13/13 15:17 ghe      added tattletail functionality and recursive calls
 * 01/15/12 11:12 jec      revisions for Gen2 framework
 * 11/07/11 11:26 jec      made the queue static
 * 10/30/11 17:59 jec      fixed references to CurrentEvent in RunTemplateSM()
 * 10/23/11 18:20 jec      began conversion from SMTemplate.c (02/20/07 rev)
 */


/*******************************************************************************
 * MODULE #INCLUDE                                                             *
 ******************************************************************************/
#include <stdio.h>
#include "ES_Configure.h"
#include "ES_Framework.h"
#include "BOARD.h"
#include "EwokHSM.h"
#include "positioningSubSubSubHSM.h"

/*******************************************************************************
 * MODULE #DEFINES                                                             *
 ******************************************************************************/
#define LIST_OF_TEMPLATE_STATES(STATE) \
        STATE(InitPositioningSubState) \
        STATE(reverseP) /*Make sure state names are unique in their hierachy*/ \
        STATE(initialPivotShort)  \
        STATE(initialPivotLong)  \
        STATE(driveStriaghtShort) \
        STATE(driveStriaghtLong) \
        STATE(waitingForBackRightOn) \
        STATE(finalPivotShort) \
        STATE(reverseIntoT) \
        STATE(PivotCWfromLong) \
        STATE(waitingForMidRightOn) \
        STATE(waitingForMidRightOff) \
        STATE(finalPivotLong) \
        STATE(arcBackLeft) \
        STATE(pivotSlightLeft) \
        STATE(arcForwardRight) \
        STATE(pivotOnT) \
        STATE(pause) \
        STATE(reverseToCenterShort) \

#define ENUM_FORM(STATE) STATE, //Enums are reprinted verbatim and comma'd

typedef enum {
    LIST_OF_TEMPLATE_STATES(ENUM_FORM)
} PositioningSubSubSubState_t;

#define STRING_FORM(STATE) #STATE, //Strings are stringified and comma'd
static const char *StateNames[] = {
    LIST_OF_TEMPLATE_STATES(STRING_FORM)
};

//Defines used for both short and long depot
#define INITIAL_REVERSE_TIME 1000
#define FINAL_REVERSE_TIME 800
#define ARC_FORWARD_TIME 700
#define PIVOT_ON_T_TIME 200
#define FINAL_REVERSE_PAUSE_TIME 1400
#define POS_OH_SHIT_FORWARD_TIME 4000

//Defines used for short depot only
#define INITIAL_PIVOT_SHORT_TIME 285 //? at full charge
#define FINAL_PIVOT_SHORT_TIME 800 //? at full charge
#define REVERSE_TO_CENTER_SHORT_TIME 125

//Defines used for long depot only
#define INITIAL_PIVOT_LONG_TIME 950 //900 at full charge
#define DRIVE_SRTRAIGHT_LONG_TIME 1900 //1900 at full charge
#define PIVOT_CW_FROM_LONG_TIME 650 //600 at full charge
#define FINAL_PIVOT_LONG_TIME 2200 //2200 at full charge
#define ARC_BACK_LEFT_TIME 520 //520 at full charge
#define PIVOT_SLIGHT_LEFT_TIME 410 //410 at full charge


#define FRONT_RIGHT_TAPE    0x02
#define BACK_RIGHT_TAPE     0x01
#define MID_RIGHT_TAPE      0x04
#define FRONT_LEFT_TAPE     0x20
#define BACK_LEFT_TAPE      0x10
#define MID_LEFT_TAPE       0x08
#define LEFT_BUMP           0x01
#define RIGHT_BUMP          0x02
#define MID_BUMP            0x03

/*******************************************************************************
 * PRIVATE FUNCTION PROTOTYPES                                                 *
 ******************************************************************************/
/* Prototypes for private functions for this machine. They should be functions
   relevant to the behavior of this state machine */

/*******************************************************************************
 * PRIVATE MODULE VARIABLES                                                            *
 ******************************************************************************/
/* You will need MyPriority and the state variable; you may need others as well.
 * The type of state variable should match that of enum in header file. */

static PositioningSubSubSubState_t CurrentState = InitPositioningSubState; // <- change name to match ENUM
static uint8_t MyPriority;
static uint16_t transitionBumpValues;
static uint16_t transitionTapeValue;
static int reverseAttemptCount = 0;


/*******************************************************************************
 * PUBLIC FUNCTIONS                                                            *
 ******************************************************************************/

/**
 * @Function InitTemplateSubHSM(uint8_t Priority)
 * @param Priority - internal variable to track which event queue to use
 * @return TRUE or FALSE
 * @brief This will get called by the framework at the beginning of the code
 *        execution. It will post an ES_INIT event to the appropriate event
 *        queue, which will be handled inside RunTemplateFSM function. Remember
 *        to rename this to something appropriate.
 *        Returns TRUE if successful, FALSE otherwise
 * @author J. Edward Carryer, 2011.10.23 19:25 */
uint8_t InitPositioningSubSubSubHSM(void) {
    ES_Event returnEvent;

    CurrentState = InitPositioningSubState;
    returnEvent = RunPositioningSubSubSubHSM(INIT_EVENT);
    if (returnEvent.EventType == ES_NO_EVENT) {
        return TRUE;
    }
    return FALSE;
}

/**
 * @Function RunTemplateSubHSM(ES_Event ThisEvent)
 * @param ThisEvent - the event (type and param) to be responded.
 * @return Event - return event (type and param), in general should be ES_NO_EVENT
 * @brief This function is where you implement the whole of the heirarchical state
 *        machine, as this is called any time a new event is passed to the event
 *        queue. This function will be called recursively to implement the correct
 *        order for a state transition to be: exit current state -> enter next state
 *        using the ES_EXIT and ES_ENTRY events.
 * @note Remember to rename to something appropriate.
 *       The lower level state machines are run first, to see if the event is dealt
 *       with there rather than at the current level. ES_EXIT and ES_ENTRY events are
 *       not consumed as these need to pass pack to the higher level state machine.
 * @author J. Edward Carryer, 2011.10.23 19:25
 * @author Gabriel H Elkaim, 2011.10.23 19:25 */
ES_Event RunPositioningSubSubSubHSM(ES_Event ThisEvent) {
    uint8_t makeTransition = FALSE; // use to flag transition
    PositioningSubSubSubState_t nextState; // <- change type to correct enum
    ES_Event thisEvent;
    static uint32_t entryTime;

    ES_Tattle(); // trace call stack

    switch (CurrentState) {
        case InitPositioningSubState: // If current state is initial Psedudo State
            if (ThisEvent.EventType == ES_ENTRY) {
                reverseAttemptCount = 0;
                transitionBumpValues = ThisEvent.EventParam;
                //if ((transitionBumpValues & 0x4) == 0x4){} //this isnt actually a bumper. it is a value used to send us straight to ramming the ammo depot
                nextState = reverseP;
                makeTransition = TRUE;
                ThisEvent.EventType = ES_NO_EVENT;
            }

            break; //break InitPositionBumpValues

        case reverseP: // in the first state - reverse positioning
            if (ThisEvent.EventType != ES_NO_EVENT) { // An event is still active
                switch (ThisEvent.EventType) {
                    case ES_ENTRY:
                        printf("enter reverseP\n");
                        driveForward(-1 * SPEED);
                        ES_Timer_InitTimer(POSITIONING_TIMER, INITIAL_REVERSE_TIME);
                        break;
                    case ES_EXIT:
                        break;

                    case ES_TIMEOUT:
                        if (transitionBumpValues == RIGHT_BUMP) {
                            printf("Timer expired and RIGHT BUMP\n");
                            nextState = initialPivotShort;
                            makeTransition = TRUE;
                            ThisEvent.EventType = ES_NO_EVENT;
                            transitionBumpValues = 0;
                        } else if (transitionBumpValues == MID_BUMP) {
                            printf("Timer expired and MID BUMP\n");
                            nextState = initialPivotLong;
                            makeTransition = TRUE;
                            ThisEvent.EventType = ES_NO_EVENT;
                            transitionBumpValues = 0;
                        }
                        break;
                    default: // all unhandled events pass the event back up to the next level
                        break; //End
                }
            }
            break; //break reverseP


        case initialPivotShort: // example of a state without a sub-statemachine
            if (ThisEvent.EventType != ES_NO_EVENT) { // An event is still active
                switch (ThisEvent.EventType) {
                    case ES_ENTRY:
                        printf("enter initialPivotShort\n");
                        pivotLeft(SPEED);
                        ES_Timer_InitTimer(POSITIONING_TIMER, INITIAL_PIVOT_SHORT_TIME);
                        break;

                    case ES_TIMEOUT:
                        if (ThisEvent.EventParam == POSITIONING_TIMER) {
                            printf("initialPivotShort timed out\n");
                            nextState = driveStriaghtShort;
                            makeTransition = TRUE;
                            ThisEvent.EventType = ES_NO_EVENT;
                        }
                        break;
                    case ES_EXIT:
                        // this is where you would put any actions associated with the
                        // exit from this state
                        break;
                    default: // all unhandled events pass the event back up to the next level
                        break;
                }
            }
            break; //end shortTrek


        case initialPivotLong:
            if (ThisEvent.EventType != ES_NO_EVENT) { // An event is still active
                switch (ThisEvent.EventType) {
                    case ES_ENTRY:
                        printf("enter initialPivotLong\n");
                        pivotLeft(SPEED);
                        ES_Timer_InitTimer(POSITIONING_TIMER, INITIAL_PIVOT_LONG_TIME);
                        break;

                    case ES_TIMEOUT:
                        if (ThisEvent.EventParam == POSITIONING_TIMER) {
                            printf("initialPivotShort timed out\n");
                            nextState = driveStriaghtLong;
                            makeTransition = TRUE;
                            ThisEvent.EventType = ES_NO_EVENT;
                        }
                        break;
                    case ES_EXIT:
                        // this is where you would put any actions associated with the
                        // exit from this state
                        break;
                    default: // all unhandled events pass the event back up to the next level
                        break;
                }
            }
            break; //end initial pivot long


        case driveStriaghtShort:
            if (ThisEvent.EventType != ES_NO_EVENT) { // An event is still active
                switch (ThisEvent.EventType) {
                    case ES_ENTRY:
                        printf("enter driveStraightShort.\n");
                        entryTime = ES_Timer_GetTime();
                        ES_Timer_InitTimer(OH_SHIT_TIMER, POS_OH_SHIT_FORWARD_TIME);
                        driveForward(4);
                        break;

                    case TAPE_CHANGE:
                        if ((ES_Timer_GetTime() - entryTime) > 1000) {
                            if ((ThisEvent.EventParam & 0x2) == 0x2) {//TAPE STATE XXXX1X
                                nextState = waitingForBackRightOn;
                                makeTransition = TRUE;
                                ThisEvent.EventType = ES_NO_EVENT;
                            }
                        }
                        break;

                    case BUMP_CHANGE:
                        if (ThisEvent.EventParam == 0x2) { //right bumper
                            nextState = arcBackLeft;
                            makeTransition = TRUE;
                            ThisEvent.EventType = ES_NO_EVENT;
                        }
                        break;

                    case ES_TIMEOUT:
                        if (ThisEvent.EventParam == OH_SHIT_TIMER) {
                            thisEvent.EventType = RETRY_T;
                            PostEwokHSM(thisEvent);
                        }
                        break;

                    case ES_EXIT:
                        // this is where you would put any actions associated with the
                        // exit from this state
                        break;
                    default: // all unhandled events pass the event back up to the next level
                        break;
                }
            }
            break; //end driveStraight


        case driveStriaghtLong:
            if (ThisEvent.EventType != ES_NO_EVENT) { // An event is still active
                switch (ThisEvent.EventType) {
                    case ES_ENTRY:
                        printf("enter drive straight long.\n");
                        driveForward(4);
                        ES_Timer_InitTimer(POSITIONING_TIMER, DRIVE_SRTRAIGHT_LONG_TIME);
                        break;

                    case ES_TIMEOUT:
                        if (ThisEvent.EventParam == POSITIONING_TIMER) {
                            printf("initialPivotShort timed out\n");
                            nextState = PivotCWfromLong;
                            makeTransition = TRUE;
                            ThisEvent.EventType = ES_NO_EVENT;
                        }
                        break;

                    case ES_EXIT:
                        // this is where you would put any actions associated with the
                        // exit from this state
                        break;
                    default: // all unhandled events pass the event back up to the next level
                        break;
                }
            }
            break; //end drive straight long

        case PivotCWfromLong:
            if (ThisEvent.EventType != ES_NO_EVENT) { // An event is still active
                switch (ThisEvent.EventType) {
                    case ES_ENTRY:
                        printf("enter PivotCWfromLong\n");
                        pivotRight(SPEED - 2);
                        ES_Timer_InitTimer(POSITIONING_TIMER, PIVOT_CW_FROM_LONG_TIME);
                        break;

                    case ES_TIMEOUT:
                        if (ThisEvent.EventParam == POSITIONING_TIMER) {
                            printf("initialPivotShort timed out\n");
                            nextState = driveStriaghtShort;
                            makeTransition = TRUE;
                            ThisEvent.EventType = ES_NO_EVENT;
                        }
                        break;

                    case ES_EXIT:
                        // this is where you would put any actions associated with the
                        // exit from this state
                        break;
                    default: // all unhandled events pass the event back up to the next level
                        break;
                }
            }
            break; //end pivot cw from long


        case waitingForMidRightOn:
            if (ThisEvent.EventType != ES_NO_EVENT) { // An event is still active
                switch (ThisEvent.EventType) {
                    case ES_ENTRY:
                        printf("enter waitingForMidRightOn\n");
                        driveForward(SPEED - 1);
                        break;

                    case TAPE_CHANGE:
                        if ((ThisEvent.EventParam & 0x8) == 0x8) { //bit pattern is XX1XXX
                            nextState = waitingForMidRightOff;
                            makeTransition = TRUE;
                            ThisEvent.EventType = ES_NO_EVENT;
                        }
                        break;

                    case BUMP_CHANGE:
                        if (ThisEvent.EventParam == 0x2) { //right bumper
                            nextState = arcBackLeft;
                            makeTransition = TRUE;
                            ThisEvent.EventType = ES_NO_EVENT;
                        }
                        break;


                    case ES_EXIT:
                        // this is where you would put any actions associated with the
                        // exit from this state
                        break;
                    default: // all unhandled events pass the event back up to the next level
                        break;
                }
            }
            break; //end waitingForMidLeftOn

        case arcBackLeft:
            if (ThisEvent.EventType != ES_NO_EVENT) { // An event is still active
                switch (ThisEvent.EventType) {
                    case ES_ENTRY:
                        printf("enter arcBackLeft\n");
                        genericTurn(-5, -2);
                        ES_Timer_InitTimer(POSITIONING_TIMER, ARC_BACK_LEFT_TIME);
                        break;

                    case ES_TIMEOUT:
                        if (ThisEvent.EventParam == POSITIONING_TIMER) { 
                                nextState = pivotSlightLeft;
                                makeTransition = TRUE;
                                ThisEvent.EventType = ES_NO_EVENT;
                            }
                        break;
                    case ES_EXIT:
                        // this is where you would put any actions associated with the
                        // exit from this state
                        break;
                    default: // all unhandled events pass the event back up to the next level
                        break;
                }
            }
            break; //end arcBackLeft

        case pivotSlightLeft:
            if (ThisEvent.EventType != ES_NO_EVENT) { // An event is still active
                switch (ThisEvent.EventType) {
                    case ES_ENTRY:
                        printf("enter pivotSlightLeft\n");
                                pivotLeft(3);
                                ES_Timer_InitTimer(POSITIONING_TIMER, PIVOT_SLIGHT_LEFT_TIME);
                        break;

                    case ES_TIMEOUT:
                        if (ThisEvent.EventParam == POSITIONING_TIMER) {
                                nextState = driveStriaghtShort;
                                makeTransition = TRUE;
                                ThisEvent.EventType = ES_NO_EVENT;
                            }
                        break;
                    case ES_EXIT:
                        // this is where you would put any actions associated with the
                        // exit from this state
                        break;
                    default: // all unhandled events pass the event back up to the next level
                        break;
                }
            }
            break; //end pivotSlightLeft

        case waitingForMidRightOff:
            if (ThisEvent.EventType != ES_NO_EVENT) { // An event is still active
                switch (ThisEvent.EventType) {
                    case ES_ENTRY:
                        printf("enter waitingForMidRightOff\n");
                        driveForward(SLOW_SPEED);
                        break;

                    case TAPE_CHANGE:
                        if ((ThisEvent.EventParam & 0x8) == 0) { //bit pattern is XX0XXX
                            nextState = finalPivotLong;
                            makeTransition = TRUE;
                            ThisEvent.EventType = ES_NO_EVENT;
                        }
                        break;
                    case ES_EXIT:
                        // this is where you would put any actions associated with the
                        // exit from this state
                        break;
                    default: // all unhandled events pass the event back up to the next level
                        break;
                }
            }
            break; //end waitingForMidRightOff


//        case waitingForMidLeftOff:
//            if (ThisEvent.EventType != ES_NO_EVENT) { // An event is still active
//                switch (ThisEvent.EventType) {
//                    case ES_ENTRY:
//                        printf("IN FOR waitingForMidLeftOff\n");
//                        driveForward(SLOW_SPEED);
//                        break;
//
//                    case TAPE_CHANGE:
//                        if ((ThisEvent.EventParam & 0x8) == 0) { //bit pattern is XX1XXX
//                            nextState = waitingForMidLeftOff;
//                            makeTransition = TRUE;
//                            ThisEvent.EventType = ES_NO_EVENT;
//                        }
//                        break;
//                    case ES_EXIT:
//                        // this is where you would put any actions associated with the
//                        // exit from this state
//                        break;
//                    default: // all unhandled events pass the event back up to the next level
//                        break;
//                }
//            }
//            break; //end waitingForMidRightOff

        case waitingForBackRightOn:
            if (ThisEvent.EventType != ES_NO_EVENT) { // An event is still active
                switch (ThisEvent.EventType) {
                    case ES_ENTRY:
                        printf("enter waitingForBackRightOn\n");
                        driveForward(SLOW_SPEED);
                        break;

                    case TAPE_CHANGE:
                        if ((ThisEvent.EventParam & 0x1) == 0x1) { //bit pattern is XXXXX1
                            nextState = reverseToCenterShort;
                            makeTransition = TRUE;
                            ThisEvent.EventType = ES_NO_EVENT;
                        }
                        break;
                    case ES_EXIT:
                        // this is where you would put any actions associated with the
                        // exit from this state
                        break;
                    default: // all unhandled events pass the event back up to the next level
                        break;
                }
            }
            break; //end waitingForMidRightOff


        case finalPivotLong:
            if (ThisEvent.EventType != ES_NO_EVENT) { // An event is still active
                switch (ThisEvent.EventType) {
                    case ES_ENTRY:
                        printf("enter finalPivotLong\n");
                        pivotLeft(SLOW_SPEED);
                        ES_Timer_InitTimer(POSITIONING_TIMER, FINAL_PIVOT_LONG_TIME);
                        break;

                    case ES_TIMEOUT:
                        if (ThisEvent.EventParam == POSITIONING_TIMER) {
                            nextState = reverseIntoT;
                            makeTransition = TRUE;
                            ThisEvent.EventType = ES_NO_EVENT;
                        }
                        break;
                    case ES_EXIT:
                        // this is where you would put any actions associated with the
                        // exit from this state
                        break;
                    default: // all unhandled events pass the event back up to the next level
                        break;
                }
            }
            break; //finalPivotLong

        case finalPivotShort:
            if (ThisEvent.EventType != ES_NO_EVENT) { // An event is still active
                switch (ThisEvent.EventType) {
                    case ES_ENTRY:
                        pivotLeft(4);
                        printf("enter finalPivotShort\n");
                        ES_Timer_InitTimer(POSITIONING_TIMER, FINAL_PIVOT_SHORT_TIME);
                        break;

                    case TAPE_CHANGE:
                        //                        if ((ThisEvent.EventParam & 0x4) == 0x4) { //bit pattern is XXX1XX
                        //                            //nextState = pivotCCWuntilFRoff;
                        //                            nextState = pivotCCWuntilAFon;
                        //                            makeTransition = TRUE;
                        //                            ThisEvent.EventType = ES_NO_EVENT;
                        //                        }
                        break;

                    case ES_TIMEOUT:
                        if (ThisEvent.EventParam == POSITIONING_TIMER) {
                            nextState = reverseIntoT;
                            makeTransition = TRUE;
                            ThisEvent.EventType = ES_NO_EVENT;
                        }
                        break;

                    case ES_EXIT:
                        // this is where you would put any actions associated with the
                        // exit from this state
                        break;
                    default: // all unhandled events pass the event back up to the next level
                        break;
                }
            }
            break; //end pivotCCWuntilT

        case reverseIntoT:
            if (ThisEvent.EventType != ES_NO_EVENT) { // An event is still active
                switch (ThisEvent.EventType) {
                    case ES_ENTRY:
                        printf("enter reversIntoT\n");
                        driveForward(-10);
                        ES_Timer_InitTimer(POSITIONING_TIMER, FINAL_REVERSE_TIME);


                        break;

                    case ES_TIMEOUT:
                         if (ThisEvent.EventParam == POSITIONING_TIMER) {
                            nextState = pause;
                            makeTransition = TRUE;
                            ThisEvent.EventType = ES_NO_EVENT;
                         }
                        break;

                    case TAPE_CHANGE:

                        break;

                    case ES_EXIT:
                        if (reverseAttemptCount > 4 ){
                            thisEvent.EventType = RETRY_T;
                            PostEwokHSM(thisEvent);
                        }
                        reverseAttemptCount++;
                        break;
                    default: // all unhandled events pass the event back up to the next level
                        break;
                }
            }
            break; //end reverseIntoT


        case arcForwardRight:
            if (ThisEvent.EventType != ES_NO_EVENT) { // An event is still active
                switch (ThisEvent.EventType) {
                    case ES_ENTRY:
                        printf("enter arcForwardRight\n");
                        genericTurn(3, 7);
                        ES_Timer_InitTimer(POSITIONING_TIMER, ARC_FORWARD_TIME);
                        break;

                    case ES_TIMEOUT:
                         if (ThisEvent.EventParam == POSITIONING_TIMER) {
                            nextState = pivotOnT;
                            makeTransition = TRUE;
                            ThisEvent.EventType = ES_NO_EVENT;
                         }
                        break;

                    case TAPE_CHANGE:

                        break;

                    case ES_EXIT:
                        // this is where you would put any actions associated with the
                        // exit from this state
                        break;
                    default: // all unhandled events pass the event back up to the next level
                        break;
                }
            }
            break; //end reverseIntoT

         case pause:
            if (ThisEvent.EventType != ES_NO_EVENT) { // An event is still active
                switch (ThisEvent.EventType) {
                    case ES_ENTRY:
                        printf("enter pause\n");
                        driveForward(0);
                        ES_Timer_InitTimer(POSITIONING_TIMER, FINAL_REVERSE_PAUSE_TIME);
                        break;

                    case ES_TIMEOUT:
                         if (ThisEvent.EventParam == POSITIONING_TIMER) {
                            nextState = arcForwardRight ;
                            makeTransition = TRUE;
                            ThisEvent.EventType = ES_NO_EVENT;
                         }
                        break;

                    case TAPE_CHANGE:

                        break;

                    case ES_EXIT:
                        // this is where you would put any actions associated with the
                        // exit from this state
                        break;
                    default: // all unhandled events pass the event back up to the next level
                        break;
                }
            }
            break; //end reverseIntoT

        case pivotOnT:
            if (ThisEvent.EventType != ES_NO_EVENT) { // An event is still active
                switch (ThisEvent.EventType) {
                    case ES_ENTRY:
                        printf("enter pivotOnT\n");
                        pivotLeft(5);
                        ES_Timer_InitTimer(POSITIONING_TIMER, PIVOT_ON_T_TIME);
                        break;

                    case ES_TIMEOUT:
                         if (ThisEvent.EventParam == POSITIONING_TIMER) {
                            nextState = reverseIntoT;
                            makeTransition = TRUE;
                            ThisEvent.EventType = ES_NO_EVENT;
                         }
                        break;

                    case TAPE_CHANGE:

                        break;

                    case ES_EXIT:
                        // this is where you would put any actions associated with the
                        // exit from this state
                        break;
                    default: // all unhandled events pass the event back up to the next level
                        break;
                }
            }
            break; //end reverseIntoT

        case reverseToCenterShort:
            if (ThisEvent.EventType != ES_NO_EVENT) { // An event is still active
                switch (ThisEvent.EventType) {
                    case ES_ENTRY:
                        printf("enter reverseToCenterShort\n");
                        driveForward(-5);
                        ES_Timer_InitTimer(POSITIONING_TIMER, REVERSE_TO_CENTER_SHORT_TIME);
                        break;

                    case ES_TIMEOUT:
                         if (ThisEvent.EventParam == POSITIONING_TIMER) {
                            nextState = finalPivotShort;
                            makeTransition = TRUE;
                            ThisEvent.EventType = ES_NO_EVENT;
                         }
                        break;

                    case TAPE_CHANGE:

                        break;

                    case ES_EXIT:
                        // this is where you would put any actions associated with the
                        // exit from this state
                        break;
                    default: // all unhandled events pass the event back up to the next level
                        break;
                }
            }
            break; //end reverseToCenterShort


    } // end switch on Current State

    if (makeTransition == TRUE) { // making a state transition, send EXIT and ENTRY
        // recursively call the current state with an exit event
        RunPositioningSubSubSubHSM(EXIT_EVENT); // <- rename to your own Run function
        CurrentState = nextState;
        RunPositioningSubSubSubHSM(ENTRY_EVENT); // <- rename to your own Run function
    }

    ES_Tail(); // trace call stack end
    return ThisEvent;
}


/*******************************************************************************
 * PRIVATE FUNCTIONS                                                           *
 ******************************************************************************/


/*******************************************************************************
 * TEST HARNESS                                                                *
 ******************************************************************************/

#ifdef TEMPLATESUBHSM_TEST // <-- change this name and define it in your MPLAB-X
//     project to run the test harness
#include <stdio.h>

void main(void) {
    ES_Return_t ErrorType;
    BOARD_Init();
    // When doing testing, it is useful to annouce just which program
    // is running.

    printf("Starting the Hierarchical State Machine Test Harness \r\n");
    printf("using the 2nd Generation Events & Services Framework\n\r");

    // Your hardware initialization function calls go here

    // now initialize the Events and Services Framework and start it running
    ErrorType = ES_Initialize();

    if (ErrorType == Success) {
        ErrorType = ES_Run();
    }

    //
    //if we got to here, there was an error
    //

    switch (ErrorType) {
        case FailedPointer:
            printf("Failed on NULL pointer");
            break;
        case FailedInit:
            printf("Failed Initialization");
            break;
        default:
            printf("Other Failure");
            break;
    }

    while (1) {
        ;
    }
}

#endif // TEMPLATESUBHSM_TEST
