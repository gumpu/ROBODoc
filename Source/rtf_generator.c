/* vi: spell ff=unix
 */

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


/****h* ROBODoc/RTF_Generator
 * FUNCTION
 *   A collection of functions to generate output in RTF format.
 *   Supports sections upto 7 levels deep.
 * TODO
 *   Documentation
 * MODIFICATION HISTORY
 *   ????-??-??   Anthon Pang       V1.0
 *   2003-02-03   Frans Slothouber  Refactoring
 *   Anthon Pang
 *****
 * $Id: rtf_generator.c,v 1.29 2008/06/17 11:49:28 gumpu Exp $ 
 */


#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "rtf_generator.h"
#include "util.h"
#include "robodoc.h"
#include "globals.h"
#include "headers.h"
#include "generator.h"

#ifdef DMALLOC
#include <dmalloc.h>
#endif


void RB_RTF_Generate_False_Link(
    FILE *dest_doc,
    char *name )
{
    /* Find out how to emphasize this in rtf */
    fprintf( dest_doc, "%s", name );
}

void RB_RTF_Generate_TOC_2(
    FILE *dest_doc,
    struct RB_header **headers,
    int count )
{
    int                 i;

    for ( i = 0; i < count; ++i )
    {
        struct RB_header   *cur_header = headers[i];

        if ( cur_header->name && cur_header->function_name )
        {
            fprintf( dest_doc, "{" );
            RB_RTF_Generate_String( dest_doc, cur_header->name );
            fprintf( dest_doc, "}{\\v " );
            RB_RTF_Generate_String( dest_doc, cur_header->function_name );
            fprintf( dest_doc, "}\\line\n" );
        }
    }
}

void RB_RTF_Generate_Label(
    FILE *dest_doc,
    char *name )
{
    USE( dest_doc );
    USE( name );
    /* Empty */
}

void RB_RTF_Generate_Item_Name(
    FILE *dest_doc,
    char *name )
{
    fprintf( dest_doc, "\\par \\fs18 %s\\line\n", name );
}

char               *RB_RTF_Get_Default_Extension(
    void )
{
    return ".rtf";
}


void RB_RTF_Generate_BeginSection(
    FILE *dest_doc,
    int depth,
    char *name )
{
    fprintf( dest_doc, "\\par {\\s%d %s \\par} \\pard\\plain\n", depth, name );
}

void RB_RTF_Generate_EndSection(
    FILE *dest_doc,
    int depth,
    char *name )
{
    USE( dest_doc );
    USE( depth );
    USE( name );
    /* empty */
}

/*x**f* RTF_Generator/RB_RTF_Generate_Doc_Start
 * NAME
 *   RB_RTF_Generate_Doc_Start --
 ******
 */

void RB_RTF_Generate_Doc_Start(
    FILE *dest_doc,
    char *src_name,
    char *name,
    char toc )
{
    USE( toc );

    /* RTF header */
    fprintf( dest_doc, "{\\rtf1\\ansi \\deff0"
             "{\\fonttbl;"
             "\\f0\\fswiss MS Sans Serif;"
             "\\f1\\fmodern Courier New;"
             "\\f2\\ftech Symbol;"
             "}"
             "{\\colortbl;"
             "\\red255\\green255\\blue255;"
             "\\red0\\green0\\blue0;" "\\red0\\green0\\blue255;" "}" );

    /* RTF document info */
    fprintf( dest_doc, "{\\info"
             "{\\title %s}" "{\\comment\n" " Source: %s\n", name, src_name );
    if ( course_of_action.do_nogenwith )
    {
        fprintf( dest_doc, " " "}" "}" );
    }
    else
    {
        fprintf( dest_doc, " " COMMENT_ROBODOC " " "}" "}" );
    }

    /* RTF stylesheet */
    fprintf( dest_doc,
             "{\\stylesheet\n"
             "{\\s1 \\fs20 \\keepn\\widctlpar\\jclisttab \\outlinelevel0\\adjustright \\cgrid \\sbasedon0 \\snext0 heading 1;}\n"
             "{\\s2 \\fs20 \\keepn\\widctlpar\\jclisttab \\outlinelevel1\\adjustright \\cgrid \\sbasedon0 \\snext0 heading 2;}\n"
             "{\\s3 \\fs20 \\keepn\\widctlpar\\jclisttab \\outlinelevel2\\adjustright \\cgrid \\sbasedon0 \\snext0 heading 3;}\n"
             "{\\s4 \\fs20 \\keepn\\widctlpar\\jclisttab \\outlinelevel3\\adjustright \\cgrid \\sbasedon0 \\snext0 heading 4;}\n"
             "{\\s5 \\fs20 \\keepn\\widctlpar\\jclisttab \\outlinelevel4\\adjustright \\cgrid \\sbasedon0 \\snext0 heading 5;}\n"
             "{\\s6 \\fs20 \\keepn\\widctlpar\\jclisttab \\outlinelevel5\\adjustright \\cgrid \\sbasedon0 \\snext0 heading 6;}\n"
             "{\\s7 \\fs20 \\keepn\\widctlpar\\jclisttab \\outlinelevel6\\adjustright \\cgrid \\sbasedon0 \\snext0 heading 7;}}\n" );
    /* RTF document format */
    fprintf( dest_doc, "{\\margl1440\\margr1440}\\fs18\n" );

    /* RTF document section */
    fprintf( dest_doc, "\\f0\\cb1\\cf3\\b1\\qc"
             "{\\super #{\\footnote{\\super #}%s_TOC}}"
             "{\\super ${\\footnote{\\super $}Contents}}"
             "{TABLE OF CONTENTS}\\ql\\b0\\fs20\\cf2\\par\n", src_name );
}

