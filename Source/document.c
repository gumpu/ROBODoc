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

/****h* ROBODoc/Document
 * FUNCTION
 *   This module contains functions to manipulate the central data
 *   structure (RB_Document) that contains information about the
 *   source files, and documentation files, and headers.
 *
 *   The name is a bit confusing because it sort of implies that
 *   it contains the documentation extracted from the sourcefiles.
 *
 *   For each run a RB_Document structure is created, it is filled
 *   by the analyser and directory module and then used by the
 *   generator module to create the documentation.
 * MODIFICATION HISTORY
 *   * ????-??-??   Frans Slothouber  V1.0
 *   * 2003-02-03   Frans Slothouber  Refactoring
 *   * 2003-10-30   David White       Removed unistd.h for Borland
 *******
 */

#include <assert.h>
#include <stdlib.h>
#include <ctype.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "robodoc.h"
#include "document.h"
#include "part.h"
#include "path.h"
#include "directory.h"
#include "headers.h"
#include "links.h"
#include "util.h"
#include <string.h>
#include "generator.h"
#include "file.h"
#include "globals.h"


#ifdef DMALLOC
#include <dmalloc.h>
#endif


/****f* Document/RB_Document_Add_Part
 * FUNCTION
 *   Add a new part to the document.
 * SYNOPSIS
 */
void RB_Document_Add_Part(
    struct RB_Document *document,
    struct RB_Part *part )
/*
 * INPUTS
 *   o document  -- the document the part is to be added to.
 *   o part      -- the part to be added
 * SOURCE
 */
{
    part->next = document->parts;
    document->parts = part;
}

/*****/


/* TODO Documentation */
void RB_Free_RB_Document_Parts(
    struct RB_Document *document )
{
    if ( document->parts )
    {
        struct RB_Part     *a_part = NULL;
        struct RB_Part     *a_part2 = NULL;

        for ( a_part = document->parts; a_part; a_part = a_part2 )
        {
            a_part2 = a_part->next;
            RB_Free_RB_Part( a_part );
        }
    }
    document->parts = NULL;
}


/* TODO Documentation */
void RB_Free_RB_Document(
    struct RB_Document *document )
{
    RB_Free_RB_Document_Parts( document );
    if ( document->headers )
    {
        unsigned long       i;

        for ( i = 0; i < document->no_headers; ++i )
        {
            RB_Free_Header( document->headers[i] );

        }
        free( document->headers );
    }
    free( document );
}

/****f* Document/RB_Document_Create_Parts
 * FUNCTION
 *   Create all the parts of a document based on the sourcefiles in
 *   the source tree.  This creates a new RB_Part for each file in
 *   the source tree.
 * INPUTS
 *    o document -- the document for which the parts are generated.
 * SOURCE
 */

void RB_Document_Create_Parts(
    struct RB_Document *document )
{
    struct RB_Filename *i_file = NULL;

    assert( document );
    assert( document->srctree );

    for ( i_file = document->srctree->first; i_file; i_file = i_file->next )
    {
        struct RB_Part     *rbpart;

        rbpart = RB_Get_RB_Part(  );
        RB_Part_Add_Source( rbpart, i_file );
        RB_Document_Add_Part( document, rbpart );
    }
}

/*******/


/****f* Document/RB_Fill_Header_Filename
 * FUNCTION
 *   Fill the file_name attribute of all headers based either on the
 *   part or the singledoc name.   The file_name tells in which file
 *   the documentation for the header is to be stored.
 * SYNOPSIS
 */
void RB_Fill_Header_Filename(
    struct RB_Document *document )
/*
 * SOURCE
 */
{
    struct RB_Part     *i_part;

    RB_Say( "Computing file_name attribute for all headers.\n", SAY_DEBUG );
    for ( i_part = document->parts; i_part; i_part = i_part->next )
    {
        struct RB_header   *i_header;

        for ( i_header = i_part->headers;
              i_header; i_header = i_header->next )
        {
            if ( document->actions.do_singledoc )
            {
                i_header->file_name = document->singledoc_name;
            }
            else if ( document->actions.do_multidoc )
            {
                i_header->file_name = RB_Get_FullDocname( i_part->filename );
            }
            else if ( document->actions.do_singlefile )
            {
                i_header->file_name = document->singledoc_name;
            }
            else
            {
                assert( 0 );
            }
        }
    }
}

/******/


