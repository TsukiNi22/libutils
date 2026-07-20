/**************************************************************\
Edition:
##  @date 23/03/2026 by @author Tsukini

File Name:
##  @file Char.hpp

File Description:
##  Definition of some special char
\**************************************************************/

#ifndef CHAR_H
    #define CHAR_H

namespace utils::iomanip { // namespace start
//----------------------------------------------------------------//
/* TYPEDEF */

/* special chars */
enum class Char: char {
    #ifndef EOF
        // -> Can potentialy be already defined by default with 'stdio.h'
        EOF = -1,
    #endif
    NUL = 0,   // Null
    SOH = 1,   // Start of Header
    STX = 2,   // Start of Text
    ETX = 3,   // End of Text
    EOT = 4,   // End of Transmission
    ENQ = 5,   // Enquiry
    ACK = 6,   // Acknowledge
    BEL = 7,   // Bell
    BS  = 8,   // Backspace
    HT  = 9,   // Horizontal Tab
    LF  = 10,  // Line Feed
    VT  = 11,  // Vertical Tab
    FF  = 12,  // Form Feed
    CR  = 13,  // Carriage Return
    SO  = 14,  // Shift Out
    SI  = 15,  // Shift In
    DLE = 16,  // Data Link Escape
    DC1 = 17,  // Device Control 1
    DC2 = 18,  // Device Control 2
    DC3 = 19,  // Device Control 3
    DC4 = 20,  // Device Control 4
    NAK = 21,  // Negative Acknowledge
    SYN = 22,  // Synchronous Idle
    ETB = 23,  // End of Transmission Block
    CAN = 24,  // Cancel
    EM  = 25,  // End of Medium
    SUB = 26,  // Substitute
    ESC = 27,  // Escape
    FS  = 28,  // File Separator
    GS  = 29,  // Group Separator
    RS  = 30,  // Record Separator
    US  = 31,  // Unit Separator
    DEL = 127,  // Delete
};

} // namespace end
#endif /* CHAR_H */
