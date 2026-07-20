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

#include "utils/exception/ExceptionDefine.hpp"
#include "utils/exception/basic/ErrorException.hpp" 
#include "utils/manip/iomanip/ANSI.hpp"
#include "utils/manip/iomanip/Style.hpp"
#include <unistd.h>
#include <initializer_list>
#include <iostream>
#include <sstream>
#include <cstdint>
#include <format>
#include <string>

std::string utils::iomanip::resetStyle(std::initializer_list<utils::iomanip::ResetStyle> styles)
{
    std::string s_styles;

    bool first = true;
    for (const utils::iomanip::ResetStyle& style : styles) {
        if (!first) s_styles += ";";
        s_styles += std::to_string(static_cast<std::uint8_t>(style));
        first = false;
    }

    // Empty list -> reset all style
    if (styles.size() == 0)
        s_styles = "0";

    return std::format("{}[{}m", static_cast<char>(utils::iomanip::Char::ESC), s_styles);
}

std::string utils::iomanip::setStyle(std::initializer_list<utils::iomanip::Style> styles)
{
    std::string s_styles;

    bool first = true;
    for (const utils::iomanip::Style& style : styles) {
        if (!first) s_styles += ";";
        s_styles += std::to_string(static_cast<std::uint8_t>(style));
        first = false;
    }

    return std::format("{}[{}m", static_cast<char>(utils::iomanip::Char::ESC), s_styles);
}

// Report format -> "ESC[rows;colsR"
std::pair<int, int> utils::iomanip::readCursorPosition(void)
{
    char buffer[32] = {'\0'};
    std::size_t i = 0;

    // Get the awnser from the term
    for (; i < sizeof(buffer) - 1; ++i) {
        if (read(STDIN_FILENO, &buffer[i], 1) != 1)
            break;
        if (buffer[i] == 'R')
            break;
    }
    buffer[i] = '\0';

    // Get the values
    int rows = 0, cols = 0;
    if (sscanf(buffer, "\x1b[%d;%dR", &rows, &cols) != 2)
        return {-1, -1};
    return {rows, cols};
}

// Report format -> "ESC[Mb;x;y"
utils::iomanip::MouseEvent utils::iomanip::readMouseEvent(void)
{
    utils::iomanip::MouseEvent event;
    std::string buffer;
    char c = '\0';

    // Get the input
    bool started = false;
    for (std::size_t i = 0; std::cin.get(c) && i < 128; ++i)
    {
        if (c == static_cast<char>(utils::iomanip::Char::ESC)) started = true;
        if (started) buffer += c;
        if (started && std::isdigit(c)) continue;
        if (started && c == '\n') break;
    }

    // Check the buffer
    if (buffer.size() < 6)
        throw utils::exception::ErrorException(utils::exception::Code::ANSIMouseEvent, "Mouse sequence too short");
    if (buffer.find("[M") == std::string::npos)
        throw utils::exception::ErrorException(utils::exception::Code::ANSIMouseEvent, "Invalid classic mouse format");

    // Get the start of the data
    auto start = buffer.find('M');

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
        throw utils::exception::ErrorException(utils::exception::Code::ANSIMouseEvent, std::format("{}: {}", "Failed parsing classic mouse event", e.what()));
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

// Report format -> "ESC[<b;x;y(M|m)"
utils::iomanip::AdvancedMouseEvent utils::iomanip::readAdvancedMouseEvent(void)
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
        throw utils::exception::ErrorException(utils::exception::Code::ANSIMouseEvent, "No mouse sequence received");
    if (buffer.size() < 8)
        throw utils::exception::ErrorException(utils::exception::Code::ANSIMouseEvent, "Mouse sequence too short");
    if (buffer.find("[<") == std::string::npos)
        throw utils::exception::ErrorException(utils::exception::Code::ANSIMouseEvent, "Invalid SGR mouse format");

    // Button status
    event.pressed = (buffer.back() == 'M');

    // Get the start of the data
    auto start = buffer.find('<');

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
        throw utils::exception::ErrorException(utils::exception::Code::ANSIMouseEvent, std::format("{}: {}", "Failed parsing classic mouse event", e.what()));
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
