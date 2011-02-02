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

/****h* ROBODoc/Headers
 * FUNCTION
 *   This module contains a set of variables that define how headers
 *   start and how they end in various programming languages.
 * NOTES
 *   Added C++/ACM header option (David White)
 *   Enables documentation only comments (//!) to be extracted from C++ 
 *   and ACM files, rather than all comments.
 ******
 */

#include <assert.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include "robodoc.h"
#include "headers.h"
#include "globals.h"
#include "roboconfig.h"
#include "util.h"

/* This limits the total number of possible markers. */
/* TODO should be an enum */
#define NO_MARKER_LOCKED 100000
#define NO_MARKER        100002

static int          locked_header_marker = NO_MARKER_LOCKED;
static int          locked_end_marker    = NO_MARKER_LOCKED;
static int          locked_remark_marker = NO_MARKER_LOCKED;

/****v* Headers/header_markers
 * NAME
 *   header_markers -- strings that mark the begin of a header.
 * FUNCTION
 *   These specify what robodoc recognizes as the beginning
 * NOTE
 *   The numbers at the beginning of the lines make it easier to keep
 *   them in sync with the src_constants of a header.
 * SOURCE
 */

char               *header_markers[] = {
    "/****",                    /* 0   C, C++ */
    "//!****",                  /* 1   C++, ACM */
    "//****",                   /* 2   C++ */
    "(****",                    /* 3   Pascal, Modula-2, B52 */
    "{****",                    /* 4   Pascal */
    ";;!****",                  /* 5   Aspen Plus */
    ";****",                    /* 6   M68K assembler */
    "****",                     /* 7   M68K assembler */
    "C     ****",               /* 8   Fortran */
    "REM ****",                 /* 9   BASIC */
    "%****",                    /* 10  LaTeX, TeX, Postscript */
    "#****",                    /* 11  Tcl/Tk */
    "      ****",               /* 12  COBOL */
    "--****",                   /* 13  Occam */
    "<!--****",                 /* 14  HTML Code */
    "<!---****",                /* 15  HTML Code,  the three-dashed comment
                                 * tells the [server] pre-processor not
                                 * to send that comment with the HTML 
                                 */
    "|****",                    /* 16  GNU Assembler */
    "!****",                    /* 17  FORTRAN 90 */
    "!!****",                   /* 18  FORTRAN 90 */
    "$!****",                   /* 19  DCL */
    "'****",                    /* 20  VB, LotusScript */
    ".****",                    /* 21  DB/C */
    "\\ ****",                  /* 22  Forth */
    "<!-- ****",                /* 23  XML */
    NULL
};

/****/

/****v* Headers/robo_header
 * NAME
 *   robo_header -- the distinct robodoc header - 
 *                  alternative to using header_markers
 * FUNCTION
 *   This is an alternative to using header_markers - sometimes
 *   ROBODOC confuses asterisks commonly used in comments as a header.
 *   To use this header instead of header_markers use the -rh switch.
 * NOTE
 *   Added by David Druffner.   OBSOLETE
 * SOURCE
 */

char               *robo_header = "/*ROBODOC*"; /* TODO Remove */

/****/


/****v* Headers/remark_markers
 * NAME
 *   remark_markers
 * FUNCTION
 *   These specify what robodoc recognizes as a comment marker.
 * TODO
 *   (1) All the markers that start with one or more spaces are
 *   never recognized, and should be removed.
 *   (2) The numbers at the beginning of the lines make it easier
 *   to keep them in sync with the src_remark_constants
 * SOURCE
 */

char               *remark_markers[] = {

    " *",                       /* 0  C, C++, Pascal, Modula-2 */
    "//!",                      /* 1  C++, ACM *//* MUST CHECK BEFORE C++ */
    "//",                       /* 2  C++ */
    "*",                        /* 3  C, C++, M68K assembler, Pascal,  Modula-2 */
    ";;!",                      /* 4  Aspen Plus *//* MUST CHECK BEFORE M68K */
    ";*",                       /* 5  M68K assembler */
    ";",                        /* 6  M68K assembler */
    "C",                        /* 7  Fortran */
    "REM",                      /* 8  BASIC */
    "%",                        /* 9  LaTeX, TeX, Postscript */
    "#",                        /* 10 Tcl/Tk */
    "      *",                  /* 11 COBOL */
    "--",                       /* 12 Occam */
    "|",                        /* 13 GNU Assembler */
    "!!",                       /* 14 FORTRAN 90 */
    "!",                        /* 15 FORTRAN 90 */
    "$!",                       /* 16 DCL */
    "'*",                       /* 17 VB */
    ".*",                       /* 18 DB/C */
    "\\",                       /* 19 Forth */
    NULL
};

