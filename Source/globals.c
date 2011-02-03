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


/****h* ROBODoc/Globals
 * FUNCTION
 *   A number of global variables.
 * TODO
 *   Documentation.
 *****
 * $Id: globals.c,v 1.24 2009/03/23 21:15:04 gumpu Exp $
 */

#include <stdio.h>
#include <stdlib.h>
#include "robodoc.h"
#include "globals.h"
#include "links.h"

#ifdef DMALLOC
#include <dmalloc.h>
#endif

int                 number_of_warnings = 0;

/* Pointer to the name of the current file that is being analysed,
   use by RB_Panic */

char               *current_file = 0;

/****v* Globals/document_title
 * NAME
 *   documentat_title -- title for the documentation.
 * PURPOSE
 *   Used as the title for master index files or for latex documentation.
 * SOURCE
 */

char               *document_title = NULL;

/******/


/****v* Globals/output_mode [2.0]
 * NAME
 *   output_mode -- the mode of output
 * FUNCTION
 *   Controls which type of output will be generated.
 * SOURCE
 */

T_RB_DocType        output_mode = ASCII;

/*******/


/****v* Globals/course_of_action [2.0]
 * NAME
 *   course_of_action
 * FUNCTION
 *   Global Variable that defines the course of action.
 * SOURCE
 */

actions_t           course_of_action;

/*******/

/****v* Globals/debugmode
 * NAME
 *   debugmode
 * FUNCTION
 *   A bitfield determining the output levels
 * SOURCE
 */

long                debugmode = 0;

/*******/


/****v* Globals/line_buffer [2.0]
 * NAME
 *   line_buffer -- global line buffer
 * FUNCTION
 *   Temporary storage area for lines
 *   that are read from an input file.
 * SOURCE
 */

char                line_buffer[MAX_LINE_LEN];

/*******/

/****v* Globals/myLine
 * NAME
 *   myLine -- dynamic buffer for current line
 * FUNCTION
 *   Temporary storage area for lines
 *   that are read from an input file.
 * SOURCE
 */

char               *myLine = NULL;

/*******/

/****v* Globals/readChars
 * NAME
 *   readChars -- number of characters in the currently bufferd line
 * FUNCTION
 *   Temporary storage area for lines
 *   that are read from an input file.
 * SOURCE
 */

int                 readChars = 0;

/*******/

/****v* Globals/line_number [2.0]
 * NAME
 *   line_number -- global line counter
 * PURPOSE
 *   Keeps track of the number of lines that are read from the source file.
 * AUTHOR
 *   Koessi
 * SOURCE
 */

int                 line_number = 0;

/*******/

/*
 * Global variables 
 */
/* TODO  Document these. */

char               *source_file;        /* DCD */
char               *whoami = NULL;      /* me,myself&i */
int                 tab_stops[MAX_TABS];        /* Tab stop positions */

/* Number of header item names before linebreak */
int                 header_breaks = DEFAULT_HEADER_BREAKS;


/* Path and filename to dot tool */
char               *dot_name = DEFAULT_DOT_NAME;


/* Name of item to add source comments */
char               *use_source_comments = NULL;

/****i* Globals/RB_Close_The_Shop [3.0b]
 * NAME
 *   RB_Close_The_Shop -- free resources.
 * SYNOPSIS
 *   void RB_Close_The_Shop ()
 * FUNCTION
 *   Frees all resources used by robodoc.
 * SEE ALSO
 *   RB_Free_Header(), RB_Free_Link()
 * SOURCE
 */

void RB_Close_The_Shop(
    void )
{
/* TODO    if (link_index) { free(link_index); } */
    if ( myLine )
    {
        free( myLine );
    }
}

/******/
