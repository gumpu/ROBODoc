#ifndef ROBODOC_FILE_H
#define ROBODOC_FILE_H
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
#include "path.h"
#include "links.h"

/****s* Filename/RB_Filename
 * NAME
 *   RB_Filename --
 * ATTRIBUTES
 *   * next   pointer to the next RB_File.
 *   * name   null terminated string with the name of the file,
 *            (Without the path, but including the extension).
 *   * fullname 
 *   * path   pointer to a RB_Path structure that holds
 *            the path for this file.
 *   * link   The link used to represent this file while in multidoc
 *            mode.
 * SOURCE
 */

struct RB_Filename
{
    struct RB_Filename *next;
    char               *name;
    char               *docname;
    char               *fullname;
    char               *fulldocname;
    struct RB_Path     *path;
    struct RB_link     *link;
};

/******/


struct RB_Filename *RB_Get_RB_Filename(
    char *arg_filename,
    struct RB_Path *arg_rb_path );
void                RB_Free_RB_Filename(
    struct RB_Filename *arg_rb_filename );

/* */
char               *Get_Fullname(
    struct RB_Filename *arg_rb_filename );
void                RB_Set_FullDocname(
    struct RB_Filename *arg_rb_filename,
    char *name );
char               *RB_Get_FullDocname(
    struct RB_Filename *arg_rb_filename );
char               *RB_Get_Path(
    struct RB_Filename *arg_rb_filename );
char               *RB_Get_Filename(
    struct RB_Filename *arg_rb_filename );
char               *RB_Get_Extension(
    struct RB_Filename *arg_rb_filename );
struct RB_Filename *RB_Copy_RB_Filename(
    struct RB_Filename *arg_rb_filename );

/* */
void                RB_Filename_Dump(
    struct RB_Filename *arg_rb_filename );


#endif /* ROBODOC_FILE_H */