/****/


/****v* Headers/end_markers [3.0h]
 * NAME
 *   end_markers -- strings that mark the end of a header.
 * FUNCTION
 *   These specify what robodoc recognizes as the end of a 
 *   documentation header. In most cases this will be
 *   "***" or " ***". If the header contains a SOURCE item
 *   then the end of the source has to be marked, which
 *   is when the other strings in this array are used.
 * NOTE
 *   The numbers at the beginning of the lines make it easier
 *   to find a special index-number.
 * SOURCE
 */

char               *end_markers[] = {
    "/***",                     /* 0  C, C++ */
    "//!***",                   /* 1  C++, ACM *//* Must check before C++ */
    "//***",                    /* 2  C++ */
    " ***",                     /* 3  C, C++, Pascal, Modula-2 */
    "{***",                     /* 4  Pascal */
    "(***",                     /* 5  Pascal, Modula-2, B52 */
    ";;!***",                   /* 6  Aspen Plus *//* Must check before M68K */
    ";***",                     /* 7  M68K assembler */
    "***",                      /* 8  M68K assembler */
    "C     ***",                /* 9  Fortran */
    "REM ***",                  /* 10 BASIC */
    "%***",                     /* 11 LaTeX, TeX, Postscript */
    "#***",                     /* 12 Tcl/Tk */
    "      ***",                /* 13 COBOL */
    "--***",                    /* 14 Occam */
    "<!--***",                  /* 15 HTML */
    "<!---***",                 /* 16 HTML */
    "|***",                     /* 17 GNU Assembler */
    "!!***",                    /* 18 FORTRAN 90 */
    "!***",                     /* 19 FORTRAN 90 */
    "$!***",                    /* 20 DCL */
    "'***",                     /* 21 VB, LotusScript */
    ".***",                     /* 22 DB/C */
    "\\ ***",                   /* 23 Forth */
    "<!-- ***",                 /* 24 XML */
    NULL
};

/****/

/****v* Headers/end_remark_markers
 * NAME
 *   end_remark_markers -- strings that mark the end of a comment.
 * NOTE
 *   The numbers at the beginning of the lines make it easier
 *   to keep them in sync with the end_remark_constants
 * SOURCE
 */

char               *end_remark_markers[] = {
    "*/",                       /* 0   C, C++ */
    ( char * ) NULL,            /* 1   C++, ACM */
    ( char * ) NULL,            /* 2   C++ */
    ( char * ) NULL,            /* 3   C, C++, Pascal, Modula-2 */
    "***}",                     /* 5   Pascal */
    "***)",                     /* 6   Pascal, Modula-2, B52 */
    ( char * ) NULL,            /* 7   M68K assembler */
    ( char * ) NULL,            /* 8   M68K assembler */
    ( char * ) NULL,            /* 9   Fortran */
    ( char * ) NULL,            /* 10  BASIC */
    ( char * ) NULL,            /* 11  LaTeX, TeX, Postscript */
    ( char * ) NULL,            /* 12  Tcl/Tk */
    ( char * ) NULL,            /* 13  COBOL */
    ( char * ) NULL,            /* 14  Occam */
    "-->",                      /* 15  HTML  & XML */
    "--->",                     /* 16  HTML */
    ( char * ) NULL,            /* 17  GNU Assembler */
    ( char * ) NULL,            /* 18  FORTRAN 90 !! */
    ( char * ) NULL,            /* 19  FORTRAN 90 ! */
    ( char * ) NULL,            /* 20  VB */
    ( char * ) NULL,            /* 21  Aspen Plus */
    ( char * ) NULL             /* 22  Forth */
};

/****/


/****v* Headers/robo_end [3.0h]
 * NAME
 *   robo_end[] -- the distinct robodoc end marker - 
 *                 alternative to using end_markers
 * FUNCTION
 *   This is an alternative to using end_markers - sometimes ROBODOC
 *   confuses asterisks commonly used in comments as an end marker. To
 *   use this footer instead of end_markers use the -rh switch.
 * NOTE
 *   Added by David Druffner.
 * SOURCE
 */

