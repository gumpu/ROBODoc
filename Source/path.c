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

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "path.h"
#include "robodoc.h"
#include "headers.h"
#include "util.h"

#ifdef DMALLOC
#include <dmalloc.h>
#endif

/* TODO Documentation */

struct RB_Path     *
RB_Get_RB_Path( char *arg_pathname )
{
    struct RB_Path     *rb_path;
    int                 needs_slash = FALSE;

    if ( strlen( arg_pathname ) == 0 )
    {
        RB_Panic( "Trying to use a path with as name \"\"\n" );
        return 0;               /* Keep the compiler happy. */
    }
    else
    {
        /* Check if the path ends with a / if not we will need to add one. */
        needs_slash = ( arg_pathname[strlen( arg_pathname ) - 1] != '/' );
        rb_path = ( struct RB_Path * ) malloc( sizeof( struct RB_Path ) );

        if ( ! rb_path )
        {
            RB_Panic( "Out of memory! %s()\n", "RB_Get_RB_Path" );
        }

        /* 2 extra for the '/' and '\0'  */
        rb_path->name =
            ( char * ) calloc( strlen( arg_pathname ) + 2, sizeof( char ) );

        if ( ! rb_path->name )
        {
            RB_Panic( "Out of memory! %s()\n", "RB_Get_RB_Path" );
        }

        *( rb_path->name ) = '\0';
        rb_path->parent = NULL;
        rb_path->next = NULL;
        strcat( rb_path->name, arg_pathname );
        if ( needs_slash )
        {
            strcat( rb_path->name, "/" );
        }
        rb_path->docname = NULL;
    }
    return rb_path;
}

/*x**f* ROBODoc/RB_Get_RB_Path2
 * NAME
 *   RB_Get_RB_Path2 -- create a new RB_Path structure.
 * FUNCTION
 * NOTE
 *   Has a wrong name...
 *****
 */

/* TODO Documentation */
struct RB_Path     *
RB_Get_RB_Path2( char *arg_current_path, char *arg_subdirectory )
{
    struct RB_Path     *rb_path;
    rb_path = ( struct RB_Path * ) malloc( sizeof( struct RB_Path ) );
    /* allocate memory for the path name,
       it will consist of the current_pathname plus the
       subdirectory plus a '\0' */
    rb_path->name =
        ( char * ) malloc( strlen( arg_current_path ) +
                           strlen( arg_subdirectory ) + 2 );

    if ( ! rb_path->name )
    {
        RB_Panic( "Out of memory! %s()\n", "RB_Get_RB_Path2" );
    }

    strcpy( rb_path->name, arg_current_path );
    strcat( rb_path->name, arg_subdirectory );
    if ( arg_subdirectory[strlen( arg_subdirectory ) - 1] != '/' )
    {
        strcat( rb_path->name, "/" );
    }
    rb_path->docname = NULL;
    rb_path->parent = NULL;
    rb_path->next = 0;
    return rb_path;
}

/*x**f* ROBODoc/RB_Free_RB_Path
 * NAME
 *   RB_Free_RB_Path -- free a RB_Path structure.
 *****
 * TODO Documentation
 */

void
RB_Free_RB_Path( struct RB_Path *arg_rb_path )
{
    free( arg_rb_path->name );
    if ( arg_rb_path->docname )
    {
        free( arg_rb_path->docname );
    }
    free( arg_rb_path );
}

