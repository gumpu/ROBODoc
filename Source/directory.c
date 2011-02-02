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

/****h* ROBODoc/Directory
 * NAME
 *   This module contains function to scan a directory tree and to
 *   create a RB_Directory structure.  Most of the OS dependend parts
 *   of ROBODoc can be found in this module..
 *
 *   This module currently works under:
 *   o Cygwin   http://cygwin.com 
 *   o Redhad 7.3 Linux
 *   o VC++ under Windows NT 
 *   o MINGW    http://www.mingw.org/ 
 *   o OS/X
 *
 * AUTHOR 
 *   Frans Slothouber
 *****
 * $Id: directory.c,v 1.42 2008/03/20 16:00:42 thuffir Exp $
 */

#include <assert.h>
#include <stdlib.h>
#include "robodoc.h"
#include "directory.h"
#include "util.h"
#include "globals.h"
#include "roboconfig.h"

#if defined _DIRENT_HAVE_D_TYPE
     /* Empty */
#else
#include <sys/stat.h>
#endif

  /* no dirent in strict ansi !!! */
#include <dirent.h>

#include <stdio.h>
#include <string.h>

#ifdef DMALLOC
#  include <dmalloc.h>
#endif


/****v* Directory/content_buffer
 * FUNCTION
 *   Temporary buffer file file content and filenames.
 * SOURCE
 */

#define RB_CBUFFERSIZE 8191
char                content_buffer[RB_CBUFFERSIZE + 1];

/*****/


/* Local functions */
static int          RB_Is_PathCharacter(
    int c );


/****f* Directory/RB_FileType
 * FUNCTION
 *   Determine the type of the file.  This function is used for all
 *   compilers except VC++.
 *
 *   If _DIRENT_HAVE_D_TYPE is defined we can find the filetype
 *   directly in the a_direntry.  If not we have to stat the file to
 *   find out.
 * SYNOPSIS
 */
T_RB_FileType RB_FileType(
    char *arg_pathname,
    struct dirent *a_direntry )
/*
 * INPUTS
 *   o arg_pathname -- the path leading up to this file
 *   o a_direntry -- a directory entry read by readdir()
 * RESULT
 *   The type of the file.
 ******
 */
{
    T_RB_FileType       file_type = RB_FT_UNKNOWN;

#if defined _DIRENT_HAVE_D_TYPE
    if ( a_direntry->d_type == DT_REG )
    {
        file_type = RB_FT_FILE;
    }
    else if ( a_direntry->d_type == DT_DIR )
    {
        file_type = RB_FT_DIRECTORY;
    }
    else
    {
        file_type = RB_FT_UNKNOWN;
    }
#endif
    if ( file_type == RB_FT_UNKNOWN )
    {
        char               *file_name = a_direntry->d_name;
        struct stat         filestat;
        int                 result;

        /* Either we do not have d_type, or it gives
         * no information, so we try it a different
         * way. (BUG 715778)
         */
        content_buffer[0] = '\0';
        strcat( content_buffer, arg_pathname );
        if ( content_buffer[strlen( content_buffer ) - 1] != '/' )
        {
            strcat( content_buffer, "/" );
        }
        strcat( content_buffer, file_name );
        result = stat( content_buffer, &filestat );
        if ( result == 0 )
        {
            if ( S_ISREG( filestat.st_mode ) )
            {
                file_type = RB_FT_FILE;
            }
            else if ( S_ISDIR( filestat.st_mode ) )
            {
                file_type = RB_FT_DIRECTORY;
            }
            else
            {
                file_type = RB_FT_UNKNOWN;
            }
        }
        else
        {
            /* Some error occurred while statting the file */
        }
    }
    return file_type;
}


/****f* Directory/RB_Directory_Insert_RB_Path
 * SYNOPSIS
 */

void RB_Directory_Insert_RB_Path(
    struct RB_Directory *arg_rb_directory,
    struct RB_Path *arg_rb_path )
