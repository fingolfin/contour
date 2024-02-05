// SPDX-License-Identifier: Apache-2.0
#include <vtbackend/StatusLineBuilder.h>
#include <vtbackend/Terminal.h>

#include <crispy/interpolated_string.h>
#include <crispy/utils.h>

namespace vtbackend
{

std::optional<StatusLineDefinitions::Item> makeStatusLineItem(
    crispy::interpolated_string_fragment const& fragment)
{
    using namespace StatusLineDefinitions;
    if (std::holds_alternative<std::string_view>(fragment))
        return StatusLineDefinitions::Text(std::string(std::get<std::string_view>(fragment)));

    auto const& interpolation = std::get<crispy::string_interpolation>(fragment);

    if (interpolation.name == "AnsiCursorLocation")
        return StatusLineDefinitions::AnsiCursorLocation {};

    if (interpolation.name == "AppTitle")
        return StatusLineDefinitions::AppTitle {};

    if (interpolation.name == "Cell")
    {
        if (interpolation.flags.count("SGR"))
            return StatusLineDefinitions::CellSGR {};
        if (interpolation.flags.count("UTF-32"))
            return StatusLineDefinitions::CellTextUtf32 {};
        if (interpolation.flags.count("UTF-8"))
            return StatusLineDefinitions::CellTextUtf8 {};
    }

    if (interpolation.name == "Clock")
        return StatusLineDefinitions::Clock {};

    if (interpolation.name == "Hyperlink")
        return StatusLineDefinitions::Hyperlink {};

    if (interpolation.name == "InputMode")
        return StatusLineDefinitions::InputMode {};

    if (interpolation.name == "MousePosition")
        return StatusLineDefinitions::MouseCursorLocation {};

    if (interpolation.name == "Search")
        return StatusLineDefinitions::Search {
            .prompt = std::string(interpolation.attributes.at("prompt")),
            .styles = {},
        };

    if (interpolation.name == "Shell")
        return StatusLineDefinitions::ShellCommand {
            .command = std::string(interpolation.attributes.at("command")),
        };

    if (interpolation.name == "Text")
        return StatusLineDefinitions::Text {
            .text = std::string(interpolation.attributes.at("text")),
        };

    if (interpolation.name == "VTType")
        return StatusLineDefinitions::VTType {};

    return std::nullopt;
}

StatusLineSegment parseStatusLineSegment(std::string_view text)
{
    auto segment = StatusLineSegment {};

    // Parses a string like:
    // "{Clock:Bold,Italic,Color=#FFFF00} | {VTType} | {InputMode} {Search:Bold,Color=Yellow}"

    auto const interpolations = crispy::parse_interpolated_string(text);

    for (auto const& fragment: interpolations)
    {
        if (std::holds_alternative<std::string_view>(fragment))
        {
            using namespace StatusLineDefinitions;
            segment.emplace_back(
                StatusLineDefinitions::Text(std::string(std::get<std::string_view>(fragment))));
        }
        else if (auto const item = makeStatusLineItem(std::get<crispy::string_interpolation>(fragment)))
        {
            segment.emplace_back(*item);
        }
    }

    return segment;
}

struct VTSerializer
{
    Terminal const& vt;
    std::string result {};

    std::string operator()(StatusLineDefinitions::Item const& item)
    {
        std::visit([this](auto const& item) { visit(item); }, item);
        return result;
    }

    std::string operator()(StatusLineSegment const& segment)
    {
        std::string result;
        for (auto const& item: segment)
            result += std::visit(*this, item);
        return result;
    }

    // {{{
    void visit(StatusLineDefinitions::AnsiCursorLocation const& item)
    {
        crispy::ignore_unused(item);
        auto const position = vt.currentScreen().cursor().position;
        result += fmt::format("{}:{}", position.line, position.column);
    }

    void visit(StatusLineDefinitions::AppTitle const& item)
    {
        crispy::ignore_unused(item);
        result += vt.windowTitle();
    }

    void visit(StatusLineDefinitions::CellSGR const& item)
    {
        crispy::ignore_unused(item);
        result += "CellSGR"; // TODO(pr)
    }

    void visit(StatusLineDefinitions::CellTextUtf32 const& item)
    {
        crispy::ignore_unused(item);
        result += "CellTextUtf32"; // TODO(pr)
    }

    void visit(StatusLineDefinitions::CellTextUtf8 const& item)
    {
        crispy::ignore_unused(item);
        result += "CellTextUtf8"; // TODO(pr)
    }

    void visit(StatusLineDefinitions::Clock const& item)
    {
        crispy::ignore_unused(item);
        result += "Clock"; // TODO(pr)
    }

    void visit(StatusLineDefinitions::Hyperlink const& item)
    {
        crispy::ignore_unused(item);
        result += "Hyperlink"; // TODO(pr)
    }

    void visit(StatusLineDefinitions::InputMode const& item)
    {
        crispy::ignore_unused(item);
        result += "InputMode"; // TODO(pr)
    }

    void visit(StatusLineDefinitions::MouseCursorLocation const& item)
    {
        crispy::ignore_unused(item);
        auto const position = vt.currentMousePosition();
        result += fmt::format("{}:{}", position.line, position.column);
    }

    void visit(StatusLineDefinitions::Search const& item)
    {
        crispy::ignore_unused(item);
        result += "Search";
    }

    void visit(StatusLineDefinitions::ShellCommand const& item)
    {
        result += item.command; // TODO(pr)
    }

    void visit(StatusLineDefinitions::Text const& item)
    {
        result += item.text;
    }

    void visit(StatusLineDefinitions::VTType const& item)
    {
        crispy::ignore_unused(item);
        result += fmt::format("{}", vt.state().terminalId);
    }
    // }}}
};

std::string serializeToVT(Terminal const& vt, StatusLineSegment const& segment)
{
    auto serializer = VTSerializer { vt };
    for (auto const& item: segment)
        serializer(item);
    return serializer.result;
}

} // namespace vtbackend
