#ifndef ROBODOC_FILE_H
#define ROBODOC_FILE_H
/*
 * This file is part of ROBODoc, See COPYING for the license.
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
