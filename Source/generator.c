/* vi: spell ff=unix
 * */


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



/****h* ROBODoc/Generator
 * FUNCTION
 *   This contains routines to generate the documentation from the
 *   headers collected from the source code.   It contains
 *   functionality common for all document types (HTML, RTF etc).
 *   The specifics are handled in the modules for each of the several
 *   document types.
 *
 *   The behaviour of many of the functions in this module are
 *   modified by the global output_mode.
 *
 *   The general call sequence is as follows:
 *     RB_Generate_Documentation
 *     +> RB_Generate_SingleDoc
 *        +> RB_Generate_Part
 *           +> Generate_Header
 *                 +> Generate_Item_Line
 *                    +> Generate_Char
 * BUGS
 *   o Confusing use of doctype and output mode.
 * NOTES
 *   Might be a good idea to replace all the switch statements with
 *   function pointers.
 *   So instead of:
 *     switch (output_mode)
 *     {
 *         case HTML:
 *             RB_HTML_Generate_Doc_Start(dest_doc, src_name, name, toc);
 *             break;
 *         case LATEX:
 *             RB_LaTeX_Generate_Doc_Start(dest_doc, src_name, name, toc);
 *             break;
 *         case RTF:
 *             RB_RTF_Generate_Doc_Start(dest_doc, src_name, name, toc);
 *             break;
 *         case ASCII:
 *             RB_ASCII_Generate_Doc_Start(dest_doc, src_name, name, toc);
 *             break;
 *         case TROFF:
 *             RB_TROFF_Generate_Doc_Start(dest_doc, src_name, name, toc);
 *             break;
 *         default:
 *             break;
 *     }
 *   we will have
 *     (*rb_generate_doc_start)(dest_doc, src_name, name, toc);
 *
 *   were the function pointers are initialized at program start based
 *   on the output mode.
 *******
 * $Id: generator.c,v 1.114 2008/07/29 07:50:56 gumpu Exp $
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>

#include "globals.h"
#include "robodoc.h"
#include "headers.h"
#include "items.h"
#include "util.h"
#include "links.h"
#include "generator.h"
#include "document.h"
#include "part.h"
#include "file.h"
#include "roboconfig.h"

/* Generators */
#include "html_generator.h"
#include "latex_generator.h"
#include "xmldocbook_generator.h"
#include "rtf_generator.h"
#include "troff_generator.h"
#include "ascii_generator.h"
#include "test_generator.h"

/* TODO This should not be here.... */
#include "analyser.h"

#ifdef DMALLOC
#include <dmalloc.h>
#endif


/* Module functions */

static void         Generate_Item_Line(
    FILE *dest_doc,
    char *line,
    int item_type,
    char *docname,
    struct RB_header *fnames );
void                Generate_Char(
    FILE *dest_doc,
    int cur_char );
static void         Generate_Index(
    struct RB_Document *document );

static void         Generate_Header(
    FILE *f,
    struct RB_header *header,
    char *docname );

static void         Generate_Begin_Paragraph(
    FILE *dest_doc );
static void         Generate_End_Paragraph(
    FILE *dest_doc );
static void         Format_Line(
    FILE *dest_doc,
    long format );
static void         Generate_Begin_Preformatted(
    FILE *dest_doc,
    int source );
static void         Generate_End_Preformatted(
    FILE *dest_doc );
static void         Generate_Begin_List_Item(
    FILE *dest_doc );
static void         Generate_End_List_Item(
    FILE *dest_doc );
static void         Generate_Begin_List(
    FILE *dest_doc );
static void         Generate_End_List(
    FILE *dest_doc );
static void         Pipe_Line(
    FILE *dest_doc,
    char *arg_line );

static char         piping = FALSE;


/* TODO Documentation */
void RB_Generate_False_Link(
    FILE *dest_doc,
    char *name )
{
    switch ( output_mode )
    {
    case TEST:
        RB_TEST_Generate_False_Link( dest_doc, name );
        break;
    case XMLDOCBOOK:
        RB_XMLDB_Generate_False_Link( dest_doc, name );
        break;
    case HTML:
        RB_HTML_Generate_False_Link( dest_doc, name );
        break;
    case LATEX:
        RB_LaTeX_Generate_False_Link( dest_doc, name );
        break;
    case RTF:
        RB_RTF_Generate_False_Link( dest_doc, name );
        break;
    case ASCII:
        RB_ASCII_Generate_False_Link( dest_doc, name );
        break;
    case TROFF:
        RB_TROFF_Generate_False_Link( dest_doc, name );
        break;
    case UNKNOWN:
    default:
        assert( 0 );
    }
}

/****f* Generator/RB_Generate_Item_Begin
 * FUNCTION
 *   Generate the begin of an item.  This should switch to some
 *   preformatted output mode, similar to HTML's <PRE>.
 * SYNOPSIS
 */
void RB_Generate_Item_Begin(
    FILE *dest_doc,
    char *name )
/*
 * INPUTS
 *   dest_doc -- file to be written to
 *   output_mode -- global with the current output mode
 * SOURCE
 */
{
    switch ( output_mode )
    {
    case TEST:
        RB_TEST_Generate_Item_Begin( dest_doc );
        break;
    case XMLDOCBOOK:
        RB_XMLDB_Generate_Item_Begin( dest_doc );
        break;
    case HTML:
        RB_HTML_Generate_Item_Begin( dest_doc, name );
        break;
    case LATEX:
        RB_LaTeX_Generate_Item_Begin( dest_doc );
        break;
    case RTF:
        RB_RTF_Generate_Item_Begin( dest_doc );
        break;
    case ASCII:
        RB_ASCII_Generate_Item_Begin( dest_doc );
        break;
    case TROFF:
        /* nothing */
        break;
    case UNKNOWN:
    default:
        assert( 0 );
    }
}

/******/


/****f* Generator/Generate_Label
 * FUNCTION
 *   Generate a label that can be used for a link.
 *   For instance in HTML this is <a name="label">
 * SYNOPSIS
 */
void Generate_Label(
    FILE *dest_doc,
    char *name )
/*
 * INPUTS
 *   * dest_doc -- file to be written to
 *   * name -- the label's name.
 *   * output_mode -- global with the current output mode
 * SOURCE
 */
{
    switch ( output_mode )
    {
    case TEST:
        RB_TEST_Generate_Label( dest_doc, name );
        break;
    case XMLDOCBOOK:
        RB_XMLDB_Generate_Label( dest_doc, name );
        break;
    case HTML:
        RB_HTML_Generate_Label( dest_doc, name );
        break;
    case LATEX:
        /* lowtexx 21.09.2005 11:33 */
        RB_LaTeX_Generate_Label( dest_doc, name );
        break;
    case RTF:
        RB_RTF_Generate_Label( dest_doc, name );
        break;
    case ASCII:
        /* Doesn't apply */
        break;
    case TROFF:
        /* Doesn't apply */
        break;
    case UNKNOWN:
    default:
        assert( 0 );
    }
}

/******/


/****f* Generator/RB_Generate_Item_End
 * FUNCTION
 *   Generate the end of an item.  This should switch back from the
 *   preformatted mode.  So in HTML it generates the </PRE> of a <PRE>
 *   </PRE> pair.
 * INPUTS
 *   * dest_doc -- file to be written to
 *   * output_mode -- global with the current output mode
 * SOURCE
 */

void RB_Generate_Item_End(
    FILE *dest_doc,
    char *name )
{
    switch ( output_mode )
    {
    case TEST:
        RB_TEST_Generate_Item_End( dest_doc );
        break;
    case XMLDOCBOOK:
        RB_XMLDB_Generate_Item_End( dest_doc );
        break;
    case HTML:
        RB_HTML_Generate_Item_End( dest_doc, name );
        break;
    case LATEX:
        if ( piping == TRUE )
        {
            fprintf( dest_doc, "\\begin{verbatim}\n" );
            piping = FALSE;
        }
        RB_LaTeX_Generate_Item_End( dest_doc );
        break;
    case RTF:
        RB_RTF_Generate_Item_End( dest_doc );
        break;
    case ASCII:
        RB_ASCII_Generate_Item_End( dest_doc );
        break;
    case TROFF:
        /* Doesn't apply */ break;
    case UNKNOWN:

    default:
        assert( 0 );
    }
}

/****/



/****f Generator/RB_Get_Len_Extension
 * FUNCTION
 *   Compute the length of the filename extension for
 *   the current document type.
 *****
 */

size_t RB_Get_Len_Extension(
    char *extension )
{
    size_t              size = 0;

    size = strlen( extension );
    if ( *extension != '.' )
        size++;
    return size;
}