/*
 * FUNCTION
 *  Insert a RB_Path into a RB_Directory.
 *  The RB_Path is added at the beginning of the list.
 * SOURCE
 */
{
    arg_rb_path->next = arg_rb_directory->first_path;
    arg_rb_directory->first_path = arg_rb_path;
}

/*****/

/****f* Directory/RB_Directory_Insert_RB_File
 * SYNOPSIS
 */
void RB_Directory_Insert_RB_Filename(
    struct RB_Directory *arg_rb_directory,
    struct RB_Filename *arg_rb_filename )
/*
 * FUNCTION
 *   Insert an RB_File structure into a RB_Directory structure.
 *   The RB_File is added at the end of the list.
 * SOURCE
 */
{
    if ( arg_rb_directory->last == 0 )
    {
        arg_rb_directory->first = arg_rb_filename;
        arg_rb_filename->next = 0;
        arg_rb_directory->last = arg_rb_filename;
    }
    else
    {
        arg_rb_directory->last->next = arg_rb_filename;
        arg_rb_filename->next = 0;
        arg_rb_directory->last = arg_rb_filename;
    }
}

/******/

/****f* Directory/RB_Free_RB_Directory
 * FUNCTION
 *   Free all the memory use by the RB_Directory structure.
 * SYNOPSIS
 */
void RB_Free_RB_Directory(
    struct RB_Directory *arg_directory )
/*
 * INPUTS
 *   o arg_directory -- the thing to be freed.
 * SOURCE
 */
{
    struct RB_Filename *rb_filename;
    struct RB_Filename *rb_filename2;
    struct RB_Path     *rb_path;
    struct RB_Path     *rb_path2;

    /* TODO  Not complete... check for leaks. */
    rb_filename = arg_directory->first;

    while ( rb_filename )
    {
        rb_filename2 = rb_filename;
        rb_filename = rb_filename->next;
        RB_Free_RB_Filename( rb_filename2 );
    }

    rb_path = arg_directory->first_path;

    while ( rb_path )
    {
        rb_path2 = rb_path;
        rb_path = rb_path->next;
        RB_Free_RB_Path( rb_path2 );
    }

    free( arg_directory );
}

/******/


/****f* Directory/RB_Get_RB_Directory
 * NAME
 *   RB_Get_RB_Directory -- get a RB_Directory structure
 * FUNCTION
 *   Returns a RB_Directory structure to the give directory,
 *   specified by the path.
 *   This structure can then be uses to walk through all the
 *   files in the directory and it's subdirectories.
 * SYNOPSIS
 */
struct RB_Directory *RB_Get_RB_Directory(
    char *arg_rootpath_name,
    char *arg_docroot_name )
/*
 * INPUTS
 *   arg_rootpath -- the name a the directory to get,
 *                   a nul terminated string.
 *   arg_docroot_name -- the name of the directory the documentation
 *                       file are stored in.  This directory is
 *                       skipped while scanning for sourcefiles.
 *                       It can be NULL.
 * RESULT
 *   A freshly allocated RB_Directory filled with source files.
 * SOURCE
 */
{
    struct RB_Directory *rb_directory;
    struct RB_Path     *doc_path = NULL;

    rb_directory =
        ( struct RB_Directory * ) malloc( sizeof( struct RB_Directory ) );
    rb_directory->first = 0;
    rb_directory->last = 0;
    rb_directory->first_path = RB_Get_RB_Path( arg_rootpath_name );

    if ( arg_docroot_name )
    {
        doc_path = RB_Get_RB_Path( arg_docroot_name );
    }

    RB_Fill_Directory( rb_directory, rb_directory->first_path, doc_path );
    if ( ( RB_Number_Of_Filenames( rb_directory ) > 0 ) &&
         ( RB_Number_Of_Paths( rb_directory ) > 0 ) )
    {
        RB_SortDirectory( rb_directory );
    }
    else
    {

        RB_Panic( "No files found! (Or all were filtered out)\n" );
    }
    return rb_directory;
}

/********/



/****f* Directory/RB_Get_RB_SingleFileDirectory
 * NAME
 *   RB_Get_RB_SingleFileDirectory -- get a RB_Directory structure
 * SYNOPSIS
 */
