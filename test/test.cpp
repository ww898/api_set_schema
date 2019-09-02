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

#define _WIN32_WINNT 0x0600 //_WIN32_WINNT_VISTA

#define NOMINMAX

#include <sdkddkver.h>
#include <windows.h>

#include <ww898/api_set_schema.h>

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <string_view>
#include <iomanip>

struct api_set_flags
{
    explicit api_set_flags(ULONG const flags, bool const hide_empty = false) : flags_(flags), hide_empty_(hide_empty) {}

    template<typename Out>
    Out & operator()(Out & out) const
    {
        auto first = true;
        auto const append = [&out, &first] (auto msg)
        {
            if (first)
                first = false;
            else
                out << L',';
            out << msg;
        };

        if (hide_empty_ && flags_ == 0)
            return out;

        out << L'(';
        if (flags_ & API_SET_FLAG_SEALED)
            append(L"sealed");
        if (flags_ & API_SET_FLAG_EXT)
            append(L"ext");
        out << L')';
        return out;
    }

private:
    ULONG const flags_;
    bool const hide_empty_;
};

struct api_set_hash
{
    explicit api_set_hash(ULONG const hash) : hash_(hash) {}

    template<typename Out>
    Out & operator()(Out & out) const
    {
        return out << L"0x" << std::hex << std::uppercase << std::setw(8) << std::setfill(L'0') << hash_ << std::dec;
    }

private:
    ULONG const hash_;
};

template<typename Out>
Out & operator<<(Out & out, api_set_flags flags) { return flags(out); }

template<typename Out>
Out & operator<<(Out & out, api_set_hash hash) { return hash(out); }

template <typename Result>
Result const * get_address(void const * const base, ULONG const offset) noexcept
{
    return reinterpret_cast<Result const *>(reinterpret_cast<char const *>(base) + offset);
}

inline ULONG calculate_hash(void const * const base, ULONG const factor, ULONG const offset, ULONG length)
{
    length /= sizeof(WCHAR);
    ULONG hash = 0;
    for (auto str = get_address<wchar_t>(base, offset); length-- > 0; ++str)
        hash = hash * factor + *str;
    return hash;
}

inline std::wstring_view to_string_view(void const * const base, ULONG const offset, ULONG const length) noexcept
{
    return std::wstring_view(get_address<wchar_t>(base, offset), length / sizeof(wchar_t));
}

void dump(std::wostream & out, API_SET_NAMESPACE_V2 const * const ns, size_t const max_size)
{
    if (max_size < sizeof(API_SET_NAMESPACE_V2))
        throw std::runtime_error("Too small api set v2 file");
    out << L"Version: " << ns->Version << std::endl
        << L"Count: " << ns->Count << std::endl;
    ULONG n = 0;
    for (auto it = ns->Array, eit = it + ns->Count; it < eit; ++it, ++n)
    {
        auto const value = get_address<API_SET_VALUE_ENTRY_V2>(ns, it->DataOffset);
        out << n << L'|' << to_string_view(ns, it->NameOffset, it->NameLength) << L" -> [";
        auto first = true;
        for (auto it2 = value->Redirections, eit2 = it2 + value->NumberOfRedirections; it2 < eit2; ++it2)
            if (it2->ValueLength)
            {
                if (first)
                    first = false;
                else
                    out << L',';
                out << to_string_view(ns, it2->ValueOffset, it2->ValueLength);
            }
        out << L"]" << std::endl;
    }
}

