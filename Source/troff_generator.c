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


#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <assert.h>
#include <errno.h>
#include <ctype.h>

#include <sys/param.h>
#include <sys/unistd.h>

#include "troff_generator.h"
#include "util.h"
#include "robodoc.h"
#include "items.h"
#include "headers.h"
#include "generator.h"
#include "file.h"
#include "part.h"

static int          skip_space = 1;
static int          in_list = 0;
static int          preformat = 0;
static int          skippre = 0;
static int          end_of_line = 0;
static int          end_of_para = 0;

static char        *compress_cmd = NULL;
static char        *compress_ext = NULL;
static char        *man_section = NULL;

/****h* ROBODoc/TROFF_Generator
 * FUNCTION
 *   Generator for TROFF output.
 * NOTE
 *   Almost finished.
 ******
 */

char               *RB_TROFF_Get_Default_Extension(
    void )
{
    return ".3";                /* Section 3 of manual */
}

/****f* TROFF_Generator/RB_TROFF_Generate_Doc_End
 * NAME
 *   RB_TROFF_Generate_Doc_End --
 *****
 */

void RB_TROFF_Generate_Doc_End(
    FILE *dest_doc,
    char *name )
{
    USE( dest_doc );
    USE( name );
}

static FILE        *open_output_file(
    char *out )
{
    if ( compress_cmd )
    {
        FILE               *file;
        char               *cmd =
            malloc( strlen( out ) + 10 + strlen( compress_cmd ) );

        sprintf( cmd, "%s > %s", compress_cmd, out );

        file = popen( cmd, "w" );
        free( cmd );
        return file;
    }
    else
        return fopen( out, "w" );
}

static void write_comment(
    FILE *out,
    char *str )
{
    int                 start = 1;

    while ( *str )
    {
        if ( start )
        {
            fputs( ".\\\" ", out );
            start = 0;
        }
        switch ( *str )
        {
        case '.':
            fputs( "\\.", out );
            break;
        case '-':
            fputs( "\\-", out );
            break;
        case '\\':
            fputs( "\\\\", out );
            break;
        case '\n':
            start = 1;          /* no break; */
        default:
            fputc( *str, out );
        }
        str++;
    }
    if ( start == 0 )
        fputc( '\n', out );
}

static char        *basename(
    char *str )
{
    char               *base = strrchr( str, '/' );

    return base ? base + 1 : str;
}

/****f* TROFF_Generator/RB_TROFF_Generate_Header_Start
 * NAME
 *   RB_TROFF_Generate_Header_Start
 ******
 */

FILE               *RB_TROFF_Generate_Header_Start(
    FILE *dest_doc,
    struct RB_header *cur_header )
{
    static char        *manpage = NULL;
    int                sec_item;

    if ( manpage == NULL )
        manpage = RB_malloc( MAXPATHLEN );