char               *robo_end[] = { "/*ROBODOC_END*", "*ROBODOC_END*", NULL };

/****/


/***f* Headers/RB_Is_Begin_Marker
 * FUNCTION
 *   Scan a line and see if any of the begin-of-a-header-markers 
 *   defined in header_markers can be found.
 * SYNOPSIS
 */
int RB_Is_Begin_Marker(
    char *cur_line,
    char **type )
/*
 * INPUTS
 *   cur_line -- line to be searched.
 * OUTPUT
 *   type     -- the kind of header
 * RESULT
 *   TRUE  -- a begin header was found
 *   FALSE -- no begin header was found.
 * SOURCE
 */
{
    int                 found = FALSE;
    unsigned int        marker = NO_MARKER;     /* for the assert */
    char               *cur_mchar = NULL;
    char               *cur_char = NULL;

    if ( !( course_of_action.do_robo_head )
         &&
         ( ( ( course_of_action.do_lockheader ) &&
             ( locked_header_marker == NO_MARKER_LOCKED ) )
           || !( course_of_action.do_lockheader ) ) )
    {
        for ( marker = 0;
              ( marker < configuration.header_markers.number ) && !found;
              marker++ )
        {
            cur_mchar = configuration.header_markers.names[marker];
            for ( found = TRUE, cur_char = RB_Skip_Whitespace( cur_line );
                  *cur_mchar && *cur_char && found; cur_mchar++, cur_char++ )
            {
                if ( tolower( *cur_mchar ) != tolower( *cur_char ) )
                    found = FALSE;
            }
            if ( *cur_mchar != '\0' )
            {
                /* It is not a complete match */
                found = FALSE;
            }
        }
    }
    else if ( ( course_of_action.do_lockheader ) &&
              ( locked_header_marker != NO_MARKER_LOCKED ) )
    {
        cur_mchar = configuration.header_markers.names[locked_header_marker];
        for ( found = TRUE, cur_char = RB_Skip_Whitespace( cur_line );
              *cur_mchar && *cur_char && found; cur_mchar++, cur_char++ )
        {
            if ( tolower( *cur_mchar ) != tolower( *cur_char ) )
            {
                found = FALSE;
            }
            if ( *cur_mchar != '\0' )
            {
                /* It is not a complete match */
                found = FALSE;
            }
        }
    }
    else
    {
        assert( 0 );
    }

    if ( found && ( *cur_char == '\0' ) )
    {
        /* start of an header but nothing else */
        found = FALSE;
    }
    else if ( !found ) 
    {
        /* Not found */
    }
    else 
    {
        /* We found the start of a header, now check the remaining
         * part. */

        /*             ****f*
         * We are now here ^
         */
        *type = cur_char;
        ++cur_char;
        if ( *cur_char == '\0' ) 
        {
            found = FALSE;
        }
        else
        {
            /* |           ****f*
             * We are now here  ^
             */
            if ( *cur_char == '*' )
            {
                ++cur_char;
                found = utf8_isspace( *cur_char );
            }
            else if ( *cur_char && ( *( cur_char + 1 ) == '*' ) )
            {
                /* The case ****if*
                 */
                ++cur_char;
                ++cur_char;
                /* |        ****if*
                 * We are now here ^
                 */
                found = ( *cur_char ) && utf8_isspace( *cur_char );
            }
            else
            {
                found = FALSE;
            }

            if ( found )
            {
                found = FALSE;
                /* It should contain some non * characters. */
                for ( ; *cur_char; ++cur_char )
                {
                    if ( utf8_isalnum( *cur_char ) )
                    {
                        found = TRUE;
                    }
                }
            }
        }
    }

    if ( found &&
         ( course_of_action.do_lockheader ) &&
         ( locked_header_marker == NO_MARKER_LOCKED ) )
    {
        assert( marker != NO_MARKER );
        locked_header_marker = marker - 1;
        RB_Say( "header marker locked on %s\n", SAY_INFO,
                configuration.header_markers.names[locked_header_marker] );
    }
    return found;
}

/******/


/* Generic function to skip a remark begin or end marker */

static char        *RB_Skip_Remark_XXX_Marker(
    char *cur_line,
    struct Parameters *parameters )
{
    char               *cur_char;
    char               *space_pos;
    unsigned int        marker;
    int                 found = FALSE;