struct RB_Directory *RB_Get_RB_SingleFileDirectory(
    char *arg_fullpath )
/*
 * FUNCTION
 *   Returns a RB_Directory structure to the give directory,
 *   specified by the path that contains only a single file.
 *   This is used for the --singlefile option.
 * INPUT
 *   o filename -- a filename.  This may include the path.
 * RESULT
 *   a freshly allocated RB_Directory that contains only
 *   a single file.
 * SOURCE
 */
{
    struct RB_Directory *rb_directory;
    struct RB_Path     *path;
    char               *pathname = NULL;
    char               *filename = NULL;

    assert( arg_fullpath );

    pathname = RB_Get_PathName( arg_fullpath );
    filename = RB_Get_FileName( arg_fullpath );

    if ( pathname )
    {
        path = RB_Get_RB_Path( pathname );
    }
    else
    {
        /* no directory was specified so we use
         * the current directory
         */
        path = RB_Get_RB_Path( "./" );
    }

    rb_directory =
        ( struct RB_Directory * ) malloc( sizeof( struct RB_Directory ) );
    rb_directory->first = 0;
    rb_directory->last = 0;

    rb_directory->first_path = path;

    RB_Directory_Insert_RB_Filename( rb_directory,
                                     RB_Get_RB_Filename( filename, path ) );

    return rb_directory;
}

/*******/


/****f* Directory/RB_Fill_Directory
 * NAME
 *   RB_Fill_Directory -- fill a RB_Directory structure
 * SYNOPSIS
 */
void RB_Fill_Directory(
    struct RB_Directory *arg_rb_directory,
    struct RB_Path *arg_path,
    struct RB_Path *arg_doc_path )
/*
 * FUNCTION
 *   Walks through all the files in the directory pointed to
 *   by arg_path and adds all the files to arg_rb_directory.
 *   This is a recursive function.
 * INPUTS
 *   o arg_rb_directory  -- the result.
 *   o arg_path          -- the current path that is scanned.
 *   o arg_doc_path      -- the path to the documentation files.
 * RESULT
 *   a RB_Directory structure filled with all sourcefiles and 
 *   subdirectories in arg_path.
 * NOTE
 *   This a is a recursive function.
 * SOURCE
 */
{
    struct dirent      *a_direntry;
    DIR                *a_dirstream;

    RB_Say( "Scanning %s\n", SAY_INFO, arg_path->name );
    a_dirstream = opendir( arg_path->name );

    if ( a_dirstream )
    {
        T_RB_FileType       file_type;

        for ( a_direntry = readdir( a_dirstream );
              a_direntry; a_direntry = readdir( a_dirstream ) )
        {
            file_type = RB_FileType( arg_path->name, a_direntry );
            if ( file_type == RB_FT_FILE )
            {
                /* It is a regular file. See if it is a sourcefile. */
                if ( RB_Is_Source_File( arg_path, a_direntry->d_name ) )
                {
                    /* It is, so we add it to the directory tree */
                    RB_Directory_Insert_RB_Filename( arg_rb_directory,
                                                     RB_Get_RB_Filename
                                                     ( a_direntry->d_name,
                                                       arg_path ) );
                }
                else
                {
                    /* It's not a sourcefile so we skip it. */
                }
            }
            else if ( file_type == RB_FT_DIRECTORY )
            {
                if ( ( strcmp( ".", a_direntry->d_name ) == 0 ) ||
                     ( strcmp( "..", a_direntry->d_name ) == 0 ) )
                {
                    /* Don't recurse into . or ..
                       because that will result in an infinite */
                }
                else
                {
                    if ( RB_To_Be_Skipped( a_direntry->d_name ) )
                    {
                        /* User asked this directory to be skipped */
                    }
                    else
                    {
                        if ( course_of_action.do_nodesc )
                        {
                            /* Don't descent into the subdirectories */
                        }
                        else
                        {
                            struct RB_Path     *rb_path =
                                RB_Get_RB_Path2( arg_path->name,
                                                 a_direntry->d_name );

                            rb_path->parent = arg_path;
                            if ( ( arg_doc_path
                                   && strcmp( rb_path->name,
                                              arg_doc_path->name ) )
                                 || !arg_doc_path )
                            {
                                RB_Directory_Insert_RB_Path( arg_rb_directory,
                                                             rb_path );
                                RB_Fill_Directory( arg_rb_directory, rb_path,
                                                   arg_doc_path );
                            }
                            else
                            {
                                RB_Say( "skipping %s\n", SAY_INFO,
                                        rb_path->name );
                            }
                        }
                    }
                }
            }
            else
            {
                /* Not a file and also not a directory */
            }
        }
    }
    closedir( a_dirstream );
}

