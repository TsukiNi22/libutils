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
##  @file CApi.cpp

File Description:
##  C API of utils::encapsulation (Dup, Pipe, Poll, Process, SharedObject, SharedMemory)
\**************************************************************/

#include "utils/exception/basic/ErrorException.hpp"
#include "utils/encapsulation/SharedObject.hpp"
#include "utils/encapsulation/SharedMemory.hpp"
#include "utils/exception/ExceptionDefine.hpp"
#include "utils/encapsulation/Process.hpp"
#include "utils/attribute/Attribute.hpp"
#include "utils/encapsulation/CApi.hpp"
#include "utils/encapsulation/Pipe.hpp"
#include "utils/encapsulation/Poll.hpp"
#include "utils/encapsulation/Dup.hpp"
#include "utils/encapsulation/CApi.h"
#include "utils/platform/Unistd.hpp"
#include "utils/exception/CApi.hpp"
#include <unordered_map>
#include <algorithm>
#include <optional>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <string>
#include <vector>

//----------------------------------------------------------------//
/* CHECK */

/* C & C++ enums must stay the same */
static_assert(C_API_SHM_READ_ALL                     == static_cast<int>(utils::encapsulation::shm::ReadFilter::All));
static_assert(C_API_SHM_READ_ZERO_ONLY               == static_cast<int>(utils::encapsulation::shm::ReadFilter::ZeroOnly));
static_assert(C_API_SHM_READ_NON_ZERO_ONLY           == static_cast<int>(utils::encapsulation::shm::ReadFilter::NonZeroOnly));
static_assert(C_API_SHM_READ_LAST_ONLY               == static_cast<int>(utils::encapsulation::shm::ReadFilter::LastOnly));
static_assert(C_API_SHM_LAYOUT_COMPACT               == static_cast<int>(utils::encapsulation::shm::LayoutPolicy::Compact));
static_assert(C_API_SHM_LAYOUT_COMPACT_SEMI_ALIGNED  == static_cast<int>(utils::encapsulation::shm::LayoutPolicy::CompactSemiAligned));
static_assert(C_API_SHM_LAYOUT_INTERLEAVED           == static_cast<int>(utils::encapsulation::shm::LayoutPolicy::Interleaved));
static_assert(C_API_SHM_LAYOUT_INTERLEAVED_ALIGNED   == static_cast<int>(utils::encapsulation::shm::LayoutPolicy::InterleavedAligned));

//----------------------------------------------------------------//
/* TOOLS */

// Arguments of the C side (count of them, NULL only with a count of 0)
_cold _nodiscard static std::vector<std::string> arguments(const char* const* args, const std::size_t count)
{
    std::vector<std::string> res;

    if (count > 0) (void)utils::exception::c_api::not_null(args, "arguments");
    for (std::size_t i = 0; i < count; ++i)
        res.emplace_back(utils::exception::c_api::not_null(args[i], "argument"));
    return res;
}

// Bytes of the C side (NULL only with a size of 0)
_hot _nodiscard static std::vector<std::byte> bytes(const void* data, const std::size_t size)
{
    const std::byte* begin = nullptr;

    if (size == 0) return {};
    begin = static_cast<const std::byte*>(utils::exception::c_api::not_null(data, "bytes"));
    return std::vector<std::byte>(begin, begin + size);
}

// Read filter of the C side (out of the enum: InvalidArgument)
_hot _nodiscard static utils::encapsulation::shm::ReadFilter filter_of(const utils_shm_read_filter_t filter)
{
    if (filter < C_API_SHM_READ_ALL || filter > C_API_SHM_READ_LAST_ONLY) _unlikely {
        throw utils::exception::ErrorException(utils::exception::InternalCode::InvalidArgument, "The read filter is invalid");
    }
    return static_cast<utils::encapsulation::shm::ReadFilter>(filter);
}

// Target of the C side (the readed counter starts at 0)
_hot static void target_of(const utils_shm_target_t* target, utils::encapsulation::shm::Target& res)
{
    const utils_shm_target_t& given = utils::exception::c_api::not_null_ref(target, "target");

    res.sender = given.sender;
    res.limit = given.limit;
    res.ownership = given.ownership;
    res.global = given.global;
}

