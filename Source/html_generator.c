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


/****h* ROBODoc/HTML_Generator
 * FUNCTION
 *   The generator for HTML output.
 *
 *   The generator supports sections upto 7 levels deep.  It supports
 *   a Table of Contents based on all headers.  A masterindex for
 *   all headertypes and seperate masterindexes for each headertype.
 *
 * MODIFICATION HISTORY
 *   2003-02-03   Frans Slothouber  Refactoring
 *   ????-??-??   Frans Slothouber  V1.0
 *******
 * $Id: html_generator.c,v 1.94 2008/06/17 11:49:27 gumpu Exp $
 */


#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>

#include "html_generator.h"
#include "util.h"
#include "globals.h"
#include "robodoc.h"
#include "links.h"
#include "headers.h"
#include "headertypes.h"
#include "generator.h"
#include "items.h"
#include "string.h"
#include "document.h"
#include "directory.h"
#include "path.h"
#include "part.h"
#include "roboconfig.h"

#ifdef DMALLOC
#include <dmalloc.h>
#endif

static char        *css_name = NULL;
static int          in_linecomment = 0; /* are we in a line comment? */

static void         RB_HTML_Generate_String(
    FILE *dest_doc,
    const char *a_string );



/* TODO Documentation */
static void HTML_Generate_Div(
    FILE *dest_doc,
    char *id )
{

    fprintf( dest_doc, "<div id=\"%s\">\n", id );
}


/* TODO Documentation */
static void HTML_Generate_Div_End(
    FILE *dest_doc,
    char *id )
{
    fprintf( dest_doc, "</div> <!-- %s -->\n", id );
}


/* TODO Documentation */

char               *HTML_TOC_Index_Filename(
    struct RB_Document *document )
{
    char               *toc_index_path = NULL;
    char               *toc_index_name = "toc_index.html";

    assert( document->docroot->name );

    toc_index_path =
        calloc( strlen( toc_index_name ) + 2 +
                strlen( document->docroot->name ), 1 );
    strcpy( toc_index_path, document->docroot->name );
    strcat( toc_index_path, toc_index_name );

    return toc_index_path;
}



/* TODO  Documentation */
static void RB_HTML_Generate_Source_Tree_Entry(
    FILE *dest_doc,
    char *dest_name,
    struct RB_Path *parent_path,
    struct RB_Directory *srctree,
    struct RB_Document *document )
{
    struct RB_Path     *cur_path;
    struct RB_Filename *cur_filename;

    fprintf( dest_doc, "<ul>\n" );

    for ( cur_filename = srctree->first;
          cur_filename; cur_filename = cur_filename->next )
    {
        if ( cur_filename->path == parent_path )
        {
            char               *r = 0;

            if ( cur_filename->link )
            {
                if ( document->actions.do_one_file_per_header )
                {
                    fprintf( dest_doc, "<li><tt>\n" );
                    RB_HTML_Generate_String( dest_doc, cur_filename->name );
                    fprintf( dest_doc, "</tt></li>\n" );
                }
                else
                {
                    r = RB_HTML_RelativeAddress( dest_name,
                                                 cur_filename->link->
                                                 file_name );
                    fprintf( dest_doc, "<li>\n" );
                    fprintf( dest_doc, "<a href=\"%s#%s\"><tt>\n", r,
                             cur_filename->link->label_name );
                    RB_HTML_Generate_String( dest_doc, cur_filename->name );
                    fprintf( dest_doc, "</tt></a></li>\n" );
                }
            }
        }
    }
    for ( cur_path = srctree->first_path;
          cur_path; cur_path = cur_path->next )
    {
        if ( cur_path->parent == parent_path )
        {
            fprintf( dest_doc, "<li>\n" );
            RB_HTML_Generate_String( dest_doc, cur_path->name );
            RB_HTML_Generate_Source_Tree_Entry( dest_doc, dest_name, cur_path,
                                                srctree, document );
            fprintf( dest_doc, "</li>\n" );
        }
    }
    fprintf( dest_doc, "</ul>\n" );
}

/* TODO  Documentation */
void RB_HTML_Generate_Source_Tree(
    FILE *dest_doc,
    char *dest_name,
    struct RB_Document *document )
{
    struct RB_Directory *srctree;

    srctree = document->srctree;
    RB_HTML_Generate_Source_Tree_Entry( dest_doc, dest_name, NULL, srctree,
                                        document );
}


/****if* HTML_Generator/RB_HTML_Generate_String
 * FUNCTION
 *   Write a string to the destination document, escaping
 *   characters where necessary.
 * SYNOPSIS
 */
static void RB_HTML_Generate_String(
    FILE *dest_doc,
    const char *a_string )
/*
 * INPUTS
 *   o dest_doc -- the file the characters are written too
 *   o a_string -- a nul terminated string.
 * SEE ALSO
 *   RB_HTML_Generate_Char()
 * SOURCE
 */
{
    int                 i;
    int                 l = strlen( a_string );
    unsigned char       c;

    for ( i = 0; i < l; ++i )
    {
        c = a_string[i];
        RB_HTML_Generate_Char( dest_doc, c );
    }
}

/*******/


/****if* HTML_Generator/RB_HTML_Generate_False_Link
 * FUNCTION
 *   Create a representation for a link that links an word in
 *   a header to the header itself.
 * SYNOPSIS
 */
void RB_HTML_Generate_False_Link(
    FILE *dest_doc,
    char *name )
/*
 * INPUTS
 *   * dest_doc -- the file the representation is written to.
 *   * name     -- the word.
 * SOURCE
 */
{
    fprintf( dest_doc, "<strong>" );
    RB_HTML_Generate_String( dest_doc, name );
    fprintf( dest_doc, "</strong>" );

}

/*******/


/****f* HTML_Generator/RB_HTML_Color_String
 * FUNCTION
 *   Generates various colored strings
 * SOURCE
 */

static void RB_HTML_Color_String(
    FILE *dest_doc,
    int open,
    const char *class,
    const char *string )
{
    switch ( open )
    {
        /* string, closing */
    case 0:
        RB_HTML_Generate_String( dest_doc, string );
        fprintf( dest_doc, "</span>" );
        break;

        /*  opening, string */
    case 1:
        fprintf( dest_doc, "<span class=\"%s\">", class );
        RB_HTML_Generate_String( dest_doc, string );
        break;

        /*  opening, string, closing */
    case 2:
        fprintf( dest_doc, "<span class=\"%s\">", class );
        RB_HTML_Generate_String( dest_doc, string );
        fprintf( dest_doc, "</span>" );
        break;

        /*  opening, char, closing */
    case 3:
        fprintf( dest_doc, "<span class=\"%s\">", class );
        RB_HTML_Generate_Char( dest_doc, *string );
        fprintf( dest_doc, "</span>" );
        break;

        /*  Bug */
    default:
        assert( 0 );
    }
}

/*******/


/****f* HTML_Generator/RB_HTML_Generate_Line_Comment_End
 * FUNCTION
 *   Check if a line comment is active and generate ending sequence for it.
 *   Should be called at the end of each SOURCE line.
 * SYNOPSIS
 */
void RB_HTML_Generate_Line_Comment_End(
    FILE *dest_doc )
{
    /*  Check if we are in a line comment */
    if ( in_linecomment )
    {
        /*  and end the line comment */
        in_linecomment = 0;
        RB_HTML_Color_String( dest_doc, in_linecomment, COMMENT_CLASS, "" );
    }
}

/*******/


/****f* HTML_Generator/RB_HTML_Generate_Extra
 * FUNCTION
 *   Do some additional processing to detect HTML extra's like
 *   file references and other kind of links for the documentation
 *   body of an item.
 * SYNOPSIS
 */
int RB_HTML_Generate_Extra(
    FILE *dest_doc,
    enum ItemType item_type,
    char *cur_char,
    char prev_char )
