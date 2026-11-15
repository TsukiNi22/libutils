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
##  Unit tests of the C API of utils::system (gpu of the C side, hybrid memories, id handlers, load balancer, scheduler)
\**************************************************************/

#include "utils/system/CApi.h"
#include "utils.hpp"
#include <gtest/gtest.h>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <atomic>
#include <chrono>
#include <thread>

class SystemCApiTest: public ::testing::Test {
    protected:
        void SetUp(void) override
        {
            utils_exception_clear();
        };
};

namespace { // namespace start
/* gpu of the C side: the device memory is a malloc of the host */
struct CSideGpu {
    int allocations = 0;
    int stores = 0;
};
} // namespace end

static uintptr_t fakeAllocate(size_t bytes, void* data)
{
    ++static_cast<CSideGpu*>(data)->allocations;
    return reinterpret_cast<uintptr_t>(std::calloc(bytes, 1));
}

static void fakeFree(uintptr_t buffer, void* data)
{
    --static_cast<CSideGpu*>(data)->allocations;
    std::free(reinterpret_cast<void*>(buffer));
}

static int fakeStore(uintptr_t dst, const void* src, size_t bytes, size_t offset, void* data)
{
    ++static_cast<CSideGpu*>(data)->stores;
    std::memcpy(reinterpret_cast<char*>(dst) + offset, src, bytes);
    return UTILS_OK;
}

static int fakeFetch(void* dst, uintptr_t src, size_t bytes, size_t offset, _unused void* data)
{
    std::memcpy(dst, reinterpret_cast<const char*>(src) + offset, bytes);
    return UTILS_OK;
}

static int fakeMemset(uintptr_t buffer, uint8_t value, size_t bytes, size_t offset, _unused void* data)
{
    std::memset(reinterpret_cast<char*>(buffer) + offset, value, bytes);
    return UTILS_OK;
}

static int fakeMemcpy(uintptr_t dst, uintptr_t src, size_t bytes, size_t dst_offset, size_t src_offset, _unused void* data)
{
    std::memmove(reinterpret_cast<char*>(dst) + dst_offset, reinterpret_cast<const char*>(src) + src_offset, bytes);
    return UTILS_OK;
}

static int failingStore(_unused uintptr_t dst, _unused const void* src, _unused size_t bytes, _unused size_t offset, _unused void* data)
{
    return utils_fail(3, "store failed");
}

static const char* fakeName(_unused void* data)
{
    return "fake gpu";
}

/* task of the scheduler */
static void increment(void* data)
{
    ++*static_cast<std::atomic<int>*>(data);
}

//----------------------------------------------------------------//
/* GPU */

TEST_F(SystemCApiTest, GpuOfTheCSideWithAHybridMemory)
{
    CSideGpu fake;
    const utils_gpu_callbacks_t callbacks = {nullptr, fakeName, nullptr, fakeAllocate, fakeFree, fakeStore, fakeFetch, fakeMemset, fakeMemcpy, nullptr};
    utils_gpu_t* gpu = utils_gpu_create_custom(&callbacks, &fake);
    const int32_t values[] = {1, 2, 3, 4};
    int32_t out[4] = {};
    int32_t value = 0;
    bool flag = false;

    ASSERT_NE(gpu, nullptr);
    EXPECT_TRUE(utils_gpu_is_available(gpu));
    EXPECT_EQ(utils_gpu_get_mode(gpu), C_API_MEMORY_MODE_CUSTOM);
    EXPECT_STREQ(utils_gpu_get_name(gpu), "fake gpu");
    utils_hybrid_memory_i32_t* memory = utils_hybrid_memory_i32_create_gpu(4, gpu);
    ASSERT_NE(memory, nullptr) << utils_exception.info;
    EXPECT_TRUE(utils_hybrid_memory_i32_is_on_gpu(memory));
    EXPECT_EQ(utils_hybrid_memory_i32_get_mode(memory), C_API_MEMORY_MODE_CUSTOM);
    EXPECT_EQ(utils_hybrid_memory_i32_data(memory), nullptr);
    EXPECT_NE(utils_hybrid_memory_i32_native(memory), 0u);
    EXPECT_EQ(fake.allocations, 1);
    EXPECT_EQ(utils_hybrid_memory_i32_store(memory, values, 4, 0), UTILS_OK);
    EXPECT_EQ(utils_hybrid_memory_i32_fetch(memory, out, 4, 0), UTILS_OK);
    EXPECT_EQ(out[3], 4);
    EXPECT_EQ(utils_hybrid_memory_i32_fill(memory, 0x01020304), UTILS_OK); // not the same byte: uploaded by chunks
    EXPECT_EQ(utils_hybrid_memory_i32_get(memory, 2, &value), UTILS_OK);
    EXPECT_EQ(value, 0x01020304);
    EXPECT_EQ(utils_hybrid_memory_i32_get(memory, 4, &value), UTILS_KO);
    EXPECT_EQ(utils_exception.code, static_cast<std::uint64_t>(utils::exception::InternalCode::OutOfBounds));
    utils_gpu_t* shared = utils_hybrid_memory_i32_get_gpu(memory);
    ASSERT_NE(shared, nullptr);
    EXPECT_STREQ(utils_gpu_get_name(shared), "fake gpu");
    utils_gpu_destroy(shared);

    utils_hybrid_memory_bool_t* flags = utils_hybrid_memory_bool_create_gpu(3, gpu);
    ASSERT_NE(flags, nullptr);
    EXPECT_EQ(utils_hybrid_memory_bool_fill(flags, true), UTILS_OK);
    EXPECT_EQ(utils_hybrid_memory_bool_get(flags, 1, &flag), UTILS_OK);
    EXPECT_TRUE(flag);
    utils_hybrid_memory_bool_destroy(flags);
    utils_hybrid_memory_i32_destroy(memory);
    utils_gpu_destroy(gpu);
    EXPECT_EQ(fake.allocations, 0);
}

