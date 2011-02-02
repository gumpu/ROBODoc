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


#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <ctype.h>
#include "xmldocbook_generator.h"
#include "globals.h"
#include "util.h"


char               *RB_XMLDB_Get_Default_Extension(
    void )
{
    return ".xml";
}

void RB_XMLDB_Generate_String(
    FILE *dest_doc,
    char *a_string )
{
    int                 i;
    int                 l = strlen( a_string );
    unsigned char       c;

    for ( i = 0; i < l; ++i )
    {
        c = a_string[i];
        RB_XMLDB_Generate_Char( dest_doc, c );
    }
}

/* TODO Documentation */

void RB_XMLDB_Generate_Label(
    FILE *dest_doc,
    char *name )
{
    int                 i;
    int                 l = strlen( name );
    unsigned char       c;

    fprintf( dest_doc, "<anchor id=\"" );
    for ( i = 0; i < l; ++i )
    {
        c = name[i];
        if ( utf8_isalnum( c ) )
        {
            RB_XMLDB_Generate_Char( dest_doc, c );
        }
        else
        {
            char                buf[4];

            sprintf( buf, "%02x", c );
            RB_XMLDB_Generate_Char( dest_doc, buf[0] );
            RB_XMLDB_Generate_Char( dest_doc, buf[1] );
        }
    }
    fprintf( dest_doc, "\"/>\n" );
}


/****f* XMLDB_Generator/RB_XMLDB_Generate_Char
 * NAME
 *   RB_XMLDB_Generate_Char
 * SYNOPSIS
 *   void RB_XMLDB_Generate_Char( FILE * dest_doc, int c )
 * FUNCTION
 *   Switchboard to RB_XMLDB_Generate_Char
 * SOURCE
 */

void RB_XMLDB_Generate_Char(
    FILE *dest_doc,
    int c )
{
    switch ( c )
    {
    case '\n':
        assert( 0 );
        break;
    case '\t':
        assert( 0 );
        break;
    case '<':
        fprintf( dest_doc, "&lt;" );
        break;
    case '>':
        fprintf( dest_doc, "&gt;" );
        break;
    case '&':
        fprintf( dest_doc, "&amp;" );
        break;
    default:
        /* All others are printed literally */
        fputc( c, dest_doc );
    }
}

/*****/

void RB_XMLDB_Generate_Header_Start(
    FILE *dest_doc,
    struct RB_header *cur_header )
{
    fprintf( dest_doc, "<section>\n" );
    /*    fprintf( dest_doc, "<section id=\"%s\">\n", cur_header->unique_name ); */
    fprintf( dest_doc, "<title>\n" );
    RB_XMLDB_Generate_String( dest_doc, cur_header->name );
    fprintf( dest_doc, "</title>\n" );
}

void RB_XMLDB_Generate_Header_End(
    FILE *dest_doc,
    struct RB_header *cur_header )
{
    USE( cur_header );
    fprintf( dest_doc, "</section>\n" );
}

void RB_XMLDB_Generate_Link(
    FILE *dest,
    char *dest_name,
    char *filename,
    char *labelname,
    char *linkname )
{
    USE( dest_name );
    USE( filename );

    fprintf( dest, "<link linkend=\"%s\">", labelname );
    RB_XMLDB_Generate_String( dest, linkname );
    fprintf( dest, "</link>" );
}

