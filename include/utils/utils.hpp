/**************************************************************\
Edition:
##  @date 20/07/2026 by @author Tsukini

File Name:
##  @file utils.hpp

File Description:
##  Main include for every part of the utils lib
\**************************************************************/

//----------------------------------------------------------------//
/* DEFINE */

/* Desactivate all automatic warning & _deprecated message */
#ifdef _NoWarning
    #define NO_DEPRECATED_WARNING // _deprecated(...)
    #define NO_LINKER_WARNING // Linker requirement
    //#define NO_SHARED_OBJECT_WARNING // shared object
    //#warning "All warnings from 'utils/utils.hpp' are disable"
#endif

// Disabling of the auto warning for shared other
#if defined(NO_SHARED_OBJECT_WARNING) && !defined(_NoWarning)
    #warning "Automatic shared object warnings from 'utils/utils.hpp' are disable"
#endif

/* default: if nothing is defined - include everything */
#if !defined(_Handling) && !defined(_Attribute) && !defined(_Tools) && \
    /* _Handling */\
    !defined(_Exception) && !defined(_Write) && !defined(_Verbose) && !defined(_Arguments) && !defined(_Cli) && \
    /* _Tools */\
    !defined(_BLT) && !defined(_Vector) && !defined(_Concepts) && !defined(_Middleware) && !defined(_Algorithms) && \
    /* _Algorithms */\
    !defined(_C2DMP) && !defined(_SOS)

    #define _Utils
#endif

/* Activate all include */
#ifdef _Utils
    #define _Handling
    #define _Attribute
    #define _Tools
#endif

/* Activate all handling include */
#ifdef _Handling
    #define _Exception
    #define _Manip
    #define _Verbose
    #define _Arguments
    #define _Cli
#endif

/* Activate all tool include */
#ifdef _Tools
    #define _BLT // Bidirectional Lookup Table
    #define _Vector
    #define _Concepts
    #define _Middleware
    #define _Algorithms
#endif

/* Activate all manip include */
#ifdef _Manip
    #define _IOManip
    #define _SManip
#endif

/* Activate all the algorithms */
#ifdef _Algorithms
    #define _C2DMP
    #define _SOS
#endif

//----------------------------------------------------------------//
/* INCLUDE */

/* Exception */
#ifdef _Exception
    // -> Customized exception used for the error/warning handling
    #include "exception/IException.hpp"             // utils::exception::IException
    #include "exception/ExceptionDefine.hpp"        // OK/KO, utils::exception::Code, utils::exception::Type
    #include "exception/basic/NoneException.hpp"    // utils::exception::NoneException
    #include "exception/basic/ErrorException.hpp"   // utils::exception::ErrorException
    #include "exception/basic/WarningException.hpp" // utils::exception::WarningException
    #include "exception/custom/CustomException.hpp" // utils::exception::CustomException
#endif

/* Input/Output - Manip */
#ifdef _IOManip
    // -> Handling of input/output manipulation
    // -> Define some special char & other, ANSI escape sequences, ...
    #include "manip/iomanip/Color.hpp" // utils::iomanip::Color, utils::iomanip::BackColor
    #include "manip/iomanip/Char.hpp"  // utils::iomanip::Char
    #include "manip/iomanip/Style.hpp" // utils::iomanip::Style, utils::iomanip::ResetStyle
    #include "manip/iomanip/ANSI.hpp"  // different inline function for ANSI escape sequences
#endif

/* String - Manip */
#ifdef _SManip
    // -> Handling of string manipulation
    #include "manip/smanip/format.hpp"      // utils::smanip::format
    #include "manip/smanip/codec/Codec.hpp" // utils::smanip::codec::ICodec, utils::smanip::codec::*Codec
#endif

/* Cli */
#ifdef _Cli
    // -> Handling of a customizable command line interface
    #include "cli/Cli.hpp"          // utils::cli::Cli, utils::cli::ParsedData
    #include "cli/Flags.hpp"        // utils::cli::Flag, utils::cli::Flags
#endif

/* Macro */
#ifdef _Attribute
    // -> Define/Macro of attribute & keyword for optimisation & other
    #include "attribute/Attribute.hpp"  // different define/macro for optimisation & other
#endif

/* Verbose */
#ifdef _Verbose
    // -> Define/Macro of verbose usage
    #include "verbose/Verbose.hpp"  // different define/macro for verbose usage