/*
 * INPUTS
 *   o dest_doc  -- the file to write to.
 *   o item_type -- the kind of item the body belongs to.
 *   o cur_char  -- pointer to a substring of the item's body
 *   o prev_char -- the character just before cur char (zero if none)
 * RESULTS
 *   Number of characters produced.
 * SOURCE
 */
{
    char                link[1024], *str;
    int                 res = -1;
    unsigned int        i;
    static int          incomment = 0;  /* are we in comment? */
    static int          quote = 0;      /* double quote */
    static int          squote = 0;     /* single quote */

    /*  Reset comment and quote state machine if not source item */
    if ( !Works_Like_SourceItem( item_type ) )
    {
        quote = 0;
        squote = 0;
        incomment = 0;
        in_linecomment = 0;
    }
    /*  else check for quotations and string literals */
    else if ( !( incomment || in_linecomment ) )
    {
        switch ( *cur_char )
        {
            /*  Check for quoted string literals ("string") */
        case '\"':
            if ( !squote && course_of_action.do_quotes )
            {
                if ( prev_char != '\\' )
                {
                    quote = !quote;
                    RB_HTML_Color_String( dest_doc, quote,
                                          QUOTE_CLASS, "\"" );
                    return 0;
                }
                else if ( quote && *( ( char * ) ( cur_char - 2 ) ) == '\\' )
                {
                    quote = !quote;     /* case "... \\" */
                    RB_HTML_Color_String( dest_doc, quote,
                                          QUOTE_CLASS, "\"" );
                    return 0;
                }
            }
            break;

            /*  Check for single quoted string literals ('string') */
        case '\'':
            if ( !quote && course_of_action.do_squotes )
            {
                if ( prev_char != '\\' )
                {
                    squote = !squote;
                    RB_HTML_Color_String( dest_doc, squote,
                                          SQUOTE_CLASS, "\'" );
                    return 0;
                }
                else if ( squote && *( ( char * ) ( cur_char - 2 ) ) == '\\' )
                {
                    squote = !squote;   /* case '\\' */
                    RB_HTML_Color_String( dest_doc, squote,
                                          SQUOTE_CLASS, "\'" );
                    return 0;
                }
            }
            break;

        default:
            break;
        }
    }

    /*  Recognise line comments */
    if ( Works_Like_SourceItem( item_type ) && !incomment && !quote
         && !squote && course_of_action.do_line_comments )
    {
        /*  check for line comment start */
        if ( !in_linecomment )
        {
            str =
                Find_Parameter_Partial( &
                                        ( configuration.
                                          source_line_comments ), cur_char );
            if ( str )
            {
                in_linecomment = 1;
                RB_HTML_Color_String( dest_doc, in_linecomment,
                                      COMMENT_CLASS, str );
                /*   We found it, so exit */
                return strlen( str ) - 1;
            }
        }
        /*  The end of line comments are generated in */
        /*  RB_HTML_Generate_Line_Comment_End() */
    }

    /*  Recognise block comments */
    if ( Works_Like_SourceItem( item_type ) && !in_linecomment && !quote
         && !squote && course_of_action.do_block_comments )
    {
        /*  Check for block comment start */
        if ( !incomment )
        {
            str =
                Find_Parameter_Partial( &
                                        ( configuration.
                                          remark_begin_markers ), cur_char );
            if ( str )
            {
                incomment = 1;
                RB_HTML_Color_String( dest_doc, incomment,
                                      COMMENT_CLASS, str );
                /*   We found it, so exit */
                return strlen( str ) - 1;
            }
        }
        /*  Check for block comment end */
        else
        {
            str =
                Find_Parameter_Partial( &( configuration.remark_end_markers ),
                                        cur_char );
            if ( str )
            {
                incomment = 0;
                RB_HTML_Color_String( dest_doc, incomment,
                                      COMMENT_CLASS, str );
                /*   We found it, so exit */
                return strlen( str ) - 1;
            }
        }
    }

    /*  Do further source formating */
    if ( Works_Like_SourceItem( item_type ) &&
         !in_linecomment && !incomment && !quote && !squote )
    {
        /*  Check for keywords */
        if ( configuration.keywords.number && course_of_action.do_keywords )
        {
            char               *keyword;

            /*  Check if we are at the beginning of a word */
            if ( !utf8_isalnum( prev_char ) && ( prev_char != '_' ) )
            {
                /*  Count word length */
                for ( i = 1;    /*  A word should have at least one character... */
                      utf8_isalnum( cur_char[i] ) || ( cur_char[i] == '_' );
                      i++ );
                /*  Check if it is a keyword */
                if ( ( keyword = Find_Keyword( cur_char, i ) ) )
                {
                    RB_HTML_Color_String( dest_doc, 2, KEYWORD_CLASS,
                                          keyword );
                    /*  Exit function */
                    return i - 1;
                }
            }
        }

        /*  Do some fancy coloration for non-alphanumeric chars */
        if ( !utf8_isalnum( *cur_char ) && *cur_char != '_'
             && *cur_char != ' ' && course_of_action.do_non_alpha )
        {
            RB_HTML_Color_String( dest_doc, 3, SIGN_CLASS, cur_char );
            return 0;
        }
    }

    /*  Check for links, etc... */
    if ( incomment || in_linecomment || !Works_Like_SourceItem( item_type ) )
    {
        if ( strncmp( "http://", cur_char, 7 ) == 0 )
        {
            sscanf( cur_char, "%s", link );
            RB_Say( "found link %s\n", SAY_DEBUG, link );
            res = ( strlen( link ) - 1 );
            /* [ 697247 ] http://body. does not skip the '.' */
            if ( link[( strlen( link ) - 1 )] == '.' )
            {
                link[( strlen( link ) - 1 )] = '\0';
                fprintf( dest_doc, "<a href=\"%s\">%s</a>.", link, link );
            }
            else
            {
                fprintf( dest_doc, "<a href=\"%s\">%s</a>", link, link );
            }
        }
        else if ( strncmp( "href:", cur_char, 5 ) == 0 )
        {
            /*
             * handy in relative hyperlink paths, e.g.
             * href:../../modulex/
             */
            sscanf( ( cur_char + 5 ), "%s", link );
            RB_Say( "found link %s\n", SAY_DEBUG, link );
            res = ( strlen( link ) + 4 );
            fprintf( dest_doc, "<a href=\"%s\">%s</a>", link, link );
        }
        else if ( strncmp( "file:/", cur_char, strlen( "file:/" ) ) == 0 )
        {
            sscanf( cur_char, "%s", link );
            RB_Say( "found link %s\n", SAY_DEBUG, link );
            res = ( strlen( link ) - 1 );
            fprintf( dest_doc, "<a href=\"%s\">%s</a>", link, link );
        }
        else if ( strncmp( "mailto:", cur_char, 7 ) == 0 )
        {
            sscanf( ( cur_char + 7 ), "%s", link );
            RB_Say( "found mail to %s\n", SAY_DEBUG, link );
            res = ( strlen( link ) + 6 );
            fprintf( dest_doc, "<a href=\"mailto:%s\">%s</a>", link, link );
        }
        else if ( strncmp( "image:", cur_char, 6 ) == 0 )
        {
            sscanf( ( cur_char + 6 ), "%s", link );
            RB_Say( "found image %s\n", SAY_DEBUG, link );
            res = ( strlen( link ) + 5 );
            fprintf( dest_doc, "<img src=\"%s\">", link );
        }

    }
    return res;
}

/******/



void RB_HTML_Generate_Item_Name(
    FILE *dest_doc,
    char *name )
{
    fprintf( dest_doc, "<p class=\"item_name\">" );
    RB_HTML_Generate_String( dest_doc, name );
    fprintf( dest_doc, "</p>\n" );
}

void RB_HTML_Generate_Item_Begin(
    FILE *dest_doc,
    char *name )
{
    USE( dest_doc );
    USE( name );
    /* empty */
}

void RB_HTML_Generate_Item_End(
    FILE *dest_doc,
    char *name )
{
    USE( dest_doc );
    USE( name );
    /* empty */
}


int                 sectiontoc_counters[MAX_SECTION_DEPTH];

/****f* HTML_Generator/RB_HTML_Generate_TOC_Section
 * FUNCTION
 *   Create a table of contents based on the hierarchy of
 *   the headers starting for a particular point in this
 *   hierarchy (the parent).
 * SYNOPSIS
 */
void RB_HTML_Generate_TOC_Section(
    FILE *dest_doc,
    char *dest_name,
    struct RB_header *parent,
    struct RB_header **headers,
    int count,
    int depth )
