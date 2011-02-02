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



/****h* ROBODoc/HeaderTypes
 * FUNCTION
 *    Headers come in different types.  This module defines what kind
 *    of headertypes ROBODoc recognizes, and contains functions to add
 *    new headertypes and to compare headertypes.  All the headertypes
 *    are stored in an array, header_type_lookup_table.
 ******
 */

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include "headertypes.h"
#include "util.h"

/****v* HeaderTypes/header_type_lookup_table
 * FUNCTION
 *   A lookup table for all the header types that ROBODoc recognizes.
 *   At the moment is has about 127 entries.  About as many as there
 *   are characters in the standard ASCII set.  The first 32 entries
 *   can be used for special purposes.
 *
 *   Two of them are use:
 *     HT_MASTERINDEXTYPE
 *   and
 *     HT_SOURCEHEADERTYPE
 *
 *   HT_MASTERINDEXTYPE is a wildcard type. All headertypes match this
 *   type.  This is used to collect all the headers for the
 *   masterindex.
 *
 *   HT_SOURCEHEADERTYPE is used to pretend that the name of
 *   a sourcefile is a kind of header.  This makes it possible to
 *   include the names of the source files in the master index.
 * SOURCE
 */

struct RB_HeaderType header_type_lookup_table[MAX_HEADER_TYPE + 1] = {
    {'\0', NULL, NULL, 0},
    {HT_SOURCEHEADERTYPE, "Sourcefiles", "robo_sourcefiles", 0},
    {HT_MASTERINDEXTYPE, "Index", "masterindex", 0},    /* no robo_ prefix for backwards compatibility */
    {'\0', NULL, NULL, 0},
    {'\0', NULL, NULL, 0},
    {'\0', NULL, NULL, 0},
    {'\0', NULL, NULL, 0},
    {'\0', NULL, NULL, 0},
    {'\0', NULL, NULL, 0},
    {'\0', NULL, NULL, 0},
    {'\0', NULL, NULL, 0},
    {'\0', NULL, NULL, 0},
    {'\0', NULL, NULL, 0},
    {'\0', NULL, NULL, 0},
    {'\0', NULL, NULL, 0},
    {'\0', NULL, NULL, 0},
    {'\0', NULL, NULL, 0},
    {'\0', NULL, NULL, 0},
    {'\0', NULL, NULL, 0},
    {'\0', NULL, NULL, 0},
    {'\0', NULL, NULL, 0},
    {'\0', NULL, NULL, 0},
    {'\0', NULL, NULL, 0},
    {'\0', NULL, NULL, 0},
    {'\0', NULL, NULL, 0},
    {'\0', NULL, NULL, 0},
    {'\0', NULL, NULL, 0},
    {'\0', NULL, NULL, 0},
    {'\0', NULL, NULL, 0},
    {'\0', NULL, NULL, 0},
    {'\0', NULL, NULL, 0},
    {'\0', NULL, NULL, 0},
    {' ', NULL, NULL, 0},
    {'!', NULL, NULL, 0},
    {'"', NULL, NULL, 0},
    {'#', NULL, NULL, 0},
    {'$', NULL, NULL, 0},
    {'%', NULL, NULL, 0},
    {'&', NULL, NULL, 0},
    {'\0', NULL, NULL, 0},
    {'(', NULL, NULL, 0},
    {')', NULL, NULL, 0},
    {'*', "Generics", "robo_generics", 0},
    {'+', NULL, NULL, 0},
    {',', NULL, NULL, 0},
    {'-', NULL, NULL, 0},
    {'.', NULL, NULL, 0},
    {'/', NULL, NULL, 0},
    {'0', NULL, NULL, 0},
    {'1', NULL, NULL, 0},
    {'2', NULL, NULL, 0},
    {'3', NULL, NULL, 0},
    {'4', NULL, NULL, 0},
    {'5', NULL, NULL, 0},
    {'6', NULL, NULL, 0},
    {'7', NULL, NULL, 0},
    {'8', NULL, NULL, 0},
    {'9', NULL, NULL, 0},
    {':', NULL, NULL, 0},
    {';', NULL, NULL, 0},
    {'<', NULL, NULL, 0},
    {'=', NULL, NULL, 0},
    {'>', NULL, NULL, 0},
    {'?', NULL, NULL, 0},
    {'@', NULL, NULL, 0},
    {'A', NULL, NULL, 0},
    {'B', "Businessrules", "robo_businessrules", 0},
    {'C', "Contracts", "robo_contracts", 0},
    {'D', "Datasources", "robo_datasources", 0},
    {'E', "Ensure contracts", "robo_ensure_contracts", 0},
    {'F', NULL, NULL, 0},
    {'G', NULL, NULL, 0},
    {'H', NULL, NULL, 0},
    {'I', "Invariants", "robo_invariants", 0},
    {'J', NULL, NULL, 0},
    {'K', NULL, NULL, 0},
    {'L', NULL, NULL, 0},
    {'M', "Metadata", "robo_metadata", 0},
    {'N', NULL, NULL, 0},
    {'O', NULL, NULL, 0},
    {'P', "Process", "robo_processes", 0},
    {'Q', NULL, NULL, 0},
    {'R', "Require contracts", "robo_require_contracts", 0},
    {'S', "Subjects", "robo_subjects", 0},
    {'T', NULL, NULL, 0},
    {'U', NULL, NULL, 0},
    {'V', NULL, NULL, 0},
    {'W', NULL, NULL, 0},
    {'X', NULL, NULL, 0},
    {'Y', NULL, NULL, 0},
    {'Z', NULL, NULL, 0},
    {'[', NULL, NULL, 0},
    {'\0', NULL, NULL, 0},      /* Separator /  */
    {']', NULL, NULL, 0},
    {'^', NULL, NULL, 0},
    {'_', NULL, NULL, 0},
    {'`', NULL, NULL, 0},
    {'a', NULL, NULL, 0},
    {'b', NULL, NULL, 0},
    {'c', "Classes", "robo_classes", 0},
    {'d', "Definitions", "robo_definitions", 0},
    {'e', "Exceptions", "robo_exceptions", 0},
    {'f', "Functions", "robo_functions", 0},
    {'g', NULL, NULL, 0},
    {'h', "Modules", "robo_modules", 1},
    {'\0', NULL, NULL, 0},      /* Internal header flag */
    {'j', NULL, NULL, 0},
    {'k', NULL, NULL, 0},
    {'l', NULL, NULL, 0},
    {'m', "Methods", "robo_methods", 0},
    {'n', NULL, NULL, 0},
    {'o', NULL, NULL, 0},
    {'p', "Procedures", "robo_procedures", 0},
    {'q', NULL, NULL, 0},
    {'r', NULL, NULL, 0},
    {'s', "Structures", "robo_strutures", 0},
    {'t', "Types", "robo_types", 0},
    {'u', "Unittest", "robo_unittests", 0},
    {'v', "Variables", "robo_variables", 0},
    {'w', "Warehouses", "robo_warehouses", 0},
    {'x', NULL, NULL, 0},
    {'y', NULL, NULL, 0},
    {'z', NULL, NULL, 0},
    {'{', NULL, NULL, 0},
    {'|', NULL, NULL, 0},
    {'}', NULL, NULL, 0},
    {'~', NULL, NULL, 0},
    {'\0', NULL, NULL, 0}
};

