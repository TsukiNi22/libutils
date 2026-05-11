/**************************************************************\
Edition:
##  @date 27/04/2026 by @author Tsukini

File Name:
##  @file utils.hpp

File Description:
##  Main include for every part of the utils lib
\**************************************************************/

//----------------------------------------------------------------//
/* DEFINE */

/* Desactivate all automatic warning & outdated message */
#ifdef _NoWarning
    //#define NO_SHARED_OBJECT_WARNING // shared other
#endif

// Disabling of the auto warning for shared other
/*
#if defined(NO_SHARED_OBJECT_WARNING) && !defined(_WARNING_NO_SHARED_OBJECT_WARNING)
    #define _WARNING_NO_SHARED_OBJECT_WARNING
    #warning "Automatic shared object warnings from 'utils/utils.hpp' are disable"
#endif
*/

/* Activate all include */
#ifdef _Utils
    #define _Handling
    #define _Attribute
    #define _Tools
#endif

/* Activate all handling include */
#ifdef _Handling
    #define _Exception
    #define _Write
    #define _Cli
#endif

/* Activate all tool include */
#ifdef _Tools
    #define _Vector
    #define _Concepts
    #define _Algorithms
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

/* Write */
#ifdef _Write
    // -> Handling of the ANSI escape sequences
    // -> Define some special char & other
    #include "write/Color.hpp"  // utils::write::Color, utils::write::BackColor
    #include "write/Char.hpp"   // utils::write::Char
    #include "write/Style.hpp"  // utils::write::Style, utils::write::ResetStyle
    #include "write/ANSI.hpp"   // different inline function for ANSI escape sequences
    #include "write/format.hpp" // utils::write::format
#endif

/* Cli */
#ifdef _Cli
    // -> Handling of a customizable command line interface
    #include "cli/Cli.hpp"          // utils::cli::Cli, utils::cli::ParsedData
    #include "cli/Flags.hpp"        // utils::cli::Flag, utils::cli::Flags
    #include "cli/Middlewares.hpp"  // utils::cli::Middlewares
#endif

/* Macro */
#ifdef _Attribute
    // -> Define/Macro of attribute & keyword for optimisation & other
    #include "attribute/Attribute.hpp"  // different define/macro for optimisation & other
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

/* Algorithms */
#ifdef _Algorithms
    // -> Definition of different algorithms
    // -> c2dmp-hsm: heuristic string matching
    #include "algorithms/c2dmp-hsm/c2dmp-hsm.hpp"               // utils::algorithms::c2dmp::c2dmp
    #include "algorithms/c2dmp-hsm/algorithm/optimized.hpp"     // utils::algorithms::c2dmp::c2dmp_optimized
    #include "algorithms/c2dmp-hsm/algorithm/simplified.hpp"    // utils::algorithms::c2dmp::c2dmp_simplified
#endif
