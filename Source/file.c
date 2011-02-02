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

#include "file.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>


#ifdef DMALLOC
#include <dmalloc.h>
#endif
#include "util.h"

/****h* ROBODoc/Filename
 * NAME
 *   Functions to deal with keeping track
 *   of filenames and directory names.
 *****
 */

/****f* Filename/RB_Get_RB_Filename
 * NAME
 *   RB_Get_RB_Filename
 * SYNOPSIS
 */
struct RB_Filename* RB_Get_RB_Filename( char *arg_filename, struct RB_Path *arg_rb_path )
/*
 * INPUTS
 *   o arg_rb_filename --
 *   o arg_rb_path --
 * FUNCTION
 *   Create a new RB_Filename structure based on arg_filename and
 *   arg_rb_path.
 * SOURCE
 */
{
    struct RB_Filename *rb_filename =
        ( struct RB_Filename * ) malloc( sizeof( struct RB_Filename ) );
    rb_filename->name = ( char * ) malloc( strlen( arg_filename ) + 1 );
    rb_filename->docname = 0;
    rb_filename->fullname = 0;
    rb_filename->fulldocname = 0;
    strcpy( rb_filename->name, arg_filename );
    rb_filename->path = arg_rb_path;
    return rb_filename;
}

/*****/

struct RB_Filename *RB_Copy_RB_Filename( struct RB_Filename* arg_rb_filename )
{
    return RB_Get_RB_Filename( arg_rb_filename->name, arg_rb_filename->path );
}


void
RB_Filename_Dump( struct RB_Filename *arg_rb_filename )
{
    printf( "[%s %s %s]  ", RB_Get_Path( arg_rb_filename ),
            RB_Get_Filename( arg_rb_filename ),
            RB_Get_Extension( arg_rb_filename ) );
    printf( "%s\n", Get_Fullname( arg_rb_filename ) );
}

/*x**f* Filename/RB_Free_RB_Filename 
 * NAME
 *   RB_Free_RB_Filename -- free a RB_Filename structure.
 *
 *****
 * TODO Documentation
 */

void
RB_Free_RB_Filename( struct RB_Filename *arg_rb_filename )
{
    free( arg_rb_filename->name );
    if ( arg_rb_filename->docname )
    {
        free( arg_rb_filename->docname );
    }
    if ( arg_rb_filename->fullname )
    {
        free( arg_rb_filename->fullname );
    }
    if ( arg_rb_filename->fulldocname )
    {
        free( arg_rb_filename->fulldocname );
    }
    free( arg_rb_filename );
}

/* Set the fulldoc name, this is used in singledoc mode
 * since there the docname is preset by the user and not
 * derived from the sourcefile name.
 */

void RB_Set_FullDocname( struct RB_Filename *arg_rb_filename, char* name )
{
    arg_rb_filename->fulldocname = RB_StrDup( name );
}

/* TODO Documentation RB_Get_FullDocname */
char               *
RB_Get_FullDocname( struct RB_Filename *arg_rb_filename )
{
    char               *result = arg_rb_filename->fulldocname;

    if ( result == NULL )
    {
        unsigned int  size = strlen( arg_rb_filename->docname ) +
            strlen( arg_rb_filename->path->docname ) + 1;
        result = ( char * ) malloc( size * sizeof( char ) );
        assert( result );
        *result = '\0';
        strcat( result, arg_rb_filename->path->docname );
        strcat( result, arg_rb_filename->docname );
        /* Save the result so it can be reused later on, and we can properly deallocate it. */
        arg_rb_filename->fulldocname = result;
    }
    return result;
}


/****f* Filename/Get_Fullname
 * NAME
 *   Get_Fullname --
 * SYNOPSIS
 */
char* Get_Fullname( struct RB_Filename *arg_rb_filename )
/*
 * FUNCTION
 *   Give the full name of the file, that is the name of
 *   the file including the extension and the path.
 *   The path can be relative or absolute.
 * NOTE
 *   The string returned is owned by this function
 *   so don't change it.
 * SOURCE
 */
{
    char               *result = arg_rb_filename->fullname;

    if ( result == NULL )
    {
        unsigned int        size = strlen( arg_rb_filename->name ) +
            strlen( arg_rb_filename->path->name ) + 1;
        result = ( char * ) malloc( size * sizeof( char ) );
        assert( result );
        *result = '\0';
        strcat( result, arg_rb_filename->path->name );
        strcat( result, arg_rb_filename->name );
        /* Save the result so it can be reused later on, and we can properly deallocate it. */
        arg_rb_filename->fullname = result;
    }
    return result;
}
/******/

/****f* Filename/RB_Get_Path
 * SYNOPSIS
 */
char* RB_Get_Path( struct RB_Filename *arg_rb_filename )
/*
 * FUNCTION
 *   Give the path for this file.
 * NOTE
 *   The string returned is owned by this function
 *   so don't change it.
 ******
 */
{
    return arg_rb_filename->path->name;
}


/****f* Filename/RB_Get_Extension
 * NAME
 *   RB_Get_Extension --
 * FUNCTION
 *   Give the extension of this file. That is the part after
 *   the last '.' if there is any.
 * SYNOPSIS
 */
char* RB_Get_Extension( struct RB_Filename *arg_rb_filename )
/*
 * RESULT
 *   pointer to the extension
 *   pointer to a '\0' if no extension was found.
 * NOTE
 *   The string returned is owned by this function
 *   so don't change it.
 * SOURCE
 */
{
    char               *c = arg_rb_filename->name;
    int                 i = strlen( c );

    for ( c += i; c != arg_rb_filename->name && ( *c != '.' ); --c )
    {
        /* Empty */
    }
    if ( *c == '.' )
    {
        ++c;
    }
    else
    {
        c = arg_rb_filename->name;
        c += i;
    }
    return c;
}
/*****/

/****f* Filename/RB_Get_Filename
 * NAME
 *   RB_Get_Filename --
 * FUNCTION
 *   Give the name of this file. That is the name
 *   of the file without its path but with the
 *   extension.
 * SYNOPSIS
 */
char* RB_Get_Filename( struct RB_Filename *arg_rb_filename )
/*
 * RESULT
 *   pointer to the extension
 *   pointer to a '\0' if no extension was found.
 * NOTE
 *   The string returned is owned by this function
 *   so don't change it.
 ******
 */
{
    return arg_rb_filename->name;
}