    cur_char = RB_Skip_Whitespace( cur_line );
    space_pos = strchr( cur_char, ' ' );
    /* Replace the first space on the line with a '\0'
     * this makes the comparison with the remark markers
     * much easier.
     */
    if ( space_pos )
    {
        *space_pos = '\0';
    }

    for ( marker = 0; ( marker < parameters->number ) && !found; marker++ )
    {
        found =
            ( RB_Str_Case_Cmp( cur_char, parameters->names[marker] ) == 0 );
    }

    assert( found );

    if ( space_pos )
    {
        *space_pos = ' ';
        return space_pos;
    }
    else
    {
        return cur_char + strlen( parameters->names[marker - 1] );
    }
}


/* Generic function to see if there is a remark begin or end marker */

static int RB_Is_Remark_XXX_Marker(
    char *cur_line,
    struct Parameters *parameters )
{
    char               *cur_char;
    char               *space_pos;
    unsigned int        marker;
    int                 found = FALSE;

    cur_char = RB_Skip_Whitespace( cur_line );
    space_pos = strchr( cur_char, ' ' );
    /* Replace the first space on the line with a '\0'
     * this makes the comparison with the remark markers
     * much easier.
     */
    if ( space_pos )
    {
        *space_pos = '\0';
    }

    for ( marker = 0; ( marker < parameters->number ) && !found; marker++ )
    {
        found =
            ( RB_Str_Case_Cmp( cur_char, parameters->names[marker] ) == 0 );
    }

    if ( space_pos )
    {
        *space_pos = ' ';
    }

    return found;
}


/* TODO Documentation */
int RB_Is_Remark_End_Marker(
    char *cur_line )
{
    return RB_Is_Remark_XXX_Marker( cur_line,
                                    &( configuration.remark_end_markers ) );
}

/* TODO Documentation */
int RB_Is_Remark_Begin_Marker(
    char *cur_line )
{
    return RB_Is_Remark_XXX_Marker( cur_line,
                                    &( configuration.remark_begin_markers ) );
}

char               *RB_Skip_Remark_Begin_Marker(
    char *cur_line )
{
    return RB_Skip_Remark_XXX_Marker( cur_line,
                                      &( configuration.
                                         remark_begin_markers ) );
}

char               *RB_Skip_Remark_End_Marker(
    char *cur_line )
{
    return RB_Skip_Remark_XXX_Marker( cur_line,
                                      &( configuration.remark_end_markers ) );
}


/****f* Headers/RB_Is_End_Marker
 * FUNCTION
 *   Scan a line and see if any of the end of a header markers 
 *   defined in header_markers can be found.
 * SYNOPSIS
 */
int RB_Is_End_Marker(
    char *cur_line )
/*
 * INPUTS
 *   cur_line -- line to be searched.
 * OUTPUT
 *   none
 * RESULT
 *   TRUE  -- an end header was found
 *   FALSE -- none was found.
 * SOURCE
 */
{
    int                 found = FALSE;
    unsigned int        marker = NO_MARKER;     /* For the assert */
    char               *cur_mchar;
    char               *cur_char;

    if ( !( course_of_action.do_robo_head )
         &&
         ( ( ( course_of_action.do_lockheader ) &&
             ( locked_end_marker == NO_MARKER_LOCKED ) )
           || !( course_of_action.do_lockheader ) ) )
    {
        for ( marker = 0;
              ( marker < configuration.end_markers.number ) && !found;
              marker++ )
        {
            cur_mchar = configuration.end_markers.names[marker];
            cur_char = RB_Skip_Whitespace( cur_line );
            if ( *cur_char )
            {
                for ( found = TRUE;
                      *cur_mchar && *cur_char && found;
                      cur_mchar++, cur_char++ )
                {
                    if ( tolower( *cur_mchar ) != tolower( *cur_char ) )
                    {
                        found = FALSE;
                    }
                }
            }
        }
    }
    else if ( ( course_of_action.do_lockheader ) &&
              ( locked_end_marker != NO_MARKER_LOCKED ) )
    {
        cur_mchar = configuration.end_markers.names[locked_end_marker];
        cur_char = RB_Skip_Whitespace( cur_line );
        if ( *cur_char )
        {
            for ( found = TRUE;
                  *cur_mchar && *cur_char && found; cur_mchar++, cur_char++ )
            {
                if ( tolower( *cur_mchar ) != tolower( *cur_char ) )
                {
                    found = FALSE;
                }
            }
        }
    }
    else
    {
        assert( 0 );
    }

    /* Locking on end markers does not work at the moment,
     * because there can be more than one end marker for
     * a given language. TODO 

    if ( found &&
         ( course_of_action.do_LOCKHEADER ) &&
         ( locked_end_marker == NO_MARKER_LOCKED ) )
    {
        assert( marker != NO_MARKER );
        locked_end_marker = marker - 1;
        RB_Say( "end marker locked on %s\n", SAY_INFO,
                end_markers[locked_end_marker] );
    }
    */

    return found;
}

