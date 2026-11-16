+++
title = "System"
summary = "Tools for the program itself: unique ids, delayed tasks, a pool of workers and typed memory kept on the GPU (CUDA, OpenCL, Vulkan) or in the RAM."
keywords = "system IdHandler id allocate free Scheduler schedule cancel LoadBalancer worker HybridMemory gpu cuda opencl vulkan MemoryMode IGpu utils_hybrid_memory utils_id_handler utils_scheduler"
+++

| Define | Group | Namespace | Headers |
|---|---|---|---|
| `_System` | `_Tools` | `utils::system` | `utils/system/` |

Tools for the program itself: id distribution, delayed tasks, a pool of workers and memory kept on the GPU.

## IdHandler {#idhandler}

`IdHandler<T>` (`T` integer) gives unique ids and reuses the freed ones first. For an unsigned `T` the ids start at `1` (`0` is reserved as "no id"); for a signed `T` they start at `std::numeric_limits<T>::min()`. Every method takes `safe_mode = true` (lock, thread safe).

| Method | Description |
|---|---|
| `allocate()` / `allocate(T& id)` | Give a free id (and store it in `id`) |
| `free(id)` | Give back an id (`DoubleFree`, `UnknownId` for an id never given) |
| `use(id)` | Force an id as used (`DoubleUse` when already used) |
| `preview()` | Next id `allocate` would give |
| `id()` / `actual()` | Counter / last id given |
| `clear()` / `free()` | Free every id |

> [!WARNING]
> When the type limit is reached, `allocate` throws `FatalException(IdOverflow)` (ends the program), `preview` an `ErrorException(IdOverflow)`.

## Scheduler {#scheduler}

Runs a function after a delay, each task on its own thread.

| Method | Description |
|---|---|
| `schedule(delay, fn)` | Run `fn()` after `delay` (`std::chrono::milliseconds`), returns the task id |
| `cancel(id)` | Cancel one task (`UnknownId` when finished / unknown) |
| `cancel()` | Cancel every pending task (also done by the destructor) |

```cpp
utils::system::Scheduler scheduler;
std::size_t id = scheduler.schedule(std::chrono::milliseconds{500}, [](void) {std::cout << "later" << std::endl;});
scheduler.cancel(id);
```

## LoadBalancer {#loadbalancer}

