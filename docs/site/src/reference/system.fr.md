+++
title = "System"
summary = "Outils pour le programme lui-même : ids uniques, tâches différées, un pool de workers et une mémoire typée gardée sur le GPU (CUDA, OpenCL, Vulkan) ou dans la RAM."
keywords = "système IdHandler id allocate free Scheduler schedule cancel LoadBalancer worker HybridMemory gpu cuda opencl vulkan MemoryMode IGpu utils_hybrid_memory utils_id_handler utils_scheduler"
+++

| Define | Groupe | Espace de noms | En-têtes |
|---|---|---|---|
| `_System` | `_Tools` | `utils::system` | `utils/system/` |

Outils pour le programme lui-même : distribution d'ids, tâches différées, un pool de workers et une mémoire gardée sur le GPU.

## IdHandler {#idhandler}

`IdHandler<T>` (`T` entier) donne des ids uniques et réutilise d'abord ceux qui ont été libérés. Pour un `T` non signé, les ids commencent à `1` (`0` est réservé pour « pas d'id ») ; pour un `T` signé, ils commencent à `std::numeric_limits<T>::min()`. Chaque méthode prend `safe_mode = true` (verrou, thread safe).

| Méthode | Description |
|---|---|
| `allocate()` / `allocate(T& id)` | Donne un id libre (et le stocke dans `id`) |
| `free(id)` | Rend un id (`DoubleFree`, `UnknownId` pour un id jamais donné) |
| `use(id)` | Force un id comme utilisé (`DoubleUse` s'il est déjà utilisé) |
| `preview()` | Prochain id que donnerait `allocate` |
| `id()` / `actual()` | Compteur / dernier id donné |
| `clear()` / `free()` | Libère tous les ids |

> [!WARNING]
> Quand la limite du type est atteinte, `allocate` lance `FatalException(IdOverflow)` (termine le programme), `preview` une `ErrorException(IdOverflow)`.

## Scheduler {#scheduler}

Exécute une fonction après un délai, chaque tâche sur son propre thread.

| Méthode | Description |
|---|---|
| `schedule(delay, fn)` | Exécute `fn()` après `delay` (`std::chrono::milliseconds`), renvoie l'id de la tâche |
| `cancel(id)` | Annule une tâche (`UnknownId` si elle est terminée / inconnue) |
| `cancel()` | Annule toutes les tâches en attente (aussi fait par le destructeur) |

```cpp
utils::system::Scheduler scheduler;
std::size_t id = scheduler.schedule(std::chrono::milliseconds{500}, [](void) {std::cout << "later" << std::endl;});
scheduler.cancel(id);
```

## LoadBalancer {#loadbalancer}

`LoadBalancer<T>(limit = 1, lifespan = 0ms)` garde un pool de workers (`T` est un `utils::type::Worker` ou en dérive, voir [Type](page:type#worker)) et en donne un libre.

| Méthode | Description |
|---|---|
| `spawn(n = 1)` | Ajoute `n` workers (`OutOfBounds` au-delà de la limite, `0` = sans limite) |
| `getWorker()` | `std::future<T&>` résolu avec un worker libre, marqué occupé (attend qu'un worker se libère) |
| `kill()` / `kill(n)` | Supprime les workers libres / `n` workers (les libres d'abord) |
| `kill<true>()` / `kill<true>(n)` | Pareil, en supprimant aussi ceux qui sont occupés |
| `setLimit(n)` / `getLimit()` | Nombre maximum de workers |
| `setLifespan(ms)` / `getLifespan()` | Un worker libre depuis plus longtemps est supprimé (`0` = jamais) |
| `size()` | Nombre de workers |

Le worker est rendu avec `worker.setWorkingStatus(false)`. Un `getWorker` encore en attente quand le balancer est détruit lance `PromiseCanceled` depuis `future.get()`.

```cpp
utils::system::LoadBalancer<utils::type::Worker> balancer(4, std::chrono::seconds{30});
balancer.spawn(2);
utils::type::Worker& worker = balancer.getWorker().get();
// ... work ...
worker.setWorkingStatus(false);
```

## HybridMemory {#hybridmemory}

`HybridMemory<T>` (`T` trivialement copiable, en-tête `utils/system/HybridMemory.hpp`) garde `n` valeurs de `T` dans la mémoire d'un GPU quand il y en a un, sinon dans la RAM, pour de futurs calculs dessus. Le backend est choisi à la construction avec un `MemoryMode` ; les bibliothèques GPU sont chargées à l'exécution (`dlopen` / `LoadLibrary`), donc rien n'est nécessaire pour le build ou l'édition de liens, et une machine sans GPU utilise simplement la RAM.

| Mode | Backend |
|---|---|
| `Auto` | Premier GPU trouvé (`Cuda` → `OpenCL` → `Vulkan`), sinon la RAM |
| `Gpu` | Premier GPU trouvé, `GpuUnavailable` s'il n'y en a pas |
| `Ram` | Mémoire de l'hôte seulement |
| `Cuda` / `OpenCL` / `Vulkan` | Backend forcé (`libcuda` / `libOpenCL` / `libvulkan`), `GpuUnavailable` s'il manque |
| `Custom` | Handler fourni par le programme (`IGpu`, voir plus bas) |

| Méthode | Description |
|---|---|
| `HybridMemory(mode = Auto)` / `HybridMemory(size, mode = Auto)` | Choisit le backend (et alloue `size` valeurs) |
| `HybridMemory(size, std::shared_ptr<IGpu> gpu)` | Mode `Custom` avec un handler (`InvalidArgument` quand il vaut `nullptr` ou est indisponible) |
| `allocate(size)` / `resize(size)` / `free()` | Nouvelle mémoire (non initialisée) / nouvelle taille, les premières valeurs étant gardées / libération |
| `store(span<const T>, offset = 0)` | Copie des valeurs dans la mémoire à partir de l'indice `offset` (`OutOfBounds`) |
| `fetch(span<T>, offset = 0)` | Copie `out.size()` valeurs depuis la mémoire (`OutOfBounds`) |
| `get(i)` / `set(i, value)` / `operator[]` | Une valeur (un transfert sur le GPU : préférez `fetch` / `store` pour beaucoup de valeurs) |
| `fill(value)` / `clear()` | Toutes les valeurs mises à `value` / à `0` |
| `toVector()` | Toute la mémoire copiée dans un `std::vector<T>` |
| `join()` | Attend les opérations GPU en cours (rien en RAM) |
| `getMode()` / `getRequestedMode()` / `isOnGpu()` | Mode réellement utilisé (`Ram` après un `Auto` sans GPU) / mode demandé / sur un handler GPU |
| `size()` / `bytes()` / `empty()` | Nombre de `T` / taille en octets / rien d'alloué |
| `native()` | Buffer du device (`CUdeviceptr`, `cl_mem`...) pour les calculs, `0` en RAM |
| `data()` | Pointeur hôte (RAM seulement, `nullptr` sur un GPU) |
| `getGpu()` | Le handler (`IGpu`), `nullptr` en RAM |

La classe est seulement déplaçable. `IGpu` (`utils/system/gpu/IGpu.hpp`) est l'interface d'un backend, implémentée par `CudaGpu`, `OpenCLGpu` et `VulkanGpu` : `isAvailable`, `getMode`, `getName`, `getMemory`, `allocate`, `free`, `store`, `fetch`, `memset`, `memcpy`, `join` (offsets en octets). `make_gpu(mode)` (et `make_raw_gpu` / `make_shared_gpu` / `make_unique_gpu`) en construit un à la main, `nullptr` pour `Ram`.

> [!NOTE]
> `HybridMemory` ne fait que stocker et déplacer les données : les calculs s'écrivent avec l'API du backend sur `native()` (lancement de kernel, argument `cl_mem`...). Chaque appel du handler `Cuda` rend son contexte courant sur le thread appelant.

> [!WARNING]
> Sur un GPU, chaque `get` / `set` est un transfert, et le handler Vulkan crée un staging buffer pour chacun : utilisez `store` / `fetch` pour des blocs.

## Exemple {#example}

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

## Erreurs {#errors}

| Code | Levée quand |
|---|---|
| `GpuUnavailable` | Un backend forcé (ou `Gpu`) n'est pas disponible sur la machine |
| `GpuAllocation` | L'allocation de mémoire GPU a échoué |
| `GpuTransfer` | Un transfert (`store`, `fetch`, `memset`, `memcpy`) a échoué |
| `OutOfBounds` | Une plage en dehors de la mémoire |
| `OutOfMemory` | La taille déborde ou l'allocation en RAM a échoué |
| `InvalidArgument` | `Custom` sans handler (ou avec un handler indisponible) |

## En C {#in-c}

| C++ | C |
|---|---|
| `IdHandler<T>` (`T` entier) | `utils_id_handler_<name>_t*` (`<name>` = `i8` ... `u64`, `size`) : `_allocate(handler, safe_mode)` (`0` en cas d'erreur), `_free`, `_use`, `_preview` |
| `Scheduler` | `utils_scheduler_t*` : `utils_scheduler_schedule(scheduler, delay_ms, task, data)` renvoie l'id de la tâche (`0` en cas d'erreur), `utils_scheduler_cancel` / `_cancel_all` |
| `LoadBalancer<Worker>` | `utils_load_balancer_t*` de `utils_worker_t` (un `Worker` portant un `void* data`) : `utils_load_balancer_get_worker` attend un worker libre, rendez-le avec `utils_worker_set_working_status(worker, false)` |
| `HybridMemory<T>` | `utils_hybrid_memory_<name>_t*` par type fixe (+ `bool`, `char`), ou `utils_hybrid_memory_t*` en octets |
| `MemoryMode` | `utils_memory_mode_t` (`C_API_MEMORY_MODE_AUTO` ... `_CUSTOM`) |
| `IGpu` | `utils_gpu_t*` : `utils_gpu_create(mode)` pour un backend, `utils_gpu_create_custom(&callbacks, data)` pour un GPU côté C (`utils_gpu_callbacks_t`) |

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