TEST_F(SystemCApiTest, ErrorOfTheGpuOfTheCSide)
{
    CSideGpu fake;
    const utils_gpu_callbacks_t callbacks = {nullptr, nullptr, nullptr, fakeAllocate, fakeFree, failingStore, fakeFetch, fakeMemset, fakeMemcpy, nullptr};
    utils_gpu_t* gpu = utils_gpu_create_custom(&callbacks, &fake);
    utils_hybrid_memory_t* memory = utils_hybrid_memory_create_gpu(8, gpu);

    ASSERT_NE(memory, nullptr);
    EXPECT_EQ(utils_hybrid_memory_store(memory, "abc", 3, 0), UTILS_KO);
    EXPECT_STREQ(utils_exception.what, "store failed");
    EXPECT_STREQ(utils_gpu_get_name(gpu), "[None]");
    EXPECT_EQ(utils_gpu_create_custom(nullptr, nullptr), nullptr);
    utils_hybrid_memory_destroy(memory);
    utils_gpu_destroy(gpu);
}

TEST_F(SystemCApiTest, HybridMemoryInRam)
{
    utils_hybrid_memory_f64_t* memory = utils_hybrid_memory_f64_create(3, C_API_MEMORY_MODE_RAM);
    utils_hybrid_memory_t* bytes = utils_hybrid_memory_create(4, C_API_MEMORY_MODE_RAM);
    char out[5] = {};
    double value = 0.0;

    ASSERT_NE(memory, nullptr);
    EXPECT_EQ(utils_gpu_create(C_API_MEMORY_MODE_RAM), nullptr); // no gpu in ram, not an error
    EXPECT_FALSE(utils_exception.exception);
    EXPECT_FALSE(utils_hybrid_memory_f64_is_on_gpu(memory));
    EXPECT_EQ(utils_hybrid_memory_f64_get_gpu(memory), nullptr);
    EXPECT_EQ(utils_hybrid_memory_f64_size(memory), 3u);
    EXPECT_EQ(utils_hybrid_memory_f64_bytes(memory), 3 * sizeof(double));
    EXPECT_EQ(utils_hybrid_memory_f64_fill(memory, 1.5), UTILS_OK);
    EXPECT_EQ(utils_hybrid_memory_f64_set(memory, 0, 2.5), UTILS_OK);
    EXPECT_DOUBLE_EQ(utils_hybrid_memory_f64_data(memory)[1], 1.5);
    EXPECT_EQ(utils_hybrid_memory_f64_resize(memory, 5), UTILS_OK);
    EXPECT_EQ(utils_hybrid_memory_f64_get(memory, 0, &value), UTILS_OK);
    EXPECT_DOUBLE_EQ(value, 2.5);
    EXPECT_EQ(utils_hybrid_memory_f64_free(memory), UTILS_OK);
    EXPECT_TRUE(utils_hybrid_memory_f64_empty(memory));
    EXPECT_EQ(utils_hybrid_memory_store(bytes, "abcd", 4, 0), UTILS_OK);
    EXPECT_EQ(utils_hybrid_memory_fetch(bytes, out, 2, 1), UTILS_OK);
    EXPECT_STREQ(out, "bc");
    EXPECT_EQ(utils_hybrid_memory_fetch(bytes, out, 2, 3), UTILS_KO);
    utils_hybrid_memory_destroy(bytes);
    utils_hybrid_memory_f64_destroy(memory);
}