/****f* Document/RB_Document_Determine_DocFilePaths
 * FUNCTION
 *   Determine the path of each of the documentation files based on
 *   the path of the source file and the documentation root path and
 *   the source root path.
 * SYNOPSIS
 */
void RB_Document_Determine_DocFilePaths(
    struct RB_Document *document )
/*
 * EXAMPLE
 *   srcpath = ./test/mysrc/sub1/sub2
 *   srcroot = ./test/mysrc/
 *   docroot = ./test/mydoc/
 *     ==>
 *   docpath = ./test/mydoc/sub1/sub2
 * SOURCE
 */
{
    struct RB_Path     *path;
    int                 docroot_length;
    int                 srcroot_length;
    int                 length;

    assert( document->srctree );
    assert( document->srcroot );
    assert( document->docroot );

    docroot_length = strlen( document->docroot->name );
    srcroot_length = strlen( document->srcroot->name );

    for ( path = document->srctree->first_path; path; path = path->next )
    {
        char               *name;
        char               *new_name;
        char               *tail;

        name = path->name;
        length = strlen( name );
        assert( length >= srcroot_length );
        tail = name + srcroot_length;
        new_name = calloc( docroot_length +
                           ( length - srcroot_length ) + 1, sizeof( char ) );
        assert( new_name );
        strcat( new_name, document->docroot->name );
        if ( document->actions.do_no_subdirectories )
        {
            /* No subdirectory */
        }
        else
        {
            strcat( new_name, tail );
        }
        path->docname = new_name;
    }
}

/******/


/****f* Document/RB_Document_Create_DocFilePaths
 * FUNCTION
 *   This function creates the whole document directory
 *   tree.  It tests if the directories exist and if they
 *   do not the directory is created.
 * SYNOPSIS
 */
void RB_Document_Create_DocFilePaths(
    struct RB_Document *document )
/*
 * INPUTS
 *   o document -- the document for which the tree is created.
 * SOURCE
 */
{
    struct RB_Path     *path;

    for ( path = document->srctree->first_path; path; path = path->next )
    {
        char               *pathname = NULL;
        char               *c2 = NULL;

        RB_Say( "Trying to create directory %s\n", SAY_INFO, path->docname );
        /* Don't want to modify the docname in the path
           structure. So we make a copy that we later
           destroy. */

        pathname = RB_StrDup( path->docname );
        for ( c2 = pathname + 1;        /* We skip the leading '/' */
              *c2; ++c2 )
        {
            if ( *c2 == '/' )
            {
                struct stat         dirstat;

                *c2 = '\0';     /* Replace the '/' with a '\0'. */
                /* We now have one of the paths leading up to the
                   total path. Test if it exists. */
                if ( stat( pathname, &dirstat ) == 0 )
                {
                    /* Path exists. */
                }
                else if ( ( strlen( pathname ) == 2 ) &&
                          ( utf8_isalpha( pathname[0] ) ) &&
                          ( pathname[1] == ':' ) )
                {
                    /* Is is a drive indicator, ( A: B: C: etc )
                     * stat fails on this, but we should not
                     * create either, so we do nothing.
                     */
                }
                else
                {
                    int                 result;

#if defined(__MINGW32__)
                    result = mkdir( pathname );
#else
                    result = mkdir( pathname, 0770 );
#endif
                    if ( result == 0 )
                    {
                        /* Path was created. */
                    }
                    else
                    {
                        perror( NULL );
                        RB_Panic( "Can't create directory %s\n", pathname );
                    }
                }
                /* Put the '/' back in it's place. */
                *c2 = '/';
            }
        }
        free( pathname );
    }
}

/*******/


/* TODO Documentation */

/*x**f* 
 * FUNCTION
 *   Compare two header types for sorting.
 * RESULT
 *   -1  h1 <  h2
 *    0  h1 == h2
 *    1  h1 >  h2
 * SOURCE
 */

int RB_CompareHeaders(
    void *h1,
    void *h2 )
{
    struct RB_header   *header_1 = h1;
    struct RB_header   *header_2 = h2;

    /*  Check for priorities */
    if ( header_1->htype->priority > header_2->htype->priority )
    {
        /*  Header 1 has higher priority */
        return -1;
    }
    else if ( header_1->htype->priority < header_2->htype->priority )
    {
        /*  Header 2 has higher priority */
        return 1;
    }
    else
    {
        /*  Priorities are equal */
        /*  Check if we sort on full name or just the function name */
        if ( course_of_action.do_sectionnameonly )
        {
            /*  Do not include parent name in sorting if they are not displayed */
            return RB_Str_Case_Cmp( header_1->function_name,
                                    header_2->function_name );
        }
        else
        {
            /*  Sort on full name ( module/function ) */
            return RB_Str_Case_Cmp( header_1->name, header_2->name );
        }
    }
}

