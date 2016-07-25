#include <BOARD.h>
#include "ES_Configure.h"
#include "ES_Framework.h"
#include "EwokEvents.h"
#include "Ewok.h"
#include "EwokHSM.h"

#define DARK_THRESHOLD 234
#define LIGHT_THRESHOLD 88

//This is simply a placeholder. Figure out how checkBumps is called and works and implement from there.

uint8_t CheckBumps(void)
{
//    static int lastBumpState = 0;
//    int currBumpState;
//    uint8_t returnVal = FALSE;
//
//    ES_Event thisEvent;
//    currBumpState = Ewok_ReadBumpers();
//
//    if (currBumpState != lastBumpState) {
//        // posting event
////        thisEvent.EventType = BUMPED;
//        thisEvent.EventParam = (uint16_t) currBumpState;
//        PostEwokHSM(thisEvent);
//
//        returnVal = TRUE;
//        lastBumpState = currBumpState;
//    }
//    return lastBumpState;
}

uint8_t CheckTape(void)
{
//    static int lastTapeState = 0;
//    int currTapeState;
//    uint8_t returnVal = FALSE;
//
//    ES_Event thisEvent;
//    //currTapeState = Ewok_ReadTape();
//
//    if (currTapeState != lastTapeState) {
//        // Add threshold?
//        // posting event
//        //thisEvent.EventType = TAPE_ON;
//        thisEvent.EventParam = (uint16_t) currTapeState;
//
//        returnVal = TRUE;
//        lastTapeState = currTapeState;
//    }
//    return lastTapeState;
}

uint16_t CheckTrackWire(void) {
//    ES_Event thisEvent;
//    uint8_t returnVal = FALSE;
//    static uint16_t trackwire[2] = 0;
//    static uint16_t oldValue[2] = 0;
//
//   if (AD_IsNewDataReady()) {
//        trackwire[0] = AD_ReadADPin(AD_PORTW3);
//        trackwire[1] = AD_ReadADPin(AD_PORTW4);
//
//        if ((trackwire[0] - DELTA) > oldValue) || ((trackwire[0] + DELTA) < oldValue)) {
//           thisEvent.EventType = TRACK_WIRE;
//            thisEvent.EventParam = FRONT_RIGHT_TRACK;
//            PostEwokHSM(thisEvent);
//            returnVal TRUE;
//            oldvalue[0] = trackwire[0];
//        }
//
//       if ((trackwire[1] - DELTA) > oldValue) || ((trackwire[1] + DELTA) < oldValue)) {
//            thisEvent.EventType = TRACK_WIRE;
//            thisEvent.EventParam = FRONT_RIGHT_TRACK;
//            PostEwokHSM(thisEvent);
//            returnVal TRUE;
//            oldvalue[1] = trackwire[1];
//        }
//    }
}