void RB_XMLDB_Generate_Doc_Start(
    struct RB_Document *document,
    FILE *dest_doc,
    char *charset )
{
    if ( course_of_action.do_headless )
    {
        /* The user does not want the document head. */
    }
    else
    {
        fprintf( dest_doc, "<?xml version=\"1.0\" encoding=\"%s\"?>\n",
                 charset ? charset : DEFAULT_CHARSET );
        if ( document->doctype_name && document->doctype_location )
        {
            fprintf( dest_doc, "<!DOCTYPE article PUBLIC \"%s\"\n\"%s\">\n",
                     document->doctype_name, document->doctype_location );
            fprintf( dest_doc, "%s", "<article lang=\"en\">\n" );
        }
        else
        {
            fprintf( dest_doc, "%s",
                     "<!DOCTYPE article PUBLIC \"-//OASIS//DTD DocBook XML V4.2//EN\" \n"
                     "\"http://www.oasis-open.org/docbook/xml/4.2/docbookx.dtd\">\n"
                     "<article lang=\"en\">\n" );
        }
        fprintf( dest_doc,
                 "<articleinfo>\n  <title>%s</title>\n</articleinfo>\n",
                 document_title ? document_title : DEFAULT_DOCTITILE );
    }
}


void RB_XMLDB_Generate_Doc_End(
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
        fprintf( dest_doc, "%s", "</article>\n" );
    }
}


void RB_XMLDB_Generate_Item_Name(
    FILE *dest_doc,
    char *name )
{
    fprintf( dest_doc, "<formalpara><title>" );
    RB_XMLDB_Generate_String( dest_doc, name );
    fprintf( dest_doc, "</title><para></para></formalpara>\n" );
}

void RB_XMLDB_Generate_Item_Begin(
    FILE *dest_doc )
{
    USE( dest_doc );
    /* Empty */
}


void RB_XMLDB_Generate_Item_End(
    FILE *dest_doc )
{
    USE( dest_doc );
    /* Empty */
}


void RB_XMLDB_Generate_BeginSection(
    FILE *dest_doc,
    int depth,
    char *name )
{
    USE( depth );

    fprintf( dest_doc, "%s", "<section>\n<title>" );
    RB_XMLDB_Generate_String( dest_doc, name );
    fprintf( dest_doc, "%s", "</title>\n" );
}

void RB_XMLDB_Generate_EndSection(
    FILE *dest_doc,
    int depth,
    char *name )
{
    USE( depth );
    USE( name );
    fprintf( dest_doc, "%s", "</section>\n" );
}


void RB_XMLDB_Generate_False_Link(
    FILE *dest_doc,
    char *name )
{
    RB_XMLDB_Generate_String( dest_doc, name );
}



void XMLDB_Generate_Begin_Paragraph(
    FILE *dest_doc )
{
    fprintf( dest_doc, "%s", "<para>\n" );
}

void XMLDB_Generate_End_Paragraph(
    FILE *dest_doc )
{
    fprintf( dest_doc, "%s", "</para>\n" );
}


void XMLDB_Generate_Begin_Preformatted(
    FILE *dest_doc )
{
    fprintf( dest_doc, "%s", "<literallayout class=\"monospaced\">\n" );
}

void XMLDB_Generate_End_Preformatted(
    FILE *dest_doc )
{
    fprintf( dest_doc, "%s", "</literallayout>\n" );
}


void XMLDB_Generate_Begin_List(
    FILE *dest_doc )
{
    fprintf( dest_doc, "%s", "<itemizedlist>" );
}

void XMLDB_Generate_End_List(
    FILE *dest_doc )
{
    fprintf( dest_doc, "%s", "</itemizedlist>" );
}

void XMLDB_Generate_Begin_List_Item(
    FILE *dest_doc )
{
    fprintf( dest_doc, "%s", "<listitem><para>" );
}

void XMLDB_Generate_End_List_Item(
    FILE *dest_doc )
{
    fprintf( dest_doc, "%s", "</para></listitem>" );
}

/****f* XMLDB_Generator/RB_XMLDB_Generate_Item_Line_Number
 * FUNCTION
 *   Generate line numbers for SOURCE like items
 * SYNOPSIS
 */
void RB_XMLDB_Generate_Item_Line_Number(
    FILE *dest_doc,
    char *line_number_string )
/*
 * INPUTS
 *   o dest_doc           -- the file to write to.
 *   o line_number_string -- the line number as string.
 * SOURCE
 */
{
    RB_XMLDB_Generate_String( dest_doc, line_number_string );
}
/******/
