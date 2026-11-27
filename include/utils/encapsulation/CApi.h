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
##  @file CApi.h

File Description:
##  C API of utils::encapsulation: Dup, Pipe, Poll, Process, SharedObject & SharedMemory (C & C++ compatible)
##  Every function that can fail returns UTILS_OK / UTILS_KO (or NULL / 0 / -1), details in utils_exception
\**************************************************************/

// xstyle: ignore-file CPP-GUARD-NAME (C header of the module: <MODULE>_CAPI_C_H like the other C API)
// xstyle: ignore-file CPP-TYPEDEF (C header: no using in C)
// xstyle: ignore-file CPP-CLASS-NAME (C header: C types are snake_case)
// xstyle: ignore-file CPP-PREPRO-INDENT (extern "C" block at the level of the declarations it wraps)

#ifndef ENCAPSULATION_CAPI_C_H
    #define ENCAPSULATION_CAPI_C_H

    //----------------------------------------------------------------//
    /* INCLUDE */

    /* type */
    #include "../attribute/Attribute.hpp"   // _noreturn
    #include "../exception/CApi.h"          // utils_exception, UTILS_OK, UTILS_KO
    #include "../platform/CApi.h"           // pid_t, struct epoll_event, EPOLL*
    #include <stdbool.h>                    // bool
    #include <stddef.h>                     // size_t
    #include <stdint.h>                     // uint32_t

