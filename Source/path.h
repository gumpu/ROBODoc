#ifndef ROBODOC_PATH_H
#define ROBODOC_PATH_H
/*
 * This file is part of ROBODoc, See COPYING for the license.
*/


/****s* ROBODoc/RB_Path
 * NAME
 *   RB_Path -- Path to a file
 * ATTRIBUTES
 *   * next  -- pointer to the next RB_Path structure.
 *   * parent -- the parent path (one directory up).
 *   * name  -- null terminated string with the name of the path.
 *               (Path names can be relative)
 *   * docname -- the corresponding docpath.
 * SOURCE
 */

struct RB_Path
{
    struct RB_Path     *next;
    struct RB_Path     *parent;
    char               *name;
    char               *docname;
};

/*****/

struct RB_Path     *RB_Get_RB_Path(
    char *arg_pathname );
struct RB_Path     *RB_Get_RB_Path2(
    char *arg_current_path,
    char *arg_subdirectory );
void                RB_Free_RB_Path(
    struct RB_Path *arg_rb_path );

#endif /* ROBODOC_PATH_H */
