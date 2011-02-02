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


/****h* ROBODoc/Test_Generator
 * FUNCTION
 *   The generator for test output.
 *
 *   The purpose of this generator is to create output that is easily
 *   scanable by the system test scripts.  This to make it easier to
 *   write tests for ROBODoc.
 *
 *   This generator produces output in utf-8 encoding.
 *
 *   This generator is experimental.
 *
 *******
 * $Id: test_generator.c,v 1.10 2007/07/10 19:13:52 gumpu Exp $
 */


#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <ctype.h>
#include "test_generator.h"
#include "globals.h"
#include "util.h"



char               *RB_TEST_Get_Default_Extension(
    void )
{
    return ".xml";
}

void RB_TEST_Generate_String(
    FILE *dest_doc,
    char *a_string )
{
    int                 i;
    int                 l = strlen( a_string );
    unsigned char       c;

    for ( i = 0; i < l; ++i )
    {
        c = a_string[i];
        RB_TEST_Generate_Char( dest_doc, c );
    }
}

/* TODO Documentation */

void RB_TEST_Generate_Label(
    FILE *dest_doc,
    char *name )
{
    int                 i;
    int                 l = strlen( name );
    unsigned char       c;

    fprintf( dest_doc, "<label>" );
    for ( i = 0; i < l; ++i )
    {
        c = name[i];
        if ( utf8_isalnum( c ) )
        {
            RB_TEST_Generate_Char( dest_doc, c );
        }
        else
        {
            char                buf[4];

            sprintf( buf, "%02x", c );
            RB_TEST_Generate_Char( dest_doc, buf[0] );
            RB_TEST_Generate_Char( dest_doc, buf[1] );
        }
    }
    fprintf( dest_doc, "</label>" );
}


/****f* Generator/RB_TEST_Generate_Char
 * NAME
 *   RB_TEST_Generate_Char
 * SYNOPSIS
 *   void RB_TEST_Generate_Char( FILE * dest_doc, int c )
 * FUNCTION
 *   Switchboard to RB_TEST_Generate_Char
 * SOURCE
 */

void RB_TEST_Generate_Char(
    FILE *dest_doc,
    int c )
{
    switch ( c )
    {
    default:
        RB_FputcLatin1ToUtf8( dest_doc, c );
        break;
    }
}

/*****/

void RB_TEST_Generate_Header_Start(
    FILE *dest_doc,
    struct RB_header *cur_header )
{
    fprintf( dest_doc, "<header name=\"" );
    RB_TEST_Generate_String( dest_doc, cur_header->name );
    fprintf( dest_doc, "\" header_module=\"" );
    RB_TEST_Generate_String( dest_doc, cur_header->module_name );
    fprintf( dest_doc, "\"" );
    fprintf( dest_doc, " header_function_name=\"" );
    RB_TEST_Generate_String( dest_doc, cur_header->function_name );
    fprintf( dest_doc, "\" >\n" );
}

void RB_TEST_Generate_Header_End(
    FILE *dest_doc,
    struct RB_header *cur_header )
{
    USE( cur_header );

    fprintf( dest_doc, "</header>\n" );
}

void RB_TEST_Generate_Link(
    FILE *dest,
    char *dest_name,
    char *filename,
    char *labelname,
    char *linkname
     )
{
    /* TODO print the other stuff too! */
    USE( dest_name );
    USE( filename );

    fprintf( dest, "<link labelname=\"%s\" linkname=\"", labelname );
    RB_TEST_Generate_String( dest, linkname );
    fprintf( dest, "\" />\n" );
}

void RB_TEST_Generate_Doc_Start(
    FILE *dest_doc,
    char *src_name,
    char *name,
    char toc )
{
    USE( toc );

    if ( course_of_action.do_headless )
    {
        /* The user does not want the document head. */
    }
    else
    {
        fprintf( dest_doc, "<doc_start src_name=\"%s\" name=\"%s\">\n",
                 src_name, name );
    }
}


void RB_TEST_Generate_Doc_End(
    FILE *dest_doc,
    char *name )
{
    USE( name );

    if ( course_of_action.do_footless )
    {
        /* The user does not want the foot of the
         * document.
         */
    }
    else
    {
        fprintf( dest_doc, "%s", "</doc_start>\n" );
    }
}


void RB_TEST_Generate_Item_Name(
    FILE *dest_doc,
    char *name )
{
    fprintf( dest_doc, "<item id=\"" );
    RB_TEST_Generate_String( dest_doc, name );
    fprintf( dest_doc, "\">\n" );
}

void RB_TEST_Generate_Item_Begin(
    FILE *dest_doc )
{
    fprintf( dest_doc, "<item_body>\n" );
}


void RB_TEST_Generate_Item_End(
    FILE *dest_doc )
{
    fprintf( dest_doc, "</item_body>\n</item>\n" );
}


void RB_TEST_Generate_BeginSection(
    FILE *dest_doc,
    int depth,
    char *name )
{
    fprintf( dest_doc, "<section depth=\"%d\">", depth );
    RB_TEST_Generate_String( dest_doc, name );
    fprintf( dest_doc, "\n" );
}

void RB_TEST_Generate_EndSection(
    FILE *dest_doc,
    int depth,
    char *name )
{
    USE( depth );

    fprintf( dest_doc, "</section>" );
    RB_TEST_Generate_String( dest_doc, name );
    fprintf( dest_doc, "\n" );
}


void RB_TEST_Generate_False_Link(
    FILE *dest_doc,
    char *name )
{
    fprintf( dest_doc, "<false_link>" );
    RB_TEST_Generate_String( dest_doc, name );
    fprintf( dest_doc, "</false_link>" );
}



void TEST_Generate_Begin_Paragraph(
    FILE *dest_doc )
{
    fprintf( dest_doc, "<para>\n" );
}

void TEST_Generate_End_Paragraph(
    FILE *dest_doc )
{
    fprintf( dest_doc, "</para>\n" );
}


void TEST_Generate_Begin_Preformatted(
    FILE *dest_doc )
{
    fprintf( dest_doc, "<pre>\n" );
}

void TEST_Generate_End_Preformatted(
    FILE *dest_doc )
{
    fprintf( dest_doc, "</pre>\n" );
}


void TEST_Generate_Begin_List(
    FILE *dest_doc )
{
    fprintf( dest_doc, "<list>\n" );
}

void TEST_Generate_End_List(
    FILE *dest_doc )
{
    fprintf( dest_doc, "</list>\n" );
}

void TEST_Generate_Begin_List_Item(
    FILE *dest_doc )
{
    fprintf( dest_doc, "<list_item>\n" );
}

void TEST_Generate_End_List_Item(
    FILE *dest_doc )
{
    fprintf( dest_doc, "</list_item>\n" );
}
