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

/****h* ROBODoc/LaTeX_Generator
 * FUNCTION
 *   Generator for LaTeX output.  Supports singledoc mode.
 *
 *******
 * TODO  More documentation.
 */

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <ctype.h>

#include "generator.h"
#include "util.h"
#include "links.h"
#include "latex_generator.h"
#include "globals.h"
#include "robodoc.h"

#ifdef DMALLOC
#include <dmalloc.h>
#endif

static int          verbatim = FALSE;


/****f* LaTeX_Generator/RB_LaTeX_Generate_String
 * FUNCTION
 *   Write a string to the destination document, escaping
 *   characters where necessary.
 ******
 */

void RB_LaTeX_Generate_String(
    FILE *dest_doc,
    char *a_string )
{
    int                 i;
    int                 l = strlen( a_string );
    unsigned char       c;

    for ( i = 0; i < l; i++ )
    {
        c = a_string[i];
        RB_LaTeX_Generate_Char( dest_doc, c );
    }
}


void RB_LaTeX_Generate_False_Link(
    FILE *dest_doc,
    char *name )
{
    RB_LaTeX_Generate_String( dest_doc, name );
}

void RB_LaTeX_Generate_Item_Begin(
    FILE *dest_doc )
{
    USE( dest_doc );
    /* Empty */
}

void RB_LaTeX_Generate_Item_End(
    FILE *dest_doc )
{
    USE( dest_doc );
    /* Empty */
}

/* void */
/* RB_LaTeX_Generate_Item_Name( FILE* dest_doc, char *name ) */
/* { */
/*     RB_LaTeX_Generate_String( dest_doc, name ); */
/*     fprintf( dest_doc, "\n" ); */
/* } */


/* Lowtexx 21.09.2005 11:02 */
/* I think it looks better like this. This wastes less space in pdf-document. */
void RB_LaTeX_Generate_Item_Name(
    FILE *dest_doc,
    char *name )
{
    fprintf( dest_doc, "\\textbf{" );
    RB_LaTeX_Generate_String( dest_doc, name );

    /* In alternate format, item names stand alone (Thuffir) */
    if ( course_of_action.do_altlatex )
        fprintf( dest_doc, "}\n" );
    else
        fprintf( dest_doc, ":}\\hspace{0.08in}" );
}


void RB_LaTeX_Generate_BeginSection(
    FILE *dest_doc,
    int depth,
    char *name,
    struct RB_header *header )
{
    int                 i;

    /* Generate a new page for every section in alternate format */
    if ( course_of_action.do_altlatex )
        fprintf( dest_doc, "\\newpage\n" );

    /* If \part enabled, we have a level deeper */
    /* So enable the level '0' */
    if ( course_of_action.do_latexparts )
        depth--;

    switch ( depth )
    {
    case 0:
        fprintf( dest_doc, "\\part{" );
        break;
    case 1:
        fprintf( dest_doc, "\\section{" );
        break;
    case 2:
        fprintf( dest_doc, "\\subsection{" );
        break;
    case 3:
        fprintf( dest_doc, "\\subsubsection{" );
        break;
    default:
        /* Too deep so just make it a subsubsection */
        fprintf( dest_doc, "\\subsubsection{" );
    }

    /* Print Header "First" name */
    RB_LaTeX_Generate_String( dest_doc, name );
    /* Print other names */
    for ( i = 1; i < header->no_names; i++ )
    {
        RB_LaTeX_Generate_String( dest_doc, ", " );
        RB_LaTeX_Generate_String( dest_doc, header->names[i] );
    }
    fprintf( dest_doc, "}\n" );

    /* Print Parent if any */
    if ( header->parent )
    {
        fprintf( dest_doc, "\\textsl{[ " );
        RB_LaTeX_Generate_String( dest_doc, header->parent->function_name );
        fprintf( dest_doc, " ]}\n" );
    }

    /* Print Header Type */
    fprintf( dest_doc, "\\textsl{[ " );
    RB_LaTeX_Generate_String( dest_doc, header->htype->indexName );
    fprintf( dest_doc, " ]}\n\n" );
}

void RB_LaTeX_Generate_EndSection(
    FILE *dest_doc,
    int depth,
    char *name )
{
    USE( dest_doc );
    USE( depth );
    USE( name );
    /* Empty */
}

char               *RB_LaTeX_Get_Default_Extension(
    void )
{
    return ".tex";
}


