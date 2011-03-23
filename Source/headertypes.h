#ifndef ROBODOC_HEADERTYPES_H
#define ROBODOC_HEADERTYPES_H

/*
 * This file is part of ROBODoc, See COPYING for the license.
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
