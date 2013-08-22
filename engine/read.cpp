//////////////////////////////////////////////////////////////////////////
//                                                                      //
//   BOOM 2 Engine                                                      //
//                                                                      //
//   read.cpp - basic reading functions implementation                  //
//                                                                      //
//   by Ivaylo Beltchev                                                 //
//   e-mail: ivob@geocities.com                                         //
//   www:    www.geocities.com/SiliconValley/Bay/2234                   //
//                                                                      //
//////////////////////////////////////////////////////////////////////////

#define _READ_CPP
#include "read.h"

#include <File.h>
#include <fcntl.h>
#include <string.h>
#include <stdio.h>

#define MAXFILES 10

extern BFile *fd;

// opens a file for reading
bool read_start( char *fn )
{
  fd = new BFile(fn, B_READ_ONLY);
  if (fd->InitCheck() != B_OK) return false;
  return true;
}

// closes the current file
void read_end()
{
  if (fd->InitCheck() != B_OK) fd->Unset();
  delete fd;
}

// returns the current file pointer
long read_pos()
{
  return fd->Position();
}

// skips len bytes
void skip(unsigned len)
{
  fd->Seek(len,SEEK_CUR);
}

// reads size bytes in buf
void rdbuf( void *buf, unsigned size )
{
  fd->Read(buf, size );
}

// reads a 4 byte integer
long rdlong( void )
{
  long buf;
  fd->Read( &buf, sizeof(buf) );
  return buf;
}

// reads a 2 byte integer
short rdshort( void )
{
  short buf;
  fd->Read( &buf, sizeof(buf) );
  return buf;
}

// reads a 1 byte integer
char rdchar( void )
{
  char buf;
  fd->Read( &buf, sizeof(buf) );
  return buf;
}

// reads a float
float rdfloat( void )
{
  float buf;
  fd->Read( &buf, sizeof(buf) );
  return buf;
}