/*
 * INPUTS
 *   o dest_doc  -- the file to write to.
 *   o dest_name -- the name of this file.
 *   o parent    -- the parent of the headers for which the the
 *                  current level(depth) of TOC is created.
 *   o headers   -- an array of headers for which the TOC is created
 *   o count     -- the number of headers in this array
 *   o depth     -- the current depth of the TOC
 * NOTES
 *   This is a recursive function and tricky stuff.
 * SOURCE
 */
{
    struct RB_header   *header;
    int                 i, n, once = 0;

    ++sectiontoc_counters[depth];

    for ( i = depth + 1; i < MAX_SECTION_DEPTH; ++i )
    {
        sectiontoc_counters[i] = 0;
    }

    /*  List item start */
    fprintf( dest_doc, "<li>" );

    /*  Do not generate section numbers if sectionnameonly */
    if ( !( course_of_action.do_sectionnameonly ) )
    {
        for ( i = 1; i <= depth; ++i )
        {
            fprintf( dest_doc, "%d.", sectiontoc_counters[i] );
        }
        fprintf( dest_doc, " " );
    }


    /*  Generate Link to first reference name */
    RB_HTML_Generate_Link( dest_doc, dest_name, parent->file_name,
                           parent->unique_name,
                           /*  only generate function name if sectionnameonly */
                           ( course_of_action.do_sectionnameonly ) ?
                           parent->function_name : parent->name, 0 );

    /*  Generate links to further reference names */
    for ( n = 1; n < parent->no_names; n++ )
    {
        RB_HTML_Generate_String( dest_doc, ", " );
        RB_HTML_Generate_Link( dest_doc, dest_name, parent->file_name,
                               parent->unique_name, parent->names[n], 0 );
    }

    /*  List item end */
    fprintf( dest_doc, "</li>\n" );

    for ( i = 0; i < count; ++i )
    {
        header = headers[i];
        if ( header->parent == parent )
        {
            /*  Generate better TOC level hiearchy (Thuffir) */
            /*  We only generate <ul> once for a level */
            if ( !once )
            {
                once = 1;
                fprintf( dest_doc, "<ul>\n" );
            }
            RB_HTML_Generate_TOC_Section( dest_doc, dest_name, header,
                                          headers, count, depth + 1 );
        }
        else
        {
            /* Empty */
        }
    }
    /*  If we have generated an <ul> before, generate the closing one too. */
    if ( once )
        fprintf( dest_doc, "</ul>\n" );
}

/*******/


void RB_HTML_Generate_TOC_2(
    FILE *dest_doc,
    struct RB_header **headers,
    int count,
    struct RB_Part *owner,
    char *dest_name )
{
    struct RB_header   *header;
    int                 i, j;
    int                 depth = 1;

    for ( i = 0; i < MAX_SECTION_DEPTH; ++i )
    {
        sectiontoc_counters[i] = 0;
    }
    fprintf( dest_doc, "<h3>TABLE OF CONTENTS</h3>\n" );
    if ( course_of_action.do_sections )
    {
        /* --sections was specified, create a TOC based on the
         * hierarchy of the headers.
         */
        fprintf( dest_doc, "<ul>\n" );
        for ( i = 0; i < count; ++i )
        {
            header = headers[i];
            if ( owner == NULL )
            {
                if ( header->parent )
                {
                    /* Will be done in the subfunction */
                }
                else
                {
                    RB_HTML_Generate_TOC_Section( dest_doc, dest_name, header,
                                                  headers, count, depth );
                }
            }
            else
            {
                /* This is the TOC for a specific RB_Part (MultiDoc
                 * documentation). We only include the headers that
                 * are part of the subtree. That is, headers that are
                 * parth the RB_Part, or that are childern of the
                 * headers in the RB_Part.
                 */
                if ( header->owner == owner )
                {
                    /* BUG 721690 */
                    /* Any of the parents of this header should not
                     * have the same owner as this header, otherwise
                     * this header will be part of the TOC multiple times.
                     */
                    int                 no_bad_parent = TRUE;
                    struct RB_header   *parent = header->parent;

                    for ( ; parent; parent = parent->parent )
                    {
                        if ( parent->owner == owner )
                        {
                            no_bad_parent = FALSE;
                            break;
                        }
                    }
                    if ( no_bad_parent )
                    {
                        RB_HTML_Generate_TOC_Section( dest_doc, dest_name,
                                                      header, headers, count,
                                                      depth );
                    }
                }
            }
        }
        fprintf( dest_doc, "</ul>\n" );
    }
    else
    {
        /* No --section option, generate a plain, one-level
         * TOC
         */
        fprintf( dest_doc, "<ul>\n" );

        for ( i = 0; i < count; ++i )
        {
            header = headers[i];
            if ( header->name && header->function_name &&
                 ( ( owner == NULL ) || ( header->owner == owner ) ) )
            {
                for ( j = 0; j < header->no_names; j++ )
                {
                    fprintf( dest_doc, "<li>" );

                    RB_HTML_Generate_Link( dest_doc, dest_name,
                                           header->file_name,
                                           header->unique_name,
                                           header->names[j], 0 );
                    fprintf( dest_doc, "</li>\n" );
                }
            }
        }
        fprintf( dest_doc, "</ul>\n" );
    }
}



/****f* HTML_Generator/RB_HTML_Generate_Label
 * FUNCTION
 *   Generate a label (name) that can be refered too.
 *   A label should consist of only alphanumeric characters so
 *   all 'odd' characters are replaced with their ASCII code in
 *   hex format.
 * SYNOPSIS
 */
void RB_HTML_Generate_Label(
    FILE *dest_doc,
    char *name )
/*
 * INPUTS
 *   o dest_doc -- the file to write it to.
 *   o name     -- the name of the label.
 * SOURCE
 */
{
    int                 i;
    int                 l = strlen( name );
    unsigned char       c;

    fprintf( dest_doc, "<a name=\"" );
    for ( i = 0; i < l; ++i )
    {
        c = name[i];
        if ( utf8_isalnum( c ) )
        {
            RB_HTML_Generate_Char( dest_doc, c );
        }
        else
        {
            char                buf[4];

            sprintf( buf, "%02x", c );
            RB_HTML_Generate_Char( dest_doc, buf[0] );
            RB_HTML_Generate_Char( dest_doc, buf[1] );
        }
    }
    fprintf( dest_doc, "\"></a>\n" );
}

/********/



static int          section_counters[MAX_SECTION_DEPTH];


/* TODO Documentation */

void RB_HTML_Generate_BeginSection(
    FILE *dest_doc,
    int depth,
    char *name,
    struct RB_header *header )
{
    int                 i;

    ++section_counters[depth];
    for ( i = depth + 1; i < MAX_SECTION_DEPTH; ++i )
    {
        section_counters[i] = 0;
    }
    switch ( depth )
    {
    case 1:
    case 2:
    case 3:
    case 4:
    case 5:
    case 6:
    case 7:
        fprintf( dest_doc, "<h%d>", depth );
        /*  Only generate section numbers if no sectionnameonly */
        if ( !( course_of_action.do_sectionnameonly ) )
        {
            for ( i = 1; i <= depth; ++i )
            {
                fprintf( dest_doc, "%d.", section_counters[i] );
            }
            fprintf( dest_doc, "  " );
        }

        /*  Print Header "first" name */
        RB_HTML_Generate_String( dest_doc, name );

        /*  Print further names */
        for ( i = 1; i < header->no_names; i++ )
        {
            fprintf( dest_doc, ( i % header_breaks ) ? ", " : ",<br />" );
            RB_HTML_Generate_String( dest_doc, header->names[i] );
        }

        /*  Include module name if not sectionnameonly */
        if ( !( course_of_action.do_sectionnameonly ) )
        {
            fprintf( dest_doc, " [ " );
            RB_HTML_Generate_String( dest_doc, header->htype->indexName );
            fprintf( dest_doc, " ]" );
        }

        fprintf( dest_doc, "  </h%d>\n", depth );
        break;
    default:
        /* too deep, don't do anything. */
        assert( 0 );
    }
}

void RB_HTML_Generate_EndSection(
    FILE *dest_doc,
    int depth,
    char *name )
{
    USE( dest_doc );
    USE( name );
    USE( depth );
    /* Empty */
}


char               *RB_HTML_Get_Default_Extension(
    void )
{
    return ( ".html" );
}

/****f* HTML_Generator/RB_HTML_Generate_Doc_Start
 * NAME
 *   RB_HTML_Generate_Doc_Start --
 * FUNCTION
 *   Generate the first part of a HTML document.
 *   As far as ROBODoc is concerned a HTML document
 *   consists of three parts:
 *   * The start of a document
 *   * The body of a document
 *   * The end of a document
 * SYNOPSIS
 */
