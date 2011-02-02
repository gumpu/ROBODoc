#ifndef ROBODOC_LINKS_H
#define ROBODOC_LINKS_H

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


#include "headertypes.h"
#include "headers.h"
#include "document.h"

/****s* Links/RB_link
 *  NAME
 *    RB_link -- link data structure
 *  PURPOSE
 *    Structure to store links to the documentation of an component.
 *  ATTRIBUTES
 *    * label_name  -- the label under which the component can be found.
 *                     this should be a unique name.
 *    * object_name -- the proper name of the object
 *    * file_name   -- the file the component can be found in.
 *    * type        -- the type of component (the header type).
 *    * is_internal -- is the header an internal header?
 *  SOURCE
 */

struct RB_link
{
    char               *label_name;
    char               *object_name;
    char               *file_name;
    struct RB_HeaderType *htype;
    int                 is_internal;
};

/*********/

int                 Find_Link(
    char *word_begin,
    char **object_name,
    char **unique_name,
    char **file_name );
void                RB_CollectLinks(
    struct RB_Document *document,
    struct RB_header **headers,
    unsigned long count );
void                RB_Free_Links(
    void );
void                RB_Free_Link(
    struct RB_link *arg_link );

int                 RB_Number_Of_Links(
    struct RB_HeaderType *header_type,
    char *file_name,
    int internal );

#endif /* ROBODOC_LINKS_H */
