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
#include "EwokHSM.h" //top level state machine
#include "needingAmmoSubHSM.h" //current parent state machine
#include "onTapeSubSubHSM.h" //current state machine
#include "LineFollowRightSubSubSubHSM.h" // one of the child state machines
#include "rightCornerSubSubSubHSM.h" // one of the child state machines
#include "positioningSubSubSubHSM.h" // one of the child state machines

/*******************************************************************************
 * MODULE #DEFINES                                                             *
 ******************************************************************************/
#define LIST_OF_TEMPLATE_STATES(STATE) \
        STATE(InitOnTapeSubState) \
        STATE(onTape_frontRightOn) /*Make sure state names are unique in their hierachy*/ \
        STATE(pivotCCW) \
        STATE(followLineRight) \
        STATE(reverse) \
        STATE(rightCorner) \
        STATE(positioning) \
        STATE(frontLeftOn) \
        STATE(pivotCWfromMLor06Init) \
        STATE(waitForBLoff) \
        STATE(cautiousReverse) \
        STATE(pivotCCWfromCautiousRev) \
        STATE(pivotCWfromCautiousRev) \
        STATE(pivotCCWfromMRInit) \
        STATE(waitForBRoff) \
        STATE(getAwayPivot) \
        STATE(getAwayDrive) \
        STATE(reverseBeforeGetAway) \
        STATE(ONT_roachTestReverse) \
        STATE(ONT_roachTestForward) \



#define ENUM_FORM(STATE) STATE, //Enums are reprinted verbatim and comma'd

typedef enum {
    LIST_OF_TEMPLATE_STATES(ENUM_FORM)
} SubSubOnTapeState_t;

#define STRING_FORM(STATE) #STATE, //Strings are stringified and comma'd
static const char *StateNames[] = {
    LIST_OF_TEMPLATE_STATES(STRING_FORM)
};

#define CAUTIOUS_REV_PIVOT_TIME 1000
#define CAUTIOUS_REV_TIMEOUT 4000
#define GET_AWAY_ROTATE_TIME 1000
#define OH_SHIT_PIVOT_TIME 2300
#define GET_AWAY_DEBOUNCE_TIME 150
#define OH_SHIT_REVERSE_TIME 3500
#define FRONT_RIGHT_ON_TIME 5000
#define REVERSE_BEFORE_GET_AWAY_TIME 1000
#define OH_SHIT_GET_AWAY_DRIVE_TIME 2000
#define ONT_ROACH_TEST_REVERSE_TIME 1000
#define ONT_ROACH_TEST_PAUSE_TIME 1500

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