//----------------------------------------------------------------//
/* ID HANDLER */

TEST_F(SystemCApiTest, IdHandler)
{
    utils_id_handler_u16_t* handler = utils_id_handler_u16_create();

    ASSERT_NE(handler, nullptr);
    EXPECT_EQ(utils_id_handler_u16_preview(handler, true), 1u);
    EXPECT_EQ(utils_id_handler_u16_allocate(handler, true), 1u);
    EXPECT_EQ(utils_id_handler_u16_allocate(handler, true), 2u);
    EXPECT_EQ(utils_id_handler_u16_use(handler, 3, true), UTILS_OK);
    EXPECT_EQ(utils_id_handler_u16_allocate(handler, true), 4u); // 3 is forced
    EXPECT_EQ(utils_id_handler_u16_use(handler, 3, true), UTILS_KO);
    EXPECT_EQ(utils_id_handler_u16_free(handler, 1, true), UTILS_OK);
    EXPECT_EQ(utils_id_handler_u16_allocate(handler, true), 1u); // reused
    EXPECT_EQ(utils_id_handler_u16_clear(handler, true), UTILS_OK);
    EXPECT_EQ(utils_id_handler_u16_id(handler), 0u);
    utils_id_handler_u16_destroy(handler);
}

//----------------------------------------------------------------//
/* LOAD BALANCER / SCHEDULER */

TEST_F(SystemCApiTest, LoadBalancerGivesWorkersOfTheCSide)
{
    utils_load_balancer_t* balancer = utils_load_balancer_create(2, 0);
    int data = 7;

    ASSERT_NE(balancer, nullptr);
    EXPECT_EQ(utils_load_balancer_spawn(balancer, 2), UTILS_OK);
    EXPECT_EQ(utils_load_balancer_spawn(balancer, 1), UTILS_KO); // over the limit
    EXPECT_EQ(utils_load_balancer_size(balancer), 2u);
    utils_worker_t* worker = utils_load_balancer_get_worker(balancer);
    ASSERT_NE(worker, nullptr);
    EXPECT_TRUE(utils_worker_is_working(worker));
    EXPECT_EQ(utils_worker_set_data(worker, &data), UTILS_OK);
    EXPECT_EQ(utils_worker_set_working_status(worker, false), UTILS_OK);
    EXPECT_EQ(utils_load_balancer_kill_n(balancer, 1, false), UTILS_OK);
    EXPECT_EQ(utils_load_balancer_size(balancer), 1u);
    EXPECT_EQ(utils_load_balancer_set_limit(balancer, 5), UTILS_OK);
    EXPECT_EQ(utils_load_balancer_get_limit(balancer), 5u);
    EXPECT_EQ(utils_load_balancer_set_lifespan(balancer, 100), UTILS_OK);
    EXPECT_EQ(utils_load_balancer_get_lifespan(balancer), 100u);
    utils_load_balancer_destroy(balancer);
}

TEST_F(SystemCApiTest, SchedulerRunsAndCancels)
{
    utils_scheduler_t* scheduler = utils_scheduler_create();
    std::atomic<int> calls = 0;

    ASSERT_NE(scheduler, nullptr);
    const size_t run = utils_scheduler_schedule(scheduler, 1, increment, &calls);
    const size_t canceled = utils_scheduler_schedule(scheduler, 5000, increment, &calls);
    EXPECT_NE(run, 0u);
    EXPECT_NE(canceled, 0u);
    EXPECT_EQ(utils_scheduler_cancel(scheduler, canceled), UTILS_OK);
    for (int i = 0; i < 2000 && calls.load() == 0; ++i)
        std::this_thread::sleep_for(std::chrono::milliseconds{1});
    EXPECT_EQ(calls.load(), 1);
    EXPECT_EQ(utils_scheduler_schedule(scheduler, 1, nullptr, nullptr), 0u);
    EXPECT_STREQ(utils_exception.info, "The task is NULL");
    utils_scheduler_destroy(scheduler);
    EXPECT_EQ(calls.load(), 1);
}
