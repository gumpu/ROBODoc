#ifndef ROBODOC_PATH_H
#define ROBODOC_PATH_H
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
