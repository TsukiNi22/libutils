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
##  C API of utils::type (Freezable / Worker of any handle, vectors, matrices, bidirectional lookup tables)
\**************************************************************/

#include "utils/type/blt/BidirectionalLookupTable_t.hpp"
#include "utils/exception/basic/ErrorException.hpp"
#include "utils/exception/ExceptionDefine.hpp"
#include "utils/type/vector/OVector2.hpp"
#include "utils/type/vector/OVector3.hpp"
#include "utils/attribute/Attribute.hpp"
#include "utils/type/matrix/OMatrix.hpp"
#include "utils/manip/smanip/CApi.hpp"
#include "utils/exception/CApi.hpp"
#include "utils/type/CApiTypes.hpp"
#include "utils/type/Freezable.hpp"
#include "utils/type/Worker.hpp"
#include "utils/type/CApi.hpp"
#include "utils/type/CApi.h"
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <utility>
#include <chrono>
#include <string>
#include <vector>

//----------------------------------------------------------------//
/* TOOLS */

/* vectors: C struct <-> utils::type::OVector2 / OVector3 */
template<typename T, typename V>
_hot _nodiscard static utils::type::OVector2<T> o2(const V& v) {return {v.x, v.y};};
template<typename V, typename T>
_hot _nodiscard static V c2(const utils::type::OVector2<T>& v) {return V{v.x, v.y};};
template<typename T, typename V>
_hot _nodiscard static utils::type::OVector3<T> o3(const V& v) {return {v.x, v.y, v.z};};
template<typename V, typename T>
_hot _nodiscard static V c3(const utils::type::OVector3<T>& v) {return V{v.x, v.y, v.z};};

/* matrices: operations (new copy of the result) */
template<typename T>
struct MatrixOp {
    using M = utils::type::OMatrix<T>;

    // ------------ Function ---------- //
    _hot _nodiscard static inline M add(const M& a, const M& b)       {return a + b;};
    _hot _nodiscard static inline M sub(const M& a, const M& b)       {return a - b;};
    _hot _nodiscard static inline M mul(const M& a, const M& b)       {return a * b;};
    _hot _nodiscard static inline M div(const M& a, const M& b)       {return a / b;};
    _hot _nodiscard static inline M mul_scalar(const M& a, const T s) {return a * s;};
    _hot _nodiscard static inline M div_scalar(const M& a, const T s) {return a / s;};
    _hot _nodiscard static inline M neg(const M& a)                   {return -a;};
    _hot _nodiscard static inline M transpose(const M& a)             {M res = a; res.transpose(); return res;};
    _hot _nodiscard static inline M invert(const M& a)                {M res = a; res.invert(); return res;};
};

// Matrix of a handle given by the C side
template<typename H>
_hot _nodiscard static decltype((std::declval<H&>().matrix)) matrix(H* handle) {return utils::exception::c_api::not_null_ref(handle, "matrix").matrix;};

// Result of an operation in a new handle (NULL on error)
template<typename H, typename F>
_hot _nodiscard static H* matrix_new(F&& f) {return utils::exception::c_api::guard_fallback(nullptr, [&](void) -> H* {return new H(f());});};

// Result of an operation in out (computed before: out can be an operand)
template<typename H, typename F>
_hot _nodiscard static int matrix_r(H* out, F&& f) {return utils::exception::c_api::guard([&](void) {utils::exception::c_api::not_null_ref(out, "output").matrix = f();});};

/* bidirectional lookup tables */
template<typename H>
_hot _nodiscard static H& table(H* handle) {return utils::exception::c_api::not_null_ref(handle, "table");};

// Elements of the C side (count of them, NULL only with a count of 0)
template<typename C>
_cold _nodiscard static std::vector<utils::type::c_api::cpp_t<C>> elements(const C* values, const std::size_t count)
{
    std::vector<utils::type::c_api::cpp_t<C>> res;

    if (count > 0) (void)utils::exception::c_api::not_null(values, "elements");
    for (std::size_t i = 0; i < count; ++i)
        res.push_back(utils::type::c_api::Storage<C>::cpp(values[i]));
    return res;
}

