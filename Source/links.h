#ifndef ROBODOC_LINKS_H
#define ROBODOC_LINKS_H

/*
 * This file is part of ROBODoc, See COPYING for the license.
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
