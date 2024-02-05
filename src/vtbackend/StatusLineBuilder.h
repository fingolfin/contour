// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <vtbackend/Color.h>

#include <string>
#include <variant>
#include <vector>

#include "vtbackend/CellFlags.h"

namespace vtbackend
{

namespace StatusLineDefinitions
{
    struct Styles
    {
        CellRGBColorAndAlphaPair colors;
        CellFlags flags;
    };

    // clang-format off
    struct AnsiCursorLocation { Styles styles; };
    struct AppTitle { Styles styles; };
    struct CellSGR { Styles styles; };
    struct CellTextUtf32 { Styles styles; };
    struct CellTextUtf8 { Styles styles; };
    struct Clock { Styles styles; };
    struct Hyperlink { Styles styles; };
    struct InputMode { Styles styles; };
    struct MouseCursorLocation { Styles styles; };
    struct Search { std::string prompt; Styles styles; };
    struct ShellCommand { std::string command; Styles styles; };
    struct Text { std::string text; Styles styles; };
    struct VTType { Styles styles; };

    using Item = std::variant<
        AnsiCursorLocation,
        AppTitle,
        CellSGR,
        CellTextUtf32,
        CellTextUtf8,
        Clock,
        Hyperlink,
        InputMode,
        MouseCursorLocation,
        Search,
        ShellCommand,
        Text,
        VTType
    >;
    // clang-format on
} // namespace StatusLineDefinitions

using StatusLineSegment = std::vector<StatusLineDefinitions::Item>;

struct StatusLineDefinition
{
    StatusLineSegment left;
    StatusLineSegment middle;
    StatusLineSegment right;
};

// "{Clock:Bold,Italic,Color=#FFFF00} | {VTType} | {InputMode} {Search:Bold,Color=Yellow}"
StatusLineSegment parseStatusLineSegment(std::string_view text);

class Terminal;
std::string serializeToVT(Terminal const& vt, StatusLineDefinitions::Item const& item);

} // namespace vtbackend