/****f* LaTeX_Generator/RB_LaTeX_Generate_Doc_Start
 * NAME
 *   RB_LaTeX_Generate_Doc_Start --
 *****
 */

void RB_LaTeX_Generate_Doc_Start(
    FILE *dest_doc,
    char *src_name,
    char *name,
    char *charset )
{
    fprintf( dest_doc, "%% Document: %s\n", name );
    fprintf( dest_doc, "%% Source: %s\n", src_name );

    if ( course_of_action.do_nogenwith )
    {

    }
    else
    {
        fprintf( dest_doc, "%% " COMMENT_ROBODOC );
/* Documentation is the users not ours  fprintf( dest_doc, "%% " COMMENT_COPYRIGHT ); */
    }

    if ( course_of_action.do_headless )
    {
        /* The user wants a headless document, so we skip all
         * initialization.  It is up to the user to put this in.
         */
    }
    else
    {
        fprintf( dest_doc, "\\documentclass{article}\n" );
        fprintf( dest_doc, "\\usepackage{makeidx}\n" );
        fprintf( dest_doc, "\\usepackage{graphicx}\n" );

        /* Check if we have to use the inputenc package */
        if ( charset )
        {
            fprintf( dest_doc, "\\usepackage[%s]{inputenc}\n", charset );
        }

        /* Alternate mode, works better with pdflatex and DIN A4 sheets
         * At least for me :) (Thuffir) */
        if ( course_of_action.do_altlatex )
        {
            fprintf( dest_doc, "\\oddsidemargin 0 cm\n" );
            fprintf( dest_doc, "\\evensidemargin 0 cm\n" );
            fprintf( dest_doc, "\\topmargin 0 cm\n" );
            fprintf( dest_doc, "\\textwidth 16 cm\n" );
            fprintf( dest_doc, "\\textheight 22 cm\n" );
        }
        else
        {
            /* I guess this is unecessarry (same definitions twice ?)
               fprintf( dest_doc, "\\oddsidemargin  0.15 in\n" );
               fprintf( dest_doc, "\\evensidemargin 0.35 in\n" );
               fprintf( dest_doc, "\\marginparwidth 1 in   \n" );
             */

            fprintf( dest_doc, "\\oddsidemargin 0.25 in \n" );
            fprintf( dest_doc, "\\evensidemargin 0.25 in\n" );
            fprintf( dest_doc, "\\marginparwidth 0.75 in\n" );
            fprintf( dest_doc, "\\textwidth 5.875 in\n" );
        }


        fprintf( dest_doc, "\\setlength{\\parindent}{0in}\n" );
        fprintf( dest_doc, "\\setlength{\\parskip}{.08in}\n\n" );

        /* changed default header to use boldface (vs slant) */
        fprintf( dest_doc, "\\pagestyle{headings}\n" );

        /* Set document title */
        fprintf( dest_doc, "\\title{%s}\n",
                 document_title ? document_title : DEFAULT_DOCTITILE );

        if ( course_of_action.do_nogenwith )
        {
            fprintf( dest_doc, "\\author{Documentation Generator}\n" );
        }
        else
        {
            fprintf( dest_doc, "\\author{%s}\n", COMMENT_ROBODOC );
        }

        fprintf( dest_doc, "\\makeindex\n" );
        fprintf( dest_doc, "\\begin{document}\n" );
        fprintf( dest_doc, "\\maketitle\n" );

        /* In alternate mode, we generate INDEX at the end of document
         */
        if ( course_of_action.do_altlatex )
        {
            fprintf( dest_doc, "\\newpage\n" );
        }
        else
        {
            fprintf( dest_doc, "\\printindex\n" );
        }

        /* autogenerate table of contents! */
        fprintf( dest_doc, "\\tableofcontents\n" );

        /* We don't need this in alternate format, since every section begins
         * with a \newpage (Thuffir) */
        if ( !( course_of_action.do_altlatex ) )
            fprintf( dest_doc, "\\newpage\n" );

        /* trick to disable the autogenerated \newpage */
        fprintf( dest_doc, "\n" );
    }
}

/****f* LaTeX_Generator/RB_LaTeX_Generate_Doc_End
 * NAME
 *   RB_LaTeX_Generate_Doc_End --
 *****
 */

