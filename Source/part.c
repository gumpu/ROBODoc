/*
Copyright (C) 1994-2007  Frans Slothouber, Jacco van Weert, Petteri Kettunen,
Bernd Koesling, Thomas Aglassinger, Anthon Pang, Stefan Kost, David Druffner,
Sasha Vasko, Kai Hofmann, Thierry Pierron, Friedrich Haase, and Gergely Budai.

This file is part of ROBODoc

ROBODoc is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

/****h* ROBODoc/Part
 * FUNCTION
 *   Structures and functions that deal with documentation parts.  A
 *   part links a sourcefile to the documentation file and contains
 *   all the headers found in a sourcefile.  Parts (in the form of
 *   struct RB_Part) are stored in a RB_Document structure.
 *****
 */

#include <stdlib.h>
#include <assert.h>
#include "headers.h"
#include "file.h"
#include "part.h"
#include "util.h"


#ifdef DMALLOC
#include <dmalloc.h>
#endif


/****f* Part/RB_Get_RB_Part
 * FUNCTION
 *   Create a new RB_Part and initialize it.
 * SYNOPSIS
 */
struct RB_Part* RB_Get_RB_Part( void  )
/*
 * RESULT
 *   A freshly allocated and initializedand RB_Part.
 * SOURCE
 */
{
    struct RB_Part     *part = NULL;
    part = ( struct RB_Part * ) malloc( sizeof( struct RB_Part ) );
    if ( part )
    {
        part->next = NULL;
        part->filename = NULL;
        part->headers = NULL;
        part->last_header = NULL;
    }
    else
    {
        RB_Panic( "Out of memory! RB_Get_RB_Part()" );
    }
    return part;
}

/******/



/****f* Part/RB_Free_RB_Part
 * FUNCTION
 *   Free the memory used by an RB_Part.  Most of this is handled in
 *   other functions.
 * SYNOPSIS
 */
void RB_Free_RB_Part( struct RB_Part *part )
/*
 * INPUTS
 *   o part  -- the part to be freed.
 * SOURCE
 */
{
    /* part->filename  is freed by RB_Directory */
    /* part->headers.  Headers are freed by the document */
    free( part );
}

/*******/

/****f* Part/RB_Open_Source
 * FUNCTION
 *   Open the sourcefile of this part.
 * SYNOPSIS
 */
FILE* RB_Open_Source( struct RB_Part *part )
/*
 * INPUTS
 *   o part -- the part for which the file is opened.
 * SOURCE
 */
{
    char               *sourcefilename = NULL;
    FILE               *result;

    assert( part );
    assert( part->filename );
    sourcefilename = Get_Fullname( part->filename );
    result = fopen( sourcefilename, "r" );
    if ( result ) 
    {
        /* OK */
    }
    else
    {
        RB_Panic( "can't open %s!", sourcefilename );
    }
    return result;
}

/******/


/* TODO Documentation */
FILE* RB_Open_Documentation( struct RB_Part * part )
{
    char               *docfilename = NULL;
    FILE               *result;

    assert( part );
    assert( part->filename );
    docfilename = RB_Get_FullDocname( part->filename );
    RB_Say( "Creating file %s\n", SAY_DEBUG, docfilename );
    result = fopen( docfilename, "w" );
    if ( result ) 
    {
        /* OK */
    }
    else
    {
        RB_Panic( "can't open %s!", docfilename );
    }
    return result;
}


/* TODO Documentation */
void
RB_Part_Add_Source( struct RB_Part *part, struct RB_Filename *sourcefilename )
{
    /* One sourcefile per part. */
    part->filename = sourcefilename;
}

struct RB_Filename *RB_Part_Get_Source( struct RB_Part *part )
{
    return part->filename;
}

/* TODO Documentation */
void
RB_Part_Add_Header( struct RB_Part *part, struct RB_header *header )
{
    assert( header );
    assert( header->module_name );
    assert( header->function_name );

    header->owner = part;
    if ( part->last_header )
    {
        header->next = NULL;
        part->last_header->next = header;
        part->last_header = header;
    }
    else
    {
        header->next = NULL;
        part->headers = header;
        part->last_header = header;
    }
}