#endif

/* Bidirectional Lookup Table */
#ifdef _BLT
    // -> Bidirectional lookup table (const)
    #include "blt/BidirectionalLookupTable.hpp" // utils::blt::BidirectionalLookupTable<L, R, ...>, utils::blt::BidirectionalLookupTable<T, ...>
    /*
    #include "blt/BidirectionalLookupTable_t-t.hpp" // utils::blt::BidirectionalLookupTable<L, R, ...>
    #include "blt/BidirectionalLookupTable_t.hpp"   // utils::blt::BidirectionalLookupTable<T, ...>
    */
#endif

/* VectorX */
#ifdef _Vector
    // -> Customized vector2 & vector3 for respectively 2 & 3 value of undefined type
    #include "vector/IVector.hpp"   // utils::vector::IVector<T>
    #include "vector/Vector2.hpp"   // utils::vector::Vector2<T>
    #include "vector/Vector3.hpp"   // utils::vector::Vector3<T>
    #include "vector/OVector2.hpp"  // utils::vector::OVector2<T>
    #include "vector/OVector3.hpp"  // utils::vector::OVector3<T>
#endif

/* Concepts */
#ifdef _Concepts
    // -> Definition of different concepts
    #include "concepts/GlobalConcepts.hpp"      // Global concepts
    #include "concepts/OperationConcepts.hpp"   // Operation concepts
#endif

/* Middleware */
#ifdef _Middleware
    // -> Middlewares handling
    #include "middleware/Middlewares.hpp"   // utils::middleware::Middleware<...>, utils::middleware::Middlewares<...>
    /*
    #include "middleware/MiddlewaresType.hpp"       // utils::middleware::Middleware<...>
    #include "middleware/Middlewares_t-t.hpp"       // utils::middleware::Middlewares<T, U>
    #include "middleware/Middlewares_t-void.hpp"    // utils::middleware::Middlewares<T, void>
    #include "middleware/Middlewares_void-t.hpp"    // utils::middleware::Middlewares<void, T>
    #include "middleware/Middlewares_void-void.hpp" // utils::middleware::Middlewares<void, void>
    */
#endif

/* Arguments */
#ifdef _Arguments
    // -> Definition of different thing to handle arguments
    // -> ArgParser: basic check & parsing of arguments
    #include "arguments/ArgParser.hpp"      // utils::arguments::ArgParser
    #include "arguments/ArgParserType.hpp"  // utils::arguments::* (Type)
    #include "arguments/Settings.hpp"       // utils::arguments::Settings (Setting are sub class not public and indidualy used)
    #include "arguments/SettingsDefine.hpp" // utils::arguments::CastType
#endif

/* c2dmp-hsm */
#ifdef _C2DMP
    // -> c2dmp-hsm: heuristic string matching
    #include "algorithms/c2dmp-hsm/c2dmp-hsm.hpp"               // utils::algorithms::c2dmp::c2dmp
    #include "algorithms/c2dmp-hsm/algorithm/optimized.hpp"     // utils::algorithms::c2dmp::c2dmp_optimized
    #include "algorithms/c2dmp-hsm/algorithm/simplified.hpp"    // utils::algorithms::c2dmp::c2dmp_simplified
    #include "algorithms/c2dmp-hsm/algorithm/foptimized.hpp"    // utils::algorithms::c2dmp::c2dmp_foptimized
    #include "algorithms/c2dmp-hsm/algorithm/fsimplified.hpp"   // utils::algorithms::c2dmp::c2dmp_fsimplified
#endif

/* s.o.s */
#ifdef _SOS
    // -> s.o.s: steganography optimized and securized
    #include "algorithms/sos/sos.hpp"                           // utils::algorithms::sos::sos_embed, utils::algorithms::sos::sos_extract, utils::algorithms::sos::tools::* (convert tools), utils::algorithms::sos::* (default type)
    #include "algorithms/sos/algorithm/embed_optimized.hpp"     // utils::algorithms::sos::sos_embed_optimized
    #include "algorithms/sos/algorithm/extract_optimized.hpp"   // utils::algorithms::sos::sos_extract_optimized
    #include "algorithms/sos/tools/convert.hpp"                 // utils::algorithms::sos::to_bytes, utils::algorithms::sos::bytes_to
#endif
