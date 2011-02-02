#ifndef ROBODOC_DIRECTORY_H
#define ROBODOC_DIRECTORY_H
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


#include "file.h"

/****s* Directory/RB_Directory
 * NAME
 *   RB_Directory -- the directory tree with the source files.
 * FUNCTION
 *   Stores information about files in a directory tree.
 *   The whole structure consist of two linked lists.  One for
 *   directory paths, and one for filenames.
 * EXAMPLE
 *   The following show an example structure.
 *     RB_Directory             RB_Path
 *     +-------+   +------+    +-------+   +-----------+
 *     |       +-->| .    |--->| ./sub |-->| ./sub/sub |
 *     |       |   +------+    +-------+   +-----------+
 *     |       |      ^              ^            ^
 *     |       |      |              |            |
 *     |       |      |-----------+  +------+     +------+
 *     |       |      |           |         |            |
 *     |       |      |           |         |            |
 *     |       |   +------+    +------+   +------+    +-------+
 *     |       +-->|  a.c |--->| b.c  |-->| sa.c  |-->| ssb.c |
 *     +-------+   +------+    +------+   +------+    +-------+
 *                  RB_Filename
 *
 * ATTRIBUTES
 *   * first    --  first RB_Filename in the list of files
 *   * current  --  the last file that was returned in
 *                  RB_Get_Next_Filename.
 *   * last     --  the last RB_Filename in the list of files
 *                   used for the insert operation
 *   * first_path -- first RB_Path in the list of paths.
 * SOURCE
 */

struct RB_Directory
{
    struct RB_Filename *first;  /* TODO should be called files */
    struct RB_Filename *last;
    struct RB_Path     *first_path;     /* TODO should be called paths */
};

/******/


/****t* Directory/T_RB_FileType
 * FUNCTION
 *   Constants for the two different filetypes that
 *   ROBODoc recognizes.
 * SOURCE
 */

typedef enum
{
    RB_FT_DIRECTORY = 1,
    RB_FT_FILE = 2,
    RB_FT_UNKNOWN = 3
} T_RB_FileType;

/******/


struct RB_Directory *RB_Get_RB_Directory(
    char *arg_rootpath,
    char *arg_docroot_name );
struct RB_Directory *RB_Get_RB_SingleFileDirectory(
    char *arg_fullpath );
void                RB_Dump_RB_Directory(
    struct RB_Directory *arg_rb_directory );
void                RB_Free_RB_Directory(
    struct RB_Directory *arg_directory );
void                RB_Directory_Insert_RB_Path(
    struct RB_Directory *arg_rb_directory,
    struct RB_Path *arg_rb_path );
void                RB_Directory_Insert_RB_Filename(
    struct RB_Directory *arg_rb_directory,
    struct RB_Filename *arg_rb_filename );

void                RB_Fill_Directory(
    struct RB_Directory *arg_rb_directory,
    struct RB_Path *arg_path,
    struct RB_Path *arg_doc_path );
int                 RB_Is_Source_File(
    struct RB_Path *path,
    char *filename );
int                 RB_To_Be_Skipped(
    char *filename );
int                 RB_Not_Accepted(
    char *filename );


char               *RB_Get_FileName(
    char *arg_fullpath );
char               *RB_Get_PathName(
    char *arg_fullpath );
void                RB_SortDirectory(
    struct RB_Directory *arg_rb_directory );

int                 RB_Path_Compare(
    void *p1,
    void *p2 );
int                 RB_Filename_Compare(
    void *p1,
    void *p2 );
unsigned int        RB_Number_Of_Filenames(
    struct RB_Directory *arg_rb_directory );
unsigned int        RB_Number_Of_Paths(
    struct RB_Directory *arg_rb_directory );

#endif /* ROBODOC_DIRECTORY_H */