/*****/

/****f* Directory/RB_Is_Source_File
 * NAME
 *   RB_Is_Source_File -- Is a file a sourcefile?
 * SYNOPSIS
 */
int RB_Is_Source_File(
    struct RB_Path *path,
    char *filename )
/*
 * FUNCTION
 *   This functions examines the content of a file to
 *   see whether or not it is a sourcefile.
 *
 *   Currently it checks if there are no nul characters
 *   in the first 8191 characters of the file.
 * SOURCE
 */
{
    int                 is_source = 1;

    if ( RB_Not_Accepted( filename ) )
    {
        /* File needs to be skipped based on it's name */
        is_source = 0;
    }
    else
    {
        unsigned int        size = 0;

        /* Compute the length of the filename including
           the path. */
        size += strlen( path->name );
        size += 1;
        size += strlen( filename );
        if ( size < RB_CBUFFERSIZE )
        {
            FILE               *file;

            /* We use the content_buffer buffer temporarily to
               store the filename. */
            content_buffer[0] = '\0';
            strcat( content_buffer, path->name );
            strcat( content_buffer, filename );
            if ( ( file = fopen( content_buffer, "rb" ) ) )
            {
                int                 no_read;
                no_read =
                    fread( content_buffer, sizeof( char ), RB_CBUFFERSIZE,
                           file );
                if ( no_read > 10 )
                {
                    char               *c;

                    for ( c = content_buffer; no_read; --no_read, c++ )
                    {
                        if ( *c == 0 )
                        {
                            is_source = 0;
                            break;
                        }
                    }
                }
                else
                {
                    /* A file with only 9 characters can not
                       contain any source plus documentation. */
                    is_source = 0;
                }
                fclose( file );
            }
            else
            {
                /* The file could not be opened.   */
                is_source = 0;
            }
        }
        else
        {
            /* The filename is longer than 8191 characters,
               that's way too long... so we skip it. */
            is_source = 0;
        }
    }
    return is_source;
}

/*****/


/****f* Directory/RB_To_Be_Skipped
 * FUNCTION
 *   Test if a file should not be included in the list of source files
 *   that are scanned for documentation. 
 *
 *   This test is done based on the wildcard expressions specified
 *   in configuration.ignore_files.
 * SYNOPSIS
 */
int RB_To_Be_Skipped(
    char *filename )
/*
 * INPUTS
 *   o filename -- the name of the file
 * SOURCE
 */
{
    unsigned int        i;
    int                 skip = FALSE;

    for ( i = 0; i < configuration.ignore_files.number; ++i )
    {
        if ( RB_Match( filename, configuration.ignore_files.names[i] ) )
        {
            skip = TRUE;
            break;
        }
    }
    return skip;
}

/******/


/****f* Directory/RB_Not_Accepted
 * FUNCTION
 *   Test if a file should be skipped, 
 *   because it does not match a pattern in "accept files:"
 *
 *   This test is done based on the wildcard expressions specified
 *   in configuration.accept_files.
 * SYNOPSIS
 */
int RB_Not_Accepted(
    char *filename )