#ifdef __cplusplus
    extern "C" {
#endif
//----------------------------------------------------------------//
/* TYPEDEF */

/* opaque types */
typedef struct utils_dup utils_dup_t; // utils::encapsulation::Dup
typedef struct utils_pipe utils_pipe_t; // utils::encapsulation::Pipe
typedef struct utils_poll utils_poll_t; // utils::encapsulation::Poll
typedef struct utils_process utils_process_t; // utils::encapsulation::Process
typedef struct utils_shared_object utils_shared_object_t; // utils::encapsulation::SharedObject
typedef struct utils_shm utils_shm_t; // utils::encapsulation::SharedMemory
typedef struct utils_shm_messages utils_shm_messages_t; // messages read from a shared memory (id -> payloads)

//----------------------------------------------------------------//
/* ENUM */

/* filter of a read (utils::encapsulation::shm::ReadFilter) */
typedef enum utils_shm_read_filter {
    C_API_SHM_READ_ALL = 0,
    C_API_SHM_READ_ZERO_ONLY,
    C_API_SHM_READ_NON_ZERO_ONLY,
    C_API_SHM_READ_LAST_ONLY
} utils_shm_read_filter_t;

/* layout of the memory (utils::encapsulation::shm::LayoutPolicy) */
typedef enum utils_shm_layout {
    C_API_SHM_LAYOUT_COMPACT = 0, // (meta,meta,...)(byte,byte,...): 1 writer, N readers
    C_API_SHM_LAYOUT_COMPACT_SEMI_ALIGNED, // idem + alignement per groups: some writers, some at the same time
    C_API_SHM_LAYOUT_INTERLEAVED, // (meta,byte,meta,byte,...): N writers moderated, payload >= cache-line (advise)
    C_API_SHM_LAYOUT_INTERLEAVED_ALIGNED // idem + aligned per slot: N writers, many at the same time
} utils_shm_layout_t;

//----------------------------------------------------------------//
/* STRUCT */

/* status of a waited process (utils::encapsulation::Status) */
typedef struct utils_process_status {
    bool unknown;
    bool exited;
    int code; // exited
    int sig; // killed
} utils_process_status_t;

/* id of a request of a shared memory (utils::encapsulation::shm::Id) */
typedef struct utils_shm_id {
    size_t id; // 0: invalid / unset
    pid_t ownership;
} utils_shm_id_t;

/* target of a request of a shared memory (utils::encapsulation::shm::Target) */
typedef struct utils_shm_target {
    pid_t sender; // ownership of the sender
    size_t limit; // number of readers (0: every connected reader for a global request)
    pid_t ownership; // to only a specific reader
    bool global; // to every reader
} utils_shm_target_t;

//----------------------------------------------------------------//
/* PROTOTYPE */

/* dup (trigger: dup2 of origin on clone, a new clone when it's -1) */
_nodiscard _nothrow utils_dup_t* utils_dup_create(int origin, int clone); // NULL on error
_nothrow void utils_dup_destroy(utils_dup_t* dup); // NULL: nothing (the fds are not closed)
_nodiscard _nothrow int utils_dup_trigger(utils_dup_t* dup);
_nodiscard _nothrow int utils_dup_close_origin(utils_dup_t* dup);
_nodiscard _nothrow int utils_dup_close_clone(utils_dup_t* dup);
_nodiscard _nothrow int utils_dup_close(utils_dup_t* dup);
_nodiscard _nothrow int utils_dup_clear(utils_dup_t* dup); // forget the fds (not closed)
_nodiscard _nothrow int utils_dup_get_origin(const utils_dup_t* dup); // -1 on error
_nodiscard _nothrow int utils_dup_get_clone(const utils_dup_t* dup); // -1 on error
_nodiscard _nothrow int utils_dup_set_origin(utils_dup_t* dup, int fd);
_nodiscard _nothrow int utils_dup_set_clone(utils_dup_t* dup, int fd);

/* pipe (trigger: create the pipe, both fds must be closed) */
_nodiscard _nothrow utils_pipe_t* utils_pipe_create(int read, int write); // NULL on error
_nothrow void utils_pipe_destroy(utils_pipe_t* pipe); // NULL: nothing (closes the fds)
_nodiscard _nothrow int utils_pipe_trigger(utils_pipe_t* pipe);
_nodiscard _nothrow int utils_pipe_close_read(utils_pipe_t* pipe);
_nodiscard _nothrow int utils_pipe_close_write(utils_pipe_t* pipe);
_nodiscard _nothrow int utils_pipe_close(utils_pipe_t* pipe);
_nodiscard _nothrow int utils_pipe_clear(utils_pipe_t* pipe); // forget the fds (not closed)
_nodiscard _nothrow int utils_pipe_get_read(const utils_pipe_t* pipe); // -1 on error
_nodiscard _nothrow int utils_pipe_get_write(const utils_pipe_t* pipe); // -1 on error
_nodiscard _nothrow int utils_pipe_set_read(utils_pipe_t* pipe, int fd);
_nodiscard _nothrow int utils_pipe_set_write(utils_pipe_t* pipe, int fd);

/* poll (epoll) */
_nodiscard _nothrow utils_poll_t* utils_poll_create(int flags); // EPOLL_CLOEXEC by default in c++, NULL on error
_nothrow void utils_poll_destroy(utils_poll_t* poll); // NULL: nothing (closes it)
_nodiscard _nothrow int utils_poll_init(utils_poll_t* poll, int flags);
_nodiscard _nothrow int utils_poll_close(utils_poll_t* poll);
_nodiscard _nothrow int utils_poll_link(utils_poll_t* poll, int fd, uint32_t events, void* data); // data: NULL = the fd in the event
_nodiscard _nothrow int utils_poll_edit(const utils_poll_t* poll, int fd, uint32_t events, void* data);
_nodiscard _nothrow int utils_poll_unlink(utils_poll_t* poll, int fd);
_nodiscard _nothrow size_t utils_poll_wait(const utils_poll_t* poll, int delay, struct epoll_event* events, size_t max); // events filled (max at most), returns their number (0: timeout or error, see utils_exception.exception)
_nodiscard _nothrow size_t utils_poll_size(const utils_poll_t* poll);
_nodiscard _nothrow bool utils_poll_contains(const utils_poll_t* poll, int fd);
_nodiscard _nothrow int utils_poll_get_fd(const utils_poll_t* poll); // -1 on error

/* process */
_nodiscard _nothrow utils_process_t* utils_process_create(void); // NULL on error
_nothrow void utils_process_destroy(utils_process_t* process); // NULL: nothing (the parent kills the child)
_nodiscard _nothrow bool utils_process_is(const utils_process_t* process); // the child is running
_nodiscard _nothrow pid_t utils_process_spawn(utils_process_t* process); // fork (Windows: error, no fork), -1 on error
_nodiscard _nothrow _null_terminated_string_arg(2) pid_t utils_process_spawn_exec(utils_process_t* process, const char* path, const char* const* args, size_t count); // execvp (argv[0] = path, args after it), -1 on error
_nothrow _noreturn _null_terminated_string_arg(2) void utils_process_replace(utils_process_t* process, const char* path, const char* const* args, size_t count); // execvp in this process (never return: exec or _exit(127))
_nodiscard _nothrow int utils_process_wait(utils_process_t* process, utils_process_status_t* status); // waitpid
_nodiscard _nothrow int utils_process_kill(utils_process_t* process); // -s 9
_nodiscard _nothrow int utils_process_clear(utils_process_t* process); // clear the pipes / dups (fds closed)
_nodiscard _nothrow int utils_process_pipe(utils_process_t* process, int read, int write);
_nodiscard _nothrow int utils_process_dup(utils_process_t* process, int origin, int clone);
_nodiscard _nothrow int utils_process_add_pipe(utils_process_t* process, utils_pipe_t* pipe); // the fds are moved in the process (pipe cleared)
_nodiscard _nothrow int utils_process_add_dup(utils_process_t* process, utils_dup_t* dup); // the fds are moved in the process (dup cleared)
_nodiscard _nothrow size_t utils_process_pipes_count(const utils_process_t* process);
_nodiscard _nothrow size_t utils_process_dups_count(const utils_process_t* process);
_nodiscard _nothrow int utils_process_get_pipe(const utils_process_t* process, size_t i, int* read, int* write);
_nodiscard _nothrow int utils_process_get_dup(const utils_process_t* process, size_t i, int* origin, int* clone);
_nodiscard _nothrow bool utils_process_is_child(const utils_process_t* process);
_nodiscard _nothrow bool utils_process_is_parent(const utils_process_t* process);
_nodiscard _nothrow pid_t utils_process_get_pid(const utils_process_t* process); // -1: not spawned or error

/* shared object (dlopen) */
_nodiscard _nothrow _null_terminated_string_arg(1) utils_shared_object_t* utils_shared_object_create(const char* path); // loaded (RTLD_NOW), NULL on error
_nothrow void utils_shared_object_destroy(utils_shared_object_t* so); // NULL: nothing (dlclose)
_nodiscard _nothrow bool utils_shared_object_is_loaded(const utils_shared_object_t* so);
_nodiscard _nothrow const char* utils_shared_object_path(const utils_shared_object_t* so); // valid while so lives (NULL on error)
_nodiscard _nothrow void* utils_shared_object_get(const utils_shared_object_t* so); // handle of dlopen (NULL on error)
_nodiscard _nothrow _null_terminated_string_arg(2) void* utils_shared_object_load_function(utils_shared_object_t* so, const char* name); // cast it in the function type (NULL on error)

/* shared memory (create: the 'server' (size of a slot & queue), else a 'client' that opens it, group_size: power of 2 from 1 to 64, only for COMPACT_SEMI_ALIGNED) */
_nodiscard _nothrow utils_shm_t* utils_shm_create(pid_t ownership); // NULL on error
_nodiscard _nothrow utils_shm_t* utils_shm_create_default(void); // ownership: getpid(), NULL on error
_nothrow void utils_shm_destroy(utils_shm_t* shm); // NULL: nothing (closes it)
_nodiscard _nothrow _read_only(2, 3) int utils_shm_init(utils_shm_t* shm, const char* name, size_t size, size_t queue, bool create, utils_shm_layout_t layout, size_t group_size, bool forced);
_nodiscard _nothrow int utils_shm_close(utils_shm_t* shm);
_nodiscard _nothrow pid_t utils_shm_get_ownership(const utils_shm_t* shm); // -1 on error
_nodiscard _nothrow int utils_shm_set_ownership(utils_shm_t* shm, pid_t ownership);
_nodiscard _nothrow _read_only(2, 3) int utils_shm_send(utils_shm_t* shm, const void* bytes, size_t size, const utils_shm_target_t* target, bool last_sending, bool last_transmission, bool failsafe, utils_shm_id_t* id); // a new id for the request (id: optional)
_nodiscard _nothrow _read_only(2, 3) int utils_shm_send_id(utils_shm_t* shm, const void* bytes, size_t size, utils_shm_id_t id, const utils_shm_target_t* target, bool last_sending, bool last_transmission, bool failsafe); // force an id
_nodiscard _nothrow _read_only(2, 3) int utils_shm_send_own_id(utils_shm_t* shm, const void* bytes, size_t size, size_t id, const utils_shm_target_t* target, bool last_sending, bool last_transmission, bool failsafe); // id of this ownership
_nodiscard _nothrow int utils_shm_trigger(utils_shm_t* shm); // read now (automatic in the normal case)
_nodiscard _nothrow bool utils_shm_readable(const utils_shm_t* shm, utils_shm_read_filter_t filter);
_nodiscard _nothrow bool utils_shm_readable_id(const utils_shm_t* shm, utils_shm_id_t id);
_nodiscard _nothrow int utils_shm_join(const utils_shm_t* shm, bool last); // wait for anything to be read (or any last)
_nodiscard _nothrow int utils_shm_join_id(const utils_shm_t* shm, utils_shm_id_t id, bool last); // wait for a specific id
_nodiscard _nothrow utils_shm_messages_t* utils_shm_read(utils_shm_t* shm, utils_shm_read_filter_t filter); // NULL: nothing read (utils_exception.exception false) or error
_nodiscard _nothrow utils_shm_messages_t* utils_shm_read_id(utils_shm_t* shm, utils_shm_id_t id); // NULL: nothing read (utils_exception.exception false) or error

/* messages read (ids in any order, payloads of an id in the order of the reception) */
_nothrow void utils_shm_messages_destroy(utils_shm_messages_t* messages); // NULL: nothing
_nodiscard _nothrow size_t utils_shm_messages_count(const utils_shm_messages_t* messages); // number of ids
_nodiscard _nothrow int utils_shm_messages_id(const utils_shm_messages_t* messages, size_t i, utils_shm_id_t* id);
_nodiscard _nothrow size_t utils_shm_messages_size(const utils_shm_messages_t* messages, size_t i); // number of payloads of the id i
_nodiscard _nothrow const void* utils_shm_messages_get(const utils_shm_messages_t* messages, size_t i, size_t j, size_t* size); // payload j of the id i, valid while messages lives (NULL on error)

#ifdef __cplusplus
    } // end of: extern "C"
#endif
#endif /* ENCAPSULATION_CAPI_C_H */