/****f* Generator/RB_Add_Extension
 * FUNCTION
 *   Add an extension to the filename base based on on the current
 *   output mode.
 * INPUTS
 *   * doctype -- output mode
 *   * name    -- the name of the file without extension and with
 *              enough room left to add the extension.
 * OUTPUT
 *   name    -- the name of the file including the extension.
 * SOURCE
 */

void RB_Add_Extension(
    char *extension,
    char *name )
{
    if ( *extension != '.' )
        strcat( name, "." );
    strcat( name, extension );
}

/******/

/*x**f Generator/RB_Default_Len_Extension
 * FUNCTION
 *   Returns default extension for
 *   the current document type.
 *****
 */

char               *RB_Get_Default_Extension(
    T_RB_DocType doctype )
{
    char               *extension = NULL;

    switch ( doctype )
    {
    case TEST:
        extension = RB_TEST_Get_Default_Extension(  );
        break;
    case XMLDOCBOOK:
        extension = RB_XMLDB_Get_Default_Extension(  );
        break;
    case HTML:
        extension = RB_HTML_Get_Default_Extension(  );
        break;
    case LATEX:
        extension = RB_LaTeX_Get_Default_Extension(  );
        break;
    case RTF:
        extension = RB_RTF_Get_Default_Extension(  );
        break;
    case ASCII:
        extension = RB_ASCII_Get_Default_Extension(  );
        break;
    case TROFF:
        extension = RB_TROFF_Get_Default_Extension(  );
        break;
    case UNKNOWN:
    default:
        assert( 0 );
    }
    return extension;
}


/****f* Generator/RB_Generate_BeginSection
 * FUNCTION
 *   Generate a section of level depth in the current output mode.
 *   This is used for the --sections option.  Where each header is
 *   placed in a section based on the header hierarchy.
 * INPUTS
 *   * dest_doc    -- the destination file.
 *   * doctype     -- document type
 *   * depth       -- the level of the section
 *   * name        -- the name of the section
 *   * header      -- pointer to the header structure
 *   * output_mode -- global with the current output mode.
 * SOURCE
 */

void RB_Generate_BeginSection(
    FILE *dest_doc,
    int depth,
    char *name,
    struct RB_header *header )
{
    switch ( output_mode )
    {
    case TEST:
        RB_TEST_Generate_BeginSection( dest_doc, depth, name );
        break;
    case XMLDOCBOOK:
        RB_XMLDB_Generate_BeginSection( dest_doc, depth, name );
        break;
    case HTML:
        RB_HTML_Generate_BeginSection( dest_doc, depth, name, header );
        break;
    case LATEX:
        RB_LaTeX_Generate_BeginSection( dest_doc, depth, name, header );
        break;
    case RTF:
        RB_RTF_Generate_BeginSection( dest_doc, depth, name );
        break;
    case TROFF:
/* RB_TROFF_Generate_BeginSection( dest_doc, depth, name ); */
        break;
    case ASCII:
        RB_ASCII_Generate_BeginSection( dest_doc, depth, name, header );
        break;
    case UNKNOWN:
    default:
        assert( 0 );
    }
}

/*******/


/****f* Generator/RB_Generate_EndSection
 * FUNCTION
 *   Generate the end of a section base on the current output mode.
 *   The functions is used for the --section option.
 *   It closes a section in the current output mode.
 * INPUTS
 *   * dest_doc -- the destination file.
 *   * doctype  --
 *   * depth    -- the level of the section
 *   * name     -- the name of the section
 *   * output_mode -- global with the current output mode.
 * SOURCE
 */

void RB_Generate_EndSection(
    FILE *dest_doc,
    int depth,
    char *name )
{
    switch ( output_mode )
    {
    case TEST:
        RB_TEST_Generate_EndSection( dest_doc, depth, name );
        break;
    case XMLDOCBOOK:
        RB_XMLDB_Generate_EndSection( dest_doc, depth, name );
        break;
    case HTML:
        RB_HTML_Generate_EndSection( dest_doc, depth, name );
        break;
    case LATEX:
        RB_LaTeX_Generate_EndSection( dest_doc, depth, name );
        break;
    case RTF:
        RB_RTF_Generate_EndSection( dest_doc, depth, name );
        break;
    case TROFF:
        /* doesn't apply */
        break;
    case ASCII:
        RB_HTML_Generate_EndSection( dest_doc, depth, name );
        break;
    case UNKNOWN:

    default:
        assert( 0 );
    }
}

/******/


/****f* Generator/RB_Generate_Index_Entry
 * FUNCTION
 *   Generate an entry for an auto generated index.  This works only
 *   for output modes that support this, LaTeX for instance.   This
 *   has nothting to do with the master index.
 * SYNOPSIS
 */
void RB_Generate_Index_Entry(
    FILE *dest_doc,
    T_RB_DocType doctype,
    struct RB_header *header )
/*
 * INPUTS
 *   * dest_doc -- the destination file.
 *   * header   -- pointer to the header the index entry is for.
 *   * output_mode -- global with the current output mode.
 * SOURCE
 */
{
    switch ( doctype )
    {
    case TEST:
        /* TODO */
        break;
    case XMLDOCBOOK:
        /* TODO */
        break;
    case HTML:
        /* TODO */
        break;
    case LATEX:
        RB_LaTeX_Generate_Index_Entry( dest_doc, header );
        break;
    case RTF:
        /* TODO */
        break;
    case ASCII:
        /* No index available */
        break;
    case TROFF:
        /* No index available */
        break;
    case UNKNOWN:
    default:
        assert( 0 );
    }
}

/*******/


/****f* Generator/RB_Generate_TOC_2
 * FUNCTION
 *   Create a Table of Contents based on the headers found in
 *   _all_ source files.   There is also a function to create
 *   a table of contents based on the headers found in a single
 *   source file RB_Generate_TOC_1
 * SYNOPSIS
 */
void RB_Generate_TOC_2(
    FILE *dest_doc,
    struct RB_header **headers,
    int count,
    struct RB_Part *owner,
    char *dest_name )
/*
 * INPUTS
 *   * dest_doc -- the destination file.
 *   * headers  -- an array of pointers to all the headers.
 *   * count    -- the number of pointers in the array.
 *   * output_mode -- global with the current output mode.
 *   * owner    -- The owner of the TOC. Only the headers that are owned
 *               by this owner are included in the TOC.  Can be NULL,
 *               in which case all headers are included.
 * SOURCE
 */
{
    switch ( output_mode )
    {
    case TEST:
        break;
    case TROFF:
        break;
    case XMLDOCBOOK:
        break;
    case HTML:
        RB_HTML_Generate_TOC_2( dest_doc, headers, count, owner, dest_name );
        break;
    case LATEX:
        /* LaTeX has it's own mechanism for creating
         * a table of content */
        break;
    case RTF:
        RB_RTF_Generate_TOC_2( dest_doc, headers, count );
        break;
    case ASCII:
        /* TODO: No TOC in ASCII mode */
        break;
    case UNKNOWN:

    default:
        assert( 0 );
    }
}

/******/


/****f* Generator/RB_Generate_Doc_Start
 * NAME
 *   RB_Generate_Doc_Start -- Generate document header.
 * SYNOPSIS
 */
void RB_Generate_Doc_Start(
    struct RB_Document *document,
    FILE *dest_doc,
    char *src_name,
    char *title,
    char toc,
    char *dest_name,
    char *charset )
/*
 * FUNCTION
 *   Generates for depending on the output_mode the text that
 *   will be at the start of a document.
 *   Including the table of contents.
 * INPUTS
 *   o dest_doc - pointer to the file to which the output will
 *                be written.
 *   o src_name - the name of the source file or directory.
 *   o name     - the name of this file.
 *   o output_mode - global variable that indicates the output
 *                   mode.
 *   o toc      - generate table of contens
 * SEE ALSO
 *   RB_Generate_Doc_End
 * SOURCE
 */
{
    switch ( output_mode )
    {
    case TEST:
        RB_TEST_Generate_Doc_Start( dest_doc, src_name, title, toc );
        break;
    case XMLDOCBOOK:
        RB_XMLDB_Generate_Doc_Start( document, dest_doc, charset );
        break;
    case HTML:
        RB_HTML_Generate_Doc_Start( dest_doc, src_name, title, dest_name,
                                    charset );
        break;
    case LATEX:
        RB_LaTeX_Generate_Doc_Start( dest_doc, src_name, title, charset );
        break;
    case RTF:
        RB_RTF_Generate_Doc_Start( dest_doc, src_name, title, toc );
        break;
    case ASCII:
        RB_ASCII_Generate_Doc_Start( dest_doc, src_name, title, toc );
        break;
    case TROFF:
        /* */ ;
        break;
    case UNKNOWN:
    default:
        ;
    }
}

/***************/


/****f* Generator/RB_Generate_Doc_End
 * NAME
 *   RB_Generate_Doc_End -- generate document trailer.
 * SYNOPSIS
 */