/*
 * INPUTS
 *   o filename -- the name of the file
 * SOURCE
 */
{
    unsigned int        i;
    int                 skip = FALSE;

    skip = RB_To_Be_Skipped( filename );

    if ( !skip && configuration.accept_files.number > 0 )
    {
        skip = TRUE;
        for ( i = 0; i < configuration.accept_files.number; ++i )
        {
            if ( RB_Match( filename, configuration.accept_files.names[i] ) )
            {
                RB_Say( "accept >%s< with >%s<\n", SAY_INFO, filename,
                        configuration.accept_files.names[i] );
                skip = FALSE;
                break;
            }
        }
    }
    return skip;
}

/******/


/****f* Directory/RB_Get_FileName
 * NAME
 *   RB_Get_PathName -- extract the file name 
 * SYNOPSIS
 */
char               *RB_Get_FileName(
    char *arg_fullpath )
/*
 * FUNCTION
 *   Given a full path to a file, that is a filename, or a filename
 *   prefixed with a pathname, return the filename.
 *   So
 *      "./filename"           returns "filename"
 *      "/home/et/filename"    returns "filename"
 *      "filename"             return  "filename"
 * INPUTS
 *   arg_fullpath -- a full path to a file, with or without a path.
 *
 * RESULT
 *   0  -- The full path did not contain a filename
 *   pointer to the filename -- otherwise.
 * NOTES
 *   You are responsible for deallocating it.
 * SOURCE
 */
{
    int                 n;
    int                 i;
    int                 found;
    char               *result = 0;

    assert( arg_fullpath );

    n = strlen( arg_fullpath );

    /* Try and find a path character ( ':' or '/' ) */
    for ( found = FALSE, i = 0; i < n; ++i )
    {
        if ( RB_Is_PathCharacter( arg_fullpath[i] ) )
        {
            found = TRUE;
            break;
        }
    }

    if ( !found )
    {
        /* The full path does not contain a pathname,
         * so we can return the arg_fullpath as
         * the filename.
         */
        result = RB_StrDup( arg_fullpath );
    }
    else
    {
        /* The full path does contain a pathname,
         * strip this and return the remainder
         */

        for ( i = n - 1; i > 0; --i )
        {
            if ( RB_Is_PathCharacter( arg_fullpath[i] ) )
            {
                assert( i < ( n - 1 ) );
                result = RB_StrDup( &( arg_fullpath[i + 1] ) );
                break;
            }
        }
    }

    return result;
}

/********/



/****f* Directory/RB_Get_PathName
 * NAME
 *   RB_Get_PathName -- extract the path name 
 * SYNOPSIS
 */
char               *RB_Get_PathName(
    char *arg_fullpath )
/*
 * FUNCTION
 *   Given a full path to a file, that is a filename, or a filename
 *   prefixed with a pathname, return the pathname.
 *   So
 *      "./filename"           returns "./"
 *      "/home/et/filename"    returns "/home/et/"
 *      "filename"             return  ""
 * INPUTS
 *   arg_fullpath -- a full path to a file, with or without a path.
 *
 * RESULT
 *   0  -- The full path did not contain a path
 *   pointer to the pathname -- otherwise.
 * NOTES
 *   You are responsible for deallocating it.
 * SOURCE
 */
{
    int                 n;
    int                 i;
    int                 found;
    char               *result = 0;

    assert( arg_fullpath );

    n = strlen( arg_fullpath );

    /* Try and find a path character ( ':' or '/' ) */
    for ( found = FALSE, i = 0; i < n; ++i )
    {
        if ( RB_Is_PathCharacter( arg_fullpath[i] ) )
        {
            found = TRUE;
            break;
        }
    }

    if ( found )
    {
        /* Copy the whole file name and then
           replace the character after the 
           first path character found
           counting from the back with a '\0'
         */
        result = RB_StrDup( arg_fullpath );

        for ( i = n - 1; i > 0; --i )
        {
            if ( RB_Is_PathCharacter( result[i] ) )
            {
                assert( i < ( n - 1 ) );
                result[i + 1] = '\0';
                break;
            }
        }
    }

    return result;
}

/*******/