void RB_HTML_Generate_Doc_Start(
    FILE *dest_doc,
    char *src_name,
    char *name,
    char *dest_name,
    char *charset )
/*
 * INPUTS
 *   o dest_doc  --  the output file.
 *   o src_name  --  The file or directoryname from which 
 *                   this document is generated.
 *   o name      --  The title for this document
 *   o dest_name --  the name of the output file.
 *   o charset   --  the charset to be used for the file.
 * SOURCE
 */
{

    if ( course_of_action.do_headless )
    {
        /* The user wants a headless document, so we skip everything
         * upto and until <BODY>
         */
    }
    else
    {
        /* Append document type and title */
        fprintf( dest_doc, "<?xml version=\"1.0\" encoding=\"%s\"?>\n",
                 charset ? charset : DEFAULT_CHARSET );
        fprintf( dest_doc,
                 "<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Strict//EN\"\n" );
        fprintf( dest_doc,
                 "                      \"http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd\">\n" );

        fprintf( dest_doc,
                 "<html  xmlns=\"http://www.w3.org/1999/xhtml\" xml:lang=\"en\" lang=\"en\">\n" );
        fprintf( dest_doc, "<head>\n" );
        fprintf( dest_doc,
                 "<meta http-equiv=\"Content-Style-Type\" content=\"text/css\" />\n" );
        /* TODO is charset still needed?? */
        fprintf( dest_doc,
                 "<meta http-equiv=\"Content-type\" content=\"text/html; charset=%s\" />\n",
                 charset ? charset : DEFAULT_CHARSET );
        RB_InsertCSS( dest_doc, dest_name );
        fprintf( dest_doc, "<title>%s</title>\n", name );

        /* append SGML-comment with document- and copyright-info. This code
         * ensures that every line has an own comment to avoid problems with
         * buggy browsers */
        fprintf( dest_doc, "<!-- Source: %s -->\n", src_name );
        if ( course_of_action.do_nogenwith )
        {

        }
        else
        {
            static const char   copyright_text[]
                = COMMENT_ROBODOC /* COMMENT_COPYRIGHT */ ;
            size_t              i = 0;
            char                previous_char = '\n';
            char                current_char = copyright_text[i];

            while ( current_char )
            {
                if ( previous_char == '\n' )
                {
                    fprintf( dest_doc, "<!-- " );
                }
                if ( current_char == '\n' )
                {
                    fprintf( dest_doc, " -->" );
                }
                else if ( ( current_char == '-' )
                          && ( previous_char == '-' ) )
                {
                    /* avoid "--" inside SGML-comment, and use "-_" instead; this
                     * looks a bit strange, but one should still be able to figure
                     * out what is meant when reading the output */
                    current_char = '_';
                }
                fputc( current_char, dest_doc );
                i += 1;
                previous_char = current_char;
                current_char = copyright_text[i];
            }
        }

        /* append heading and start list of links to functions */
        fprintf( dest_doc, "</head>\n" );
        fprintf( dest_doc, "<body>\n" );
    }

/*     HTML_Generate_Div( dest_doc, "container" ); */

    /* Generate document title if available (Thuffir) */
    HTML_Generate_Div( dest_doc, "logo" );
    fprintf( dest_doc, "<a name=\"robo_top_of_doc\">" );
    if ( document_title )
        RB_HTML_Generate_String( dest_doc, document_title );
    fprintf( dest_doc, "</a>\n" );
    HTML_Generate_Div_End( dest_doc, "logo" );



}

/******/


/* TODO */
/*x**if* HTML_Generator/RB_HTML_Generate_Doc_End
 * NAME
 *   RB_HTML_Generate_Doc_End --
 * FUNCTION
 *   Close of the document with the proper end tags.
 ******
 */

void RB_HTML_Generate_Doc_End(
    FILE *dest_doc,
    char *name,
    char *src_name )
{

    USE( name );


    HTML_Generate_Div( dest_doc, "footer" );
    /* TODO This should be done with
     * RB_Generate_Label() 
     */
    if ( course_of_action.do_nogenwith )
    {
        fprintf( dest_doc, "<p>Generated from %s on ", src_name );
        RB_TimeStamp( dest_doc );
        fprintf( dest_doc, "</p>\n" );
    }
    else
    {
        fprintf( dest_doc,
                 "<p>Generated from %s with <a href=\"http://www.xs4all.nl/~rfsber/Robo/robodoc.html\">ROBODoc</a> V%s on ",
                 src_name, VERSION );
        RB_TimeStamp( dest_doc );
        fprintf( dest_doc, "</p>\n" );
    }
    HTML_Generate_Div_End( dest_doc, "footer" );

/*     HTML_Generate_Div_End( dest_doc, "container" ); */

    if ( course_of_action.do_footless )
    {
        /* The user does not want the foot of the
         * document.
         */
    }
    else
    {
        fprintf( dest_doc, "</body>\n</html>\n" );
    }
}


void RB_HTML_Generate_Nav_Bar(
    struct RB_Document *document,
    FILE *current_doc,
    struct RB_header *current_header )
{
    char               *current_filename = NULL;
    char               *target_filename = NULL;
    char               *label = NULL;
    char               *label_name = NULL;

    current_filename = RB_Get_FullDocname( current_header->owner->filename );
    target_filename = RB_Get_FullDocname( current_header->owner->filename );
    label = Get_Fullname( current_header->owner->filename );
    /* The navigation bar */
    fprintf( current_doc, "<p>" );

    /*  [ Top ] */
    fprintf( current_doc, "[ " );
    RB_HTML_Generate_Link( current_doc, current_filename, NULL,
                           "robo_top_of_doc", "Top", 0 );
    fprintf( current_doc, " ] " );

    /*  [ "Parentname" ] */
    if ( current_header->parent )
    {
        fprintf( current_doc, "[ " );
        target_filename =
            RB_Get_FullDocname( current_header->parent->owner->filename );
        label = current_header->parent->unique_name;
        label_name = current_header->parent->function_name;
        RB_HTML_Generate_Link( current_doc, current_filename, target_filename,
                               label, label_name, 0 );
        fprintf( current_doc, " ] " );
    }

    /*  [ "Modulename" ] */
    fprintf( current_doc, "[ " );
    label_name = current_header->htype->indexName;
    if ( ( course_of_action.do_index ) && ( course_of_action.do_multidoc ) )
    {
        target_filename = RB_Get_SubIndex_FileName( document->docroot->name,
                                                    document->extension,
                                                    current_header->htype );
        RB_HTML_Generate_Link( current_doc, current_filename, target_filename,
                               "robo_top_of_doc", label_name, 0 );
        free( target_filename );
    }
    else
    {
        RB_HTML_Generate_String( current_doc, label_name );
    }
    fprintf( current_doc, " ]</p>\n" );
}



/* TODO Documentation */

void RB_HTML_Generate_Nav_Bar_One_File_Per_Header(
    struct RB_Document *document,
    FILE *current_doc,
    struct RB_header *current_header )
{
    char               *current_filename = NULL;
    char               *target_filename = NULL;
    char               *label = NULL;
    char               *label_name = NULL;

    current_filename = RB_Get_FullDocname( current_header->owner->filename );
    target_filename = RB_Get_FullDocname( current_header->owner->filename );
    label = Get_Fullname( current_header->owner->filename );
    /* The navigation bar */
    if ( current_header->parent )
    {
        target_filename =
            RB_Get_FullDocname( current_header->parent->owner->filename );
        label = current_header->parent->unique_name;
        label_name = current_header->parent->function_name;
        RB_HTML_Generate_Link( current_doc, current_filename, target_filename,
                               label, label_name, "menuitem" );
    }
    /* FS TODO  one_file_per_header without   index is not logical */
    if ( ( course_of_action.do_index ) && ( course_of_action.do_multidoc ) )
    {
        target_filename = RB_Get_SubIndex_FileName( document->docroot->name,
                                                    document->extension,
                                                    current_header->htype );
        label_name = current_header->htype->indexName;
        RB_HTML_Generate_Link( current_doc, current_filename, target_filename,
                               "robo_top_of_doc", label_name, "menuitem" );
        free( target_filename );
    }
}





/* TODO Documentation */
/*x**if* HTML_Generator/RB_HTML_Generate_Header_Start
 * NAME
 *   RB_HTML_Generate_Header_Start --
 ******
 */

void RB_HTML_Generate_Header_Start(
    FILE *dest_doc,
    struct RB_header *cur_header )
{
    struct RB_HeaderType *header_type;
    int                 i;

