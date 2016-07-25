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
#include "offTapeSubSubHSM.h"

/*******************************************************************************
 * MODULE #DEFINES                                                             *
 ******************************************************************************/
#define LIST_OF_TEMPLATE_STATES(STATE) \
        STATE(InitOffTapeSubState) \
        STATE(spinningCircles) /*Make sure state names are unique in their hierachy*/ \
        STATE(pivotCW)       \
        STATE(pivotCCW)  \
        STATE(reverse) \

#define ENUM_FORM(STATE) STATE, //Enums are reprinted verbatim and comma'd

typedef enum {
    LIST_OF_TEMPLATE_STATES(ENUM_FORM)
} SubTemplateState_t;

#define STRING_FORM(STATE) #STATE, //Strings are stringified and comma'd
static const char *StateNames[] = {
    LIST_OF_TEMPLATE_STATES(STRING_FORM)
};

#define SPINNING_CIRCLES_TIMER 1000
#define CCW_PIVOT_TIME 150
#define CW_PIVOT_TIME 400
#define REVERSE_TIME 150
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

static SubTemplateState_t CurrentState = InitOffTapeSubState; // <- change name to match ENUM
static uint8_t MyPriority;
static int radius = 0;
static int count = 0;
static int nextDirection = 0;
static int firstRun = 1;

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
uint8_t InitOffTapeSubSubHSM(void) {
    ES_Event returnEvent;
    if (firstRun){
        CurrentState = InitOffTapeSubState;
    } else {
        CurrentState = spinningCircles;
    }
    
    returnEvent = RunOffTapeSubSubHSM(INIT_EVENT);
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
ES_Event RunOffTapeSubSubHSM(ES_Event ThisEvent) {
    uint8_t makeTransition = FALSE; // use to flag transition
    SubTemplateState_t nextState; // <- change type to correct enum

    ES_Tattle(); // trace call stack

    switch (CurrentState) {
        case InitOffTapeSubState: // If current state is initial Psedudo State
            if (ThisEvent.EventType == ES_INIT)// only respond to ES_Init
            {
                // this is where you would put any actions associated with the
                // transition from the initial pseudo-state into the actual
                // initial state

                // now put the machine into the actual initial state
                nextState = spinningCircles;
                makeTransition = TRUE;
                ThisEvent.EventType = ES_NO_EVENT;
            }
            break;

        case spinningCircles: // in the first state, replace this with correct names
            if (ThisEvent.EventType != ES_NO_EVENT) { // An event is still active
                switch (ThisEvent.EventType) {
                    case ES_ENTRY:
                        printf("enter spinningCircles\n");
                        if (radius == 0) {
                            genericTurn(10, 0);
                        }
                        ES_Timer_InitTimer(POSITIONING_TIMER, SPINNING_CIRCLES_TIMER);
                        break;

                    case ES_TIMEOUT:
                        switch (ThisEvent.EventParam) {
                            case POSITIONING_TIMER:
                                radius++;
                                if (radius < 5) {
                                    genericTurn(10, radius);
                                    //printf("generic turn: 10, %d\n", radius);
                                    ES_Timer_InitTimer(POSITIONING_TIMER, SPINNING_CIRCLES_TIMER);
                                } else {
                                    if (radius <= 10) {
                                        genericTurn(15 - radius, 5);
                                        //printf("generic turn: %d, 5\n", 15 - radius);
                                        ES_Timer_InitTimer(POSITIONING_TIMER, SPINNING_CIRCLES_TIMER);
                                    } else {
                                        driveForward(SPEED);
                                    }

                                }
                                break;
                        }
                        break; //end ES_TIMEOUT


                    case BUMP_CHANGE:
                        if (((ThisEvent.EventParam & 0x3) ^ 0x2) == 0x3){
                            //printf("OffTape::bumpleft");
                            nextDirection = 1;
                            nextState = reverse;
                            makeTransition = TRUE;
                        } else {
                            //printf("OffTape::bumpright");
                            nextDirection = 0;
                            nextState = reverse;
                            makeTransition = TRUE;
                        }
                        //printf("off tape bump");
                        ThisEvent.EventType = ES_NO_EVENT;
                        break;


                    case ES_KEYINPUT:
                        // this is an example where the state does NOT transition
                        // do things you need to do in this state
                        // event consumed
                        ThisEvent.EventType = ES_NO_EVENT;
                        break;


                    default: // all unhandled events pass the event back up to the next level
                        break;
                }
            }
            break; //end SpinningCircles

        case pivotCW: // If current state is state OtherState
            if (ThisEvent.EventType != ES_NO_EVENT) { // An event is active
                switch (ThisEvent.EventType) {
                    case ES_ENTRY:
                        printf("enter pivotCW\n");
                        pivotRight(SPEED);
                        ES_Timer_InitTimer(POSITIONING_TIMER, CW_PIVOT_TIME);
                        break;

                    case ES_EXIT:
                        // this is where you would put any actions associated with the
                        // exit from this state
                        break;

                    case ES_TIMEOUT:
                        if (ThisEvent.EventParam == POSITIONING_TIMER) {
                            nextState = spinningCircles;
                            makeTransition = TRUE;
                            ThisEvent.EventType = ES_NO_EVENT;

                        }
                        break;

                    default: // all unhandled events pass the event back up to the next level
                        break;
                }
            }
            break; //end pivotCW

        case pivotCCW: // example of a state without a sub-statemachine
            if (ThisEvent.EventType != ES_NO_EVENT) { // An event is active
                switch (ThisEvent.EventType) {
                    case ES_ENTRY:
                        printf("enter pivotCCW\n");
                        pivotLeft(SPEED);
                        ES_Timer_InitTimer(POSITIONING_TIMER, CCW_PIVOT_TIME);
                        break;

                    case ES_EXIT:
                        // this is where you would put any actions associated with the
                        // exit from this state
                        break;

                    case ES_TIMEOUT:
                        if (ThisEvent.EventParam == POSITIONING_TIMER) {
                            nextState = spinningCircles;
                            makeTransition = TRUE;
                            ThisEvent.EventType = ES_NO_EVENT;

                        }
                        break;

                    default: // all unhandled events pass the event back up to the next level
                        break;
                }
            }
            break; //end pivotCCW

            case reverse: // example of a state without a sub-statemachine
            if (ThisEvent.EventType != ES_NO_EVENT) { // An event is active
                switch (ThisEvent.EventType) {
                    case ES_ENTRY:
                        printf("enter reverse\n");
                        driveForward(-1 * SPEED);
                        ES_Timer_InitTimer(POSITIONING_TIMER, REVERSE_TIME);
                        break;

                    case ES_EXIT:
                        // this is where you would put any actions associated with the
                        // exit from this state
                        break;

                    case ES_TIMEOUT:
                        if (ThisEvent.EventParam == POSITIONING_TIMER) {
                            if (nextDirection == 0) {
                                nextState = pivotCCW;
                                makeTransition = TRUE;
                                ThisEvent.EventType = ES_NO_EVENT;
                            } else {
                                nextState = pivotCW;
                                makeTransition = TRUE;
                                ThisEvent.EventType = ES_NO_EVENT;
                            }
                        }
                        break;

                    default: // all unhandled events pass the event back up to the next level
                        break;
                }
            }
            break; //end reverse

        default: // all unhandled states fall into here
            break;
    } // end switch on Current State

    if (makeTransition == TRUE) { // making a state transition, send EXIT and ENTRY
        // recursively call the current state with an exit event
        RunOffTapeSubSubHSM(EXIT_EVENT); // <- rename to your own Run function
        CurrentState = nextState;
        RunOffTapeSubSubHSM(ENTRY_EVENT); // <- rename to your own Run function
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
