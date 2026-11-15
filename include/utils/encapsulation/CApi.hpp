/**************************************************************\

 ██╗  ██╗ █████╗ ██████╗ ████████╗ █████╗ ███╗   ██╗██╗ █████╗
 ╚██╗██╔╝██╔══██╗██╔══██╗╚══██╔══╝██╔══██╗████╗  ██║██║██╔══██╗
  ╚███╔╝ ███████║██████╔╝   ██║   ███████║██╔██╗ ██║██║███████║
  ██╔██╗ ██╔══██║██╔══██╗   ██║   ██╔══██║██║╚██╗██║██║██╔══██║
 ██╔╝ ██╗██║  ██║██║  ██║   ██║   ██║  ██║██║ ╚████║██║██║  ██║
 ╚═╝  ╚═╝╚═╝  ╚═╝╚═╝  ╚═╝   ╚═╝   ╚═╝  ╚═╝╚═╝  ╚═══╝╚═╝╚═╝  ╚═╝
Edition:
##  @date 15/11/2026 by @author Tsukini

File Name:
##  @file CApi.hpp

File Description:
##  Handles of the C API of utils::encapsulation (usable in c++ to get the object)
\**************************************************************/

// xstyle: ignore-file CPP-CLASS-NAME (C handle: C types are snake_case)

#ifndef ENCAPSULATION_CAPI_H
    #define ENCAPSULATION_CAPI_H

    //----------------------------------------------------------------//
    /* INCLUDE */

    /* type */
    #include "../exception/CApi.hpp"    // utils_handle
    #include "SharedObject.hpp"         // utils::encapsulation::SharedObject
    #include "SharedMemory.hpp"         // utils::encapsulation::SharedMemory, utils::encapsulation::shm::Id
    #include "Process.hpp"              // utils::encapsulation::Process
    #include "Pipe.hpp"                 // utils::encapsulation::Pipe
    #include "Poll.hpp"                 // utils::encapsulation::Poll
    #include "Dup.hpp"                  // utils::encapsulation::Dup
    #include "CApi.h"                   // utils_dup_t, utils_pipe_t, utils_poll_t, utils_process_t, utils_shared_object_t, utils_shm_t, utils_shm_messages_t
    #include <cstddef>                  // std::byte
    #include <utility>                  // std::pair
    #include <string>                   // std::string
    #include <vector>                   // std::vector

//----------------------------------------------------------------//
/* STRUCT */

/* handles of the C API (the object itself) */
struct utils_dup: public utils_handle, public utils::encapsulation::Dup {
    // ---------- Constructor --------- //
    utils_dup(const int origin, const int clone): utils::encapsulation::Dup(origin, clone) { /* Nothing */ };
};

struct utils_pipe: public utils_handle, public utils::encapsulation::Pipe {
    // ---------- Constructor --------- //
    utils_pipe(const int read, const int write): utils::encapsulation::Pipe(read, write) { /* Nothing */ };
};

struct utils_poll: public utils_handle, public utils::encapsulation::Poll {
    // ---------- Constructor --------- //
    utils_poll(const int flags): utils::encapsulation::Poll(flags) { /* Nothing */ };
};

struct utils_process: public utils_handle, public utils::encapsulation::Process {};

struct utils_shared_object: public utils_handle, public utils::encapsulation::SharedObject {
    // ---------- Constructor --------- //
    utils_shared_object(const std::string& path): utils::encapsulation::SharedObject(path) { /* Nothing */ };
};

struct utils_shm: public utils_handle, public utils::encapsulation::SharedMemory {
    // ---------- Constructor --------- //
    utils_shm(const pid_t ownership): utils::encapsulation::SharedMemory(ownership) { /* Nothing */ };
};

struct utils_shm_messages: public utils_handle {
    std::vector<std::pair<utils::encapsulation::shm::Id, std::vector<std::vector<std::byte>>>> messages;
};

#endif /* ENCAPSULATION_CAPI_H */
