#ifndef ROBODOC_UTIL_H
#define ROBODOC_UTIL_H
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


/* TODO all the functions that operate
 on headers should be in headers.h */

#include "robodoc.h"
#include "headers.h"
#include "document.h"

typedef int         (
    *TCompare )         (
    void *,
    void * );


void                RB_SetCurrentFile(
    char *filename );
char               *RB_GetCurrentFile(
     );

char               *RB_FilePart(
    char * );
void                RB_Analyse_Defaults_File(
    void );

void                RB_Slow_Sort(
    void );
void                RB_Reverse_List(
    void );
void                RB_Insert_In_List(
    struct RB_header **,
    struct RB_header * );
void                RB_Remove_From_List(
    struct RB_header **,
    struct RB_header * );
struct RB_header   *RB_Alloc_Header(
    void );
void                RB_Free_Header(
    struct RB_header * );
char               *RB_StrDup(
    char * );
char               *RB_StrDupLen(
    char *str,
    size_t length );
char               *RB_CookStr(
    char * );
void                RB_Say(
    char *,
    long mode,
    ... );
void                RB_Warning_Full(
    char *arg_filename,
    int arg_line_number,
    char *arg_format,
    ... );
void                RB_Warning(
    char *format,
    ... );
void                RB_Panic(
    char *,
    ... );
int                 RB_Str_Case_Cmp(
    char *s,
    char *t );
void                RB_TimeStamp(
    FILE *f );
char               *RB_Skip_Whitespace(
    char *buf );

void               *RB_Calloc(
    size_t count,
    size_t size );
void               *RB_Malloc(
    size_t size );
void                RB_Free(
    void *p );
void                RB_FputcLatin1ToUtf8(
    FILE *fp,
    int c );
void                RB_CopyFile(
    char *sourceFileName,
    char *destinationFileName );
int                 RB_Match(
    char *target,
    char *wildcard_expression );
void                RB_QuickSort(
    void **array,
    int left,
    int right,
    TCompare f );
void                RB_StripCR(
    char *line );
int                 Stat_Path(
    char required,
    char *path );

char               *ExpandTab(
    char *line );

int                 RB_ContainsNL(
    char *line );
char               *RB_ReadWholeLine(
    FILE *file,
    char *buf,
    int *arg_readChars );
void                RB_FreeLineBuffer(
     );

void               *RB_malloc(
    size_t bytes );

void                RB_Change_To_Docdir(
    char * );
void                RB_Change_Back_To_CWD(
    void );

FILE               *RB_Open_Pipe(
    char *pipe_name );
void                RB_Close_Pipe(
    FILE *arg_pipe );

FILE               *RB_Open_File(
    char *,
    char * );
void                RB_Close_File(
    FILE * );


void                _RB_Mem_Check(
    void *ptr,
    const char *fname,
    const char *funcname,
    int linenum );



int                 utf8_isalnum(
    unsigned int arg_c );
int                 utf8_isalpha(
    unsigned int arg_c );
int                 utf8_iscntrl(
    unsigned int arg_c );
int                 utf8_isdigit(
    unsigned int arg_c );
int                 utf8_isgraph(
    unsigned int arg_c );
int                 utf8_islower(
    unsigned int arg_c );
int                 utf8_isprint(
    unsigned int arg_c );
int                 utf8_ispunct(
    unsigned int arg_c );
int                 utf8_isspace(
    unsigned int arg_c );
int                 utf8_isxdigit(
    unsigned int arg_c );

#ifndef HAVE_SNPRINTF
int                 snprintf(
    char *,
    size_t,
    const char *,
    ... );
#endif

/****f* Utilities/RB_Mem_Check
 * FUNCTION
 *   Check for memory allocation failures.
 * SOURCE
 */

#define RB_Mem_Check(ptr) if (!ptr) RB_Panic("%s:%s:%d: Out of memory!\n", __FILE__, __FUNCTION__, __LINE__)

/*******/

#endif /* ROBODOC_UTIL_H */