void RB_LaTeX_Generate_Doc_End(
    FILE *dest_doc,
    char *name )
{
    USE( name );

    /* In alternate mode, we generate INDEX at the end of document */
    if ( course_of_action.do_altlatex )
    {
        fprintf( dest_doc, "\\printindex\n" );
    }

    if ( course_of_action.do_footless )
    {
        /* The user does not want the foot of the
         * document
         */
    }
    else
    {
        fprintf( dest_doc, "\\end{document}\n" );
    }
}


/****f* LaTeX_Generator/RB_LaTeX_Generate_Index_Entry
 * FUNCTION
 *   Creates a entry for the index.
 *
 *******
 */

void RB_LaTeX_Generate_Index_Entry(
    FILE *dest_doc,
    struct RB_header *cur_header )
{
    assert( cur_header->function_name );

    fprintf( dest_doc, "\\index{unsorted!" );
    RB_LaTeX_Generate_String( dest_doc, cur_header->function_name );
    if ( cur_header->is_internal )
    {
        fprintf( dest_doc, "}\\index{internal\\_%s!",
                 cur_header->htype->indexName );
    }
    else
    {
        fprintf( dest_doc, "}\\index{%s!", cur_header->htype->indexName );
    }
    RB_LaTeX_Generate_String( dest_doc, cur_header->function_name );
    fprintf( dest_doc, "}\n" );
}

/****f* LaTeX_Generator/RB_LaTeX_Generate_Header_Start
 * NAME
 *   RB_LaTeX_Generate_Header_Start --
 *****
 */

void RB_LaTeX_Generate_Header_Start(
    FILE *dest_doc,
    struct RB_header *cur_header )
{
    fprintf( dest_doc, "\\subsection{" );
    RB_LaTeX_Generate_String( dest_doc, cur_header->name );
    fprintf( dest_doc, "}\n" );
}


/****f* LaTeX_Generator/RB_LaTeX_Generate_Header_End
 * NAME
 *   RB_LaTeX_Generate_Header_End --
 *****
 */

void RB_LaTeX_Generate_Header_End(
    FILE *dest_doc,
    struct RB_header *cur_header )
{
    USE( cur_header );
    fputc( '\n', dest_doc );
}


/****f* LaTeX_Generator/RB_LaTeX_Generate_Index_Table
 * NAME
 *   RB_LaTeX_Generate_Index_Table --
 *****
 */

void RB_LaTeX_Generate_Index_Table(
    FILE *dest,
    int type,
    char *title )
{
    USE( dest );
    USE( type );
    USE( title );

    /* Empty */
}


/****f* LaTeX_Generator/RB_LaTeX_Generate_Empty_Item
 * NAME
 *   RB_LaTeX_Generate_Empty_Item --
 *****
 */

void RB_LaTeX_Generate_Empty_Item(
    FILE *dest_doc )
{
    fprintf( dest_doc, "\\\\\n" );
}


/****f* LaTeX_Generator/RB_LaTeX_Generate_EscapedChar
 * FUNCTION
 *   Generate a single character.  These characters are outside
 *   a begin{verbatim} end{verbatim} block. So we need to escape is
 *   special characters.
 * SYNOPSIS
 *   void RB_LaTeX_Generate_EscapedChar( FILE* dest_doc, int c )
 * SEE ALSO
 *   RB_LaTeX_Generate_Char()
 *****
 */

void RB_LaTeX_Generate_EscapedChar(
    FILE *dest_doc,
    int c )
{
    switch ( c )
    {
    case '&':
    case '_':
    case '%':
    case '^':
    case '{':
    case '}':
    case '$':
    case '#':
        fprintf( dest_doc, "\\%c", c );
        break;
    case '<':
    case '>':
    case '|':
        fprintf( dest_doc, "$%c$", c );
        break;
    case '~':
        fprintf( dest_doc, "$\\sim$" );
        break;
    case '\\':
        fprintf( dest_doc, "$\\backslash$" );
        break;
    default:
        fputc( c, dest_doc );
        break;
    }
}


/****f* LaTeX_Generator/RB_LaTeX_Generate_Char
 * FUNCTION
 *   Generate a single character.  These characters are generated
 *   within a begin{verbatim} end{verbatim} block So no escaping is
 *   necessary.
 * SYNOPSIS
 *   void RB_LaTeX_Generate_Char( FILE* dest_doc, int c )
 *****
 */