/* shared memory: the template init of the c++ (create, layout, group size, forced) chosen at runtime */
template<bool create, bool forced, std::size_t group>
_cold static void init_group(utils::encapsulation::SharedMemory& shm, const std::string& name, const std::size_t size, const std::size_t queue)
{
    shm.template init<create, utils::encapsulation::shm::LayoutPolicy::CompactSemiAligned, group, forced>(name, size, queue);
}

template<bool create, bool forced>
_cold static void init_layout(utils::encapsulation::SharedMemory& shm, const std::string& name, const std::size_t size, const std::size_t queue, const utils_shm_layout_t layout, const std::size_t group)
{
    switch (layout) {
        case C_API_SHM_LAYOUT_COMPACT:             shm.template init<create, utils::encapsulation::shm::LayoutPolicy::Compact, 2, forced>(name, size, queue);            return;
        case C_API_SHM_LAYOUT_INTERLEAVED:         shm.template init<create, utils::encapsulation::shm::LayoutPolicy::Interleaved, 2, forced>(name, size, queue);        return;
        case C_API_SHM_LAYOUT_INTERLEAVED_ALIGNED: shm.template init<create, utils::encapsulation::shm::LayoutPolicy::InterleavedAligned, 2, forced>(name, size, queue); return;
        case C_API_SHM_LAYOUT_COMPACT_SEMI_ALIGNED: break;
        default: throw utils::exception::ErrorException(utils::exception::InternalCode::InvalidArgument, "The layout of the shared memory is invalid");
    }
    switch (group) {
        case 1:  init_group<create, forced, 1>(shm, name, size, queue);  return;
        case 2:  init_group<create, forced, 2>(shm, name, size, queue);  return;
        case 4:  init_group<create, forced, 4>(shm, name, size, queue);  return;
        case 8:  init_group<create, forced, 8>(shm, name, size, queue);  return;
        case 16: init_group<create, forced, 16>(shm, name, size, queue); return;
        case 32: init_group<create, forced, 32>(shm, name, size, queue); return;
        case 64: init_group<create, forced, 64>(shm, name, size, queue); return;
        default: throw utils::exception::ErrorException(utils::exception::InternalCode::InvalidArgument, "The group size must be a power of 2 from 1 to 64");
    }
}

// Messages read (nullptr when nothing was read)
_cold _nodiscard static utils_shm_messages_t* messages_of(std::optional<std::unordered_map<utils::encapsulation::shm::Id, std::vector<std::vector<std::byte>>>>&& read)
{
    utils_shm_messages_t* res = nullptr;

    if (!read) return nullptr;
    res = new utils_shm_messages_t();
    for (auto &[id, payloads]: *read)
        res->messages.emplace_back(id, std::move(payloads));
    return res;
}

//----------------------------------------------------------------//
/* DUP */

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_cold utils_dup_t* utils_dup_create(int origin, int clone)
{
    return utils::exception::c_api::guard_fallback(nullptr, [&](void) -> utils_dup_t* {return new utils_dup_t(origin, clone);});
}