/*****/

/* TODO Documentation */
void RB_Document_Sort_Headers(
    struct RB_Document *document )
{
    struct RB_Part     *i_part;
    unsigned long       part_count = 0;

    RB_Say( "Sorting headers per part (file)\n", SAY_INFO );
    for ( i_part = document->parts; i_part; i_part = i_part->next )
    {
        struct RB_header   *i_header;

        /* Count the number of headers */
        for ( part_count = 0, i_header = i_part->headers;
              i_header; i_header = i_header->next )
        {
            part_count++;
        }

        if ( part_count )
        {
            /* Sort them */
            struct RB_header  **temp_headers =
                calloc( part_count, sizeof( struct RB_header * ) );
            unsigned int        i = 0;

            i_header = i_part->headers;
            for ( i = 0; i < part_count; ++i )
            {
                assert( i_header );
                temp_headers[i] = i_header;
                i_header = i_header->next;
            }
            RB_QuickSort( ( void ** ) temp_headers, 0, part_count - 1,
                          RB_CompareHeaders );
            i_part->headers = temp_headers[0];
            i_part->headers->next = NULL;
            i_header = temp_headers[0];
            for ( i = 1; i < part_count; ++i )
            {
                assert( i_header );
                i_header->next = temp_headers[i];
                i_header = i_header->next;
            }
            temp_headers[part_count - 1]->next = NULL;
            free( temp_headers );
        }
    }
    RB_Say( "Sorting all headers\n", SAY_INFO );
    RB_QuickSort( ( void ** ) document->headers, 0, document->no_headers - 1,
                  RB_CompareHeaders );
}


/****f* Document/RB_Document_Collect_Headers
 * FUNCTION
 *   Create a table of pointers to all headers.  This is done to
 *   have easy access to all heades without having to scan all
 *   RB_Parts.
 * INPUTS
 *   o document -- the document for which the table is created.
 * OUTPUT
 *   o document->headers
 *   o document->no_headers
 * SOURCE
 */

void RB_Document_Collect_Headers(
    struct RB_Document *document )
{
    struct RB_Part     *i_part;
    struct RB_header  **headers;        /* Pointer to an array of pointers RB_headers. */
    unsigned long       count = 0;
    unsigned long       part_count = 0;
    unsigned long       i = 0;

    RB_Say( "Collecting all headers in a single table\n", SAY_INFO );
    for ( i_part = document->parts; i_part; i_part = i_part->next )
    {
        struct RB_header   *i_header;

        /* Count the number of headers */
        for ( part_count = 0, i_header = i_part->headers;
              i_header; i_header = i_header->next )
        {
            part_count++;
        }
        /* Compute the total count */
        count += part_count;
    }
    headers =
        ( struct RB_header ** ) calloc( count, sizeof( struct RB_header * ) );
    for ( i_part = document->parts; i_part; i_part = i_part->next )
    {
        struct RB_header   *i_header;

        for ( i_header = i_part->headers;
              i_header; i_header = i_header->next )
        {
            headers[i] = i_header;
            i++;
        }
    }
    document->headers = headers;
    document->no_headers = count;
}

/*******/

/* TODO Documentation */

struct RB_header   *RB_Document_Check_For_Duplicate(
    struct RB_Document *arg_document,
    struct RB_header *hdr )
{
    struct RB_Part     *i_part;

    for ( i_part = arg_document->parts; i_part; i_part = i_part->next )
    {
        struct RB_header   *i_header;

        for ( i_header = i_part->headers; i_header;
              i_header = i_header->next )
        {
            int                 i;

            if ( hdr == i_header )
                continue;

            for ( i = 0; i < hdr->no_names; i++ )
                if ( strcmp( hdr->names[i], i_header->name ) == 0 )
                    return i_header;
        }
    }
    return NULL;
}


/*  TODO Documentation 
 If A is called   qqqq/ffff and B is called  ffff/zzzz then A is the
 parent of B
*/

