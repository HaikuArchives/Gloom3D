//////////////////////////////////////////////////////////////////////////
//                                                                      //
//   BOOM 2 Engine                                                      //
//                                                                      //
//   read.h - basic reading functions                                   //
//                                                                      //
//   by Ivaylo Beltchev                                                 //
//   e-mail: ivob@geocities.com                                         //
//   www:    www.geocities.com/SiliconValley/Bay/2234                   //
//                                                                      //
//////////////////////////////////////////////////////////////////////////

#ifndef _READ_H
#define _READ_H

#include "types.h"

// opens a file for reading
bool read_start( char *fn );
// closes the current file
void read_end();
// returns the current file pointer
long read_pos();
// skips len bytes
void skip(unsigned len);
// reads size bytes in buf
void rdbuf( void *buf, unsigned size );

// reads a 4 byte integer
long rdlong();
// reads a 2 byte integer
short rdshort();
// reads a 1 byte integer
char rdchar();
// reads a float
float rdfloat();

#endif
