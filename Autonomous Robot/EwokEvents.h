/* 
 * File:   RoachFrameworkEvents.h
 * Author: mdunne
 *
 * Created on January 3, 2013, 4:53 PM
 */

#ifndef ROACHFRAMEWORKEVENTS_H
#define	ROACHFRAMEWORKEVENTS_H


typedef enum {
    DARK_TO_LIGHT, LIGHT_TO_DARK
} lightstate_t;

uint8_t CheckBumps(void);
uint8_t CheckTape(void);

#endif	/* ROACHFRAMEWORKEVENTS_H */

