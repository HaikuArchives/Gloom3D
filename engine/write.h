//////////////////////////////////////////////////////////////////////////
//                                                                      //
//   BOOM 2 Engine                                                      //
//                                                                      //
//   write.h - basic writting functions                                 //
//                                                                      //
//   by Ivaylo Beltchev                                                 //
//   e-mail: ivob@geocities.com                                         //
//   www:    www.geocities.com/SiliconValley/Bay/2234                   //
//                                                                      //
//////////////////////////////////////////////////////////////////////////

#ifndef _WRITE_H
#define _WRITE_H

#include "types.h"

// opens a file for writting
void write_start( char *fn );
// closes the current file
bool write_end();
// writes size bytes from buf to the current file
void wrbuf( void *buf, unsigned size );

// writes a 4 byte integer
void wrlong( long l );
// writes a 2 byte integer
void wrshort( short s );
// writes a 1 byte integer
void wrchar( char c );
// writes a float
void wrfloat( float buf );

#endif