void RB_Generate_Doc_End(
    FILE *dest_doc,
    char *name,
    char *src_name )
/*
 * FUNCTION
 *   Generates for depending on the output_mode the text that
 *   will be at the end of a document.
 * INPUTS
 *   o dest_doc - pointer to the file to which the output will
 *                be written.
 *   o name     - the name of this file.
 *   o output_mode - global variable that indicates the output
 *                   mode.
 * NOTES
 *   Doesn't do anything with its arguments, but that might
 *   change in the future.
 * BUGS
 * SOURCE
 */
{
    switch ( output_mode )
    {
    case TEST:
        RB_TEST_Generate_Doc_End( dest_doc, name );
        break;
    case XMLDOCBOOK:
        RB_XMLDB_Generate_Doc_End( dest_doc, name );
        break;
    case HTML:
        RB_HTML_Generate_Doc_End( dest_doc, name, src_name );
        break;
    case LATEX:
        RB_LaTeX_Generate_Doc_End( dest_doc, name );
        break;
    case RTF:
        RB_RTF_Generate_Doc_End( dest_doc, name );
        break;
    case TROFF:
        break;
    case ASCII:
        break;
    case UNKNOWN:

    default:
        assert( 0 );
    }
}

/************/


/****f* Generator/RB_Generate_Header_Start [3.0h]
 * NAME
 *   RB_Generate_Header_Start -- generate header start text.
 * SYNOPSIS
 */
FILE               *RB_Generate_Header_Start(
    FILE *dest_doc,
    struct RB_header *cur_header )
/*
 * FUNCTION
 *   Generates depending on the output_mode the text that
 *   will be at the end of each header.
 * INPUTS
 *   o dest_doc - pointer to the file to which the output will
 *                be written.
 *   o cur_header - pointer to a RB_header structure.
 * SEE ALSO
 *   RB_Generate_Header_End
 * SOURCE
 */
{
    switch ( output_mode )
    {
    case TEST:
        RB_TEST_Generate_Header_Start( dest_doc, cur_header );
        break;
    case XMLDOCBOOK:
        RB_XMLDB_Generate_Header_Start( dest_doc, cur_header );
        break;
    case HTML:
        RB_HTML_Generate_Header_Start( dest_doc, cur_header );
        break;
    case LATEX:
        RB_LaTeX_Generate_Header_Start( dest_doc, cur_header );
        break;
    case RTF:
        RB_RTF_Generate_Header_Start( dest_doc, cur_header );
        break;
    case ASCII:
        RB_ASCII_Generate_Header_Start( dest_doc, cur_header );
        break;
    case TROFF:
        dest_doc = RB_TROFF_Generate_Header_Start( dest_doc, cur_header );
        break;
    case UNKNOWN:
    default:
        assert( 0 );
    }
    return dest_doc;
}

/******/


/****f* Generator/RB_Generate_Header_End [3.0h]
 * NAME
 *   RB_Generate_Header_End
 * SYNOPSIS
 */
void RB_Generate_Header_End(
    FILE *dest_doc,
    struct RB_header *cur_header )
/*
 * FUNCTION
 *   Generates for depending on the output_mode the text that
 *   will be at the end of a header.
 *   This function is used if the option --section is _not_
 *   used.
 * INPUTS
 *   o dest_doc - pointer to the file to which the output will
 *              be written.
 *   o cur_header - pointer to a RB_header structure.
 * SEE ALSO
 *   RB_Generate_Header_Start, RB_Generate_EndSection,
 *   RB_Generate_BeginSection
 * SOURCE
 */
{
    switch ( output_mode )
    {                           /* switch by *koessi */
    case TEST:
        RB_TEST_Generate_Header_End( dest_doc, cur_header );
        break;
    case XMLDOCBOOK:
        RB_XMLDB_Generate_Header_End( dest_doc, cur_header );
        break;
    case HTML:
        RB_HTML_Generate_Header_End( dest_doc, cur_header );
        break;
    case LATEX:
        RB_LaTeX_Generate_Header_End( dest_doc, cur_header );
        break;
    case RTF:
        RB_RTF_Generate_Header_End( dest_doc, cur_header );
        break;
    case ASCII:
        RB_ASCII_Generate_Header_End( dest_doc, cur_header );
        break;
    case TROFF:
        RB_TROFF_Generate_Header_End( dest_doc, cur_header );
        break;
    case UNKNOWN:
    default:
        break;
    }
}

/*****/


/****f* Generator/Generate_Item_Name [2.01]
 * NAME
 *   Generate_Item_Name -- fast&easy
 * SYNOPSIS
 */
void Generate_Item_Name(
    FILE *dest_doc,
    int item_type )
/*
 * FUNCTION
 *   write the item's name to the doc
 * INPUTS
 *   o FILE* dest_doc      -- destination file
 *   o int item_type       -- the type of item
 * AUTHOR
 *   Koessi
 * NOTES
 *   uses globals: output_mode
 * SOURCE
 */
{
    char               *name = configuration.items.names[item_type];

    switch ( output_mode )
    {
    case TEST:
        RB_TEST_Generate_Item_Name( dest_doc, name );
        break;
    case XMLDOCBOOK:
        RB_XMLDB_Generate_Item_Name( dest_doc, name );
        break;
    case HTML:
        RB_HTML_Generate_Item_Name( dest_doc, name );
        break;
    case LATEX:
        RB_LaTeX_Generate_Item_Name( dest_doc, name );
        break;
    case RTF:
        RB_RTF_Generate_Item_Name( dest_doc, name );
        break;
    case ASCII:
        RB_ASCII_Generate_Item_Name( dest_doc, name );
        break;
    case TROFF:
        RB_TROFF_Generate_Item_Name( dest_doc, name, Works_Like_SourceItem(item_type) );
        break;
        break;
    case UNKNOWN:
    default:
        assert( 0 );
    }
}

/*********/


void Generate_Begin_Content(
    FILE *dest_doc )
{
    switch ( output_mode )
    {
    case HTML:
        HTML_Generate_Begin_Content( dest_doc );
        break;
    default:
        break;
    }
}

void Generate_End_Content(
    FILE *dest_doc )
{
    switch ( output_mode )
    {
    case HTML:
        HTML_Generate_End_Content( dest_doc );
        break;
    default:
        break;
    }
}

void Generate_Begin_Navigation(
    FILE *dest_doc )
{
    switch ( output_mode )
    {
    case HTML:
        HTML_Generate_Begin_Navigation( dest_doc );
        break;
    default:
        break;
    }

}

void Generate_End_Navigation(
    FILE *dest_doc )
{
    switch ( output_mode )
    {
    case HTML:
        HTML_Generate_End_Navigation( dest_doc );
        break;
    default:
        break;
    }

}

void Generate_IndexMenu(
    FILE *dest_doc,
    char *filename,
    struct RB_Document *document )
{
    switch ( output_mode )
    {
    case HTML:
        RB_HTML_Generate_IndexMenu( dest_doc, filename, document, NULL );
        break;
    default:
        break;
    }

}

void Generate_Begin_Extra(
    FILE *dest_doc )
{
    switch ( output_mode )
    {
    case HTML:
        HTML_Generate_Begin_Extra( dest_doc );
        break;
    default:
        break;
    }
}

void Generate_End_Extra(
    FILE *dest_doc )
{
    switch ( output_mode )
    {
    case HTML:
        HTML_Generate_End_Extra( dest_doc );
        break;
    default:
        break;
    }
}






/* TODO Documentation */
void RB_Generate_Nav_Bar(
    struct RB_Document *document,
    FILE *current_doc,
    struct RB_header *current_header )
{
    switch ( output_mode )
    {
    case TEST:
        break;
    case XMLDOCBOOK:
        break;
    case HTML:
        if ( course_of_action.do_one_file_per_header )
        {
            /* Nothing */
        }
        else
        {
            RB_HTML_Generate_Nav_Bar( document, current_doc, current_header );
        }
        break;
    case LATEX:
        break;
    case RTF:
        break;
    case ASCII:
        break;
    case TROFF:
        break;
    case UNKNOWN:
    default:
        /* Bug */
        assert( 0 );
    }
}



/* TODO Documentation */
int RB_HTML_Extra(
    FILE *dest_doc,
    int item_type,
    char *cur_char,
    char prev_char )
{
    int                 res = -1;

    switch ( output_mode )
    {
    case TEST:
        break;
    case XMLDOCBOOK:
        break;
    case HTML:
        res =
            RB_HTML_Generate_Extra( dest_doc, item_type, cur_char,
                                    prev_char );
        break;
    case LATEX:
        /* TODO */
        break;
    case RTF:
        /* TODO */
        break;
    case ASCII:
        /* TODO */
        break;
    case TROFF:
        res = RB_TROFF_Generate_Extra( dest_doc, item_type, cur_char );
        break;
        break;
    case UNKNOWN:
    default:
        /* Bug */
        assert( 0 );
    }
    return res;
}