    if ( cur_header->name && cur_header->function_name )
    {
        fprintf( dest_doc, "<hr />\n" );
        RB_HTML_Generate_Label( dest_doc, cur_header->name );
        fprintf( dest_doc, "<a name=\"%s\"></a><h2>",
                 cur_header->unique_name );

        header_type = RB_FindHeaderType( cur_header->htype->typeCharacter );

        for ( i = 1; i <= cur_header->no_names; i++ )
        {
            /*  If Section names only, do not print module name */
            if ( i == 1 && ( course_of_action.do_sectionnameonly ) )
                RB_HTML_Generate_String( dest_doc,
                                         cur_header->function_name );
            else
                RB_HTML_Generate_String( dest_doc, cur_header->names[i - 1] );

            /*  Break lines after a predefined number of header names */
            if ( i < cur_header->no_names )
                fprintf( dest_doc, ( i % header_breaks ) ? ", " : ",<br />" );
        }

        /*  Print header type (if available and not Section names only) */
        if ( header_type && !( course_of_action.do_sectionnameonly ) )
        {
            fprintf( dest_doc, " [ " );
            RB_HTML_Generate_String( dest_doc, header_type->indexName );
            fprintf( dest_doc, " ]" );
        }

        fprintf( dest_doc, "</h2>\n\n" );
    }
}

/* TODO */
/*x**f* HTML_Generator/RB_HTML_Generate_Header_End
 * NAME
 *   RB_HTML_Generate_Header_End --
 ******
 */

void RB_HTML_Generate_Header_End(
    FILE *dest_doc,
    struct RB_header *cur_header )
{
    USE( cur_header );
    fprintf( dest_doc, "\n" );
}


/****f* HTML_Generator/RB_HTML_Generate_IndexMenu
 * FUNCTION
 *   Generates a menu to jump to the various master index files for
 *   the various header types.  The menu is generated for each of the
 *   master index files.  The current header type is highlighted.
 * SYNOPSIS
 */
void RB_HTML_Generate_IndexMenu(
    FILE *dest_doc,
    char *filename,
    struct RB_Document *document,
    struct RB_HeaderType *cur_type )
    /* TODO  Use cur_type */
/*
 * INPUTS
 *   * dest_doc       -- the output file.
 *   * filename       -- the name of the output file
 *   * document       -- the gathered documention.
 *   * cur_headertype -- the header type that is to be highlighted.
 ******
 */
{
    unsigned char       type_char;
    char               *toc_index_path = NULL;

    USE( cur_type );            /* TODO FS make use of this */
    assert( dest_doc );
    assert( filename );
    assert( document );

    toc_index_path = HTML_TOC_Index_Filename( document );
    RB_HTML_Generate_Link( dest_doc,
                           filename,
                           toc_index_path,
                           "top", "Table of Contents", "menuitem" );
    free( toc_index_path );
    fprintf( dest_doc, "\n" );

    for ( type_char = MIN_HEADER_TYPE;
          type_char < MAX_HEADER_TYPE; ++type_char )
    {
        struct RB_HeaderType *header_type;
        int                 n;

        header_type = RB_FindHeaderType( type_char );
        if ( header_type )
        {
            n = RB_Number_Of_Links( header_type, NULL, FALSE ) +
                RB_Number_Of_Links( header_type, NULL, TRUE );

            if ( n )
            {
                char               *targetfilename = 0;

                targetfilename =
                    RB_Get_SubIndex_FileName( document->docroot->name,
                                              document->extension,
                                              header_type );
                assert( targetfilename );

                RB_HTML_Generate_Link( dest_doc,
                                       filename,
                                       targetfilename,
                                       "top",
                                       header_type->indexName, "menuitem" );
                free( targetfilename );
                fprintf( dest_doc, "\n" );
            }
        }
    }
}

/****f* HTML_Generator/RB_HTML_Generate_Index_Page
 * FUNCTION
 *   Generate a single file with a index table for headers
 *   of one specific type of headers
 * SYNOPSIS
 */
void RB_HTML_Generate_Index_Page(
    struct RB_Document *document,
    struct RB_HeaderType *header_type )
/*
 * INPUTS
 *   o document    -- the document
 *   o header_type -- the type for which the table is to
 *                    be generated.
 ******
 */
{
    char               *filename = 0;
    FILE               *file;

    assert( document );
    assert( header_type );

    filename = RB_Get_SubIndex_FileName( document->docroot->name,
                                         document->extension, header_type );
    assert( filename );

    file = fopen( filename, "w" );
    if ( !file )
    {
        RB_Panic( "can't open (%s)!\n", filename );
    }
    else
    {
        /* File opened, now we generate an index
         * for the specified header type 
         */
        RB_HTML_Generate_Doc_Start( file,
                                    document->srcroot->name,
                                    header_type->indexName,
                                    filename, document->charset );

        /* breadcrumbtrail */
        HTML_Generate_Begin_Extra( file );
        /* No content for extra section yet... */
        HTML_Generate_End_Extra( file );

        /* Menu for navigation */
        HTML_Generate_Begin_Navigation( file );
        RB_HTML_Generate_IndexMenu( file, filename, document, header_type );
        HTML_Generate_End_Navigation( file );

        /* Content */
        HTML_Generate_Begin_Content( file );
        if ( RB_CompareHeaderTypes
             ( header_type, RB_FindHeaderType( HT_SOURCEHEADERTYPE ) )
             && ( header_type->typeCharacter != HT_MASTERINDEXTYPE ) )
        {
            RB_HTML_Generate_Source_Tree( file, filename, document );
        }
        else
        {
            RB_HTML_Generate_Index_Table( file,
                                          filename,
                                          header_type,
                                          header_type->indexName );
        }
        HTML_Generate_End_Content( file );
        RB_HTML_Generate_Doc_End( file, filename, document->srcroot->name );
        fclose( file );
    }

    free( filename );
}


/* Create an index page that contains only the table of content */

void HTML_Generate_TOC_Index_Page(
    struct RB_Document *document )
{
    FILE               *file = NULL;
    char               *toc_index_path = HTML_TOC_Index_Filename( document );

    file = fopen( toc_index_path, "w" );
    if ( !file )
    {
        RB_Panic( "can't open (%s)!\n", toc_index_path );
    }
    else
    {
        RB_HTML_Generate_Doc_Start( file, document->srcroot->name,
                                    "Table of Contents",
                                    toc_index_path, document->charset );

        /* breadcrumbtrail */
        HTML_Generate_Begin_Extra( file );
        /* No content for extra section yet... */
        HTML_Generate_End_Extra( file );

        /* Menu for navigation */
        HTML_Generate_Begin_Navigation( file );
        RB_HTML_Generate_IndexMenu( file, toc_index_path, document, NULL );
        HTML_Generate_End_Navigation( file );

        /* Content */
        HTML_Generate_Begin_Content( file );
        RB_Generate_TOC_2( file, document->headers,
                           document->no_headers, NULL, toc_index_path );
        HTML_Generate_End_Content( file );

        /* End part */
        RB_HTML_Generate_Doc_End( file, toc_index_path,
                                  document->srcroot->name );
        fclose( file );
    }

    free( toc_index_path );
}





/* TODO */
/*x**if* HTML_Generator/RB_HTML_Generate_Index
 * NAME
 *   RB_HTML_Generate_Index --
 ******
 */


/* Should be called indexes */
void RB_HTML_Generate_Index(
    struct RB_Document *document )
{
    unsigned char       type_char = 0;

    assert( document );

    for ( type_char = MIN_HEADER_TYPE;
          type_char < MAX_HEADER_TYPE; ++type_char )
    {
        struct RB_HeaderType *header_type;
        int                 n;

        header_type = RB_FindHeaderType( type_char );
        if ( header_type )
        {
            n = RB_Number_Of_Links( header_type, NULL, FALSE ) +
                RB_Number_Of_Links( header_type, NULL, TRUE );
            if ( n )
            {
                /* There are headers of this type, so create an index page
                 * for them
                 */
                RB_HTML_Generate_Index_Page( document, header_type );
            }
        }
    }

    RB_HTML_Generate_Index_Page( document,
                                 RB_FindHeaderType( HT_MASTERINDEXTYPE ) );

    HTML_Generate_TOC_Index_Page( document );
}