_cold void utils_dup_destroy(utils_dup_t* dup)
{
    (void)utils::exception::c_api::guard([&](void) {delete dup;}); // like free(NULL)
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_cold int utils_dup_trigger(utils_dup_t* dup)
{
    return utils::exception::c_api::guard([&](void) {utils::exception::c_api::not_null_ref(dup, "dup").trigger();});
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_cold int utils_dup_close_origin(utils_dup_t* dup)
{
    return utils::exception::c_api::guard([&](void) {utils::exception::c_api::not_null_ref(dup, "dup").closeOrigin();});
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_cold int utils_dup_close_clone(utils_dup_t* dup)
{
    return utils::exception::c_api::guard([&](void) {utils::exception::c_api::not_null_ref(dup, "dup").closeClone();});
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_cold int utils_dup_close(utils_dup_t* dup)
{
    return utils::exception::c_api::guard([&](void) {utils::exception::c_api::not_null_ref(dup, "dup").close();});
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_cold int utils_dup_clear(utils_dup_t* dup)
{
    return utils::exception::c_api::guard([&](void) {utils::exception::c_api::not_null_ref(dup, "dup").clear();});
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_cold int utils_dup_get_origin(const utils_dup_t* dup)
{
    return utils::exception::c_api::guard_fallback(-1, [&](void) -> int {return utils::exception::c_api::not_null_ref(dup, "dup").getOrigin();});
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_cold int utils_dup_get_clone(const utils_dup_t* dup)
{
    return utils::exception::c_api::guard_fallback(-1, [&](void) -> int {return utils::exception::c_api::not_null_ref(dup, "dup").getClone();});
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_cold int utils_dup_set_origin(utils_dup_t* dup, int fd)
{
    return utils::exception::c_api::guard([&](void) {utils::exception::c_api::not_null_ref(dup, "dup").setOrigin(fd);});
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_cold int utils_dup_set_clone(utils_dup_t* dup, int fd)
{
    return utils::exception::c_api::guard([&](void) {utils::exception::c_api::not_null_ref(dup, "dup").setClone(fd);});
}

//----------------------------------------------------------------//
/* PIPE */

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_cold utils_pipe_t* utils_pipe_create(int read, int write)
{
    return utils::exception::c_api::guard_fallback(nullptr, [&](void) -> utils_pipe_t* {return new utils_pipe_t(read, write);});
}

_cold void utils_pipe_destroy(utils_pipe_t* pipe)
{
    (void)utils::exception::c_api::guard([&](void) {delete pipe;}); // like free(NULL)
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_cold int utils_pipe_trigger(utils_pipe_t* pipe)
{
    return utils::exception::c_api::guard([&](void) {utils::exception::c_api::not_null_ref(pipe, "pipe").trigger();});
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_cold int utils_pipe_close_read(utils_pipe_t* pipe)
{
    return utils::exception::c_api::guard([&](void) {utils::exception::c_api::not_null_ref(pipe, "pipe").closeRead();});
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_cold int utils_pipe_close_write(utils_pipe_t* pipe)
{
    return utils::exception::c_api::guard([&](void) {utils::exception::c_api::not_null_ref(pipe, "pipe").closeWrite();});
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_cold int utils_pipe_close(utils_pipe_t* pipe)
{
    return utils::exception::c_api::guard([&](void) {utils::exception::c_api::not_null_ref(pipe, "pipe").close();});
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_cold int utils_pipe_clear(utils_pipe_t* pipe)
{
    return utils::exception::c_api::guard([&](void) {utils::exception::c_api::not_null_ref(pipe, "pipe").clear();});
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_cold int utils_pipe_get_read(const utils_pipe_t* pipe)
{
    return utils::exception::c_api::guard_fallback(-1, [&](void) -> int {return utils::exception::c_api::not_null_ref(pipe, "pipe").getRead();});
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_cold int utils_pipe_get_write(const utils_pipe_t* pipe)
{
    return utils::exception::c_api::guard_fallback(-1, [&](void) -> int {return utils::exception::c_api::not_null_ref(pipe, "pipe").getWrite();});
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_cold int utils_pipe_set_read(utils_pipe_t* pipe, int fd)
{
    return utils::exception::c_api::guard([&](void) {utils::exception::c_api::not_null_ref(pipe, "pipe").setRead(fd);});
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_cold int utils_pipe_set_write(utils_pipe_t* pipe, int fd)
{
    return utils::exception::c_api::guard([&](void) {utils::exception::c_api::not_null_ref(pipe, "pipe").setWrite(fd);});
}

//----------------------------------------------------------------//
/* POLL */

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_cold utils_poll_t* utils_poll_create(int flags)
{
    return utils::exception::c_api::guard_fallback(nullptr, [&](void) -> utils_poll_t* {return new utils_poll_t(flags);});
}

_cold void utils_poll_destroy(utils_poll_t* poll)
{
    (void)utils::exception::c_api::guard([&](void) {delete poll;}); // like free(NULL)
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_cold int utils_poll_init(utils_poll_t* poll, int flags)
{
    return utils::exception::c_api::guard([&](void) {utils::exception::c_api::not_null_ref(poll, "poll").init(flags);});
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_cold int utils_poll_close(utils_poll_t* poll)
{
    return utils::exception::c_api::guard([&](void) {utils::exception::c_api::not_null_ref(poll, "poll").close();});
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_cold int utils_poll_link(utils_poll_t* poll, int fd, uint32_t events, void* data)
{
    return utils::exception::c_api::guard([&](void) {utils::exception::c_api::not_null_ref(poll, "poll").link(fd, events, data);});
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_cold int utils_poll_edit(const utils_poll_t* poll, int fd, uint32_t events, void* data)
{
    return utils::exception::c_api::guard([&](void) {utils::exception::c_api::not_null_ref(poll, "poll").edit(fd, events, data);});
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_cold int utils_poll_unlink(utils_poll_t* poll, int fd)
{
    return utils::exception::c_api::guard([&](void) {utils::exception::c_api::not_null_ref(poll, "poll").unlink(fd);});
}

_hot size_t utils_poll_wait(const utils_poll_t* poll, int delay, struct epoll_event* events, size_t max)
{
    return utils::exception::c_api::guard_fallback(0, [&](void) -> std::size_t {
        const utils_poll_t& self = utils::exception::c_api::not_null_ref(poll, "poll");
        struct epoll_event* out = utils::exception::c_api::not_null(events, "events");
        std::vector<struct epoll_event> ready;

        if (max == 0) _unlikely {
            throw utils::exception::ErrorException(utils::exception::InternalCode::InvalidArgument, "The max of events must be at least 1");
        }
        ready = self.wait(delay, max);
        std::copy_n(ready.begin(), std::min(max, ready.size()), out);
        return std::min(max, ready.size());
    });
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_hot size_t utils_poll_size(const utils_poll_t* poll)
{
    return utils::exception::c_api::guard_fallback(0, [&](void) -> std::size_t {return utils::exception::c_api::not_null_ref(poll, "poll").size();});
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_hot bool utils_poll_contains(const utils_poll_t* poll, int fd)
{
    return utils::exception::c_api::guard_fallback(false, [&](void) -> bool {return utils::exception::c_api::not_null_ref(poll, "poll").contains(fd);});
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_cold int utils_poll_get_fd(const utils_poll_t* poll)
{
    return utils::exception::c_api::guard_fallback(-1, [&](void) -> int {return utils::exception::c_api::not_null_ref(poll, "poll").getFd();});
}

//----------------------------------------------------------------//
/* PROCESS */

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_cold utils_process_t* utils_process_create(void)
{
    return utils::exception::c_api::guard_fallback(nullptr, [](void) -> utils_process_t* {return new utils_process_t();});
}

_cold void utils_process_destroy(utils_process_t* process)
{
    (void)utils::exception::c_api::guard([&](void) {delete process;}); // like free(NULL)
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_cold bool utils_process_is(const utils_process_t* process)
{
    return utils::exception::c_api::guard_fallback(false, [&](void) -> bool {return utils::exception::c_api::not_null_ref(process, "process").is();});
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_cold pid_t utils_process_spawn(utils_process_t* process)
{
    return utils::exception::c_api::guard_fallback(-1, [&](void) -> pid_t {return utils::exception::c_api::not_null_ref(process, "process").spawn();});
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_cold pid_t utils_process_spawn_exec(utils_process_t* process, const char* path, const char* const* args, size_t count)
{
    return utils::exception::c_api::guard_fallback(-1, [&](void) -> pid_t {return utils::exception::c_api::not_null_ref(process, "process").spawn(utils::exception::c_api::not_null(path, "path"), arguments(args, count));});
}

_cold void utils_process_replace(utils_process_t* process, const char* path, const char* const* args, size_t count)
{
    (void)utils::exception::c_api::guard([&](void) {
        utils::encapsulation::Process& self = utils::exception::c_api::not_null_ref(process, "process");

        self.replace(utils::exception::c_api::not_null(path, "path"), arguments(args, count));
    });
    std::_Exit(127); // the exec was never reached (error in utils_exception)
}

_cold int utils_process_wait(utils_process_t* process, utils_process_status_t* status)
{
    return utils::exception::c_api::guard([&](void) {
        utils_process_status_t& out = utils::exception::c_api::not_null_ref(status, "status");
        const utils::encapsulation::Status res = utils::exception::c_api::not_null_ref(process, "process").wait();

        out.unknown = res.unknown;
        out.exited = res.exited;
        out.code = res.code;
        out.sig = res.sig;
    });
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_cold int utils_process_kill(utils_process_t* process)
{
    return utils::exception::c_api::guard([&](void) {utils::exception::c_api::not_null_ref(process, "process").kill();});
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_cold int utils_process_clear(utils_process_t* process)
{
    return utils::exception::c_api::guard([&](void) {utils::exception::c_api::not_null_ref(process, "process").clear();});
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_cold int utils_process_pipe(utils_process_t* process, int read, int write)
{
    return utils::exception::c_api::guard([&](void) {utils::exception::c_api::not_null_ref(process, "process").pipe(read, write);});
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_cold int utils_process_dup(utils_process_t* process, int origin, int clone)
{
    return utils::exception::c_api::guard([&](void) {utils::exception::c_api::not_null_ref(process, "process").dup(origin, clone);});
}

_cold int utils_process_add_pipe(utils_process_t* process, utils_pipe_t* pipe)
{
    return utils::exception::c_api::guard([&](void) {
        utils::encapsulation::Process& self = utils::exception::c_api::not_null_ref(process, "process");

        self.pipe(static_cast<utils::encapsulation::Pipe&>(utils::exception::c_api::not_null_ref(pipe, "pipe")));
    });
}

_cold int utils_process_add_dup(utils_process_t* process, utils_dup_t* dup)
{
    return utils::exception::c_api::guard([&](void) {
        utils::encapsulation::Process& self = utils::exception::c_api::not_null_ref(process, "process");

        self.dup(static_cast<utils::encapsulation::Dup&>(utils::exception::c_api::not_null_ref(dup, "dup")));
    });
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_cold size_t utils_process_pipes_count(const utils_process_t* process)
{
    return utils::exception::c_api::guard_fallback(0, [&](void) -> std::size_t {return utils::exception::c_api::not_null_ref(process, "process").getPipes().size();});
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_cold size_t utils_process_dups_count(const utils_process_t* process)
{
    return utils::exception::c_api::guard_fallback(0, [&](void) -> std::size_t {return utils::exception::c_api::not_null_ref(process, "process").getDups().size();});
}

_cold int utils_process_get_pipe(const utils_process_t* process, size_t i, int* read, int* write)
{
    return utils::exception::c_api::guard([&](void) {
        const utils::encapsulation::Pipe& pipe = utils::exception::c_api::not_null_ref(process, "process").getPipes().at(i);

        if (read) *read = pipe.getRead();
        if (write) *write = pipe.getWrite();
    });
}

_cold int utils_process_get_dup(const utils_process_t* process, size_t i, int* origin, int* clone)
{
    return utils::exception::c_api::guard([&](void) {
        const utils::encapsulation::Dup& dup = utils::exception::c_api::not_null_ref(process, "process").getDups().at(i);

        if (origin) *origin = dup.getOrigin();
        if (clone) *clone = dup.getClone();
    });
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_cold bool utils_process_is_child(const utils_process_t* process)
{
    return utils::exception::c_api::guard_fallback(false, [&](void) -> bool {return utils::exception::c_api::not_null_ref(process, "process").isChild();});
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_cold bool utils_process_is_parent(const utils_process_t* process)
{
    return utils::exception::c_api::guard_fallback(false, [&](void) -> bool {return utils::exception::c_api::not_null_ref(process, "process").isParent();});
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_cold pid_t utils_process_get_pid(const utils_process_t* process)
{
    return utils::exception::c_api::guard_fallback(-1, [&](void) -> pid_t {return utils::exception::c_api::not_null_ref(process, "process").getPid();});
}

//----------------------------------------------------------------//
/* SHARED OBJECT */

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_cold utils_shared_object_t* utils_shared_object_create(const char* path)
{
    return utils::exception::c_api::guard_fallback(nullptr, [&](void) -> utils_shared_object_t* {return new utils_shared_object_t(utils::exception::c_api::not_null(path, "path"));});
}

_cold void utils_shared_object_destroy(utils_shared_object_t* so)
{
    (void)utils::exception::c_api::guard([&](void) {delete so;}); // like free(NULL)
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_cold bool utils_shared_object_is_loaded(const utils_shared_object_t* so)
{
    return utils::exception::c_api::guard_fallback(false, [&](void) -> bool {return utils::exception::c_api::not_null_ref(so, "shared object").isLoaded();});
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_cold const char* utils_shared_object_path(const utils_shared_object_t* so)
{
    return utils::exception::c_api::guard_fallback(nullptr, [&](void) -> const char* {return utils::exception::c_api::not_null_ref(so, "shared object").path().data();}); // view of a std::string: '\0' terminated
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_cold void* utils_shared_object_get(const utils_shared_object_t* so)
{
    return utils::exception::c_api::guard_fallback(nullptr, [&](void) -> void* {return utils::exception::c_api::not_null_ref(so, "shared object").get();});
}

_cold void* utils_shared_object_load_function(utils_shared_object_t* so, const char* name)
{
    return utils::exception::c_api::guard_fallback(nullptr, [&](void) -> void* {
        utils::encapsulation::SharedObject& self = utils::exception::c_api::not_null_ref(so, "shared object");

        return self.loadFunction<void*>(utils::exception::c_api::not_null(name, "name"));
    });
}

//----------------------------------------------------------------//
/* SHARED MEMORY */

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_cold utils_shm_t* utils_shm_create(pid_t ownership)
{
    return utils::exception::c_api::guard_fallback(nullptr, [&](void) -> utils_shm_t* {return new utils_shm_t(ownership);});
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_cold utils_shm_t* utils_shm_create_default(void)
{
    return utils_shm_create(utils::platform::getpid());
}

_cold void utils_shm_destroy(utils_shm_t* shm)
{
    (void)utils::exception::c_api::guard([&](void) {delete shm;}); // like free(NULL)
}

_cold int utils_shm_init(utils_shm_t* shm, const char* name, size_t size, size_t queue, bool create, utils_shm_layout_t layout, size_t group_size, bool forced)
{
    return utils::exception::c_api::guard([&](void) {
        utils::encapsulation::SharedMemory& self = utils::exception::c_api::not_null_ref(shm, "shared memory");
        const std::string path = utils::exception::c_api::not_null(name, "name");

        if (create && forced) init_layout<true, true>(self, path, size, queue, layout, group_size);
        else if (create) init_layout<true, false>(self, path, size, queue, layout, group_size);
        else if (forced) init_layout<false, true>(self, path, size, queue, layout, group_size);
        else init_layout<false, false>(self, path, size, queue, layout, group_size);
    });
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_cold int utils_shm_close(utils_shm_t* shm)
{
    return utils::exception::c_api::guard([&](void) {utils::exception::c_api::not_null_ref(shm, "shared memory").close();});
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_cold pid_t utils_shm_get_ownership(const utils_shm_t* shm)
{
    return utils::exception::c_api::guard_fallback(-1, [&](void) -> pid_t {return utils::exception::c_api::not_null_ref(shm, "shared memory").ownership();});
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_cold int utils_shm_set_ownership(utils_shm_t* shm, pid_t ownership)
{
    return utils::exception::c_api::guard([&](void) {utils::exception::c_api::not_null_ref(shm, "shared memory").ownership(ownership);});
}

_hot int utils_shm_send(utils_shm_t* shm, const void* bytes_, size_t size, const utils_shm_target_t* target, bool last_sending, bool last_transmission, bool failsafe, utils_shm_id_t* id)
{
    return utils::exception::c_api::guard([&](void) {
        utils::encapsulation::SharedMemory& self = utils::exception::c_api::not_null_ref(shm, "shared memory");
        utils::encapsulation::shm::Target to;

        target_of(target, to);
        const utils::encapsulation::shm::Id res = self.send(bytes(bytes_, size), to, last_sending, last_transmission, failsafe);
        if (id) *id = {res.id, res.ownership};
    });
}

_hot int utils_shm_send_id(utils_shm_t* shm, const void* bytes_, size_t size, utils_shm_id_t id, const utils_shm_target_t* target, bool last_sending, bool last_transmission, bool failsafe)
{
    return utils::exception::c_api::guard([&](void) {
        utils::encapsulation::SharedMemory& self = utils::exception::c_api::not_null_ref(shm, "shared memory");
        utils::encapsulation::shm::Target to;

        target_of(target, to);
        self.send(bytes(bytes_, size), utils::encapsulation::shm::Id{id.id, id.ownership}, to, last_sending, last_transmission, failsafe);
    });
}

_hot int utils_shm_send_own_id(utils_shm_t* shm, const void* bytes_, size_t size, size_t id, const utils_shm_target_t* target, bool last_sending, bool last_transmission, bool failsafe)
{
    return utils::exception::c_api::guard([&](void) {
        utils::encapsulation::SharedMemory& self = utils::exception::c_api::not_null_ref(shm, "shared memory");
        utils::encapsulation::shm::Target to;

        target_of(target, to);
        self.send(bytes(bytes_, size), id, to, last_sending, last_transmission, failsafe);
    });
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_hot int utils_shm_trigger(utils_shm_t* shm)
{
    return utils::exception::c_api::guard([&](void) {utils::exception::c_api::not_null_ref(shm, "shared memory").trigger();});
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_hot bool utils_shm_readable(const utils_shm_t* shm, utils_shm_read_filter_t filter)
{
    return utils::exception::c_api::guard_fallback(false, [&](void) -> bool {return utils::exception::c_api::not_null_ref(shm, "shared memory").readable(filter_of(filter));});
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_hot bool utils_shm_readable_id(const utils_shm_t* shm, utils_shm_id_t id)
{
    return utils::exception::c_api::guard_fallback(false, [&](void) -> bool {return utils::exception::c_api::not_null_ref(shm, "shared memory").readable(utils::encapsulation::shm::Id{id.id, id.ownership});});
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_hot int utils_shm_join(const utils_shm_t* shm, bool last)
{
    return utils::exception::c_api::guard([&](void) {utils::exception::c_api::not_null_ref(shm, "shared memory").join(last);});
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_hot int utils_shm_join_id(const utils_shm_t* shm, utils_shm_id_t id, bool last)
{
    return utils::exception::c_api::guard([&](void) {utils::exception::c_api::not_null_ref(shm, "shared memory").join(utils::encapsulation::shm::Id{id.id, id.ownership}, last);});
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_hot utils_shm_messages_t* utils_shm_read(utils_shm_t* shm, utils_shm_read_filter_t filter)
{
    return utils::exception::c_api::guard_fallback(nullptr, [&](void) -> utils_shm_messages_t* {return messages_of(utils::exception::c_api::not_null_ref(shm, "shared memory").read(filter_of(filter)));});
}

_hot utils_shm_messages_t* utils_shm_read_id(utils_shm_t* shm, utils_shm_id_t id)
{
    return utils::exception::c_api::guard_fallback(nullptr, [&](void) -> utils_shm_messages_t* {
        const utils::encapsulation::shm::Id key{id.id, id.ownership};
        std::optional<std::vector<std::vector<std::byte>>> read = utils::exception::c_api::not_null_ref(shm, "shared memory").read(key);
        utils_shm_messages_t* res = nullptr;

        if (!read) return nullptr;
        res = new utils_shm_messages_t();
        res->messages.emplace_back(key, std::move(*read));
        return res;
    });
}

//----------------------------------------------------------------//
/* SHARED MEMORY MESSAGES */

_cold void utils_shm_messages_destroy(utils_shm_messages_t* messages)
{
    (void)utils::exception::c_api::guard([&](void) {delete messages;}); // like free(NULL)
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_hot size_t utils_shm_messages_count(const utils_shm_messages_t* messages)
{
    return utils::exception::c_api::guard_fallback(0, [&](void) -> std::size_t {return utils::exception::c_api::not_null_ref(messages, "messages").messages.size();});
}

_hot int utils_shm_messages_id(const utils_shm_messages_t* messages, size_t i, utils_shm_id_t* id)
{
    return utils::exception::c_api::guard([&](void) {
        utils_shm_id_t& out = utils::exception::c_api::not_null_ref(id, "id");
        const utils::encapsulation::shm::Id& res = utils::exception::c_api::not_null_ref(messages, "messages").messages.at(i).first;

        out = {res.id, res.ownership};
    });
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_hot size_t utils_shm_messages_size(const utils_shm_messages_t* messages, size_t i)
{
    return utils::exception::c_api::guard_fallback(0, [&](void) -> std::size_t {return utils::exception::c_api::not_null_ref(messages, "messages").messages.at(i).second.size();});
}

_hot const void* utils_shm_messages_get(const utils_shm_messages_t* messages, size_t i, size_t j, size_t* size)
{
    return utils::exception::c_api::guard_fallback(nullptr, [&](void) -> const void* {
        const std::vector<std::byte>& payload = utils::exception::c_api::not_null_ref(messages, "messages").messages.at(i).second.at(j);

        if (size) *size = payload.size();
        return payload.data();
    });
}