/****f* Generator/RB_Name_Headers
 * FUNCTION
 *   Give all headers a unique name. This makes sure that if
 *   two headers have the same name linking to one of the headers
 *   still works.
 * SYNOPSIS
 */
void RB_Name_Headers(
    struct RB_header **headers,
    long count )
/*
 * SOURCE
 */
#define MAX_UNIQUE_ID_LENGTH 80
{
    int                 i;
    char                id[MAX_UNIQUE_ID_LENGTH + 1];

    RB_Say( "Assigning headers a unique name.\n", SAY_DEBUG );
    for ( i = 0; i < count; ++i )
    {
        struct RB_header   *header;

        header = headers[i];
        sprintf( id, "robo%d", i );
        header->unique_name = RB_StrDup( id );
    }
}

/******/

/****f* Generator/RB_Sort_Items
 * FUNCTION
 *   Sort the items in all the headers according to the order
 *   specified in the 'item order' block in the robodoc.rc
 *   file.
 * SYNOPSIS
 */
static void RB_Sort_Items(
    struct RB_header **headers,
    long header_count )
/*
 * SOURCE
 */
{
    int                 n_order = configuration.item_order.number;

    if ( n_order )
    {
        int                 i = 0;
        int                 j = 0;
        int                 max_number_of_items = 0;
        struct RB_Item    **items = NULL;
        struct RB_Item    **items_sorted = NULL;

        RB_Say( "Sorting items in %d headers.\n", SAY_DEBUG, header_count );

        /* Compute the maximum number of items in any given header */
        for ( j = 0; j < header_count; ++j )
        {
            struct RB_header   *header;
            struct RB_Item     *item;
            int                 item_count = 0;

            header = headers[j];
            for ( item = header->items; item; item = item->next )
            {
                ++item_count;
            }

            if ( item_count > max_number_of_items )
            {
                max_number_of_items = item_count;
            }
        }
        /* Allocate an array for the items, this makes it easier to
         * sort. */

        RB_Say( "Largest header has %d items.\n", SAY_DEBUG,
                max_number_of_items );

        if ( max_number_of_items == 0 )
        {
            /* No items in any of the headers, do nothing */
        }
        else
        {
            items = calloc( max_number_of_items, sizeof( struct RB_Item * ) );
            items_sorted =
                calloc( max_number_of_items, sizeof( struct RB_Item * ) );

            /* Sort items */
            for ( j = 0; j < header_count; ++j )
            {
                struct RB_header   *header = NULL;
                struct RB_Item     *item = NULL;
                int                 item_index = 0;
                int                 sorted_item_index = 0;
                int                 item_count = 0;

                header = headers[j];

                /* Copy item pointers to array */
                for ( item = header->items; item;
                      item = item->next, ++item_index )
                {
                    items[item_index] = item;
                    items_sorted[item_index] = item;
                };
                item_count = item_index;

                if ( item_count == 0 )
                {
                    /* No items in this header, do nothing. */
                }
                else
                {
                    assert( item_count <= max_number_of_items );

                    /* First copy the items in the order that is defined in
                     * item_order[] to sorted_items[] */
                    sorted_item_index = 0;
                    for ( i = 0; i < n_order; ++i )
                    {
                        for ( item_index = 0; item_index < item_count;
                              ++item_index )
                        {
                            if ( items[item_index] )
                            {
                                if ( strcmp
                                     ( configuration.items.
                                       names[items[item_index]->type],
                                       configuration.item_order.names[i] ) ==
                                     0 )
                                {
                                    /* copy to items_sorted */
                                    items_sorted[sorted_item_index] =
                                        items[item_index];
                                    ++sorted_item_index;
                                    items[item_index] = NULL;
                                }
                            }
                        }
                    }
                    /* Then copy the remaining items to items_sorted[] */
                    for ( item_index = 0; item_index < item_count;
                          ++item_index )
                    {
                        if ( items[item_index] )
                        {
                            /* copy to items_sorted */
                            items_sorted[sorted_item_index] =
                                items[item_index];
                            ++sorted_item_index;
                            items[item_index] = NULL;
                        }
                    }

                    assert( sorted_item_index == item_count );

                    /* Now copy the items in items_sorted[] back into the linked list in
                     * the header */
                    if ( item_count > 1 )
                    {
                        /* Chain all the items */
                        for ( item_index = 0; item_index < item_count - 1;
                              ++item_index )
                        {
                            items_sorted[item_index]->next =
                                items_sorted[item_index + 1];
                        }
                    }

                    assert( item_count > 0 );
                    items_sorted[item_count - 1]->next = NULL;
                    /* now link the first item to the header. */
                    header->items = items_sorted[0];
                }
            }

            free( items_sorted );
            free( items );
        }
    }

    RB_Say( "Done sorting items in %d headers.\n", SAY_DEBUG, header_count );
}

/*******/


/****f* Generator/RB_Generate_Documentation
 * FUNCTION
 *   Generate the documentation for all the information contained in a
 *   RB_Document structure.
 * SYNOPSIS
 */
void RB_Generate_Documentation(
    struct RB_Document *document )
/*
 * INPUTS
 *   document -- pointer to the RB_Document structure.
 * SOURCE
 */
{
    RB_SetCurrentFile( NULL );

    if ( document->actions.do_singledoc )
    {
        RB_Generate_SingleDoc( document );
    }
    else if ( document->actions.do_multidoc )
    {
        RB_Generate_MultiDoc( document );
    }
    else if ( document->actions.do_singlefile )
    {
        RB_Generate_SingleDoc( document );
    }
}

/*****/

/****f* Generator/RB_Generate_MultiDoc
 * FUNCTION
 *   Create documentation by creating a file for each
 *   individual source file that was scanned.
 * SYNOPSIS
 */
void RB_Generate_MultiDoc(
    struct RB_Document *document )
/*
 * INPUTS
 *   document -- pointer to the RB_Document structure.
 * SOURCE
 */
{
    struct RB_Part     *i_part;
    FILE               *document_file = NULL;

    RB_Document_Determine_DocFilePaths( document );
    RB_Document_Create_DocFilePaths( document );
    if ( document->actions.do_one_file_per_header )
    {
        RB_Document_Split_Parts( document );
    }
    RB_Document_Determine_DocFileNames( document );
    RB_Document_Collect_Headers( document );
    if ( document->actions.do_nosort )
    {
        /* Nothing */
    }
    else
    {
        RB_Document_Sort_Headers( document );
    }
    RB_Document_Link_Headers( document );
    RB_Loop_Check( document );
    RB_Fill_Header_Filename( document );
    RB_Name_Headers( document->headers, document->no_headers );
    RB_Sort_Items( document->headers, document->no_headers );
    RB_CollectLinks( document, document->headers, document->no_headers );
    if ( output_mode == HTML )
    {
        RB_Create_CSS( document );
    }


    for ( i_part = document->parts; i_part != NULL; i_part = i_part->next )
    {

        char               *srcname = Get_Fullname( i_part->filename );
        char               *docname = RB_Get_FullDocname( i_part->filename );

        /* Nothing found in this part, do not generate it */
        if ( i_part->headers == 0 )
            continue;


        if ( output_mode != TROFF )
        {
            document_file = RB_Open_Documentation( i_part );
            RB_Generate_Doc_Start( document,
                                   document_file, srcname, srcname, 1,
                                   docname, document->charset );

            Generate_Begin_Navigation( document_file );
            if ( document->actions.do_one_file_per_header )
            {
                RB_HTML_Generate_Nav_Bar_One_File_Per_Header( document,
                                                              document_file,
                                                              i_part->
                                                              headers );
            }
            else
            {
                Generate_IndexMenu( document_file, docname, document );
            }
            Generate_End_Navigation( document_file );

            Generate_Begin_Content( document_file );

            if ( ( document->actions.do_toc ) && document->no_headers )
            {
                RB_Generate_TOC_2( document_file,
                                   document->headers, document->no_headers,
                                   i_part, docname );
            }
            RB_Generate_Part( document_file, document, i_part );
            Generate_End_Content( document_file );

            RB_Generate_Doc_End( document_file, docname, srcname );
            fclose( document_file );
        }
        else
        {
            RB_Generate_Part( document_file, document, i_part );
        }
    }

    if ( document->actions.do_index )
    {
        Generate_Index( document );
    }

    RB_Free_Links(  );
}

/*****/


/****f* Generator/RB_Generate_SingleDoc
 * FUNCTION
 *   Create documentation by creating a single file for all individual
 *   source file that were scanned.
 *
 *   This function is called when the option --singledoc is used.
 *   Based on whether the option --sections is used this function then
 *   calls RB_Generate_Sections or RB_Generate_Part
 * SYNOPSIS
 */