/****f* Directory/RB_Is_PathCharacter
 * FUNCTION
 *   Test if a character is part of the group of
 *   characters that you would normally find in 
 *   a path.
 * SYNOPSIS
 */
static int RB_Is_PathCharacter(
    int c )
/*
 * INPUTS
 *   c -- the character to be tested.
 * RESULT
 *   TRUE  -- it is a path character.
 *   FALSE -- it is not.
 * SOURCE
 */
{
    return ( ( c == ':' ) || ( c == '/' ) );
}

/******/


/* Sort the files and paths */

/* TODO FS Document */
unsigned int RB_Number_Of_Filenames(
    struct RB_Directory *arg_rb_directory )
{
    unsigned int        number_of_filenames = 0;
    struct RB_Filename *rb_filename = NULL;

    for ( rb_filename = arg_rb_directory->first;
          rb_filename;
          ++number_of_filenames, rb_filename = rb_filename->next )
    {
        /* Empty */
    }
    return number_of_filenames;
}


/* TODO FS Document */
unsigned int RB_Number_Of_Paths(
    struct RB_Directory *arg_rb_directory )
{
    unsigned int        number_of_paths = 0;
    struct RB_Path     *rb_path = NULL;

    for ( rb_path = arg_rb_directory->first_path;
          rb_path; ++number_of_paths, rb_path = rb_path->next )
    {
        /* Empty */
    }
    return number_of_paths;
}

/* TODO FS Document */
int RB_Path_Compare(
    void *p1,
    void *p2 )
{
    struct RB_Path     *path_1 = p1;
    struct RB_Path     *path_2 = p2;

    return RB_Str_Case_Cmp( path_1->name, path_2->name );
}

/* TODO FS Document */
int RB_Filename_Compare(
    void *p1,
    void *p2 )
{
    struct RB_Filename *filename_1 = p1;
    struct RB_Filename *filename_2 = p2;

    return RB_Str_Case_Cmp( filename_1->name, filename_2->name );
}


/* TODO FS Document */
void RB_SortDirectory(
    struct RB_Directory *arg_rb_directory )
{
    unsigned int        number_of_filenames =
        RB_Number_Of_Filenames( arg_rb_directory );
    unsigned int        number_of_paths =
        RB_Number_Of_Paths( arg_rb_directory );
    unsigned int        i;
    struct RB_Path     *rb_path = NULL;
    struct RB_Filename *rb_filename = NULL;
    struct RB_Path    **paths = NULL;
    struct RB_Filename **filenames = NULL;

    assert( number_of_filenames > 0 );
    assert( number_of_paths > 0 );
    paths = calloc( number_of_paths, sizeof( struct RB_Path * ) );
    filenames = calloc( number_of_filenames, sizeof( struct RB_Filename * ) );


    RB_Say( "Sorting Directory\n", SAY_INFO );
    for ( i = 0, rb_path = arg_rb_directory->first_path;
          rb_path; rb_path = rb_path->next )
    {
        assert( i < number_of_paths );
        paths[i] = rb_path;
        ++i;
    }

    for ( i = 0, rb_filename = arg_rb_directory->first;
          rb_filename; rb_filename = rb_filename->next )
    {
        assert( i < number_of_filenames );
        filenames[i] = rb_filename;
        ++i;
    }

    RB_QuickSort( ( void ** ) paths, 0, number_of_paths - 1,
                  RB_Path_Compare );
    RB_QuickSort( ( void ** ) filenames, 0, number_of_filenames - 1,
                  RB_Filename_Compare );

    for ( i = 0; i < number_of_paths - 1; ++i )
    {
        paths[i]->next = paths[i + 1];
    }
    paths[number_of_paths - 1]->next = NULL;
    arg_rb_directory->first_path = paths[0];

    for ( i = 0; i < number_of_filenames - 1; ++i )
    {
        filenames[i]->next = filenames[i + 1];
    }
    filenames[number_of_filenames - 1]->next = NULL;
    arg_rb_directory->first = filenames[0];
    arg_rb_directory->last = filenames[number_of_filenames - 1];

    free( paths );
    free( filenames );
}