void RB_HTML_Generate_Table_Body(
    FILE *dest,
    char *dest_name,
    struct RB_HeaderType *type,
    int internal )
{
    struct RB_link     *cur_link;
    unsigned int        i;
    char                first_char = ' ';
    int                 found = FALSE;

    /* Compute the number of columns we need for
     * this type of header.
     */
    for ( i = 0; i < link_index_size; ++i )
    {
        cur_link = link_index[i];
        if ( cur_link->htype &&
             RB_CompareHeaderTypes( cur_link->htype, type ) &&
             ( ( cur_link->is_internal && internal ) ||
               ( !cur_link->is_internal && !internal ) ) )
        {
            char               *r = 0;

            r = RB_HTML_RelativeAddress( dest_name, cur_link->file_name );
            if ( toupper( cur_link->object_name[0] ) != first_char )
            {
                first_char = toupper( cur_link->object_name[0] );
                if ( found )
                {
/*                     fprintf( dest, "</div>\n" ); */
                }
                fprintf( dest, "<h2><a name=\"%c\"></a>", first_char );
                RB_HTML_Generate_Char( dest, first_char );
                fprintf( dest, "</h2>" );
/*               fprintf( dest, "<div class=\"indexitem\">\n" ); */
                found = TRUE;
            }
            fprintf( dest, "<a href=\"%s#%s\" class=\"indexitem\" >", r,
                     cur_link->label_name );
            RB_HTML_Generate_String( dest, cur_link->object_name );
            fprintf( dest, "</a>\n" );
        }
    }
    if ( found )
    {
/*         fprintf( dest, "</div>\n" ); */
    }
}


/****if* HTML_Generator/RB_HTML_Generate_Index_Shortcuts
 * NAME
 *   RB_HTML_Generate_Index_Shortcuts
 * FUNCTION
 *   Generates alphabetic shortcuts to index entries.
 * SYNOPSIS
 */
static void RB_HTML_Generate_Index_Shortcuts(
    FILE *dest )
/*
 * INPUTS
 *   o dest        -- the file to write to
 * TODO
 *   - Only list used letters.
 *   - List all letters (accented, signs, etc), not just the common ones.
 *   - Should be better to implement it as a <div> ?
 * SOURCE
 */
{
    unsigned char       c;

    fprintf( dest, "<h2>" );

    /*  Generate A - Z */
    for ( c = 'A'; c <= 'Z'; c++ )
    {
        fprintf( dest, "<a href=\"#%c\">", c );
        RB_HTML_Generate_Char( dest, c );
        fprintf( dest, "</a> - " );
    }

    /*  Generate 0 - 9 */
    for ( c = '0'; c <= '9'; c++ )
    {
        fprintf( dest, "<a href=\"#%c\">", c );
        RB_HTML_Generate_Char( dest, c );
        fprintf( dest, "</a>" );

        /*  Do not generate separator for the last char */
        if ( c != '9' )
        {
            fprintf( dest, " - " );
        }
    }

    fprintf( dest, "</h2>\n" );
}

/********/

/****if* HTML_Generator/RB_HTML_Generate_Index_Table
 * NAME
 *   RB_HTML_Generate_Index_Table --
 * FUNCTION
 *   Create a HTML TABLE containing links to headers of a particular
 *   type.  This creates two tables, a table for normal headers as
 *   well as one for internal headers.
 * SYNOPSIS
 */
void RB_HTML_Generate_Index_Table(
    FILE *dest,
    char *dest_name,
    struct RB_HeaderType *type,
    char *title )
/*
 * INPUTS
 *   o dest        -- the file in which to write the table
 *   o dest_name   -- the name of this file
 *   o type        -- the type of header for which to generate
 *                    the table
 *   o title       -- the title of the table.
 * SOURCE
 */
{
    /* Compute the number of columns we need for
     * this type of header.
     */

    /*  Generate Index Title */
    fprintf( dest, "<h1>" );
    RB_HTML_Generate_String( dest, title );
    fprintf( dest, "</h1>\n" );

    /*  Generate Shortcuts at the begining */
    RB_HTML_Generate_Index_Shortcuts( dest );

    if ( RB_Number_Of_Links( type, NULL, FALSE ) )
    {
        if ( RB_Number_Of_Links( type, NULL, TRUE ) )
        {
            /* only print a title if there are two tables. */
            fprintf( dest, "<h2>Normal</h2>" );
        }
        RB_HTML_Generate_Table_Body( dest, dest_name, type, FALSE );
    }

    if ( RB_Number_Of_Links( type, NULL, TRUE ) )
    {
        /* Always print the  Internal title, since
         * these headers are special and the user should know
         * he is looking at something special.
         */
        fprintf( dest, "<h2>Internal</h2>" );
        RB_HTML_Generate_Table_Body( dest, dest_name, type, TRUE );
    }

    /*  Generate Shortcuts at the end */
    RB_HTML_Generate_Index_Shortcuts( dest );
}

/********/



/* TODO */
/*x**if* HTML_Generator/RB_HTML_Generate_Empty_Item
 * NAME
 *   RB_HTML_Generate_Empty_Item --
 ******
 */

void RB_HTML_Generate_Empty_Item(
    FILE *dest_doc )
{
    fprintf( dest_doc, "<br>\n" );
}




/****f* HTML_Generator/RB_HTML_Generate_Link
 * NAME
 *   RB_HTML_Generate_Link --
 * SYNOPSIS
 */
void RB_HTML_Generate_Link(
    FILE *cur_doc,
    char *cur_name,
    char *filename,
    char *labelname,
    char *linkname,
    char *classname )
/*
 * INPUTS
 *   cur_doc  --  the file to which the text is written
 *   cur_name --  the name of the destination file
 *                (the file from which we link)
 *   filename --  the name of the file that contains the link
 *                (the file we link to)
 *   labelname--  the name of the unique label of the link.
 *   linkname --  the name of the link as shown to the user.
 * SOURCE
 */
{
    if ( classname )
    {
        fprintf( cur_doc, "<a class=\"%s\" ", classname );
    }
    else
    {
        fprintf( cur_doc, "<a " );
    }
    if ( filename && strcmp( filename, cur_name ) )
    {
        char               *r = RB_HTML_RelativeAddress( cur_name, filename );

        fprintf( cur_doc, "href=\"%s#%s\">", r, labelname );
        RB_HTML_Generate_String( cur_doc, linkname );
        fprintf( cur_doc, "</a>" );

    }
    else
    {
        fprintf( cur_doc, "href=\"#%s\">", labelname );
        RB_HTML_Generate_String( cur_doc, linkname );
        fprintf( cur_doc, "</a>" );
    }
}

/******/


/****f* HTML_Generator/RB_HTML_RelativeAddress
 * FUNCTION
 *   Link to 'that' from 'this' computing the relative path.  Here
 *   'this' and 'that' are both paths.  This function is used to
 *   create links from one document to another document that might be
 *   in a completely different directory.
 * SYNOPSIS
 */
char               *RB_HTML_RelativeAddress(
    char *thisname,
    char *thatname )
/*
 * EXAMPLE
 *   The following two
 *     this /sub1/sub2/sub3/f.html
 *     that /sub1/sub2/g.html
 *   result in 
 *     ../g.html
 *
 *     this /sub1/f.html
 *     that /sub1/sub2/g.html
 *     ==
 *     ./sub2/g.html
 *
 *     this /sub1/f.html
 *     that /sub1/g.html
 *     ==
 *     ./g.html
 *
 *     this /sub1/doc3/doc1/tt.html
 *     that /sub1/doc5/doc2/qq.html
 *     ==
 *     ../../doc5/doc2/qq.html
 *
 * NOTES
 *   Notice the execelent docmentation.
 * SOURCE
 */