    if ( cur_header->name )
    {
        char               *file =
            RB_Get_FullDocname( cur_header->owner->filename );
        char               *name = basename( cur_header->name );
        char               *path = basename( file );
        int                 len, i;
        struct RB_Item      *item;
        char                *section = NULL;

        /* Search for SECTION header and use this section for man pages */
        sec_item = RB_Get_Item_Type("SECTION");
        for( item = cur_header->items; item ; item = item->next) {
            if( item->type == sec_item ) {
                /* Item found, search for nonempty line */
                for(i = 0 ; i < item->no_lines; i++ ) {
                    section = item->lines[i]->line;
                    while( *section && utf8_isspace(*section) ) section++;
                    if( *section ) {
                        /* nonempty line found, break */
                        break;
                    }
                }
                break;
            }
        }

        len = path - file;
        memcpy( manpage, file, len );

        /* we found a section header, strip at next whitespace */
        if( section && *section ) {
            char *to, *ptr;
            ptr = RB_malloc( strlen(section) + 1 );
            to = ptr;
            while( *section && !utf8_isspace(*section) ) {
                *to++ = *section++;
            }
            *to++ = 0;
            section = ptr;
        }
        else {
	    section = man_section;
        }

        sprintf( manpage + len, "%s.%s%s", name, section, compress_ext );

        dest_doc = open_output_file( manpage );
        if ( dest_doc == NULL )
        {
            RB_Panic( "Cannot open %s: %s\n", manpage, strerror( errno ) );
        }

        RB_Say( "+ Generating man page \"%s\"\n", SAY_INFO, manpage );

        /* Check for aliases if we have symlink() */
#ifdef HAVE_SYMLINK
        for ( i = 1; i < cur_header->no_names; i++ )
        {
            char               *base = basename( cur_header->names[i] );
            char               *buf = strchr( manpage, 0 ) + 1;

            memcpy( buf, file, len );
            sprintf( buf + len, "%s.%s%s", base, section, compress_ext );
            unlink( buf );
            symlink( basename( manpage ), buf );
            RB_Say( "+ Linked with \"%s\"\n", SAY_INFO, buf );
        }
#else
        USE( i );
#endif

        /* Append document type and title */
        fprintf( dest_doc, ".\\\" Source: %s\n",
                 cur_header->owner->filename->fullname );

        write_comment( dest_doc, COMMENT_ROBODOC COMMENT_COPYRIGHT );

        fprintf( dest_doc, ".TH %s %s \"", name, section );
        {
            time_t              ttp;
            char                timeBuffer[255];

            time( &ttp );
            strftime( timeBuffer, sizeof timeBuffer, "%b %d, %Y",
                      localtime( &ttp ) );
            fputs( timeBuffer, dest_doc );

            if ( name > cur_header->name )
            {
                path = cur_header->name;
                len = name - path - 1;
            }
            else
                len = strlen( path = name );
            if ( len >= (signed)sizeof timeBuffer )
                len = sizeof timeBuffer - 1;
            memcpy( timeBuffer, path, len );
            timeBuffer[len] = 0;
            fprintf( dest_doc, "\" %s \"%s Reference\"\n", timeBuffer,
                     timeBuffer );
        }
        if( section != man_section ) {
             free(section);
        }
    }
    return dest_doc;
}


/****f* TROFF_Generator/RB_TROFF_Generate_Header_End
 * NAME
 *   RB_TROFF_Generate_Header_End --
 *****
 */
void RB_TROFF_Generate_Header_End(
    FILE *dest_doc,
    struct RB_header *cur_header )
{
	USE( cur_header );
    fclose( dest_doc );
}


void RB_TROFF_Generate_Char(
    FILE *dest_doc,
    int c )
{
    if ( skip_space && utf8_isspace( c ) )
        return;
    skip_space = 0;
    if ( in_list == 1 && utf8_isspace( c ) )
    {
        fprintf( dest_doc, "\n\\-\\ " );
        in_list = 2;
        skip_space = 1;
        return;
    }
    else
        switch ( c )
        {
        case '\n':
            assert( 0 );
            break;
        case '\t':
            assert( 0 );
            break;
        case '\\':
        case '-':
        case '.':
            fputc( '\\', dest_doc );
            fputc( c, dest_doc );
            break;
        default:
            fputc( c, dest_doc );
        }
    end_of_para = 0;
}

void RB_TROFF_Generate_False_Link(
    FILE *out,
    char *name )
{
    fprintf( out, "\\fB%s\\fR", name );
    if ( in_list )
        fprintf( out, " \\- " );
    in_list = 0;
    skip_space = 0;
}

void RB_TROFF_Generate_Item_Name(
    FILE *out,
    char *name,
    int pre )
{
    if ( preformat )
    {
        fprintf( out, "\n.fi" );
    }
    fprintf( out, "\n.SH %s\n", name );
    if ( ( preformat = pre ) )
    {
        fprintf( out, ".nf\n" );
        skippre = -1;
        skippre = 0; 
    }
    else
        skippre = 0;
    in_list = 0;
}

void RB_TROFF_Start_New_Line(
    FILE *out )
{
    if ( end_of_para && in_list )
    {
        fprintf( out, "\n.PP" );
        in_list = 0;
    }
    skip_space = 1;
    end_of_line = 1;
    end_of_para = 1;
}