void RB_Generate_SingleDoc(
    struct RB_Document *document )
/*
 * INPUTS
 *   document -- pointer to the RB_Document structure.
 * SOURCE
 */
{
    FILE               *document_file;
    struct RB_Part     *i_part;

    RB_Document_Collect_Headers( document );
    if ( document->actions.do_nosort )
    {
        /* Nothing */
    }
    else
    {
        RB_Document_Sort_Headers( document );
    }
    RB_Document_Link_Headers( document );
    RB_Loop_Check( document );
    RB_Fill_Header_Filename( document );
    RB_Name_Headers( document->headers, document->no_headers );
    RB_Sort_Items( document->headers, document->no_headers );
    RB_CollectLinks( document, document->headers, document->no_headers );

    for ( i_part = document->parts; i_part != NULL; i_part = i_part->next )
    {
        RB_Set_FullDocname( i_part->filename, document->singledoc_name );
    }

    if ( output_mode == HTML )
    {
        RB_Create_CSS( document );
    }

    document_file = RB_Open_SingleDocumentation( document );
    assert( document->parts->filename->name );


    RB_Generate_Doc_Start( document,
                           document_file,
                           document->srcroot->name,
                           document->singledoc_name, 1,
                           document->singledoc_name, document->charset );

    if ( ( document->actions.do_toc ) && document->no_headers )
    {
        RB_Generate_TOC_2( document_file,
                           document->headers, document->no_headers, NULL,
                           document->parts->filename->name );
    }
    if ( document->actions.do_sections )
    {
        RB_Generate_Sections( document_file, document );
    }
    else
    {
        for ( i_part = document->parts;
              i_part != NULL; i_part = i_part->next )
        {
            RB_Generate_Part( document_file, document, i_part );
        }
    }

    RB_Generate_Doc_End( document_file, "singledoc",
                         document->srcroot->name );
    fclose( document_file );

    RB_Free_Links(  );
}

/******/


/****f* Generator/RB_Generate_Sections
 * FUNCTION
 *   Creates the documentation for all headers found in all source
 *   files.  The order in which they are generated depends on the
 *   header hierarchy.  First the top level header's documentation
 *   is generated then, the documentation for all it's childern, then
 *   the next top level header's documentation is generated.
 *   This is a recursive proces.
 *   The idea is to create something like:
 *     1. Parentheader1
 *     1.1 Child1
 *     1.2 Child2
 *     1.2.1 Child's child1
 *     2. Parentheader2
 *   etc
 * SYNOPSIS
 */
void RB_Generate_Sections(
    FILE *document_file,
    struct RB_Document *document )
/*
 * INPUTS
 *   o document_file -- destination file.
 *   o document -- pointer to the RB_Document structure.
 * SOURCE
 */
{
    unsigned long       i;
    int                 depth = 1;
    struct RB_header   *header;

    depth = document->first_section_level;

    RB_Say( "Generating Sections\n", SAY_INFO );
    for ( i = 0; i < document->no_headers; ++i )
    {
        header = ( document->headers )[i];
        if ( header->parent )
        {
            /* This will be in one of the subsections */
        }
        else
        {
            RB_Generate_Section( document_file, header, document, depth );
        }
    }
}

/******/



/****f* Generator/RB_Generate_Section
 * FUNCTION
 *   Generate the documentation for a header and all
 *   its childern.
 * INPUTS
 *   o document_file -- destination file
 *   o parent -- the parent of the header for which the documentation
 *               is to be generated.
 *   o document -- pointer to the RB_Document structure.
 *   o depth -- level of sectioning ( 1  1.1  1.1.1  etc)
 * SYNOPSIS
 */
void RB_Generate_Section(
    FILE *document_file,
    struct RB_header *parent,
    struct RB_Document *document,
    int depth )
/*
 * NOTE
 *   This is a recursive function.
 * SEE ALSO
 *    RB_Generate_Sections
 * SOURCE
 */
{
    unsigned long       i;
    char               *headername;

    /* We pass either modulename/name or just the name */
    if ( course_of_action.do_sectionnameonly )
    {
        headername = parent->function_name;
    }
    else
    {
        headername = parent->name;
    }

    switch ( output_mode )
    {
    case XMLDOCBOOK:
        {
            RB_Generate_BeginSection( document_file, depth, headername,
                                      parent );
            /* Docbook output does not like the labels to be
             * generated before the <section> part
             */
            Generate_Label( document_file, parent->unique_name );
            Generate_Label( document_file, parent->name );
        }
        break;
        /* lowtexx 21.09.2005 11:37 */
    case LATEX:
        {
            RB_Generate_BeginSection( document_file, depth, headername,
                                      parent );
            /* We have to start section before labeling in latex */
            Generate_Label( document_file, parent->unique_name );
            Generate_Label( document_file, parent->name );
        }
        break;
        /* --- */
    default:
        {
            Generate_Label( document_file, parent->unique_name );
            Generate_Label( document_file, parent->name );
            RB_Generate_BeginSection( document_file, depth, headername,
                                      parent );
        }
        break;
    }

    RB_Generate_Nav_Bar( document, document_file, parent );
    RB_Generate_Index_Entry( document_file, document->doctype, parent );
    Generate_Header( document_file, parent, document->singledoc_name );
    for ( i = 0; i < document->no_headers; ++i )
    {
        struct RB_header *header = ( document->headers )[i];
        if ( header->parent == parent )
        {
            RB_Generate_Section( document_file, header, document, depth + 1 );
        }
        else
        {
            /* Leeg */
        }
    }
    RB_Generate_EndSection( document_file, depth, parent->name );
}

/******/



/****f* Generator/RB_Generate_Part
 * FUNCTION
 *   Generate the documention for all the headers found in a single
 *   source file.
 * SYNOPSIS
 */
void RB_Generate_Part(
    FILE *document_file,
    struct RB_Document *document,
    struct RB_Part *part )
/*
 * INPUTS
 *   * document_file -- The file were it stored.
 *   * document      -- All the documentation.
 *   * part          -- pointer to a RB_Part that contains all the headers found
 *                    in a single source file.
 * SOURCE
 */
{
    struct RB_header   *i_header;
    char               *docname = NULL;
    char               *srcname = Get_Fullname( part->filename );

    RB_Say( "generating documentation for file \"%s\"\n", SAY_INFO, srcname );
    if ( document->actions.do_singledoc )
    {
        docname = document->singledoc_name;
    }
    else if ( document->actions.do_multidoc )
    {
        docname = RB_Get_FullDocname( part->filename );
    }
    else if ( document->actions.do_singlefile )
    {
        docname = document->singledoc_name;
    }
    else
    {
        assert( 0 );
    }

    if ( output_mode == TROFF )
    {
        RB_TROFF_Set_Param( document->compress, document->section );
    }


    for ( i_header = part->headers; i_header; i_header = i_header->next )
    {
        RB_Say( "generating documentation for header \"%s\"\n", SAY_INFO,
                i_header->name );
        document_file = RB_Generate_Header_Start( document_file, i_header );
        RB_Generate_Nav_Bar( document, document_file, i_header );
        RB_Generate_Index_Entry( document_file, document->doctype, i_header );
        Generate_Header( document_file, i_header, docname );
        RB_Generate_Header_End( document_file, i_header );
    }
}

/******/


/****f* Generator/RB_Get_DOT_Type
 * FUNCTION
 *   Returns the type of the DOT file for the given output mode
 * SOURCE
 */
char               *RB_Get_DOT_Type(
    void )
{
    switch ( output_mode )
    {
    case HTML:
        return DOT_HTML_TYPE;
        break;

    case LATEX:
        return DOT_LATEX_TYPE;
        break;

    default:
        break;
    }

    return NULL;
}

/******/


/****f* Generator/RB_Generate_DOT_Image_Link
 * FUNCTION
 *   Generates the image link for the created dot graphics
 * SOURCE
 */
void RB_Generate_DOT_Image_Link(
    FILE *f,
    int dot_nr,
    char *dot_type )
{
    switch ( output_mode )
    {
    case HTML:
        fprintf( f, "<img src=\"%s%d.%s\">\n", DOT_GRAPH_NAME, dot_nr,
                 dot_type );
        break;

    case LATEX:
        fprintf( f, "\\includegraphics{%s%d}\n", DOT_GRAPH_NAME, dot_nr );
        break;

    default:
        break;
    }
}

/******/

/****f* Generator/RB_Generate_Item_Line_Number
 * FUNCTION
 *   Generate line numbers for SOURCE like items
 * SYNOPSIS
 */
void RB_Generate_Item_Line_Number(
    FILE *dest_doc,
    int arg_line_number,
    int max_lines )