#define MAX_RELATIVE_SIZE 1024
{
    static char         relative[MAX_RELATIVE_SIZE + 1];
    char               *i_this;
    char               *i_that;
    char               *i_this_slash = NULL;
    char               *i_that_slash = NULL;

    relative[0] = '\0';

    assert( thisname );
    assert( thatname );

    for ( i_this = thisname, i_that = thatname;
          ( *i_this && *i_that ) && ( *i_this == *i_that );
          ++i_this, ++i_that )
    {
        if ( *i_this == '/' )
        {
            i_this_slash = i_this;
        }
        if ( *i_that == '/' )
        {
            i_that_slash = i_that;
        }
    }

    if ( i_this_slash && i_that_slash )
    {
        int                 this_slashes_left = 0;
        int                 that_slashes_left = 0;
        char               *i_c;

        for ( i_c = i_this_slash + 1; *i_c; ++i_c )
        {
            if ( *i_c == '/' )
            {
                ++this_slashes_left;
            }
        }

        for ( i_c = i_that_slash + 1; *i_c; ++i_c )
        {
            if ( *i_c == '/' )
            {
                ++that_slashes_left;
            }
        }

        if ( this_slashes_left )
        {
            int                 i;

            for ( i = 0; i < this_slashes_left; ++i )
            {
                strcat( relative, "../" );
            }
            strcat( relative, i_that_slash + 1 );
        }
        else if ( that_slashes_left )
        {
            /* !this_slashes_left && that_slashes_left */
            strcat( relative, "./" );
            strcat( relative, i_that_slash + 1 );
        }
        else
        {
            /* !this_slashes_left && !that_slashes_left */
            strcat( relative, "./" );
            strcat( relative, i_that_slash + 1 );
        }
    }
    return relative;
}

/******/



/****f* HTML_Generator/RB_HTML_Generate_Char
 * NAME
 *   RB_HTML_Generate_Char -- generate a single character for an item.
 * SYNOPSIS
 */
void RB_HTML_Generate_Char(
    FILE *dest_doc,
    int c )
/*
 * FUNCTION
 *   This function is called for every character that goes
 *   into an item's body.  This escapes all the reserved
 *   HTML characters such as '&', '<', '>', '"'.
 * SOURCE
 */
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
        /*  All others are printed literally */
        fputc( c, dest_doc );
    }
}

/*******/


void HTML_Generate_Begin_Content(
    FILE *dest_doc )
{
    HTML_Generate_Div( dest_doc, "content" );
}

void HTML_Generate_End_Content(
    FILE *dest_doc )
{
    HTML_Generate_Div_End( dest_doc, "content" );
}

void HTML_Generate_Begin_Navigation(
    FILE *dest_doc )
{
    HTML_Generate_Div( dest_doc, "navigation" );
}

void HTML_Generate_End_Navigation(
    FILE *dest_doc )
{

    HTML_Generate_Div_End( dest_doc, "navigation" );
}

void HTML_Generate_Begin_Extra(
    FILE *dest_doc )
{
    HTML_Generate_Div( dest_doc, "extra" );
}

void HTML_Generate_End_Extra(
    FILE *dest_doc )
{
    HTML_Generate_Div_End( dest_doc, "extra" );
}




/****f* HTML_Generator/RB_Create_CSS
 * FUNCTION
 *   Create the .css file.  Unless the user specified it's own css
 *   file robodoc creates a default one.
 *
 *   For multidoc mode the name of the .css file is
 *      robodoc.css
 *   For singledoc mode the name of the .css file is equal
 *   to the name of the documentation file.
 * SYNOPSIS
 */
void RB_Create_CSS(
    struct RB_Document *document )
