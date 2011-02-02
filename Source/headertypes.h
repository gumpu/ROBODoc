#ifndef ROBODOC_HEADERTYPES_H
#define ROBODOC_HEADERTYPES_H

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

/****s* Headers/RB_HeaderType
 * NAME
 *   RB_HeaderType -- Information about a header type
 * ATTRIBUTES
 *   o typeCharacter -- The character used to indicate it 
 *   o indexName     -- The name used for the master index 
 *   o fileName      -- The name of the file use to store 
 *                      the master index for this type of headers.
 *   o priority      -- The sorting priority of this header.
 *                      Higher priorities appear first
 * SOURCE
 */

struct RB_HeaderType
{
    unsigned char       typeCharacter;
    char               *indexName;
    char               *fileName;
    unsigned int        priority;
};

/*******/

#define HT_SOURCEHEADERTYPE ((unsigned char)1)
#define HT_MASTERINDEXTYPE  ((unsigned char)2)

#define MIN_HEADER_TYPE 1       /* ' ' */
#define MAX_HEADER_TYPE 127


int                 RB_AddHeaderType(
    unsigned int typeCharacter,
    char *indexName,
    char *indexFile,
    unsigned int priority );
struct RB_HeaderType *RB_FindHeaderType(
    unsigned char typeCharacter );
void                RB_InitHeaderTypes(
    void );
int                 RB_IsInternalHeader(
    unsigned char type_character );
int                 RB_CompareHeaderTypes(
    struct RB_HeaderType *ht1,
    struct RB_HeaderType *ht2 );

#endif /* ROBODOC_HEADERTYPES_H */
