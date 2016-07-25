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
#include "needingAmmoSubHSM.h"
#include "onTapeSubSubHSM.h"
#include "offTapeSubSubHSM.h"

/*******************************************************************************
 * MODULE #DEFINES                                                             *
 ******************************************************************************/
#define LIST_OF_TEMPLATE_STATES(STATE) \
        STATE(InitNeedingAmmoSubState) \
        STATE(offTape) /*Make sure state names are unique in their hierachy*/ \
        STATE(onTape)       \
        STATE(offTapeTesting) \

#define ENUM_FORM(STATE) STATE, //Enums are reprinted verbatim and comma'd
typedef enum {
    LIST_OF_TEMPLATE_STATES(ENUM_FORM)
} SubNeedingAmmoState_t;

#define STRING_FORM(STATE) #STATE, //Strings are stringified and comma'd
static const char *StateNames[] = {
    LIST_OF_TEMPLATE_STATES(STRING_FORM)
};


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

static SubNeedingAmmoState_t CurrentState = InitNeedingAmmoSubState;   // <- change name to match ENUM
static uint8_t MyPriority;
static uint16_t transitionTapeValues;


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
uint8_t InitNeedingAmmoSubHSM(void)
{
     ES_Event returnEvent;

    CurrentState = InitNeedingAmmoSubState;
    returnEvent = RunNeedingAmmoSubHSM(INIT_EVENT);

    //init all sub states
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
ES_Event RunNeedingAmmoSubHSM(ES_Event ThisEvent)
{
    uint8_t makeTransition = FALSE; // use to flag transition
    SubNeedingAmmoState_t nextState; // <- change type to correct enum

    ES_Tattle(); // trace call stack

    switch (CurrentState) {
        case InitNeedingAmmoSubState: // If current state is initial Psedudo State
            if (ThisEvent.EventType == ES_INIT)// only respond to ES_Init
            {
                // now put the machine into the actual initial state
                nextState = offTape;
                makeTransition = TRUE;
                ThisEvent.EventType = ES_NO_EVENT;
            }
            break;

        case offTape:
            ThisEvent = RunOffTapeSubSubHSM(ThisEvent);
            if (ThisEvent.EventType != ES_NO_EVENT) { // An event is still active
                switch (ThisEvent.EventType) {
                    case ES_ENTRY:
                        break;
                    case ES_EXIT:
                        InitOffTapeSubSubHSM();
                        break;
                    case TAPE_CHANGE:
                        //driveForward(0);
                        transitionTapeValues = ThisEvent.EventParam;
                        nextState = onTape;
                        makeTransition = TRUE;
                        ThisEvent.EventType = ES_NO_EVENT;
                        break;

                    default: // all unhandled events pass the event back up to the next level
                        break;
                }
            }
            break; //end offTape

        case offTapeTesting:
            /*DANNY: The first state. we are off the tape and need to find it. for now
             *      this code just has the robot drive straight forward. This will need
             *      to be changed. A TAPE_CHANGE event changes us to onTape.
             */
            if (ThisEvent.EventType != ES_NO_EVENT) { // An event is still active
                switch (ThisEvent.EventType) {
                case ES_ENTRY:
                    printf("\n\n-------IN OFF TAPE--------\n\n");
                    driveForward(SPEED);
                    break;
                case ES_EXIT:
                    break;
                case TAPE_CHANGE:
                    /*DANNY: We hit a tape line. So we should transition to the onTape
                     *      sub machine. This part gets kind of weird because we want to pass
                     *      the current tape sensor values to the onTape state machine. As far
                     *      as I can tell ES_Framework doesn't allow for this. So I made
                     *      a static global variable up at the top of this file that
                     *      I use to store the current tape values. Then we can access these
                     *      values from the onTape sub sate machine.
                     */
                    transitionTapeValues = ThisEvent.EventParam;
                    nextState = onTape;
                    makeTransition = TRUE;
                    ThisEvent.EventType = ES_NO_EVENT;
                    break;

                    case BUMP_CHANGE:

                        driveSlightBackRight(8);
                        ES_Timer_InitTimer(ASTEROID_TIMER, 800);
                        ThisEvent.EventType = ES_NO_EVENT;
                        break;

                    case ES_TIMEOUT:
                        if (ThisEvent.EventParam == ASTEROID_TIMER){
                            driveForward(0);
                            ThisEvent.EventType = ES_TIMEOUT;
                        }

                        break;


                    default: // all unhandled events pass the event back up to the next level
                    break;
                }
        }
        break;

    case onTape:
        if (transitionTapeValues) {
            ThisEvent.EventParam = transitionTapeValues;
            transitionTapeValues = 0;
        }
        ThisEvent = RunOnTapeSubSubHSM(ThisEvent); // run sub-state machine for this state
        if (ThisEvent.EventType != ES_NO_EVENT) { // An event is active
            switch (ThisEvent.EventType) {

            case ES_ENTRY:

                break;

           case LOST_TAPE:
               nextState = offTape;
               makeTransition = TRUE;
               ThisEvent.EventType = ES_NO_EVENT;
               break;

            case ES_EXIT:
                InitOnTapeSubSubHSM();
                break;


            default: // all unhandled events pass the event back up to the next level
                break;
            }
        }
        break;


        
    default: // all unhandled states fall into here
        break;
    } // end switch on Current State

    if (makeTransition == TRUE) { // making a state transition, send EXIT and ENTRY
        // recursively call the current state with an exit event
        RunNeedingAmmoSubHSM(EXIT_EVENT);   // <- rename to your own Run function
        CurrentState = nextState;
        RunNeedingAmmoSubHSM(ENTRY_EVENT);  // <- rename to your own Run function
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

void main(void)
{
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
