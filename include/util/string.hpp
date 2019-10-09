#pragma once

#include <string_view>

inline static bool string_ends_with(const std::string_view str, const std::string_view suffix)
{
    return str.size() >= suffix.size()
        && str.substr(str.size() - suffix.size(), suffix.size()) == suffix;
}