`LoadBalancer<T>(limit = 1, lifespan = 0ms)` keeps a pool of workers (`T` is a `utils::type::Worker` or derives it, see [Type](page:type#worker)) and gives a free one.

| Method | Description |
|---|---|
| `spawn(n = 1)` | Add `n` workers (`OutOfBounds` over the limit, `0` = no limit) |
| `getWorker()` | `std::future<T&>` resolved with a free worker, marked working (waits until one is free) |
| `kill()` / `kill(n)` | Remove the free workers / `n` workers (the free ones first) |
| `kill<true>()` / `kill<true>(n)` | Same, also removing the working ones |
| `setLimit(n)` / `getLimit()` | Maximum number of workers |
| `setLifespan(ms)` / `getLifespan()` | A worker free for longer is removed (`0` = never) |
| `size()` | Number of workers |

The worker is given back with `worker.setWorkingStatus(false)`. A `getWorker` still waiting when the balancer is destroyed throws `PromiseCanceled` from `future.get()`.

```cpp
utils::system::LoadBalancer<utils::type::Worker> balancer(4, std::chrono::seconds{30});
balancer.spawn(2);
utils::type::Worker& worker = balancer.getWorker().get();
// ... work ...
worker.setWorkingStatus(false);
```

## HybridMemory {#hybridmemory}

`HybridMemory<T>` (`T` trivially copyable, header `utils/system/HybridMemory.hpp`) keeps `n` values of `T` in the memory of a GPU when one is available, else in the RAM, for future computes on it. The backend is chosen at the construction with a `MemoryMode`; the GPU libraries are loaded at runtime (`dlopen` / `LoadLibrary`), so nothing is needed to build or link and a machine without GPU simply uses the RAM.

| Mode | Backend |
|---|---|
| `Auto` | First GPU found (`Cuda` → `OpenCL` → `Vulkan`), else the RAM |
| `Gpu` | First GPU found, `GpuUnavailable` without one |
| `Ram` | Host memory only |
| `Cuda` / `OpenCL` / `Vulkan` | Forced backend (`libcuda` / `libOpenCL` / `libvulkan`), `GpuUnavailable` when missing |
| `Custom` | Handler given by the program (`IGpu`, see below) |

| Method | Description |
|---|---|
| `HybridMemory(mode = Auto)` / `HybridMemory(size, mode = Auto)` | Select the backend (and allocate `size` values) |
| `HybridMemory(size, std::shared_ptr<IGpu> gpu)` | `Custom` mode with a handler (`InvalidArgument` when `nullptr` or unavailable) |
| `allocate(size)` / `resize(size)` / `free()` | New memory (uninitialized) / new size, the first values kept / release |
| `store(span<const T>, offset = 0)` | Copy values into the memory from the index `offset` (`OutOfBounds`) |
| `fetch(span<T>, offset = 0)` | Copy `out.size()` values from the memory (`OutOfBounds`) |
| `get(i)` / `set(i, value)` / `operator[]` | One value (a transfer on the GPU: prefer `fetch` / `store` for many) |
| `fill(value)` / `clear()` | Every value set to `value` / to `0` |
| `toVector()` | Whole memory copied in a `std::vector<T>` |
| `join()` | Wait for the pending GPU operations (nothing in RAM) |
| `getMode()` / `getRequestedMode()` / `isOnGpu()` | Mode really used (`Ram` after an `Auto` without GPU) / mode asked / on a GPU handler |
| `size()` / `bytes()` / `empty()` | Number of `T` / size in bytes / nothing allocated |
| `native()` | Device buffer (`CUdeviceptr`, `cl_mem`...) for the computes, `0` in RAM |
| `data()` | Host pointer (RAM only, `nullptr` on a GPU) |
| `getGpu()` | The handler (`IGpu`), `nullptr` in RAM |

The class is move only. `IGpu` (`utils/system/gpu/IGpu.hpp`) is the interface of a backend, implemented by `CudaGpu`, `OpenCLGpu` and `VulkanGpu`: `isAvailable`, `getMode`, `getName`, `getMemory`, `allocate`, `free`, `store`, `fetch`, `memset`, `memcpy`, `join` (offsets in bytes). `make_gpu(mode)` (and `make_raw_gpu` / `make_shared_gpu` / `make_unique_gpu`) builds one by hand, `nullptr` for `Ram`.

> [!NOTE]
> `HybridMemory` only stores and moves the data: the computes are written with the API of the backend on `native()` (kernel launch, `cl_mem` argument...). Every call of the `Cuda` handler makes its context current on the calling thread.

> [!WARNING]
> On a GPU every `get` / `set` is a transfer, and the Vulkan handler creates a staging buffer for each one: use `store` / `fetch` for blocks.

## Example {#example}

```cpp
#define _System
#include <utils/utils.hpp>
#include <vector>

std::vector<float> values = {1.0f, 2.0f, 3.0f, 4.0f};
std::vector<float> out(values.size());

utils::system::HybridMemory<float> memory(values.size());   // Auto: GPU when available, else RAM
memory.store(values);                                       // host -> memory
memory.set(0, 10.0f);                                       // one value
memory.join();                                              // wait for the GPU
memory.fetch(out);                                          // memory -> host: {10, 2, 3, 4}

utils::system::HybridMemory<float> vulkan(8, utils::system::MemoryMode::Vulkan);   // GpuUnavailable without Vulkan
```

## Errors {#errors}

| Code | Raised when |
|---|---|
| `GpuUnavailable` | A forced backend (or `Gpu`) isn't available on the machine |
| `GpuAllocation` | The GPU memory allocation failed |
| `GpuTransfer` | A transfer (`store`, `fetch`, `memset`, `memcpy`) failed |
| `OutOfBounds` | A range outside of the memory |
| `OutOfMemory` | The size overflows or the RAM allocation failed |
| `InvalidArgument` | `Custom` without a handler (or with an unavailable one) |

## In C {#in-c}

| C++ | C |
|---|---|
| `IdHandler<T>` (integer `T`) | `utils_id_handler_<name>_t*` (`<name>` = `i8` ... `u64`, `size`): `_allocate(handler, safe_mode)` (`0` on error), `_free`, `_use`, `_preview` |
| `Scheduler` | `utils_scheduler_t*`: `utils_scheduler_schedule(scheduler, delay_ms, task, data)` returns the task id (`0` on error), `utils_scheduler_cancel` / `_cancel_all` |
| `LoadBalancer<Worker>` | `utils_load_balancer_t*` of `utils_worker_t` (a `Worker` carrying a `void* data`): `utils_load_balancer_get_worker` waits for a free one, give it back with `utils_worker_set_working_status(worker, false)` |
| `HybridMemory<T>` | `utils_hybrid_memory_<name>_t*` per fixed type (+ `bool`, `char`), or `utils_hybrid_memory_t*` in bytes |
| `MemoryMode` | `utils_memory_mode_t` (`C_API_MEMORY_MODE_AUTO` ... `_CUSTOM`) |
| `IGpu` | `utils_gpu_t*`: `utils_gpu_create(mode)` for a backend, `utils_gpu_create_custom(&callbacks, data)` for a GPU of the C side (`utils_gpu_callbacks_t`) |

```c
#define _System
#include <utils.h>
#include <stdio.h>

int main(void)
{
    float values[] = {1.0f, 2.0f, 3.0f, 4.0f};
    float out[4] = {0};
    utils_hybrid_memory_f32_t* memory = utils_hybrid_memory_f32_create(4, C_API_MEMORY_MODE_AUTO); // GPU when available, else RAM

    if (!memory
        || utils_hybrid_memory_f32_store(memory, values, 4, 0) != UTILS_OK
        || utils_hybrid_memory_f32_set(memory, 0, 10.0f) != UTILS_OK
        || utils_hybrid_memory_f32_join(memory) != UTILS_OK
        || utils_hybrid_memory_f32_fetch(memory, out, 4, 0) != UTILS_OK) {
        fprintf(stderr, "%s\n", utils_exception_format());
        utils_hybrid_memory_f32_destroy(memory);
        return 1;
    }
    printf("%g %g %g %g (on the gpu: %d)\n", out[0], out[1], out[2], out[3], utils_hybrid_memory_f32_is_on_gpu(memory));
    utils_hybrid_memory_f32_destroy(memory);
    return 0;
}
```