void RB_Document_Link_Headers(
    struct RB_Document *document )
{
    unsigned long       i;
    unsigned long       j;
    struct RB_header   *parent;
    struct RB_header   *child;
    char               *parent_name;
    char               *child_name;

    RB_Say( "Linking all %d headers.\n", SAY_INFO, document->no_headers );
    for ( i = 0; i < document->no_headers; i++ )
    {
        parent = ( document->headers )[i];
        parent_name = parent->function_name;
        for ( j = 0; j < document->no_headers; j++ )
        {
            if ( i != j )
            {
                child = ( document->headers )[j];
                child_name = child->module_name;
                if ( strcmp( child_name, parent_name ) == 0 )
                {
                    child->parent = parent;
                }
            }
        }
    }
}


/****f* Document/RB_Loop_Check
 * FUNCTION
 *   This function checks for loops in the headersr; that is
 *   by mistake the following relation between headers
 *   can be specified.
 *     y.parent -> x.parent -> z.parent -+
 *     ^                                 |
 *     |---------------------------------+
 *   This functions detects these loops, prints a warning
 *   and breaks the loop.
 *
 *   If left unbroken ROBODoc can hang.
 * SYNOPSIS
 */

void RB_Loop_Check(
    struct RB_Document *document
        )

 /* INPUTS
  *   document -- document to be checked for loops.
  *
  * SOURCE
  */
{
    int i;

    RB_Say( "Check all %d headers for loops.\n", SAY_INFO, document->no_headers );
    for ( i = 0; i < document->no_headers; i++ )
    {
        struct RB_header   *parent;
        struct RB_header   *grant_parent;
        parent = ( document->headers )[i];
        for ( grant_parent = parent->parent;
              grant_parent && ( parent != grant_parent );
              grant_parent = grant_parent->parent ) {
            /* Empty */
        }
        if ( parent == grant_parent ) {
            /* Problem, from a parent I can get
             * back to the parent itself. This means 
             * there is a loop. Warn the user about this.
             */

             RB_Warning( "Warning: Header %s/%s eventually points back to itself.\n",
                   parent->module_name, parent->function_name  );
             /* TODO print whole linkage line */
             for ( grant_parent = parent->parent;
                   grant_parent && ( parent != grant_parent );
                   grant_parent = grant_parent->parent ) {
                    /* Empty */
                 RB_Warning("--> %s/%s\n", 
                   grant_parent->module_name, grant_parent->function_name  );
             }
             parent->parent = NULL;
             RB_Warning( "Breaking the linkage for %s/%s\n",
                   parent->module_name, parent->function_name  );
        }
    }
}

/*******/


/* TODO Documentation */
void RB_Document_Split_Parts(
    struct RB_Document *document )
{
    struct RB_Part     *i_part = NULL;
    struct RB_Part    **new_parts = NULL;
    int                 new_number_of_parts = 0;
    int                 n = 0;
    int                 i;

    /* split eacht part in several parts. One for each header
     * in the original part.
     */
    for ( i_part = document->parts; i_part; i_part = i_part->next )
    {
        struct RB_header   *i_header;

        for ( i_header = i_part->headers;
              i_header; i_header = i_header->next )
        {
            ++new_number_of_parts;
        }
    }

    new_parts = calloc( new_number_of_parts, sizeof( struct RB_Part * ) );

    if ( new_parts )
    {
        /* Create new parts */

        RB_Say( "Splitting parts based on headers.\n", SAY_DEBUG );
        for ( i_part = document->parts; i_part; i_part = i_part->next )
        {
            struct RB_header   *i_header;
            struct RB_header   *i_next_header;

            for ( i_header = i_part->headers;
                  i_header; i_header = i_next_header )
            {
                struct RB_Part     *new_part;

                i_next_header = i_header->next;

                RB_Say( "Creating new part.\n", SAY_DEBUG );
                new_part = RB_Get_RB_Part(  );
                RB_Part_Add_Source( new_part,
                                    RB_Copy_RB_Filename( RB_Part_Get_Source
                                                         ( i_part ) ) );
                /* remove header from i_part and add to new_part.
                 */
                RB_Part_Add_Header( new_part, i_header );
                assert( n < new_number_of_parts );
                new_parts[n] = new_part;
                ++n;
            }
            i_part->headers = NULL;
            i_part->last_header = NULL;
        }
        /* Remove old part from document */
        RB_Free_RB_Document_Parts( document );
        /* Add new parts to document */
        for ( i = 0; i < n; ++i )
        {
            RB_Document_Add_Part( document, new_parts[i] );
        }
        /* clean-up temp array */
        free( new_parts );
    }
    else
    {
        RB_Panic( "Out of memory! RB_Document_Split_Parts()" );
    }
}