/*
 * INPUTS
 *   o dest_doc    -- the file to write to.
 *   o line_number -- the actual line number.
 *   o max_lines   -- the maximal line number in this item.
 * SOURCE
 */
{
    /* First check if we should generate line numbers at all */
    if ( course_of_action.do_source_line_numbers )
    {
        char                fmt[TEMP_BUF_SIZE], str[TEMP_BUF_SIZE];
        int                 len;

        /* produce the maximal line number as a string */
        snprintf( str, sizeof( str ), "%d", max_lines );
        /* get the length of the maximal line number */
        len = strlen( str );
        /* create the formatting string for the line numbers */
        snprintf( fmt, sizeof( fmt ), "%%%dd ", len );
        /* and create the line number as a string */
        snprintf( str, sizeof( str ), fmt, arg_line_number );

        /* handle the various output modes */
        switch ( output_mode )
        {
        case ASCII:
            RB_ASCII_Generate_Item_Line_Number( dest_doc, str );
            break;

        case HTML:
            RB_HTML_Generate_Item_Line_Number( dest_doc, str );
            break;

        case LATEX:
            RB_LaTeX_Generate_Item_Line_Number( dest_doc, str );
            break;

        case RTF:
            RB_RTF_Generate_Item_Line_Number( dest_doc, str );
            break;

        case TROFF:
            RB_TROFF_Generate_Item_Line_Number( dest_doc, str );
            break;

        case XMLDOCBOOK:
            RB_XMLDB_Generate_Item_Line_Number( dest_doc, str );
            break;

        default:
            break;
        }
    }
}

/******/

/****f* Generator/Generate_Item
 * SYNOPSIS
 */
static void Generate_Item(
    FILE *f,
    struct RB_header *header,
    struct RB_Item *cur_item,
    char *docname )
 /*
  * FUNCTION
  *   Generate the documentation for a single item.
  *
  * NOTE
  *   This function is way too long...
  *
  * SOURCE
  */
{
    static int          dot_nr = 1;
    int                 line_nr;
    char               *dot_type = NULL;
    FILE               *tool = NULL;    /* Pipe handler to the tool we use */
    enum ItemType       item_type = cur_item->type;
    char               *name = configuration.items.names[item_type];


    Generate_Item_Name( f, item_type );
    RB_Generate_Item_Begin( f, name );
    for ( line_nr = 0; line_nr < cur_item->no_lines; ++line_nr )
    {
        struct RB_Item_Line *item_line = cur_item->lines[line_nr];
        char               *line = item_line->line;

        /* Plain item lines */
        if ( !Works_Like_SourceItem( item_type ) &&
             ( item_line->kind == ITEM_LINE_PLAIN ) )
        {
            Format_Line( f, item_line->format );
            Generate_Item_Line( f, line, item_type, docname, header );
        }
        /* Last line */
        else if ( item_line->kind == ITEM_LINE_END )
        {
            Format_Line( f, item_line->format );
        }
        /* Normal Pipes */
        else if ( !Works_Like_SourceItem( item_type ) &&
                  ( item_line->kind == ITEM_LINE_PIPE ) )
        {
            Format_Line( f, item_line->format );
            if ( item_line->pipe_mode == output_mode )
            {
                Pipe_Line( f, line );
            }
        }
        /* Tool start */
        else if ( !Works_Like_SourceItem( item_type ) &&
                  ( item_line->kind == ITEM_LINE_TOOL_START ) )
        {
            Format_Line( f, item_line->format );

            /* Change to docdir */
            RB_Change_To_Docdir( docname );

            /* Open pipe to tool */
            tool = RB_Open_Pipe( line );

            /* Get back to working dir */
            RB_Change_Back_To_CWD(  );
        }
        /* Tool (or DOT) body */
        else if ( !Works_Like_SourceItem( item_type ) &&
                  ( item_line->kind == ITEM_LINE_TOOL_BODY ) )
        {
            if ( tool != NULL )
            {
                fprintf( tool, "%s\n", line );
            }
        }
        /* Tool end */
        else if ( !Works_Like_SourceItem( item_type ) &&
                  ( item_line->kind == ITEM_LINE_TOOL_END ) )
        {
            /* Close pipe */
            RB_Close_Pipe( tool );
            tool = NULL;
        }
        /* DOT start */
        else if ( !Works_Like_SourceItem( item_type ) &&
                  ( item_line->kind == ITEM_LINE_DOT_START ) )
        {
            Format_Line( f, item_line->format );

            /* Get DOT file type */
            dot_type = RB_Get_DOT_Type(  );

            if ( dot_type )
            {
                char                pipe_str[TEMP_BUF_SIZE];

                /* Change to docdir */
                RB_Change_To_Docdir( docname );
                snprintf( pipe_str, sizeof( pipe_str ),
                          "%s -T%s -o%s%d.%s", dot_name,
                          dot_type, DOT_GRAPH_NAME, dot_nr, dot_type );
                tool = RB_Open_Pipe( pipe_str );
            }
        }
        /* DOT end */
        else if ( !Works_Like_SourceItem( item_type ) &&
                  ( item_line->kind == ITEM_LINE_DOT_END ) )
        {
            if ( tool )
            {
                /* Close pipe */
                RB_Close_Pipe( tool );
                tool = NULL;

                /* Generate link to image */
                RB_Generate_DOT_Image_Link( f, dot_nr, dot_type );

                /* Get back to working dir */
                RB_Change_Back_To_CWD(  );

                /* Increment dot file number */
                dot_nr++;
            }
        }
        /* DOT file include */
        else if ( !Works_Like_SourceItem( item_type ) &&
                  ( item_line->kind == ITEM_LINE_DOT_FILE ) )
        {
            Format_Line( f, item_line->format );

            /* Get DOT file type */
            dot_type = RB_Get_DOT_Type(  );

            if ( dot_type )
            {
                FILE               *in = NULL, *dot_pipe = NULL;
                char                str[TEMP_BUF_SIZE];

                /* Open the dot source */
                snprintf( str, sizeof( str ), "%s%s",
                          header->owner->filename->path->name, line );
                in = RB_Open_File( str, "r" );

                /* Change to docdir */
                RB_Change_To_Docdir( docname );

                /* Open a pipe to DOT */
                snprintf( str, sizeof( str ),
                          "%s -T%s -o%s%d.%s", dot_name,
                          dot_type, DOT_GRAPH_NAME, dot_nr, dot_type );
                dot_pipe = RB_Open_Pipe( str );

                /* Pipe data to DOT */
                while ( fgets( str, sizeof( str ), in ) != NULL )
                    fputs( str, dot_pipe );

                /* Close file handlers */
                RB_Close_File( in );
                RB_Close_Pipe( dot_pipe );

                /* Generate link to image */
                RB_Generate_DOT_Image_Link( f, dot_nr, dot_type );

                /* Get back to working dir */
                RB_Change_Back_To_CWD(  );

                /* Increment dot file number */
                dot_nr++;
            }
        }
        /* Exec item */
        else if ( !Works_Like_SourceItem( item_type ) &&
                  ( item_line->kind == ITEM_LINE_EXEC ) )
        {
            Format_Line( f, item_line->format );

            /* Change to docdir */
            RB_Change_To_Docdir( docname );

            /* Execute line */
            system( line );

            /* Get back to working dir */
            RB_Change_Back_To_CWD(  );
        }
        /* Source lines */
        else if ( Works_Like_SourceItem( item_type ) )
        {
            Format_Line( f, item_line->format );
            /* Generate line numbers for SOURCE like items */
            RB_Generate_Item_Line_Number( f, item_line->line_number,
                                          cur_item->max_line_number );
            /* Generate item line */
            Generate_Item_Line( f, line, item_type, docname, header );
        }
        else
        {
            /* This item line is ignored */
        }
    }
    RB_Generate_Item_End( f, name );
}

/******/


/****f* Generator/Generate_Header
 * FUNCTION
 *   Generate the documentation for all the items found in
 *   a header except for any items specified in
 *   configuration.ignore_items.
 * SYNOPSIS
 */
static void Generate_Header(
    FILE *f,
    struct RB_header *header,
    char *docname )
/*
 * INPUTS
 *   * f -- destination file
 *   * header -- header to be searched.
 *   * srcname -- name of the source file the header was found in.
 *   * docname -- name of the documentation file.
 * BUGS
 *   This skips the first item body if the first item name was
 *   not correctly spelled.
 * SOURCE
 */
{
    struct RB_Item     *cur_item;

    for ( cur_item = header->items; cur_item; cur_item = cur_item->next )
    {
        enum ItemType       item_type = cur_item->type;
        char               *name = configuration.items.names[item_type];

        if ( Is_Ignore_Item( name ) )
        {
            /* User does not want this item */
        }
        else if ( Works_Like_SourceItem( item_type )
                  && ( course_of_action.do_nosource ) )
        {
            /* User does not want source items */
        }
        else
        {
            Generate_Item( f, header, cur_item, docname );
        }
    }
}

