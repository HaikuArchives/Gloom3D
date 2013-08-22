//////////////////////////////////////////////////////////////////////////
//                                                                      //
//   BOOM 2 Engine                                                      //
//                                                                      //
//   player.h - player movement interface                               //
//                                                                      //
//   by Ivaylo Beltchev                                                 //
//   e-mail: ivob@geocities.com                                         //
//   www:    www.geocities.com/SiliconValley/Bay/2234                   //
//                                                                      //
//////////////////////////////////////////////////////////////////////////

#ifndef _PLAYER_H
#define _PLAYER_H

#include "types.h"

#ifdef _PLAYER_CPP
  #define GLOBAL
#else
  #define GLOBAL extern
#endif

// one bit for each key
#define kFORWARD    0x0000001
#define kBACKWARD   0x0000002
#define kLEFT       0x0000004
#define kRIGHT      0x0000008
#define kMOVELEFT   0x0000010
#define kMOVERIGHT  0x0000020
#define kCROUCH     0x0000040
#define kJUMP       0x0000080
#define kLOOKUP     0x0000100
#define kLOOKDOWN   0x0000200
#define kROTLEFT    0x0000400
#define kROTRIGHT   0x0000800
#define kCENTER     0x0001000
#define kRUN        0x0002000
#define kQUIT       0x0004000

GLOBAL volatile dword player_keys; // current keys state (must be set by the application)
GLOBAL coord3d height;  // current player height

#undef GLOBAL

// initialize the player movement
void player_init(coord3d x,coord3d y,coord3d z);
// rotates the player with the given angles
void player_rotate(angle ax,angle ay,angle az);
// advances the player movement with the elapsed time
bool player_idle(long time);

#endif