static SubSubOnTapeState_t CurrentState = InitOnTapeSubState; // <- change name to match ENUM
static uint8_t MyPriority;
static uint16_t transitionTapeValues;
static uint16_t transitionBumpValues;
static int pivotEntryCount = 0;

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
uint8_t InitOnTapeSubSubHSM(void) {
    ES_Event returnEvent;
    pivotEntryCount = 0;
    CurrentState = InitOnTapeSubState;
    returnEvent = RunOnTapeSubSubHSM(INIT_EVENT);
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
ES_Event RunOnTapeSubSubHSM(ES_Event ThisEvent) {
    uint8_t makeTransition = FALSE; // use to flag transition
    SubSubOnTapeState_t nextState; // <- change type to correct enum
    ES_Event thisEvent;

    ES_Tattle(); // trace call stack

    switch (CurrentState) {
        case InitOnTapeSubState: // If current state is initial Psedudo State
            if (ThisEvent.EventType == ES_INIT)// only respond to ES_Init
            {
            }
             /*DANNY: The normal ES_Framework has us transition out of this state here.
             *      I have us stay in this InitSudoState until we get an actual event.
             *      The actual event will be processed by the 'if' statement below and
             *      we will be sent to the correct next state.
             */

            if (ThisEvent.EventType == ES_ENTRY) {
                printf("\n\n-------IN ON TAPE--------\n\n");
                if ((ThisEvent.EventParam & 0x2) == 0x2) { //front right is on tape
                    nextState = onTape_frontRightOn;
                    makeTransition = TRUE;
                    ThisEvent.EventType = ES_NO_EVENT;
                } else if ((ThisEvent.EventParam & 0x20) == 0x20) { //front left is on tape
                    nextState = frontLeftOn;
                    makeTransition = TRUE;
                    ThisEvent.EventType = ES_NO_EVENT;
//                } else if ((ThisEvent.EventParam & 0x8) == 0x8) { //mid left is on tape. this means we drove into a T
//                    nextState = pivotCWfromMLor06Init;
//                    makeTransition = TRUE;
//                    ThisEvent.EventType = ES_NO_EVENT;
//                } else if ((ThisEvent.EventType & 0x4) == 0x4) { //mid right is on tape. This means we drove into a T
//                    nextState = pivotCCWfromMRInit;
//                    makeTransition = TRUE;
//                    ThisEvent.EventType = ES_NO_EVENT;
//                } else if ((ThisEvent.EventParam & 0x6) == 0x6){ //both FR and MR went on must be on
//                    nextState = pivotCWfromMLor06Init;
//                    makeTransition = TRUE;
//                    ThisEvent.EventType = ES_NO_EVENT;
                } else {
                    printf("invalid entry, leaving onTape\n");
                    thisEvent.EventType = LOST_TAPE;
                    PostEwokHSM(thisEvent);
                }
            }
            break; //end initPsubState


        case onTape_frontRightOn: // in the first state, replace this with correct names
            if (ThisEvent.EventType != ES_NO_EVENT) { // An event is still active
                switch (ThisEvent.EventType) {
                    case ES_ENTRY:
                        printf("enter frontRightOn\n");
                        driveForward(SLOW_SPEED); //we need to go slow so that we can carefully line up with the tape
                        ES_Timer_InitTimer(OH_SHIT_TIMER, FRONT_RIGHT_ON_TIME );
                        break;
                    case TAPE_CHANGE:
                        if ((ThisEvent.EventParam & 0x2) == 0) { //sensor state is XXXX0X. This means front right sensor is now off tape. we need to pivot CCW
                            nextState = pivotCCW;
                            makeTransition = TRUE;
                            ThisEvent.EventType = ES_NO_EVENT;
                        } else if ((ThisEvent.EventParam & 0x3) == 0x3) { //sensor state is XXXX11. This means front right and back right are on tape. we need to follow line
                            transitionTapeValues = ThisEvent.EventParam;
                            nextState = followLineRight;
                            makeTransition = TRUE;
                            ThisEvent.EventType = ES_NO_EVENT;
                        } else if (((ThisEvent.EventParam & 0x6) ^ 0x4) == 0x6) { //sensor state is XXX01X. Mid right came off before front right. must be at T. Get Away.
                            nextState = getAwayPivot;
                            makeTransition = TRUE;
                            ThisEvent.EventType = ES_NO_EVENT;
                        }
                        break;
                    case ES_TIMEOUT:
                        if (ThisEvent.EventParam == OH_SHIT_TIMER) {
                            nextState = reverseBeforeGetAway;
                            makeTransition = TRUE;
                            ThisEvent.EventParam = ES_NO_EVENT;
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
            break; //end frontRightOn

        case reverseBeforeGetAway: // in the first state, replace this with correct names
            if (ThisEvent.EventType != ES_NO_EVENT) { // An event is still active
                switch (ThisEvent.EventType) {
                    case ES_ENTRY:
                        printf("enter reverseBeforeGetAway\n");
                        driveForward(-SPEED);
                        ES_Timer_InitTimer(OH_SHIT_TIMER, REVERSE_BEFORE_GET_AWAY_TIME);
                        break;

                    case ES_TIMEOUT:
                        nextState = getAwayPivot;
                        makeTransition = TRUE;
                        ThisEvent.EventParam = ES_NO_EVENT;
                        break;

                    case ES_EXIT:
                        // this is where you would put any actions associated with the
                        // exit from this state
                        break;

                    default: // all unhandled events pass the event back up to the next level
                        break;
                }
            }
            break; //end frontRightOn
        case pivotCCW:
            if (ThisEvent.EventType != ES_NO_EVENT) {
                switch (ThisEvent.EventType) {
                    case ES_ENTRY:
                        printf("enter pivotCCW\n");
                        pivotLeft(SLOW_SPEED);
                        pivotEntryCount = pivotEntryCount + 1;
                        ES_Timer_InitTimer(OH_SHIT_TIMER, OH_SHIT_PIVOT_TIME);
                        if (pivotEntryCount >= 5) {
                            nextState = getAwayPivot;
                            makeTransition = TRUE;
                            ThisEvent.EventType = ES_NO_EVENT;
                        }
                        break;
                    case TAPE_CHANGE:
                        if ((ThisEvent.EventParam == 2)) {//sensor state is XXXX10. this means front right is on tape.
                            nextState = onTape_frontRightOn;
                            makeTransition = TRUE;
                            ThisEvent.EventType = ES_NO_EVENT;
                        } else if ((ThisEvent.EventParam & 0x3) == 0x3) { //sensor state is XXXX11 this means front and back right are on tape. we need to follow line
                            transitionTapeValues = ThisEvent.EventParam;
                            nextState = followLineRight;
                            makeTransition = TRUE;
                            ThisEvent.EventType = ES_NO_EVENT;
                        } else if ((ThisEvent.EventParam & 0x3) ^ 0x2) { /*sensor state is XXXX01. this means back right went on while front right was still off.
                                                                          we need to reverse until the back right comes off the tape. Otherwise, we will
                                                                          end up over rotating.*/
                            nextState = reverse;
                            makeTransition = TRUE;
                            ThisEvent.EventType = ES_NO_EVENT;
                        }

                    case ES_TIMEOUT:
                        switch (ThisEvent.EventParam) {
                            case OH_SHIT_TIMER:
//                                nextState = getAwayPivot;
//                                makeTransition = TRUE;
//                                ThisEvent.EventType = ES_NO_EVENT;
                                break;
                        }
                        break;

                    case ES_EXIT:
                        break;

                    default:
                        break;
                }
            }
            break; //End pivotCCW

        case reverse:
            if (ThisEvent.EventType != ES_NO_EVENT) {
                switch (ThisEvent.EventType) {
                    case ES_ENTRY:
                        printf("enter reverse\n");
                        driveForward(-1 * SLOW_SPEED);
                        ES_Timer_InitTimer(OH_SHIT_TIMER, OH_SHIT_REVERSE_TIME);
                        break;

                    case TAPE_CHANGE:
                        if ((ThisEvent.EventParam & 0x1) != 0x1) { //sensor state is XXXXX0. Back right is off the tape. go back to pivoting CCW
                            nextState = pivotCCW;
                            makeTransition = TRUE;
                            ThisEvent.EventType = ES_NO_EVENT;
                        } else if (ThisEvent.EventParam & 0x3 == 0x3) { //sensor state is XXXX11. back and front right are on tape. we need to line follow
                            transitionTapeValues = ThisEvent.EventParam;
                            nextState = followLineRight;
                            makeTransition = TRUE;
                        }
                        break;

                    case ES_TIMEOUT:
                        if(ThisEvent.EventParam == OH_SHIT_TIMER){
                            nextState = getAwayPivot;
                            makeTransition = TRUE;
                            ThisEvent.EventParam = ES_NO_EVENT;
                        }
                        break;

                    case ES_EXIT:
                        break;

                    default:
                        break;
                }
            }
            break;


        case followLineRight:
            /* DANNY: This is a heirarchial state. its sub machine is named
             * LineFollowRightSubSubSubHSM
             */

            if (transitionTapeValues) { //these are used to pass in the tape sensor values that got us here.
                ThisEvent.EventParam = transitionTapeValues;
                transitionTapeValues = 0;
            }
            ThisEvent = RunLineFollowRightSubSubSubHSM(ThisEvent); // run sub-state machine for this state
            if (ThisEvent.EventType != ES_NO_EVENT) { // An event is still active
                switch (ThisEvent.EventType) {
                    case ES_ENTRY:
                        printf("\n\n---------IN LINE FOLLOW-------\n\n");
                        break;
                    case TAPE_CHANGE:
                        if (((ThisEvent.EventParam & 0x28) & 0x20) == 0x20 || ((ThisEvent.EventParam & 0x28) & 0x08) == 0x08) { //sensor state is 0x28 = 1X1XXX, 0x20 = 1XXXXX, 0x08 = XX1XXX
                            nextState = rightCorner;
                            makeTransition = TRUE;
                            ThisEvent.EventType = ES_NO_EVENT;
                        }
                        break;

                    case BUMP_CHANGE:
                        transitionBumpValues = ThisEvent.EventParam;
                        nextState = ONT_roachTestReverse;
                        makeTransition = TRUE;
                        ThisEvent.EventType = ES_NO_EVENT;

                        break;

                    case ES_EXIT:
                        printf("Exiting onTape::followLineRight\n");
                        //InitLineFollowRightSubSubSubHSM();
                        break;

                    default:
                        //do nothing
                        break;
                }
            }
            break; //end followLineRight


        case rightCorner:
            ThisEvent = RunRightCornerSubSubSubHSM(ThisEvent);
            if (ThisEvent.EventType != ES_NO_EVENT) { // An event is still active
                switch (ThisEvent.EventType) {
                    case ES_ENTRY:
                        printf("\n\n---------IN RIGHT CORNER-------\n\n");
                        break;

                    case CORNER_DONE:
                        nextState = followLineRight;
                        makeTransition = TRUE;
                        break;

                    case ES_EXIT:
                        printf("exiting onTape::rightCorner\n");
                        //InitRightCornerSubSubSubHSM();
                        break;

                    default:
                        //do nothing
                        break;
                }
            }
            break; //end rightCorner


            break;

        case positioning:
            if (transitionBumpValues) {
                ThisEvent.EventParam = transitionBumpValues;
                transitionBumpValues = 0;
            }
            ThisEvent = RunPositioningSubSubSubHSM(ThisEvent);
            if (ThisEvent.EventType != ES_NO_EVENT) {
                switch (ThisEvent.EventType) {
                    case ES_ENTRY:
                        printf("\n\n---------IN POSITIONING-------\n\n");
                        break;

                    case RETRY_T:
                        nextState = getAwayPivot;
                        makeTransition = TRUE;
                        ThisEvent.EventType = ES_NO_EVENT;
                        break;

                    case ES_EXIT:
                        printf("exiting onTape::positioning\n");
                        InitPositioningSubSubSubHSM();
                        break;

                    default:
                        break;
                }
            }
            break; /*END POSITIONING*/


        case frontLeftOn:
            if (ThisEvent.EventType != ES_NO_EVENT) { // An event is still active
                switch (ThisEvent.EventType) {
                    case ES_ENTRY:
                        printf("enter frontLeftOn\n");
                        pivotLeft(5);
                        break;

                    case TAPE_CHANGE:
                        if ((ThisEvent.EventParam & 0x2) == 0x2) {//TAPE STATE XXXX1X
                            nextState = onTape_frontRightOn;
                            makeTransition = TRUE;
                            ThisEvent.EventType = ES_NO_EVENT;
                        }
                        break;

                    default:
                        //do nothing
                        break;
                }
            }
            break; //end frontLeftOn

        case pivotCWfromMLor06Init:
            if (ThisEvent.EventType != ES_NO_EVENT) { // An event is still active
                switch (ThisEvent.EventType) {
                    case ES_ENTRY:
                        printf("enter pivotCWfromMLInit\n");
                        pivotRight(3);
                        break;

                    case TAPE_CHANGE:
                        if ((ThisEvent.EventParam & 0x10) == 0x10) {//TAPE STATE X1XXXX
                            nextState = waitForBLoff;
                            makeTransition = TRUE;
                            ThisEvent.EventType = ES_NO_EVENT;
                        }
                        break;

                    default:
                        //do nothing
                        break;
                }
            }
            break; //end pivotCWfromMLInit


        case waitForBLoff:
            if (ThisEvent.EventType != ES_NO_EVENT) { // An event is still active
                switch (ThisEvent.EventType) {
                    case ES_ENTRY:
                        printf("enter waitForBLoff\n");
                        pivotRight(3);
                        break;

                    case TAPE_CHANGE:
                        if ((ThisEvent.EventParam & 0x10) == 0) {//TAPE STATE X0XXXX
                            nextState = cautiousReverse;
                            makeTransition = TRUE;
                            ThisEvent.EventType = ES_NO_EVENT;
                        }
                        break;

                    default:
                        //do nothing
                        break;
                }
            }
            break; //end waitForBLoff

        case cautiousReverse:
            if (ThisEvent.EventType != ES_NO_EVENT) { // An event is still active
                switch (ThisEvent.EventType) {
                    case ES_ENTRY:
                        printf("enter cautiousReverse\n");
                        driveForward(-3);
                        ES_Timer_InitTimer(POSITIONING_TIMER, CAUTIOUS_REV_TIMEOUT);
                        break;

                    case TAPE_CHANGE:
                        if ((ThisEvent.EventParam & 0x1) == 0x1) {//TAPE STATE XXXXX1
                            nextState = pivotCCWfromCautiousRev;
                            makeTransition = TRUE;
                            ThisEvent.EventType = ES_NO_EVENT;
                        } else if((ThisEvent.EventParam & 0x10) == 0x10) { //TAPE STATE X1XXXX
                            nextState = pivotCWfromCautiousRev;
                            makeTransition = TRUE;
                            ThisEvent.EventType = ES_NO_EVENT;
                        }
                        break;

                    default:
                        //do nothing
                        break;
                }
            }
            break; //end cautiousReverse

        case pivotCCWfromCautiousRev:
            if (ThisEvent.EventType != ES_NO_EVENT) { // An event is still active
                switch (ThisEvent.EventType) {
                    case ES_ENTRY:
                        printf("enter pivotCCWfromCautiousRev\n");
                        pivotLeft(2);
                        ES_Timer_InitTimer(POSITIONING_TIMER, CAUTIOUS_REV_PIVOT_TIME);
                        break;

                    case ES_TIMEOUT:
                        if (ThisEvent.EventParam == POSITIONING_TIMER) {
                            nextState = cautiousReverse;
                            makeTransition = TRUE;
                            ThisEvent.EventType = ES_NO_EVENT;
                        }
                        break;

                    default:
                        //do nothing
                        break;
                }
            }
            break; //end pivotCCWfromCautiousRev

        case pivotCWfromCautiousRev:
            if (ThisEvent.EventType != ES_NO_EVENT) { // An event is still active
                switch (ThisEvent.EventType) {
                    case ES_ENTRY:
                        printf("enter pivotCWfromCautiousRev\n");
                        pivotRight(2);
                        ES_Timer_InitTimer(POSITIONING_TIMER, CAUTIOUS_REV_PIVOT_TIME);
                        break;

                    case ES_TIMEOUT:
                        if (ThisEvent.EventParam == POSITIONING_TIMER) {
                            nextState = cautiousReverse;
                            makeTransition = TRUE;
                            ThisEvent.EventType = ES_NO_EVENT;
                        }
                        break;

                    default:
                        //do nothing
                        break;
                }
            }
            break; //end pivotCWfromCautiousRev

        case pivotCCWfromMRInit:
            if (ThisEvent.EventType != ES_NO_EVENT) { // An event is still active
                switch (ThisEvent.EventType) {
                    case ES_ENTRY:
                        printf("enter pivotCCWfromMRInit\n");
                        pivotLeft(3);
                        break;

                    case TAPE_CHANGE:
                        if ((ThisEvent.EventParam & 0x1) == 0x1) {//TAPE STATE XXXXX1
                            nextState = waitForBRoff;
                            makeTransition = TRUE;
                            ThisEvent.EventType = ES_NO_EVENT;
                        }
                        break;

                    default:
                        //do nothing
                        break;
                }
            }
            break; //end pivotCCWfromMRInit

        case waitForBRoff:
            if (ThisEvent.EventType != ES_NO_EVENT) { // An event is still active
                switch (ThisEvent.EventType) {
                    case ES_ENTRY:
                        printf("enter waitForBRoff\n");
                        pivotLeft(3);
                        break;

                    case TAPE_CHANGE:
                        if ((ThisEvent.EventParam & 0x1) == 0) {//TAPE STATE XXXXX0
                            nextState = cautiousReverse;
                            makeTransition = TRUE;
                            ThisEvent.EventType = ES_NO_EVENT;
                        }
                        break;

                    default:
                        //do nothing
                        break;
                }
            }
            break; //end waitForBRoff

        case getAwayPivot:
            if (ThisEvent.EventType != ES_NO_EVENT) { // An event is still active
                switch (ThisEvent.EventType) {
                    case ES_ENTRY:
                        pivotLeft(8);
                        ES_Timer_InitTimer(POSITIONING_TIMER, GET_AWAY_ROTATE_TIME);
                        printf("enter getAwayPivot\n");
                        break;

                    case ES_TIMEOUT:
                        if (ThisEvent.EventParam == POSITIONING_TIMER) {
                            nextState = getAwayDrive;
                            makeTransition = TRUE;
                            ThisEvent.EventType = ES_NO_EVENT;
                        }
                        break;

                    default:
                        //do nothing
                        break;
                }
            }
            break; //end getAwayRotate


        case getAwayDrive:
           if (ThisEvent.EventType != ES_NO_EVENT) { // An event is still active
                switch (ThisEvent.EventType) {
                    case ES_ENTRY:
                        printf("enter getAwayDrive\n");
                        driveForward(10);
                        ES_Timer_InitTimer(POSITIONING_TIMER, GET_AWAY_DEBOUNCE_TIME);
                        ES_Timer_InitTimer(OH_SHIT_TIMER, OH_SHIT_GET_AWAY_DRIVE_TIME);
                        break;

                    case TAPE_CHANGE:
                        if (ThisEvent.EventParam == 0) { //all tape sensors are now off tape.
                            ES_Timer_InitTimer(POSITIONING_TIMER, GET_AWAY_DEBOUNCE_TIME);
                        } else {
                            ES_Timer_StopTimer(POSITIONING_TIMER);
                        }
                        ThisEvent.EventType = ES_NO_EVENT;
                        break;

                    case ES_TIMEOUT:
                        if (ThisEvent.EventParam == POSITIONING_TIMER) {
                            printf("posting LOST_TAPE\n");
                            thisEvent.EventType = LOST_TAPE;
                            PostEwokHSM(thisEvent);
                            ThisEvent.EventType = ES_NO_EVENT;
                        } else if (ThisEvent.EventParam == OH_SHIT_TIMER) {
                            nextState = getAwayPivot;
                            makeTransition = TRUE;
                            ThisEvent.EventType = ES_NO_EVENT;
                        }
                        break;

                    case ES_EXIT:
                        printf("exit getAwayDrive\n");

                    default:
                        //do nothing
                        break;
                }
            }
            break; //end getAwayDrive



        case ONT_roachTestReverse:
            if (ThisEvent.EventType != ES_NO_EVENT) { // An event is still active
                switch (ThisEvent.EventType) {
                    case ES_ENTRY:
                        driveForward(-SPEED);
                        ES_Timer_InitTimer(POSITIONING_TIMER, ONT_ROACH_TEST_REVERSE_TIME);
                        printf("enter ONT_roachTestReverse\n");
                        break;

                    case ES_TIMEOUT:
                        if (ThisEvent.EventParam == POSITIONING_TIMER) {
                            nextState = ONT_roachTestForward;
                            makeTransition = TRUE;
                            ThisEvent.EventType = ES_NO_EVENT;
                        }
                        break;

                    default:
                        //do nothing
                        break;
                }
            }
            break;


        case ONT_roachTestForward:
            if (ThisEvent.EventType != ES_NO_EVENT) { // An event is still active
                switch (ThisEvent.EventType) {
                    case ES_ENTRY:
                        driveForward(-SPEED);
                        printf("enter ONT_roachTestReverse\n");
                        ES_Timer_InitTimer(POSITIONING_TIMER, ONT_ROACH_TEST_PAUSE_TIME);
                        break;

                    case ES_TIMEOUT:
                        if (ThisEvent.EventParam == POSITIONING_TIMER) {
                            driveForward(SPEED);
                        }
                        break;


                    case BUMP_CHANGE:
                        nextState = positioning;
                        makeTransition = TRUE;
                        ThisEvent.EventType = ES_NO_EVENT;
                        break;

                    default:
                        //do nothing
                        break;
                }
            }
            break;

        default:
            break;
    } // end switch on Current State

    if (makeTransition == TRUE) { // making a state transition, send EXIT and ENTRY
        // recursively call the current state with an exit event
        RunOnTapeSubSubHSM(EXIT_EVENT); // <- rename to your own Run function
        CurrentState = nextState;
        RunOnTapeSubSubHSM(ENTRY_EVENT); // <- rename to your own Run function
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