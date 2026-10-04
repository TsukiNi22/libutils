/**************************************************************\

 ██╗  ██╗ █████╗ ██████╗ ████████╗ █████╗ ███╗   ██╗██╗ █████╗ 
 ╚██╗██╔╝██╔══██╗██╔══██╗╚══██╔══╝██╔══██╗████╗  ██║██║██╔══██╗
  ╚███╔╝ ███████║██████╔╝   ██║   ███████║██╔██╗ ██║██║███████║
  ██╔██╗ ██╔══██║██╔══██╗   ██║   ██╔══██║██║╚██╗██║██║██╔══██║
 ██╔╝ ██╗██║  ██║██║  ██║   ██║   ██║  ██║██║ ╚████║██║██║  ██║
 ╚═╝  ╚═╝╚═╝  ╚═╝╚═╝  ╚═╝   ╚═╝   ╚═╝  ╚═╝╚═╝  ╚═══╝╚═╝╚═╝  ╚═╝

Edition:
##  @date 20/07/2026 by @author Tsukini

File Name:
##  @file ANSI.cpp

File Description:
##  Different ANSI method definition
\**************************************************************/

#include "utils/attribute/Attribute.hpp"
#include "utils/exception/ExceptionDefine.hpp"
#include "utils/exception/basic/ErrorException.hpp"
#include "utils/manip/iomanip/ANSI.hpp"
#include "utils/manip/iomanip/Style.hpp"
#include <unistd.h>
#include <initializer_list>
#include <stdexcept>
#include <iostream>
#include <sstream>
#include <utility>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <format>
#include <string>

_hot _nodiscard std::string utils::iomanip::reset_style(std::initializer_list<utils::iomanip::ResetStyle> styles)
{
    std::string codes;

    bool first = true;
    for (const utils::iomanip::ResetStyle& style: styles) {
        if (!first) codes += ";";
        codes += std::to_string(static_cast<std::uint8_t>(style));
        first = false;
    }

    // Empty list -> reset all style
    if (styles.size() == 0)
        codes = "0";

    return std::format("{}[{}m", static_cast<char>(utils::iomanip::Char::ESC), codes);
}

_hot _nodiscard std::string utils::iomanip::set_style(std::initializer_list<utils::iomanip::Style> styles)
{
    std::string codes;

    bool first = true;
    for (const utils::iomanip::Style& style: styles) {
        if (!first) codes += ";";
        codes += std::to_string(static_cast<std::uint8_t>(style));
        first = false;
    }

    return std::format("{}[{}m", static_cast<char>(utils::iomanip::Char::ESC), codes);
}

// Report format -> "ESC[rows;colsR"
_cold _nodiscard std::pair<int, int> utils::iomanip::read_cursor_position(void)
{
    char buffer[32] = {'\0'};
    std::size_t i = 0;

    // Get the answer from the term
    for (; i < sizeof(buffer) - 1; ++i) {
        if (::read(STDIN_FILENO, &buffer[i], 1) != 1)
            break;
        if (buffer[i] == 'R')
            break;
    }
    buffer[i] = '\0';

    // Get the values
    int rows = 0, cols = 0;
    if (std::sscanf(buffer, "\x1b[%d;%dR", &rows, &cols) != 2)
        return {-1, -1};
    return {rows, cols};
}

