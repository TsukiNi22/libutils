+++
title = "Algorithms"
summary = "c2dmp-hsm (distance between two strings) and s.o.s (steganography), synced from their own repositories under CC BY-NC-SA 4.0."
keywords = "algorithms c2dmp hsm distance similarity string sos steganography embed extract carrier payload noise utils_c2dmp utils_sos_embed utils_sos_extract"
+++

| Define | Group | Namespaces | Headers |
|---|---|---|---|
| `_Algorithms` | `_Tools` | `utils::algorithms::c2dmp`, `utils::algorithms::sos` | `utils/algorithms/` |
| `_C2DMP` / `_SOS` (from `_Algorithms`) | `_Algorithms` | same | `utils/algorithms/c2dmp-hsm/`, `utils/algorithms/sos/` |

> [!CAUTION]
> These algorithms are synced from [c2dmp-hsm](https://github.com/TsukiNi22/c2dmp-hsm) and [s.o.s](https://github.com/TsukiNi22/s.o.s) and are under **CC BY-NC-SA 4.0** (not the MIT license of libutils): no commercial use without an agreement with the author ([Licenses](page:licenses)).

## c2dmp-hsm {#c2dmp-hsm}

`_C2DMP`: distance between two words, the smaller the closer. Case and Latin-1 accent insensitive, linear time and constant memory. Used for the hints of the [Cli](page:cli).

```cpp
template<std::uint_fast8_t prefixDepthSearch = 3, typename UIntT = std::uint_fast8_t, bool full = false>
float c2dmp(std::string_view a, std::string_view b);
```

| Parameter | Description |
|---|---|
| `prefixDepthSearch` | Depth of the common prefix search |
| `UIntT` | Counter type (`std::uint_fast8_t` up to 255 characters, bigger types for longer strings) |
| `full` | `true`: count the misplaced characters of the whole `a` (`c2dmp_foptimized`), else `c2dmp_optimized` |

Variants: `c2dmp_optimized`, `c2dmp_foptimized`, `c2dmp_simplified`, `c2dmp_fsimplified` (the `simplified` ones are deprecated).

```cpp
#define _C2DMP
#include <utils/utils.hpp>

std::vector<std::string> commands = {"help", "exit", "history"};
std::string typed = "exot";
std::string closest = *std::min_element(commands.begin(), commands.end(), [&](const std::string& a, const std::string& b) {
    return utils::algorithms::c2dmp::c2dmp(typed, a) < utils::algorithms::c2dmp::c2dmp(typed, b);
}); // "exit"
```

## s.o.s {#sos}

`_SOS`: **S**teganography **O**ptimized and **S**ecurized, hides a payload inside a signal (ex: the samples of an audio file) with an imperceptible change, optionally keyed.

| Function | Description |
|---|---|
| `sos_embed<options, magic>(carrier, payload)` / `(carrier, payload, key)` | Hide `payload` in `carrier` (edited in place) |
| `sos_extract<magic>(carrier)` / `(carrier, key)` | Get the payload back |
| `tools::to_bytes<ByteT>(range)` / `tools::bytes_to<Range>(bytes)` | Convert a string / range to the sample type (`Byte` = `std::uint16_t` by default) and back (the result can end with padding) |

| Option | Effect |
|---|---|
| `Option::None` | No noise, only the samples used to hide the data change |
| `Option::Noise` | Noise generated on every sample before hiding the data |
| `Option::GlobalNoise` | Noise generated only on the samples within the accepted amplitude (the ones that can hold data) |

> [!WARNING]
> The payload can use at most 7.5 % of the signal and the carrier needs enough valid samples: otherwise `std::out_of_range`; extracting a carrier without hidden message throws `std::invalid_argument` (the algorithms keep the standard exceptions of their repository).

```cpp
#define _SOS
#include <utils/utils.hpp>

std::vector<std::uint16_t> carrier = /* 16 bits samples */;
std::vector<std::uint16_t> payload = utils::algorithms::sos::tools::to_bytes<std::uint16_t>(std::string("secret"));
utils::algorithms::sos::sos_embed(carrier, payload, key);
std::string message = utils::algorithms::sos::tools::bytes_to<std::string>(utils::algorithms::sos::sos_extract(carrier, key));
```

## In C {#in-c}

The C API is a wrapper of libutils, outside the synced sources. The template parameters become arguments read at runtime.

| C++ | C |
|---|---|
| `c2dmp<depth, UIntT, full>(a, b)` | `utils_c2dmp(a, a_size, b, b_size, prefix_depth, counter_size, full)`: `prefix_depth` from 1 to 5, `counter_size` 1, 2, 4 or 8 bytes, `-1` on error |
| `c2dmp(a, b)` (defaults) | `utils_c2dmp_default(a, b)` (`'\0'` terminated strings) |
| `sos_embed<options>(carrier, payload, key)` | `utils_sos_embed_<name>(carrier, count, payload, payload_count, key, key_count, options)`, `<name>` = `u8`, `u16`, `u32`, `u64`, `options` = `C_API_SOS_OPTION_*` combined with `\|` |
| `sos_extract(carrier, key)` | `utils_sos_extract_<name>(carrier, count, key, key_count, out, max)`: returns the full count of the payload (`0` on error) |

The magic of the embedded payloads is fixed to `C_API_SOS_MAGIC` (`0x22`) in C. The standard exceptions of the algorithms are reported in `utils_exception` with the origin `C_API_EXCEPTION_ORIGIN_STD`.

```c
#define _C2DMP
#include <utils.h>
#include <stdio.h>

int main(void)
{
    const char* commands[] = {"help", "exit", "history"};
    const char* closest = commands[0];

    for (size_t i = 1; i < 3; ++i)
        if (utils_c2dmp_default("exot", commands[i]) < utils_c2dmp_default("exot", closest))
            closest = commands[i];
    printf("%s\n", closest); // exit
    return 0;
}
```