/******/


static void Pipe_Line(
    FILE *dest_doc,
    char *arg_line )
{
    char               *cur_char = arg_line;

    for ( ; *cur_char && ( *cur_char == ' ' || *cur_char == '\t' );
          cur_char++ )
    {
        fputc( *cur_char, dest_doc );
    }
    fprintf( dest_doc, "%s%s", cur_char,
             ( output_mode == RTF ? "\\line" : "\n" ) );
}


static void Format_Line(
    FILE *dest_doc,
    long format )
{
    if ( format & RBILA_END_LIST_ITEM )
    {
        Generate_End_List_Item( dest_doc );
    }
    if ( format & RBILA_END_LIST )
    {
        Generate_End_List( dest_doc );
    }
    if ( format & RBILA_END_PRE )
    {
        Generate_End_Preformatted( dest_doc );
    }
    if ( format & RBILA_BEGIN_PARAGRAPH )
    {
        Generate_Begin_Paragraph( dest_doc );
    }
    if ( format & RBILA_END_PARAGRAPH )
    {
        Generate_End_Paragraph( dest_doc );
    }
    if ( format & RBILA_BEGIN_PRE )
    {
        Generate_Begin_Preformatted( dest_doc,
                                     ( format & RBILA_BEGIN_SOURCE ) );
    }
    if ( format & RBILA_BEGIN_LIST )
    {
        Generate_Begin_List( dest_doc );
    }
    if ( format & RBILA_BEGIN_LIST_ITEM )
    {
        Generate_Begin_List_Item( dest_doc );
    }
}



static void Generate_Begin_List_Item(
    FILE *dest_doc )
{
    switch ( output_mode )
    {
    case TEST:
        TEST_Generate_Begin_List_Item( dest_doc );
        break;
    case XMLDOCBOOK:
        XMLDB_Generate_Begin_List_Item( dest_doc );
        break;
    case HTML:
        HTML_Generate_Begin_List_Item( dest_doc );
        break;
    case LATEX:
        LaTeX_Generate_Begin_List_Item( dest_doc );
        break;
    case RTF:
        break;
    case TROFF:
        TROFF_Generate_Begin_List_Item( dest_doc );
        break;
        break;
    case ASCII:
        break;
    case UNKNOWN:
    default:
        assert( 0 );
    }
}


static void Generate_End_List_Item(
    FILE *dest_doc )
{
    switch ( output_mode )
    {
    case TEST:
        TEST_Generate_End_List_Item( dest_doc );
        break;
    case XMLDOCBOOK:
        XMLDB_Generate_End_List_Item( dest_doc );
        break;
    case HTML:
        HTML_Generate_End_List_Item( dest_doc );
        break;
    case LATEX:
        LaTeX_Generate_End_List_Item( dest_doc );
        break;
    case RTF:
        break;
    case TROFF:
        TROFF_Generate_End_List_Item( dest_doc );
        break;
        break;
    case ASCII:
        break;
    case UNKNOWN:
    default:
        assert( 0 );
    }
}


static void Generate_Begin_List(
    FILE *dest_doc )
{
    switch ( output_mode )
    {
    case TEST:
        TEST_Generate_Begin_List( dest_doc );
        break;
    case XMLDOCBOOK:
        XMLDB_Generate_Begin_List( dest_doc );
        break;
    case HTML:
        HTML_Generate_Begin_List( dest_doc );
        break;
    case LATEX:
        LaTeX_Generate_Begin_List( dest_doc );
        break;

    case RTF:
    case TROFF:
    case ASCII:
        break;

    case UNKNOWN:
    default:
        assert( 0 );
    }
}

/* TODO FS Document */

static void Generate_End_List(
    FILE *dest_doc )
{
    switch ( output_mode )
    {
    case TEST:
        TEST_Generate_End_List( dest_doc );
        break;
    case XMLDOCBOOK:
        XMLDB_Generate_End_List( dest_doc );
        break;
    case HTML:
        HTML_Generate_End_List( dest_doc );
        break;
    case LATEX:
        LaTeX_Generate_End_List( dest_doc );
        break;

    case RTF:
    case TROFF:
    case ASCII:
        break;

    case UNKNOWN:
    default:
        assert( 0 );
    }
}


/* TODO FS Document */

static void Generate_Begin_Preformatted(
    FILE *dest_doc,
    int source )
{
    switch ( output_mode )
    {
    case TEST:
        TEST_Generate_Begin_Preformatted( dest_doc );
        break;
    case XMLDOCBOOK:
        XMLDB_Generate_Begin_Preformatted( dest_doc );
        break;
    case HTML:
        HTML_Generate_Begin_Preformatted( dest_doc, source );
        break;
    case LATEX:
        LaTeX_Generate_Begin_Preformatted( dest_doc );
        break;
    case RTF:
        break;
    case TROFF:
        TROFF_Generate_Begin_Preformatted( dest_doc );
        break;
    case ASCII:
        break;
    case UNKNOWN:
    default:
        assert( 0 );
    }
}


/* TODO FS Document */
static void Generate_End_Preformatted(
    FILE *dest_doc )
{
    switch ( output_mode )
    {
    case TEST:
        TEST_Generate_End_Preformatted( dest_doc );
        break;
    case XMLDOCBOOK:
        XMLDB_Generate_End_Preformatted( dest_doc );
        break;
    case HTML:
        HTML_Generate_End_Preformatted( dest_doc );
        break;
    case LATEX:
        LaTeX_Generate_End_Preformatted( dest_doc );
        break;
    case RTF:
        break;
    case TROFF:
        TROFF_Generate_End_Preformatted( dest_doc );
        break;
    case ASCII:
        break;
    case UNKNOWN:
    default:
        assert( 0 );
    }
}


/* TODO FS Document */
static void Generate_End_Paragraph(
    FILE *dest_doc )
{
    switch ( output_mode )
    {
    case TEST:
        TEST_Generate_End_Paragraph( dest_doc );
        break;
    case XMLDOCBOOK:
        XMLDB_Generate_End_Paragraph( dest_doc );
        break;
    case HTML:
        HTML_Generate_End_Paragraph( dest_doc );
        break;
    case LATEX:
        LaTeX_Generate_End_Paragraph( dest_doc );
        break;
    case RTF:
        break;
    case TROFF:
        TROFF_Generate_End_Paragraph( dest_doc );
        break;
    case ASCII:
        break;
    case UNKNOWN:
    default:
        assert( 0 );
    }
}


/* TODO FS Document */
static void Generate_Begin_Paragraph(
    FILE *dest_doc )
{
    switch ( output_mode )
    {
    case TEST:
        TEST_Generate_Begin_Paragraph( dest_doc );
        break;
    case XMLDOCBOOK:
        XMLDB_Generate_Begin_Paragraph( dest_doc );
        break;
    case HTML:
        HTML_Generate_Begin_Paragraph( dest_doc );
        break;
    case LATEX:
        LaTeX_Generate_Begin_Paragraph( dest_doc );
        break;
    case RTF:
        break;
    case TROFF:
        TROFF_Generate_Begin_Paragraph( dest_doc );
        break;
    case ASCII:
        break;
    case UNKNOWN:
    default:
        assert( 0 );
    }
}


static void Generate_Item_Line(
    FILE *dest_doc,
    char *line,
    int item_type,
    char *docname,
    struct RB_header *fnames )
{
    char               *cur_char = line;
    char               *object_name = NULL;
    char               *label_name = NULL;
    char               *file_name = NULL;

    enum EIL_State
    { SKIP_ALPHANUM, SKIP_SPACE, SEARCH_LINK_START_WORD, SEARCH_LINK }
    state = SKIP_SPACE;