/*****/


/****f* Headers/RB_Has_Remark_Marker
 * FUNCTION
 *   Check if a line starts with a remark marker.  This function
 *   assumes that the remark marker starts on the first character of
 *   the line.
 * SYNOPSIS
 */
int RB_Has_Remark_Marker(
    char *lline_buffer )
/*
 * INPUTS
 *   o lline_buffer -- the line of text.
 * RESULT
 *   o TRUE -- it starts with a remark marker
 *   o FALSE -- it does not.
 * SOURCE
 */
{
    unsigned int        marker = 0;
    unsigned int        marker_found = configuration.remark_markers.number;
    int                 found = FALSE;
    char               *space_pos = NULL;

    space_pos = strchr( lline_buffer, ' ' );

    /* Replace the first space on the line with a '\0'
     * this makes the comparison with the remark markers
     * much easier.
     */
    if ( space_pos )
    {
        *space_pos = '\0';
    }

    if ( ( ( course_of_action.do_lockheader ) &&
           ( locked_remark_marker == NO_MARKER_LOCKED ) )
         || !( course_of_action.do_lockheader ) )
    {
        for ( marker = 0; marker < configuration.remark_markers.number;
              marker++ )
        {
            if ( RB_Str_Case_Cmp
                 ( lline_buffer,
                   configuration.remark_markers.names[marker] ) == 0 )
            {
                marker_found = marker;
                found = TRUE;
            }
        }
    }
    else
    {
        if ( RB_Str_Case_Cmp
             ( lline_buffer,
               configuration.remark_markers.names[locked_remark_marker] ) ==
             0 )
        {
            marker_found = marker;
            found = TRUE;
        }
    }

    if ( found &&
         ( locked_remark_marker == NO_MARKER_LOCKED )
         && ( course_of_action.do_lockheader ) )
    {
        assert( marker_found < configuration.remark_markers.number );
        locked_remark_marker = marker_found;
        RB_Say( "remark marker locked on %s\n", SAY_INFO,
                configuration.remark_markers.names[locked_remark_marker] );
    }

    /* Restore the space we replaced with a '\0' */
    if ( space_pos )
    {
        *space_pos = ' ';
    }

    return found;
}

/******/


/****f* Headers/RB_Skip_Remark_Marker [2.0e]
 * NAME
 *    RB_Skip_Remark_Marker
 * SYNOPSIS
 */
char               *RB_Skip_Remark_Marker(
    char *lline_buffer )
/*
 * FUNCTION
 *    Scan and search for a recognized remark marker; skip past the
 *    marker to the body of the text
 * SOURCE
 */
{
    unsigned int        marker, found;
    char               *cur_char, *cur_mchar;

    found = FALSE;
    cur_char = NULL;
    for ( marker = 0;
          ( marker < configuration.remark_markers.number ) && !found;
          marker++ )
    {
        cur_mchar = configuration.remark_markers.names[marker];
        for ( found = TRUE, cur_char = lline_buffer;
              *cur_mchar && *cur_char && found; cur_mchar++, cur_char++ )
        {
            if ( tolower( *cur_mchar ) != tolower( *cur_char ) )
            {
                found = FALSE;
            }
        }
    }
    return ( cur_char );
}

/**************/


/* TODO Documentation */
void RB_Header_Lock_Reset(
    void )
{
    locked_header_marker = NO_MARKER_LOCKED;
    locked_end_marker = NO_MARKER_LOCKED;
}

void RB_Item_Lock_Reset(
    void )
{
    locked_remark_marker = NO_MARKER_LOCKED;
}