int RB_TROFF_Generate_Extra(
    FILE *out,
    enum ItemType item_type,
    char *str )
{
    USE( item_type );

    if ( end_of_line == 0 )
        return -1;
    end_of_line = 0;
    if ( preformat )
    {
        int                 len = strlen( str );

        if ( skippre == -1 )
        {
            for ( skippre = 0; utf8_isspace( *str ); skippre++, str++ );
            if ( skippre == 0 )
                skippre--;
        }

        len = skippre > len ? len : skippre;
        skip_space = 0;
        return len > 0 ? len - 1 : -1;
    }
    else
    {
        char               *p = str;
        char               *skip;
        char               *item;

        /* Start of a line */
        while ( utf8_isspace( *p ) )
            p++;
        skip = p;
        if ( strchr( "-o*", *p ) == NULL )
        {
            while ( !utf8_isspace( *p ) && *p )
                p++;
            item = p;
            while ( utf8_isspace( *p ) )
                p++;
            if ( *p != '-' )
                return -1;
            p++;
            if ( *p == '-' )
                p++;
            if ( !utf8_isspace( p[1] ) )
                return -1;
            in_list = 1;
            *item = 0;
            fprintf( out, ".TP\n.I %s\n\\-\\ ", skip );
            strcpy( p, p + 2 );
            skip = item;
            *item = ' ';
            in_list = 2;
        }
        else
        {
            char m = *p;
            if ( !utf8_isspace( p[1] ) )
                return -1;
            if( strchr(p, ':') != NULL ) {
                fprintf( out, ".IP \"" );
                if( m == '*' ) fprintf( out, "\\fB" );
                if( m == 'o' ) fprintf( out, "\\fI");
                /* p[0] = - / o / *  and p[1] isspace, so we can incr. p */
                p += 2;
                while( (*p) && (*p != ':') ) {
                    fputc(*p, out);
                    p++;
                    skip++;
                }
                while( *p != ':' ) {
                    p++;
                    skip++;
                }
                p++;
                skip++;
                while( utf8_isspace( *p ) ) {
                    p++;
                    skip++;
                }
                if( m == '*' ) fprintf( out, "\\fR");
                if( m == 'o' ) fprintf( out, "\\fR");
                fprintf( out, "\"\n");
                skip++;
            }
            else {
                switch( m ) {
                    case '*':
                        fprintf( out, ".IP *\n" );
                        break;
                    case 'o':
                        fprintf( out, ".IP o\n" );
                        break;
                    case '-':
                    default:
                        fprintf( out, ".IP -\n" );
                        break;
                }
            }
            skip++;
            in_list = 2;
        }
        skip_space = 1;
        return skip > str ? skip - str - 1 : -1;
    }
    return -1;
}

void RB_TROFF_Set_Param(
        char *compress,
        char *section )
{
    if ( compress == NULL || *compress == 0
            || strcasecmp( compress, "none" ) == 0 )
    {
        compress_cmd = NULL;
        compress_ext = "";
    }
    else if ( strcasecmp( compress, "gzip" ) == 0 )
    {
        compress_cmd = "gzip -c";
        compress_ext = ".gz";
    }
    else if ( strcasecmp( compress, "bzip2" ) == 0 )
    {
        compress_cmd = "bzip2 -c";
        compress_ext = ".bz2";
    }
    else
    {
        RB_Panic( "Unknown compression compress \"%s\"\n", compress );
    }
    if ( section == NULL || *section == 0 )
        man_section = "3";
    else
        man_section = section;
}

void TROFF_Generate_Begin_List_Item(
        FILE *out )
{
    USE( out );
    /*	fprintf( out, ".IP -\n" ); */
}

void TROFF_Generate_End_List_Item(
        FILE *out )
{
    USE( out );
    /*	fprintf( out, ".PP\n" ); */
}

void TROFF_Generate_Begin_Preformatted(
        FILE *out )
{
    USE( out );
    /*	fprintf( out, ".nf\n" ); */
}

void TROFF_Generate_End_Preformatted(
        FILE *out )
{
    USE( out );
    /*	fprintf( out, ".fi\n" ); */
}

void TROFF_Generate_End_Paragraph(
        FILE *out )
{
    USE( out );
    /*	fputc( '\n', out ); */
}

void TROFF_Generate_Begin_Paragraph(
        FILE *out )
{
    USE( out );
    /*	fprintf( out, ".PP\n" ); */
}

/****f* TROFF_Generator/RB_TROFF_Generate_Item_Line_Number
 * FUNCTION
 *   Generate line numbers for SOURCE like items
 * SYNOPSIS
 */
void RB_TROFF_Generate_Item_Line_Number(
        FILE *dest_doc,
        char *line_number_string )
/*
 * INPUTS
 *   o dest_doc           -- the file to write to.
 *   o line_number_string -- the line number as string.
 * SOURCE
 */
{
    fprintf( dest_doc, line_number_string );
}
/******/