/*
 * INPUTS
 *   o document -- the document for which to create the file.
 * SOURCE
 */
{
    size_t              l = 0;
    FILE               *css_file;

    /* compute the complete path to the css file */
    if ( ( document->actions.do_singledoc ) ||
         ( document->actions.do_singlefile ) )
    {
        char               *extension = ".css";

        l += strlen( document->singledoc_name );
        l += strlen( extension );
        ++l;
        css_name = malloc( l );
        strcpy( css_name, document->singledoc_name );
        strcat( css_name, extension );
    }
    else
    {
        struct RB_Path     *docroot = document->docroot;
        char               *docrootname = docroot->name;
        char               *filename = "robodoc.css";

        l = strlen( filename );
        l += strlen( docrootname );
        ++l;
        css_name = malloc( l );
        strcpy( css_name, docrootname );
        strcat( css_name, filename );
    }

    RB_Say( "Creating CSS file %s\n", SAY_DEBUG, css_name );
    if ( document->css )
    {
        /* The user specified its own css file,
         * so we use the content of that.
         */
        RB_CopyFile( document->css, css_name );
    }
    else
    {
        css_file = fopen( css_name, "w" );
        if ( css_file )
        {
                    /** BEGIN BEGIN BEGIN Don't remove */
            fprintf( css_file,
                     "/****h* ROBODoc/ROBODoc Cascading Style Sheet\n"
                     " * FUNCTION\n"
                     " *   This is the default cascading style sheet for documentation\n"
                     " *   generated with ROBODoc.\n"
                     " *   You can edit this file to your own liking and then use\n"
                     " *   it with the option\n"
                     " *      --css <filename>\n"
                     " *\n"
                     " *   This style-sheet defines the following layout\n"
                     " *      +----------------------------------------+\n"
                     " *      |    logo                                |\n"
                     " *      +----------------------------------------+\n"
                     " *      |    extra                               |\n"
                     " *      +----------------------------------------+\n"
                     " *      |                              | navi-   |\n"
                     " *      |                              | gation  |\n"
                     " *      |      content                 |         |\n"
                     " *      |                              |         |\n"
                     " *      +----------------------------------------+\n"
                     " *      |    footer                              |\n"
                     " *      +----------------------------------------+\n"
                     " *\n"
                     " *   This style-sheet is based on a style-sheet that was automatically\n"
                     " *   generated with the Strange Banana stylesheet generator.\n"
                     " *   See http://www.strangebanana.com/generator.aspx\n"
                     " *\n"
                     " ******\n"
                     " * $Id: html_generator.c,v 1.94 2008/06/17 11:49:27 gumpu Exp $\n"
                     " */\n"
                     "\n"
                     "body\n"
                     "{\n"
                     "    background-color:    rgb(255,255,255);\n"
                     "    color:               rgb(98,84,55);\n"
                     "    font-family:         Arial, serif;\n"
                     "    border-color:        rgb(226,199,143);\n"
                     "}\n"
                     "\n"
                     "pre\n"
                     "{\n"
                     "    font-family:      monospace;\n"
                     "    margin:      15px;\n"
                     "    padding:     5px;\n"
                     "    white-space: pre;\n"
                     "    color:       #000;\n"
                     "}\n"
                     "\n"
                     "pre.source\n"
                     "{\n"
                     "    background-color: #ffe;\n"
                     "    border: dashed #aa9 1px;\n"
                     "}\n"
                     "\n"
                     "p\n"
                     "{\n"
                     "    margin:15px;\n"
                     "}\n"
                     "\n"
                     "p.item_name \n"
                     "{\n"
                     "    font-weight: bolder;\n"
                     "    margin:5px;\n"
                     "    font-size: 120%%;\n"
                     "}\n"
                     "\n"
                     "#content\n" "{\n" "    font-size:           100%%;\n" );
            fprintf( css_file,
                     "    color:               rgb(0,0,0);\n"
                     "    background-color:    rgb(255,255,255);\n"
                     "    border-left-width:   0px; \n"
                     "    border-right-width:  0px; \n"
                     "    border-top-width:    0px; \n"
                     "    border-bottom-width: 0px;\n"
                     "    border-left-style:   none; \n"
                     "    border-right-style:  none; \n"
                     "    border-top-style:    none; \n"
                     "    border-bottom-style: none;\n"
                     "    padding:             40px 31px 14px 17px;\n"
                     "    border-color:        rgb(0,0,0);\n"
                     "    text-align:          justify;\n"
                     "}\n"
                     "\n"
                     "#navigation\n"
                     "{\n"
                     "    background-color: rgb(98,84,55);\n"
                     "    color:            rgb(230,221,202);\n"
                     "    font-family:      \"Times New Roman\", serif;\n"
                     "    font-style:       normal;\n"
                     "    border-color:     rgb(0,0,0);\n"
                     "}\n"
                     "\n"
                     "a.menuitem\n"
                     "{\n"
                     "    font-size: 120%%;\n"
                     "    background-color:    rgb(0,0,0);\n"
                     "    color:               rgb(195,165,100);\n"
                     "    font-variant:        normal;\n"
                     "    text-transform:      none;\n"
                     "    font-weight:         normal;\n"
                     "    padding:             1px 8px 3px 1px;\n"
                     "    margin-left:         5px; \n"
                     "    margin-right:        5px; \n"
                     "    margin-top:          5px; \n"
                     "    margin-bottom:       5px;\n"
                     "    border-color:        rgb(159,126,57);\n"
                     "    text-align:          right;\n"
                     "}\n"
                     "\n"
                     "#logo, #logo a\n"
                     "{\n"
                     "    font-size: 130%%;\n"
                     "    background-color:   rgb(198,178,135);\n"
                     "    color:              rgb(98,84,55);\n"
                     "    font-family:        Georgia, serif;\n"
                     "    font-style:         normal;\n"
                     "    font-variant:       normal;\n"
                     "    text-transform:     none;\n"
                     "    font-weight:        bold;\n"
                     "    padding:            20px 18px 20px 18px;\n"
                     "    border-color:       rgb(255,255,255);\n"
                     "    text-align:         right;\n"
                     "}\n"
                     "\n"
                     "#extra, #extra a\n"
                     "{\n"
                     "    font-size: 128%%;\n"
                     "    background-color:    rgb(0,0,0);\n"
                     "    color:               rgb(230,221,202);\n"
                     "    font-style:          normal;\n"
                     "    font-variant:        normal;\n"
                     "    text-transform:      none;\n"
                     "    font-weight:         normal;\n" );
            fprintf( css_file,
                     "    border-left-width:   0px; \n"
                     "    border-right-width:  0px; \n"
                     "    border-top-width:    0px; \n"
                     "    border-bottom-width: 0px;\n"
                     "    border-left-style:   none; \n"
                     "    border-right-style:  none; \n"
                     "    border-top-style:    none; \n"
                     "    border-bottom-style: none;\n"
                     "    padding: 12px 12px 12px 12px;\n"
                     "    border-color:        rgb(195,165,100);\n"
                     "    text-align:          center;\n"
                     "}\n"
                     "\n"
                     "#content a\n"
                     "{\n"
                     "    color:              rgb(159,126,57);\n"
                     "    text-decoration:    none;\n"
                     "}\n"
                     "\n"
                     "#content a:hover, #content a:active\n"
                     "{\n"
                     "    color:              rgb(255,255,255);\n"
                     "    background-color:   rgb(159,126,57);\n"
                     "}\n"
                     "\n"
                     "a.indexitem\n"
                     "{\n"
                     "    display: block;\n"
                     "}\n"
                     "\n"
                     "h1, h2, h3, h4, h5, h6\n"
                     "{\n"
                     "    background-color: rgb(221,221,221);\n"
                     "    font-family:      Arial, serif;\n"
                     "    font-style:       normal;\n"
                     "    font-variant:     normal;\n"
                     "    text-transform:   none;\n"
                     "    font-weight:      normal;\n"
                     "}\n"
                     "\n"
                     "h1\n"
                     "{\n"
                     "    font-size: 151%%;\n"
                     "}\n"
                     "\n"
                     "h2\n"
                     "{\n"
                     "    font-size: 142%%;\n"
                     "}\n"
                     "\n"
                     "h3\n"
                     "{\n"
                     "    font-size: 133%%;\n"
                     "}\n"
                     "\n"
                     "h4\n"
                     "{\n"
                     "    font-size: 124%%;\n"
                     "}\n"
                     "\n"
                     "h5\n"
                     "{\n"
                     "    font-size: 115%%;\n"
                     "}\n"
                     "\n"
                     "h6\n"
                     "{\n"
                     "    font-size: 106%%;\n"
                     "}\n"
                     "\n"
                     "#navigation a\n"
                     "{\n"
                     "    text-decoration: none;\n"
                     "}\n"
                     "\n"
                     ".menuitem:hover\n"
                     "{\n"
                     "    background-color:   rgb(195,165,100);\n"
                     "    color:              rgb(0,0,0);\n"
                     "}\n"
                     "\n"
                     "#extra a\n"
                     "{\n"
                     "    text-decoration: none;\n"
                     "}\n"
                     "\n"
                     "#logo a\n"
                     "{\n"
                     "    text-decoration: none;\n"
                     "}\n"
                     "\n"
                     "#extra a:hover\n"
                     "{\n"
                     "}\n"
                     "\n"
                     "/* layout */\n"
                     "#navigation\n"
                     "{\n"
                     "    width:       22%%; \n"
                     "    position:    relative; \n"
                     "    top:         0; \n"
                     "    right:       0; \n"
                     "    float:       right; \n"
                     "    text-align:  center;\n"
                     "    margin-left: 10px;\n"
                     "}\n"
                     "\n"
                     ".menuitem       {width: auto;}\n"
                     "#content        {width: auto;}\n"
                     ".menuitem       {display: block;}\n" "\n" "\n" );
            fprintf( css_file,
                     "div#footer\n"
                     "{\n"
                     "    background-color: rgb(198,178,135);\n"
                     "    color:      rgb(98,84,55);\n"
                     "    clear:      left;\n"
                     "    width:      100%%;\n"
                     "    font-size:   71%%;\n"
                     "}\n"
                     "\n"
                     "div#footer a\n"
                     "{\n"
                     "    background-color: rgb(198,178,135);\n"
                     "    color:            rgb(98,84,55);\n"
                     "}\n"
                     "\n"
                     "div#footer p\n"
                     "{\n"
                     "    margin:0;\n"
                     "    padding:5px 10px\n"
                     "}\n"
                     "\n"
                     "span.keyword\n"
                     "{\n"
                     "    color: #00F;\n"
                     "}\n"
                     "\n"
                     "span.comment\n"
                     "{\n"
                     "    color: #080;\n"
                     "}\n"
                     "\n"
                     "span.quote\n"
                     "{\n"
                     "    color: #F00;\n"
                     "}\n"
                     "\n"
                     "span.squote\n"
                     "{\n"
                     "    color: #F0F;\n"
                     "}\n"
                     "\n"
                     "span.sign\n"
                     "{\n"
                     "    color: #008B8B;\n"
                     "}\n"
                     "\n"
                     "span.line_number\n"
                     "{\n"
                     "    color: #808080;\n"
                     "}\n"
                     "\n"
                     "@media print\n"
                     "{\n"
                     "    #navigation {display: none;}\n"
                     "    #content    {padding: 0px;}\n"
                     "    #content a  {text-decoration: underline;}\n"
                     "}\n" );
                    /** END END END Don't remove */
            fclose( css_file );
        }
        else
        {
            RB_Panic( "Can't open %s for writing\n", css_name );
        }
    }
}

/*******/


void RB_InsertCSS(
    FILE *dest_doc,
    char *filename )
{
    if ( css_name )
    {
        char               *r = RB_HTML_RelativeAddress( filename, css_name );

        assert( r );
        assert( strlen( r ) );
        fprintf( dest_doc,
                 "<link rel=\"stylesheet\" href=\"%s\" type=\"text/css\" />\n",
                 r );
    }
}



void HTML_Generate_Begin_Paragraph(
    FILE *dest_doc )
{
    fprintf( dest_doc, "<p>" );
}

void HTML_Generate_End_Paragraph(
    FILE *dest_doc )
{
    fprintf( dest_doc, "</p>\n" );
}


void HTML_Generate_Begin_Preformatted(
    FILE *dest_doc,
    int source )
{
    /*  Check if we are preformatting a SOURCE item */
    if ( source )
    {
        /*  SOURCE items have their own class in the CSS */
        fprintf( dest_doc, "<pre class=\"%s\">", SOURCE_CLASS );
    }
    else
    {
        fprintf( dest_doc, "<pre>" );
    }
}

void HTML_Generate_End_Preformatted(
    FILE *dest_doc )
{
    fprintf( dest_doc, "</pre>\n" );
}


void HTML_Generate_Begin_List(
    FILE *dest_doc )
{
    fprintf( dest_doc, "<ul>" );
}

void HTML_Generate_End_List(
    FILE *dest_doc )
{
    fprintf( dest_doc, "</ul>\n" );
}

void HTML_Generate_Begin_List_Item(
    FILE *dest_doc )
{
    fprintf( dest_doc, "<li>" );
}

void HTML_Generate_End_List_Item(
    FILE *dest_doc )
{
    fprintf( dest_doc, "</li>\n" );
}

/****f* HTML_Generator/RB_HTML_Generate_Item_Line_Number
 * FUNCTION
 *   Generate line numbers for SOURCE like items
 * SYNOPSIS
 */
void RB_HTML_Generate_Item_Line_Number(
    FILE *dest_doc,
    char *line_number_string )
/*
 * INPUTS
 *   o dest_doc           -- the file to write to.
 *   o line_number_string -- the line number as string.
 * SOURCE
 */
{
    RB_HTML_Color_String( dest_doc, 2, LINE_NUMBER_CLASS,
                          line_number_string );
}
/******/