/*x**f* RTF_Generator/RB_RTF_Generate_Doc_End
 * NAME
 *   RB_RTF_Generate_Doc_End --
 ******
 */

void RB_RTF_Generate_Doc_End(
    FILE *dest_doc,
    char *name )
{
    USE( name );

    fputc( '}', dest_doc );
}

/*x**f* RTF_Generator/RB_RTF_Generate_Header_Start
 * NAME
 *   RB_RTF_Generate_Header_Start --
 ******
 */

void RB_RTF_Generate_Header_Start(
    FILE *dest_doc,
    struct RB_header *cur_header )
{
    if ( cur_header->name && cur_header->function_name )
    {
        RB_RTF_Generate_String( dest_doc, cur_header->function_name );
        fprintf( dest_doc, "\\page {\\super #{\\footnote{\\super #}" );
        RB_RTF_Generate_String( dest_doc, cur_header->function_name );
        fprintf( dest_doc, "}}{\\super ${\\footnote{\\super $}" );
        RB_RTF_Generate_String( dest_doc, cur_header->name );
        fprintf( dest_doc, "}} " );
        RB_RTF_Generate_String( dest_doc, cur_header->name );
        fprintf( dest_doc, "\\line\n" );
    }
}

/*x**f* RTF_Generator/RB_RTF_Generate_Header_End
 * NAME
 *   RB_RTF_Generate_Header_End --
 ******
 */

void RB_RTF_Generate_Header_End(
    FILE *dest_doc,
    struct RB_header *cur_header )
{
    USE( cur_header );

    fprintf( dest_doc, "\\par\n" );
}


void RB_RTF_Generate_Item_Begin(
    FILE *dest_doc )
{
    fprintf( dest_doc, "{\\f1{}\\fs18\n" );
}

void RB_RTF_Generate_Item_End(
    FILE *dest_doc )
{
    fputc( '}', dest_doc );
    fputc( '\n', dest_doc );
}

/*x**f* RTF_Generator/RB_RTF_Generate_Empty_Item
 * NAME
 *   RB_RTF_Generate_Empty_Item --
 ******
 */

void RB_RTF_Generate_Empty_Item(
    FILE *dest_doc )
{
    fprintf( dest_doc, "\n" );
}


void RB_RTF_Generate_Link(
    FILE *dest_doc,
    char *dest_name,
    char *filename,
    char *labelname,
    char *linkname
     )
{
    USE( dest_name );
    USE( filename );

    if ( strcmp( labelname, linkname ) )
    {
        fprintf( dest_doc, "{\\b " );
        RB_RTF_Generate_String( dest_doc, linkname );
        fprintf( dest_doc, "}{\\v " );
        RB_RTF_Generate_String( dest_doc, labelname );
        fprintf( dest_doc, "}" );
    }
    else
    {
        RB_RTF_Generate_String( dest_doc, labelname );
    }
}

void RB_RTF_Generate_String(
    FILE *dest_doc,
    char *a_string )
{
    int                 i;
    int                 l = strlen( a_string );
    unsigned char       c;

    for ( i = 0; i < l; ++i )
    {
        c = a_string[i];
        RB_RTF_Generate_Char( dest_doc, c );
    }
}

void RB_RTF_Generate_Char(
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
    case '\\':
    case '{':
    case '}':
        fputc( '\\', dest_doc );
        fputc( c, dest_doc );
        break;
    default:
        fputc( c, dest_doc );
    }
}

/****f* RTF_Generator/RB_RTF_Generate_Item_Line_Number
 * FUNCTION
 *   Generate line numbers for SOURCE like items
 * SYNOPSIS
 */
void RB_RTF_Generate_Item_Line_Number(
    FILE *dest_doc,
    char *line_number_string )
/*
 * INPUTS
 *   o dest_doc           -- the file to write to.
 *   o line_number_string -- the line number as string.
 * SOURCE
 */
{
    RB_RTF_Generate_String( dest_doc, line_number_string );
}
/******/