void dump(std::wostream & out, API_SET_NAMESPACE_V4 const * const ns, size_t const max_size)
{
    if (max_size < sizeof(API_SET_NAMESPACE_V4) || max_size < ns->Size)
        throw std::runtime_error("Too small api set v4 file");
    out << L"Version: " << ns->Version << std::endl
        << L"Size: " << ns->Size << std::endl
        << L"Flags: " << api_set_flags(ns->Flags) << std::endl
        << L"Count: " << ns->Count << std::endl;
    ULONG n = 0;
    for (auto it = ns->Array, eit = it + ns->Count; it < eit; ++it, ++n)
    {
        auto const value = get_address<API_SET_VALUE_ENTRY_V4>(ns, it->DataOffset);
        out << n << L'|' << to_string_view(ns, it->NameOffset, it->NameLength) << api_set_flags(it->Flags, true) << L" -> [";
        auto first = true;
        for (auto it2 = value->Redirections, eit2 = it2 + value->NumberOfRedirections; it2 < eit2; ++it2)
            if (it2->ValueLength)
            {
                if (first)
                    first = false;
                else
                    out << L',';
                out << to_string_view(ns, it2->ValueOffset, it2->ValueLength) << api_set_flags(it2->Flags, true);
            }
        out << L"]" << std::endl;
    }
}

void dump(std::wostream & out, API_SET_NAMESPACE_V6 const * const ns, size_t const max_size)
{
    if (max_size < sizeof(API_SET_NAMESPACE_V6) || max_size < ns->Size)
        throw std::runtime_error("Too small api set v6 file");
    out << L"Version: " << ns->Version << std::endl
        << L"Size: " << ns->Size << std::endl
        << L"Flags: " <<  api_set_flags(ns->Flags) << std::endl
        << L"Count: " << ns->Count << std::endl
        << L"HashFactor: " << ns->HashFactor << std::endl;
    ULONG n = 0;
    for (auto it = get_address<API_SET_NAMESPACE_ENTRY_V6>(ns, ns->EntryOffset), eit = it + ns->Count; it < eit; ++it, ++n)
    {
        out << n << L'|' << api_set_hash(calculate_hash(ns, ns->HashFactor, it->NameOffset, it->HashedLength))
            << L'|' << to_string_view(ns, it->NameOffset, it->HashedLength) << L'{' << to_string_view(ns, it->NameOffset + it->HashedLength, it->NameLength - it->HashedLength) << L"}"
            << api_set_flags(it->Flags, true) << L" -> [";
        auto first = true;
        for (auto it2 = get_address<API_SET_VALUE_ENTRY_V6>(ns, it->ValueOffset), eit2 = it2 + it->ValueCount; it2 < eit2; ++it2)
            if (it2->ValueLength)
            {
                if (first)
                    first = false;
                else
                    out << L',';
                out << to_string_view(ns, it2->ValueOffset, it2->ValueLength) << api_set_flags(it2->Flags, true);
            }
        out << L"]" << std::endl;
    }
    for (auto it = get_address<API_SET_HASH_ENTRY_V6>(ns, ns->HashOffset), eit = it + ns->Count; it < eit; ++it)
        out << api_set_hash(it->Hash) << L" -> " << it->Index << std::endl;
}

int wmain(int const argc, wchar_t const * const argv[])
{
    try
    {
        if (argc != 2)
            throw std::invalid_argument("Invalid argument, should be the path to the file with an api set schema");

        std::vector<char> data;

        {
            std::ifstream in;
            in.exceptions(std::ifstream::failbit | std::ifstream::badbit | std::ifstream::eofbit);
            in.open(argv[1], std::ios::binary | std::ios::ate);
            data.resize(static_cast<size_t>(in.tellg()));
            if (data.size() < sizeof(ULONG))
                throw std::runtime_error("Too small file");
            in.seekg(0).read(data.data(), data.size());
        }

        auto const ns = reinterpret_cast<API_SET_NAMESPACE const *>(data.data());
        switch (ns->Version)
        {
        case 2: dump(std::wcout, &ns->V2, data.size()); break;
        case 4: dump(std::wcout, &ns->V4, data.size()); break;
        case 6: dump(std::wcout, &ns->V6, data.size()); break;
        default:
            throw std::runtime_error("Unknown api set version");
        }

        return 0;
    }
    catch (std::exception const & ex)
    {
        std::cerr << "ERROR: " << ex.what() << std::endl;
        return 1;
    }
    catch (...)
    {
        std::cerr << "ERROR: Unknown" << std::endl;
        return 1;
    }
}