// Report format -> "ESC[M" Cb Cx Cy (3 raw bytes, each value + 32)
_cold _nodiscard utils::iomanip::MouseEvent utils::iomanip::read_mouse_event(void)
{
    utils::iomanip::MouseEvent event;
    char c = '\0';

    // Search the start of the sequence: ESC [ M
    std::string header;
    for (std::size_t i = 0; header != "\x1b[M" && std::cin.get(c) && i < 128; ++i) { // Limitation of 128 char to counter infinite possible loop
        if (c == static_cast<char>(utils::iomanip::Char::ESC)) header = c;
        else if (!header.empty()) header += c;
        if (header.size() > 3) header.clear();
    }
    if (header != "\x1b[M")
        throw utils::exception::ErrorException(utils::exception::InternalCode::ANSIMouseEvent, header.empty() ? "No mouse sequence received" : "Invalid classic mouse format");

    // Get the 3 raw bytes: button, x, y
    unsigned char data[3] = {0, 0, 0};
    for (std::size_t i = 0; i < 3; ++i) {
        if (!std::cin.get(c))
            throw utils::exception::ErrorException(utils::exception::InternalCode::ANSIMouseEvent, "Mouse sequence too short");
        data[i] = static_cast<unsigned char>(c);
        if (data[i] < 32)
            throw utils::exception::ErrorException(utils::exception::InternalCode::ANSIMouseEvent, "Invalid classic mouse value (each value is sent + 32)");
    }
    int cb = data[0] - 32;
    event.x = data[1] - 32;
    event.y = data[2] - 32;

    // Convert the button value
    switch (cb & 0b11) {
        case 0: event.button = utils::iomanip::MouseButton::Left;        break;
        case 1: event.button = utils::iomanip::MouseButton::Middle;      break;
        case 2: event.button = utils::iomanip::MouseButton::Right;       break;
        case 3: event.button = utils::iomanip::MouseButton::Release;     break;
        default: event.button = utils::iomanip::MouseButton::Unknown;    break;
    }

    return event;
}

// Report format -> "ESC[<b;x;y(M|m)"
_cold _nodiscard utils::iomanip::AdvancedMouseEvent utils::iomanip::read_advanced_mouse_event(void)
{
    utils::iomanip::AdvancedMouseEvent event;
    std::string buffer;
    char c = '\0';

    // Get the input until 'M' or 'm'
    bool started = false;
    for (std::size_t i = 0; std::cin.get(c) && i < 128; ++i) { // Limitation of 128 char to counter infinite possible loop
        if (c == static_cast<char>(utils::iomanip::Char::ESC)) started = true;
        if (started) buffer += c;
        if (started && (c == 'M' || c == 'm')) break;
    }

    // Check the buffer
    if (buffer.empty())
        throw utils::exception::ErrorException(utils::exception::InternalCode::ANSIMouseEvent, "No mouse sequence received");
    if (buffer.size() < 8)
        throw utils::exception::ErrorException(utils::exception::InternalCode::ANSIMouseEvent, "Mouse sequence too short");
    if (buffer.find("[<") == std::string::npos)
        throw utils::exception::ErrorException(utils::exception::InternalCode::ANSIMouseEvent, "Invalid SGR mouse format");

    // Button status
    event.pressed = (buffer.back() == 'M');

    // Get the start of the data
    std::size_t start = buffer.find('<');

    // Setup the data extraction
    std::stringstream ss(buffer.substr(start + 1));
    std::string token;

    // Get the different data
    int cb = 0;
    try {
        if (!std::getline(ss, token, ';'))
            throw std::runtime_error("Missing button field");
        cb = std::stoi(token);
        if (!std::getline(ss, token, ';'))
            throw std::runtime_error("Missing X field");
        event.x = std::stoul(token);
        if (!std::getline(ss, token, ';'))
            throw std::runtime_error("Missing Y field");
        event.y = std::stoul(token);
    } catch (const std::exception& e) {
        throw utils::exception::ErrorException(utils::exception::InternalCode::ANSIMouseEvent, std::format("{}: {}", "Failed parsing SGR mouse event", e.what()));
    }

    // Convert the button value
    switch (cb & 0b11) {
        case 0: event.button = utils::iomanip::MouseButton::Left;        break;
        case 1: event.button = utils::iomanip::MouseButton::Middle;      break;
        case 2: event.button = utils::iomanip::MouseButton::Right;       break;
        case 3: event.button = utils::iomanip::MouseButton::Release;     break;
        default: event.button = utils::iomanip::MouseButton::Unknown;    break;
    }

    return event;
}