    for ( ; *cur_char; cur_char++ )
    {
        char                c = *cur_char;

        /* This is a little statemachine to switch searching for
         * links on and off.  A link can be found for a word
         * or for a word combined with some punctuation
         * characters.  A word is a string of alpha numeric
         * characters (including a '_'),  say FunctionAA_10_B
         * All other characters are punctuation characters.
         * We do not want links to start in the middle of a word,
         * but they can start after or on a punctuation character.
         * So for a link can start at
         *     Foo_bar::SnaFu
         *     ^      ^^^
         *     |      |||
         *
         */

        /* Move to the next state based on current character.  */
        switch ( state )
        {
        case SKIP_SPACE:
            /* In this state we try to skip of a string of space
             * characters until we find something else.
             */
            {
                if ( utf8_ispunct( c ) )
                {
                    /* we were in the process of skipping spaces,
                     * but now we found a non-space character.
                     * This might be the begin of a link, so
                     * switch to the search link state.
                     */
                    state = SEARCH_LINK;
                }
                else if ( utf8_isalnum( c ) || ( c == '_' ) )
                {
                    state = SEARCH_LINK_START_WORD;
                }
                else
                {
                    /* Links can only start with a none space character,
                     * so if the current charater is a space, we skip it.
                     */
                }
                break;
            }
        case SKIP_ALPHANUM:
            /* In this state we skipping a string of alpha
             * numeric charaters after that the first
             * character in this string did not result in a
             * link.
             */
            {
                if ( utf8_isspace( c ) )
                {
                    /* We found the end of the string, so we go
                     * back to the space skipping state
                     */
                    state = SKIP_SPACE;
                }
                else if ( utf8_ispunct( c ) && ( c != '_' ) )
                {
                    /* We found a puntuation character, this end
                     * the string of alpha numeric character, but
                     * can be the begin of a new link, so we
                     * switch to the seach link state.
                     */
                    state = SEARCH_LINK;
                }
                else
                {
                    /* We stay in this state */
                }
            }
            break;
        case SEARCH_LINK_START_WORD:
            /* In this state we are at the start of a string
             * of alpha numeric characters.
             */
            {
                if ( utf8_isalnum( c ) || ( c == '_' ) )
                {
                    /* We are not at the second character of
                     * a string of alpha numeric characters,
                     * we can stop searching for links, as a
                     * link can only start at the begin of
                     * such a string.
                     */
                    state = SKIP_ALPHANUM;
                }
                else if ( utf8_ispunct( c ) && ( c != '_' ) )
                {
                    state = SEARCH_LINK;
                }
                else if ( utf8_isspace( c ) )
                {
                    state = SKIP_SPACE;
                }
                else
                {
                    state = SKIP_SPACE;
                }
            }
            break;
        case SEARCH_LINK:
            {
                /* In this state we search for links. We stop
                 * searching if we encounter a space because this
                 * marks end of the word,
                 */
                if ( utf8_isalnum( c ) || ( c == '_' ) )
                {
                    /* We are at the start of a word.
                     */
                    state = SEARCH_LINK_START_WORD;
                }
                else if ( utf8_isspace( c ) )
                {
                    state = SKIP_SPACE;
                }
                else
                {
                    /* We stay in this state */
                }
            }
            break;
        default:
            assert( 0 );
            break;
        }

        if ( ( ( state == SEARCH_LINK ) ||
               ( state == SEARCH_LINK_START_WORD ) ) &&
             Find_Link( cur_char, &object_name, &label_name, &file_name ) )
        {
            /* We found a link, so we can stop searching for one
             * for now.
             */
            state = SKIP_SPACE;

            if ( object_name && fnames->no_names > 0 )
            {
                extern char        *function_name(
    char * );
                int                 i;

                for ( i = 0; i < fnames->no_names; i++ )
                    if ( strcmp
                         ( object_name,
                           function_name( fnames->names[i] ) ) == 0 )
                        break;
                if ( i < fnames->no_names )
                {
                    RB_Generate_False_Link( dest_doc, object_name );
                    cur_char += strlen( object_name ) - 1;
                }
                else
                {
                    Generate_Link( dest_doc, docname,
                                   file_name, label_name, object_name );
                    cur_char += strlen( object_name ) - 1;
                }
            }
            else
            {
                assert( 0 );
            }
        }
        else
        {
            int                 res =
                RB_HTML_Extra( dest_doc, item_type, cur_char,
                               ( cur_char == line ) ? 0 : *( cur_char - 1 ) );

            if ( res >= 0 )
            {
                cur_char += res;
            }
            else
            {
                /* convert from signed to unsigned */
                unsigned char       c2 = *cur_char;

                Generate_Char( dest_doc, c2 );
            }
        }
    }

    /* TODO Move to the RTF_Generator */
    switch ( output_mode )
    {
    case RTF:
        fprintf( dest_doc, "\\line" );
        break;

    case TROFF:
        RB_TROFF_Start_New_Line( dest_doc );
        break;

    case HTML:
        /* Check for source line comment endings */
        RB_HTML_Generate_Line_Comment_End( dest_doc );
        break;

    case TEST:
    case ASCII:
    case LATEX:
    case XMLDOCBOOK:
    case UNKNOWN:
        break;
    default:
        break;
    }

    /* For all modes we add a newline. */
    fputc( '\n', dest_doc );
}



/****f* Generator/Generate_Link
 * FUNCTION
 *   Generate a link to another headers documentation.
 * SYNOPSIS
 */
void Generate_Link(
    FILE *dest_doc,
    char *docname,
    char *file_name,
    char *label_name,
    char *function_name )
/*
 * INPUTS
 *   * dest_doc      -- the output file
 *   * docname       -- the name of the output file
 *   * file_name     -- the name of the file that contains the link's body
 *   * label_name    -- the label for the link
 *   * function_name -- the name that is shown for the link in the
 *                      documentation
 * SOURCE
 */
{

    switch ( output_mode )
    {
    case TEST:
        RB_TEST_Generate_Link( dest_doc, docname, file_name, label_name,
                               function_name );
        break;
    case XMLDOCBOOK:
        RB_XMLDB_Generate_Link( dest_doc, docname, file_name, label_name,
                                function_name );
        break;
    case HTML:
        RB_HTML_Generate_Link( dest_doc, docname, file_name, label_name,
                               function_name, NULL );
        break;
    case RTF:
        RB_RTF_Generate_Link( dest_doc, docname, file_name, label_name,
                              function_name );
        break;
    case LATEX:
        RB_LaTeX_Generate_String( dest_doc, function_name );
        /* lowtexx 21.09.2005 11:43 */
        /* generate a simple link here */
        RB_LaTeX_Generate_Link( dest_doc, docname, file_name, label_name,
                                function_name );
        break;
    case UNKNOWN:
    case ASCII:
    case TROFF:
    default:
        fprintf( dest_doc, "%s", function_name );
    }
}

/******/


/****f* Generator/Generate_Char
 * FUNCTION
 *   Generate a single character in the current output mode.
 *   The individual generators will make sure that special
 *   characters are escaped.
 * SYNOPSIS
 */
void Generate_Char(
    FILE *dest_doc,
    int cur_char )
/*
 * INPUTS
 *   * dest_doc -- destination file.
 *   * cur_char -- character to be generated.
 *******
 */
{
    switch ( output_mode )
    {
    case TEST:
        RB_TEST_Generate_Char( dest_doc, cur_char );
        break;
    case XMLDOCBOOK:
        RB_XMLDB_Generate_Char( dest_doc, cur_char );
        break;
    case HTML:
        RB_HTML_Generate_Char( dest_doc, cur_char );
        break;
    case LATEX:
        RB_LaTeX_Generate_Char( dest_doc, cur_char );
        break;
    case RTF:
        RB_RTF_Generate_Char( dest_doc, cur_char );
        break;
    case TROFF:
        RB_TROFF_Generate_Char( dest_doc, cur_char );
        break;
    case ASCII:
        RB_ASCII_Generate_Char( dest_doc, cur_char );
        break;
    case UNKNOWN:
    default:
        assert( 0 );
    }
}


/****f* Generator/RB_Get_SubIndex_FileName
 * FUNCTION
 *   Get the name of the master index file for a specific
 *   header_type.
 * SYNOPSIS
 */
char               *RB_Get_SubIndex_FileName(
    char *docroot,
    char *extension,
    struct RB_HeaderType *header_type )
/*
 * INPUTS
 *   * docroot      -- the path to the documentation directory.
 *   * extension    -- the extension for the file
 *   * header_type  -- the header type
 * RESULT
 *   a pointer to a freshly allocated string.
 * NOTES
 *   Has too many parameters.
 * SOURCE
 */
{
    size_t              l;
    char               *filename;

    assert( docroot );

    l = strlen( docroot );

    l += RB_Get_Len_Extension( extension );
    l += strlen( docroot );
    l += strlen( header_type->fileName );

    filename = ( char * ) malloc( l + 2 );
    assert( filename );
    filename[0] = '\0';

    strcat( filename, docroot );
    strcat( filename, header_type->fileName );
    RB_Add_Extension( extension, filename );

    return filename;
}

/*****/


/****f* Generator/Generate_Index
 * NAME
 *   Generate_Index -- generate index file based on xref files.
 * SYNOPSIS
 */
static void Generate_Index(
    struct RB_Document *document )
/*
 * FUNCTION
 *   Create a master index file. It contains pointers to the
 *   documentation generated for each source file, as well as all
 *   "objects" found in the source files.
 * SOURCE
 */
{
    switch ( output_mode )
    {
    case TEST:
        break;
    case XMLDOCBOOK:
        break;
    case HTML:
        RB_HTML_Generate_Index( document );
        break;
    case LATEX:
        /* Latex has a index by default */
        break;
    case UNKNOWN:
    case ASCII:
    case TROFF:
    case RTF:
    default:
        break;
    }
}

/*****/