//----------------------------------------------------------------//
/* FREEZABLE */

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_cold int utils_freezable_freeze(void* handle)
{
    return utils::exception::c_api::guard([&](void) {utils::exception::c_api::handle_base<utils::type::Freezable>(handle, "Freezable").freeze();});
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_hot bool utils_freezable_is_frozen(const void* handle)
{
    return utils::exception::c_api::guard_fallback(false, [&](void) -> bool {return utils::exception::c_api::handle_base<utils::type::Freezable>(handle, "Freezable").isFrozen();});
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_hot int utils_freezable_require_frozen(const void* handle)
{
    return utils::exception::c_api::guard([&](void) {utils::exception::c_api::handle_base<utils::type::Freezable>(handle, "Freezable").requireFrozen();});
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_hot int utils_freezable_require_unfrozen(const void* handle)
{
    return utils::exception::c_api::guard([&](void) {utils::exception::c_api::handle_base<utils::type::Freezable>(handle, "Freezable").requireUnfrozen();});
}

//----------------------------------------------------------------//
/* WORKER */

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_cold utils_worker_t* utils_worker_create(void* data)
{
    return utils::exception::c_api::guard_fallback(nullptr, [&](void) -> utils_worker_t* {return new utils_worker_t(data);});
}

_cold void utils_worker_destroy(utils_worker_t* worker)
{
    (void)utils::exception::c_api::guard([&](void) {delete worker;}); // like free(NULL)
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_hot void* utils_worker_get_data(const utils_worker_t* worker)
{
    return utils::exception::c_api::guard_fallback(nullptr, [&](void) -> void* {return utils::exception::c_api::not_null_ref(worker, "worker").data;});
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_hot int utils_worker_set_data(utils_worker_t* worker, void* data)
{
    return utils::exception::c_api::guard([&](void) {utils::exception::c_api::not_null_ref(worker, "worker").data = data;});
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_hot int utils_worker_set_working_status(void* handle, bool status)
{
    return utils::exception::c_api::guard([&](void) {utils::exception::c_api::handle_base<utils::type::Worker>(handle, "Worker").setWorkingStatus(status);});
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_hot bool utils_worker_is_working(const void* handle)
{
    return utils::exception::c_api::guard_fallback(false, [&](void) -> bool {return utils::exception::c_api::handle_base<utils::type::Worker>(handle, "Worker").isWorking();});
}

_cold uint64_t utils_worker_get_stoped_working_timestamp(const void* handle)
{
    return utils::exception::c_api::guard_fallback(0, [&](void) -> std::uint64_t {
        const std::chrono::steady_clock::time_point time = utils::exception::c_api::handle_base<utils::type::Worker>(handle, "Worker").getStopedWorkingTimestamp();

        return static_cast<std::uint64_t>(std::chrono::duration_cast<std::chrono::nanoseconds>(time.time_since_epoch()).count());
    });
}

//----------------------------------------------------------------//
/* VECTOR */

C_API_NUMERIC_TYPES(C_API_VECTOR2_DEF)
C_API_INT_TYPES(C_API_VECTOR2_INT_DEF)
C_API_NUMERIC_TYPES(C_API_VECTOR3_DEF)
C_API_INT_TYPES(C_API_VECTOR3_INT_DEF)

//----------------------------------------------------------------//
/* MATRIX */

C_API_NUMERIC_TYPES(C_API_MATRIX_DEF)
C_API_FLOAT_TYPES(C_API_MATRIX_FLOAT_DEF)

//----------------------------------------------------------------//
/* BIDIRECTIONAL LOOKUP TABLE (bytes) */

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_cold utils_blt_t* utils_blt_create(void)
{
    return utils::exception::c_api::guard_fallback(nullptr, [](void) -> utils_blt_t* {return new utils_blt_t();});
}

_cold void utils_blt_destroy(utils_blt_t* blt)
{
    (void)utils::exception::c_api::guard([&](void) {delete blt;}); // like free(NULL)
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_cold int utils_blt_clear(utils_blt_t* blt)
{
    return utils::exception::c_api::guard([&](void) {table(blt).clear();});
}

_cold int utils_blt_set(utils_blt_t* blt, const void* left, size_t left_size, const void* right, size_t right_size, bool force)
{
    return utils::exception::c_api::guard([&](void) {
        utils_blt_t& self = table(blt);

        if (force) self.setElement<true>(utils::smanip::c_api::bytes(left, left_size, "element"), utils::smanip::c_api::bytes(right, right_size, "element"));
        else self.setElement<false>(utils::smanip::c_api::bytes(left, left_size, "element"), utils::smanip::c_api::bytes(right, right_size, "element"));
    });
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_cold int utils_blt_add(utils_blt_t* blt, const void* left, size_t left_size, const void* right, size_t right_size)
{
    return utils::exception::c_api::guard([&](void) {table(blt).addElement(utils::smanip::c_api::bytes(left, left_size, "element"), utils::smanip::c_api::bytes(right, right_size, "element"));});
}

// xstyle: ignore-next CPP-SINGLE-STATEMENT (C API: defined in the .cpp, the header is C)
_cold int utils_blt_remove(utils_blt_t* blt, const void* element, size_t size)
{
    return utils::exception::c_api::guard([&](void) {table(blt).removeElement(utils::smanip::c_api::bytes(element, size, "element"));});
}

_hot const void* utils_blt_get(const utils_blt_t* blt, const void* element, size_t size, size_t* other_size)
{
    return utils::exception::c_api::guard_fallback(nullptr, [&](void) -> const void* {
        const std::string& other = table(blt)[utils::smanip::c_api::bytes(element, size, "element")];

        if (other_size) *other_size = other.size();
        return other.data();
    });
}

//----------------------------------------------------------------//
/* BIDIRECTIONAL LOOKUP TABLE (fixed types) */

C_API_BLT_SAME_DEF(str_str, const char*)
C_API_BLT_SAME_DEF(size_size, size_t)
C_API_BLT_SAME_DEF(i64_i64, int64_t)
C_API_INT_TYPES(C_API_BLT_STR_INT_DEF)