/*****/


/****f* HeaderTypes/RB_AddHeaderType
 * FUNCTION
 *   Add a new headertype to the list of header type
 *   that robodoc recognizes.
 * RESULT
 *   * FALSE -- it is a new header type.
 *   * TRUE  -- header type already existed.
 * SOURCE
 */

int RB_AddHeaderType(
    unsigned int typeCharacter,
    char *indexName,
    char *fileName,
    unsigned int priority )
{
    if ( header_type_lookup_table[typeCharacter].typeCharacter )
    {
        header_type_lookup_table[typeCharacter].indexName = indexName;
        header_type_lookup_table[typeCharacter].fileName = fileName;
        header_type_lookup_table[typeCharacter].priority = priority;
    }
    else
    {
        RB_Panic
            ( "The character %c is not allowed as a headertype character\n",
              typeCharacter );
    }
    /* Unused */
    return 0;
}

/*****/



/****f* HeaderTypes/RB_CompareHeaderTypes
 * FUNCTION
 *   Compare two header types and check if they are equal.  If one of
 *   the header types is a HT_MASTERINDEXTYPE the comparison is always
 *   TRUE.  (This to make sure that all headers appear in the Index.)
 * SYNOPSIS
 *    int RB_CompareHeaderTypes( 
 *    struct RB_HeaderType* ht1, struct RB_HeaderType* ht2 )
 * INPUTS
 *    o ht1 and ht2 -- the header types to compare.
 * RESULT
 *    o 0     -- header types are not equal
 *    o != 0  -- header type are equal
 * SOURCE
 */

int RB_CompareHeaderTypes(
    struct RB_HeaderType *ht1,
    struct RB_HeaderType *ht2 )
{
    assert( ht1 );
    assert( ht2 );
    return ( ht1->typeCharacter == HT_MASTERINDEXTYPE ) ||
        ( ht2->typeCharacter == HT_MASTERINDEXTYPE ) ||
        ( ht1->typeCharacter == ht2->typeCharacter );
}

/******/



/****f* HeaderTypes/RB_FindHeaderType
 * FUNCTION
 *   Return the header type that corresponds to the type character.
 * RESULT
 *   * 0  -- there is no such header type
 *   * pointer to the header type otherwise.
 * SOURCE
 */

struct RB_HeaderType *RB_FindHeaderType(
    unsigned char typeCharacter )
{
    struct RB_HeaderType *headertype = 0;

    if ( typeCharacter < MAX_HEADER_TYPE )
    {
        headertype = &( header_type_lookup_table[typeCharacter] );
        if ( ( headertype->typeCharacter == typeCharacter ) &&
             ( headertype->indexName ) )
        {
            return headertype;
        }
    }
    return 0;
}

/*****/


/****f* HeaderTypes/RB_IsInternalHeader
 * FUNCTION
 *   Given the typeCharacter is this an internal header?
 * RESULT
 *   * TRUE  -- yes it is
 *   * FALSE -- no it is not
 * SOURCE
 */

int RB_IsInternalHeader(
    unsigned char type_character )
{
    return ( type_character == 'i' );
}

/*****/



#if 0
char               *RB_GetIndexName(
    struct RB_HeaderType *ht1 )
{
    /* TODO  should be used to access indexName */
    return 0;
}
#endif
