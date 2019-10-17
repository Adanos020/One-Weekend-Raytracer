#pragma once

#include <string_view>

inline static bool string_ends_with(const std::string_view str, const std::string_view suffix)
{
    return str.size() >= suffix.size()
        && str.substr(str.size() - suffix.size(), suffix.size()) == suffix;
}

inline static void string_replace_all(std::string& data, const std::string_view to_replace, const std::string_view replace_with)
{
    size_t pos = data.find(to_replace);
    while (pos != std::string::npos)
    {
        data.replace(pos, to_replace.size(), replace_with);
        pos = data.find(to_replace, pos + replace_with.size());
    }
}