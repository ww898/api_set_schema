/*
MIT License

Copyright (c) 2019 Mikhail Pilin

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#pragma once

#include <minwindef.h>

// References:
//   https://blog.quarkslab.com/runtime-dll-name-resolution-apisetschema-part-i.html
//   https://blog.quarkslab.com/runtime-dll-name-resolution-apisetschema-part-ii.html
//   https://www.geoffchappell.com/studies/windows/win32/apisetschema/index.htm

#define API_SET_FLAG_SEALED 0x01
#define API_SET_FLAG_EXT    0x02

///////////////////////////////////////////////////////////////////////////////
// ApiSet v2

struct API_SET_VALUE_ENTRY_REDIRECTION_V2
{
    ULONG  NameOffset;
    USHORT NameLength;
    ULONG  ValueOffset;
    USHORT ValueLength;
};

struct API_SET_VALUE_ENTRY_V2
{
    ULONG                              NumberOfRedirections;
    API_SET_VALUE_ENTRY_REDIRECTION_V2 Redirections[ANYSIZE_ARRAY];
};

struct API_SET_NAMESPACE_ENTRY_V2
{
    ULONG NameOffset;
    ULONG NameLength;
    ULONG DataOffset; // API_SET_VALUE_ENTRY_V2 offset
};

struct API_SET_NAMESPACE_V2
{
    ULONG                      Version;
    ULONG                      Count;
    API_SET_NAMESPACE_ENTRY_V2 Array[ANYSIZE_ARRAY];
};

///////////////////////////////////////////////////////////////////////////////
// ApiSet v4

struct API_SET_VALUE_ENTRY_REDIRECTION_V4
{
    ULONG Flags;
    ULONG NameOffset;
    ULONG NameLength;
    ULONG ValueOffset;
    ULONG ValueLength;
};

struct API_SET_VALUE_ENTRY_V4
{
    ULONG                              Flags;
    ULONG                              NumberOfRedirections;
    API_SET_VALUE_ENTRY_REDIRECTION_V4 Redirections[ANYSIZE_ARRAY];
};

struct API_SET_NAMESPACE_ENTRY_V4
{
    ULONG Flags;
    ULONG NameOffset;
    ULONG NameLength;
    ULONG AliasOffset;
    ULONG AliasLength;
    ULONG DataOffset;  // API_SET_VALUE_ENTRY_V4 offset
};

struct API_SET_NAMESPACE_V4
{
    ULONG                      Version;
    ULONG                      Size;
    ULONG                      Flags;
    ULONG                      Count;
    API_SET_NAMESPACE_ENTRY_V4 Array[ANYSIZE_ARRAY];
};

///////////////////////////////////////////////////////////////////////////////
// ApiSet v6

struct API_SET_HASH_ENTRY_V6
{
    ULONG Hash;
    ULONG Index;
};

struct API_SET_VALUE_ENTRY_V6
{
    ULONG Flags;
    ULONG NameOffset;
    ULONG NameLength;
    ULONG ValueOffset;
    ULONG ValueLength;
};

struct API_SET_NAMESPACE_ENTRY_V6
{
    ULONG Flags;
    ULONG NameOffset;
    ULONG NameLength;
    ULONG HashedLength;
    ULONG ValueOffset;  // API_SET_VALUE_ENTRY_V6 offset
    ULONG ValueCount;
};

struct API_SET_NAMESPACE_V6
{
    ULONG Version;     // Should be 6
    ULONG Size;
    ULONG Flags;
    ULONG Count;
    ULONG EntryOffset; // API_SET_NAMESPACE_ENTRY_V6 offset
    ULONG HashOffset;  // API_SET_HASH_ENTRY_V6 offset
    ULONG HashFactor;
};

///////////////////////////////////////////////////////////////////////////////

struct API_SET_NAMESPACE
{
    union
    {
        ULONG Version;
        API_SET_NAMESPACE_V2 V2;
        API_SET_NAMESPACE_V4 V4;
        API_SET_NAMESPACE_V6 V6;
    };
};