void RB_LaTeX_Generate_Char(
    FILE *dest_doc,
    int c )
{
    if ( verbatim )
    {
        switch ( c )
        {
        case '\n':
            assert( 0 );
            break;
        case '\t':
            assert( 0 );
            break;
        default:
            fputc( c, dest_doc );
        }
    }
    else
    {
        RB_LaTeX_Generate_EscapedChar( dest_doc, c );
    }
}


void LaTeX_Generate_Begin_Paragraph(
    FILE *dest_doc )
{
    fprintf( dest_doc, "\n" );
}

void LaTeX_Generate_End_Paragraph(
    FILE *dest_doc )
{
    fprintf( dest_doc, "\n" );
}


void LaTeX_Generate_Begin_Preformatted(
    FILE *dest_doc )
{
    fprintf( dest_doc, "\\begin{verbatim}\n" );
    verbatim = TRUE;
}

void LaTeX_Generate_End_Preformatted(
    FILE *dest_doc )
{
    verbatim = FALSE;
    fprintf( dest_doc, "\\end{verbatim}\n" );
}


void LaTeX_Generate_Begin_List(
    FILE *dest_doc )
{
    fprintf( dest_doc, "\\begin{itemize}\n" );
}

void LaTeX_Generate_End_List(
    FILE *dest_doc )
{
    fprintf( dest_doc, "\\end{itemize}\n" );
}

void LaTeX_Generate_Begin_List_Item(
    FILE *dest_doc )
{
    fprintf( dest_doc, "  \\item " );
}

void LaTeX_Generate_End_List_Item(
    FILE *dest_doc )
{
    USE( dest_doc );
}



/* lowtexx 21.09.2005 11:23 */
/* added some functions to create links in latex documents */

/****f* LaTeX_Generator/RB_LaTeX_Generate_Label
 * NAME
 *   RB_LaTeX_Generate_Label --
 * SYNOPSIS
 *   void RB_LaTeX_Generate_Label( FILE* dest_doc, char* name)
 * INPUTS
 *   dest_doc  --  the file to which the text is written
 *   name --  the unique name of the label to create
 * SOURCE
 */

void RB_LaTeX_Generate_Label(
    FILE *dest_doc,
    char *name )
{
    int                 i;
    int                 l = strlen( name );
    unsigned char       c;

    fprintf( dest_doc, "\\label{ch:" );
    for ( i = 0; i < l; ++i )
    {
        c = name[i];
        if ( utf8_isalnum( c ) )
        {
            RB_LaTeX_Generate_Char( dest_doc, c );
        }
        else
        {
            /* think about this 
             * replaced by underscore
             */
            fputc( '_', dest_doc );
        }
    }
    fprintf( dest_doc, "}\n" );
}

/******/


/****f* LaTeX_Generator/RB_LaTeX_Generate_Link
 * NAME
 *   RB_LaTeX_Generate_Link --
 * SYNOPSIS
 *   void RB_LaTeX_Generate_Link( FILE *cur_doc, char *cur_name,
 *                                char *filename, char *labelname,
 *                                char *linkname )
 * INPUTS
 *   cur_doc  --  the file to which the text is written
 *   cur_name --  the name of the destination file (unused)
 *                (the file from which we link)
 *   filename --  the name of the file that contains the link
 *                (the file we link to) (unused)
 *   labelname--  the name of the unique label of the link.
 *   linkname --  the name of the link as shown to the user (unused).
 * SOURCE
 */

void RB_LaTeX_Generate_Link(
    FILE *cur_doc,
    char *cur_name,
    char *filename,
    char *labelname,
    char *linkname )
{
    USE( cur_name );
    USE( filename );
    USE( linkname );

    /* Only generate links outside the verbatim sections
     * LaTeX does not seem to recognise them inside (Thuffir)
     */
    if ( verbatim == FALSE )
        fprintf( cur_doc, " (\\ref{ch:%s})", labelname );
}

/******/

/****f* LaTeX_Generator/RB_LaTeX_Generate_Item_Line_Number
 * FUNCTION
 *   Generate line numbers for SOURCE like items
 * SYNOPSIS
 */
void RB_LaTeX_Generate_Item_Line_Number(
    FILE *dest_doc,
    char *line_number_string )
/*
 * INPUTS
 *   o dest_doc           -- the file to write to.
 *   o line_number_string -- the line number as string.
 * SOURCE
 */
{
    RB_LaTeX_Generate_String( dest_doc, line_number_string );
}

/******/
