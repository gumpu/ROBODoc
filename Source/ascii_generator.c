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

/* vi: spell ff=unix 
 */
/****h* ROBODoc/ASCII_Generator
 * NAME
 *   ASCII_Generator -- Generator for ASCII output
 * FUNCTION
 *   Plain ascii output, no formatting.
 * MODIFICATION HISTORY
 *   2003-06-17  Frans Slothouber V1.0
 *******
 * $Id: ascii_generator.c,v 1.23 2008/06/17 11:49:26 gumpu Exp $
 */

#include <stdio.h>
#include <assert.h>

#include "ascii_generator.h"
#include "util.h"
#include "robodoc.h"
#include "globals.h"
#include "items.h"
#include "headers.h"
#include "headertypes.h"
#include "generator.h"

#ifdef DMALLOC
#include <dmalloc.h>
#endif

/*x**h* ROBODoc/ASCII_Generator
 * FUNCTION
 *   Generator for plain ASCII output
 *******
 */


/*x**f* ASCII_Generator/RB_ASCII_Generate_Doc_Start
 * NAME
 *   RB_ASCII_Generate_Doc_Start --
 ******
 */

void RB_ASCII_Generate_Doc_Start(
    FILE *dest_doc,
    char *src_name,
    char *name,
    char toc )
{
    USE( src_name );
    USE( name );
    USE( toc );

    if ( course_of_action.do_toc )
    {
        fprintf( dest_doc, "TABLE OF CONTENTS\n" );
        /* TODO  TOC */
#if 0
        for ( cur_header = first_header, header_nr = 1;
              cur_header; cur_header = cur_header->next_header, header_nr++ )
        {
            if ( cur_header->name && cur_header->function_name )
            {
                fprintf( dest_doc, "%4.4d %s\n",
                         header_nr, cur_header->name );
            }
        }
#endif
        fputc( '\f', dest_doc );
    }
}

/*x**f* ASCII_Generator/RB_ASCII_Generate_Doc_End
 * NAME
 *   RB_ASCII_Generate_Doc_End --
 ******
 */

void RB_ASCII_Generate_Doc_End(
    FILE *dest_doc,
    char *name )
{
    USE( dest_doc );
    USE( name );
    /* Empty */
}

/*x**f* ASCII_Generator/RB_ASCII_Generate_Header_Start
 * NAME
 *   RB_ASCII_Generate_Header_Start --
 ******
 */

void RB_ASCII_Generate_Header_Start(
    FILE *dest_doc,
    struct RB_header *cur_header )
{
    fprintf( dest_doc, "%s", cur_header->name );
    fprintf( dest_doc, "\n\n" );
}

void RB_ASCII_Generate_Header_End(
    FILE *dest_doc,
    struct RB_header *cur_header )
{
    USE( cur_header );
    fprintf( dest_doc,
             "\n---------------------------------------------------------------------------\n" );
    /* form-feeds are annoying    fputc( '\f', dest_doc ); */
}

/*x**f* ASCII_Generator/RB_ASCII_Generate_Empty_Item
 * NAME
 *   RB_ASCII_Generate_Empty_Item --
 ******
 */

void RB_ASCII_Generate_Empty_Item(
    FILE *dest )
{
    USE( dest );
    /* Empty */
}

/* TODO Documentation */
void RB_ASCII_Generate_String(
    FILE *dest,
    char *string )
{
    fprintf( dest, "%s", string );
}

/* TODO Documentation */
void RB_ASCII_Generate_False_Link(
    FILE *dest_doc,
    char *name )
{
    fprintf( dest_doc, "%s", name );
}

/* TODO Documentation */
char               *RB_ASCII_Get_Default_Extension(
    void )
{
    return ( ".txt" );
}

/* TODO Documentation */
void RB_ASCII_Generate_Item_Name(
    FILE *dest_doc,
    char *name )
{
    fprintf( dest_doc, "%s\n", name );
}


/* TODO Documentation */
void RB_ASCII_Generate_Item_Begin(
    FILE *dest_doc )
{
    USE( dest_doc );
    /* Empty */
}


/* TODO Documentation */
void RB_ASCII_Generate_Char(
    FILE *dest_doc,
    int c )
{
    fputc( c, dest_doc );
}


/* TODO Documentation */
void RB_ASCII_Generate_Item_End(
    FILE *dest_doc )
{
    USE( dest_doc );
    /* Empty */
}


static int          section_counters[ASCII_MAX_SECTION_DEPTH];
void RB_ASCII_Generate_BeginSection(
    FILE *dest_doc,
    int depth,
    char *name,
    struct RB_header *header )
{
    int                 i;

    ++section_counters[depth];
    for ( i = depth + 1; i < ASCII_MAX_SECTION_DEPTH; ++i )
    {
        section_counters[i] = 0;
    }
    if ( depth < ASCII_MAX_SECTION_DEPTH )
    {
        if ( !( course_of_action.do_sectionnameonly ) )
        {
            for ( i = 1; i <= depth; ++i )
            {
                fprintf( dest_doc, "%d.", section_counters[i] );
            }
            fprintf( dest_doc, "  " );
        }

        /*  Print Header "first" name */
        RB_ASCII_Generate_String( dest_doc, name );

        /*  Print further names */
        for ( i = 1; i < header->no_names; i++ )
        {
            fprintf( dest_doc, ( i % header_breaks ) ? ", " : ",\n" );
            RB_ASCII_Generate_String( dest_doc, header->names[i] );
        }

        /*  Include module name if not sectionnameonly */
        if ( !( course_of_action.do_sectionnameonly ) )
        {
            fprintf( dest_doc, " [ " );
            RB_ASCII_Generate_String( dest_doc, header->htype->indexName );
            fprintf( dest_doc, " ]" );
        }
    }
    else
    {
        /* too deep, don't do anything. */
        assert( 0 );
    }
}

void RB_ASCII_Generate_EndSection(
    FILE *dest_doc,
    int depth,
    char *name )
{
    USE( dest_doc );
    USE( name );
    USE( depth );
    /* Empty */
}

/****f* ASCII_Generator/RB_ASCII_Generate_Item_Line_Number
 * FUNCTION
 *   Generate line numbers for SOURCE like items
 * SYNOPSIS
 */
void RB_ASCII_Generate_Item_Line_Number(
    FILE *dest_doc,
    char *line_number_string )
/*
 * INPUTS
 *   o dest_doc           -- the file to write to.
 *   o line_number_string -- the line number as string.
 * SOURCE
 */
{
    RB_ASCII_Generate_String( dest_doc, line_number_string );
}
/******/