/* TODO Documentation */
void RB_Document_Determine_DocFileNames(
    struct RB_Document *document )
{
    struct RB_Filename *filename;
    unsigned int        length = 0;
    char               *name;
    char               *c;

    struct RB_Part     *part;

    assert( document->actions.do_multidoc );

    for ( part = document->parts; part; part = part->next )
    {

        filename = part->filename;
        /* turn  mysource.c  into  mysource_c.html
           First find the total length. */
        length = strlen( filename->name );
        /* add one for the '.' */
        ++length;

        if ( document->actions.do_one_file_per_header )
        {
            struct RB_header   *i_header = part->headers;

            assert( i_header );
            /* add the name of the header to the filename */
            /* We make this twice as long because some of the
             * characters in the file are escaped to 2 hexadecimal
             * digits.
             */
            length += 2 * strlen( i_header->name );
        }

        length += RB_Get_Len_Extension( document->extension );
        /* plus one for the '\0' */
        ++length;
        name = ( char * ) calloc( length, sizeof( char ) );
        assert( name );
        strcat( name, filename->name );
        for ( c = name; *c != '\0'; c++ )
        {
            if ( *c == '.' )
            {
                *c = '_';
            }
        }

        if ( document->actions.do_one_file_per_header )
        {
            unsigned int        i;
            struct RB_header   *i_header = part->headers;

            assert( i_header );
            /* add the name of the header to the filename */
            for ( i = 0; i < strlen( i_header->name ); ++i )
            {
                if ( utf8_isalnum( i_header->name[i] ) )
                {
                    sprintf( c, "%c", i_header->name[i] );
                    c++;
                }
                else
                {
                    sprintf( c, "%2X", i_header->name[i] );
                    c++;
                    c++;
                }
            }
        }
        RB_Add_Extension( document->extension, name );

        RB_Say( "Filename for part is %s\n", SAY_DEBUG, name );
        part->filename->docname = name;
    }
}

/****f* Document/RB_Open_SingleDocumentation
 * FUNCTION
 *   Open the file that will contain the documentation in
 *   case we create a single document.
 * SYNOPSIS
 */
FILE               *RB_Open_SingleDocumentation(
    struct RB_Document *document )
/*
 * RESULT
 *   An opened file.
 * SOURCE
 */
{
    FILE               *file;
    static char        *default_name = "singledoc";
    char               *name = NULL;
    size_t              size = 0;

    if ( document->singledoc_name )
    {
        size += strlen( document->singledoc_name );
    }
    else
    {
        size += strlen( default_name );
    }
    size++;                     /* and the '\0'; */
    size += RB_Get_Len_Extension( document->extension );

    name = ( char * ) calloc( size, sizeof( char ) );
    assert( name );
    if ( document->singledoc_name )
    {
        strcat( name, document->singledoc_name );
    }
    else
    {
        strcat( name, default_name );
    }
    RB_Add_Extension( document->extension, name );

    file = fopen( name, "w" );
    if ( file )
    {
        /* File opened  */
    }
    else
    {
        RB_Panic( "Can't open %s\n", name );
    }
    free( name );
    return file;
}

/****/

/****f* Document/RB_Get_RB_Document
 * FUNCTION
 *   Allocate and initialize an RB_Document structure.
 * SYNOPSIS
 */
struct RB_Document *RB_Get_RB_Document(
    void )
/*
 * RESULT
 *   An initialized document structure.
 * SOURCE
 */
{
    struct RB_Document *document = 0;
    document =
        ( struct RB_Document * ) malloc( sizeof( struct RB_Document ) );
    if ( document )
    {
        document->cur_part = NULL;
        document->parts = NULL;
        document->links = NULL;
        document->headers = NULL;
        document->doctype = UNKNOWN;
        document->actions = No_Actions();
        document->srctree = NULL;
        document->srcroot = NULL;
        document->docroot = NULL;
        document->singledoc_name = NULL;
        document->no_headers = 0;
        document->charset = NULL;
        document->extension = NULL;
        document->first_section_level = 1;
        document->doctype_name = NULL;
        document->doctype_location = NULL;
    }
    else
    {
        RB_Panic( "out of memory" );
    }
    return document;
}

/*******/
