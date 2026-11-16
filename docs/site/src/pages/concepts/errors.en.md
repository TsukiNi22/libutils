+++
title = "Errors"
summary = "How libutils reports errors: an exception with a type and a code in C++, a return value and a thread local status in C, and the codes generated from JSON."
section = "concepts"
order = 20
keywords = "errors exception type code restriction failsafe internal external json generated status utils_exception C boundary"
+++

Every error of libutils is an exception carrying a **type** and a **code**, never a return code. The C API turns the same exception into a return value and a status. The classes and the full list of codes are on [Exceptions](page:exception).

## Type and code {#type-code}

The **type** says how serious it is, the **code** what happened:

| Class | Type | Meaning |
|---|---|---|
| `NoneException` | `None` | not an error (ex: help displayed, `Exit`) |
| `WarningException` | `Warning` | the program can continue |
| `ErrorException` | `Error` | error of the call |
| `CustomException` | given | a combination of types |
| `FatalException` | `Fatal` | prints and calls `std::abort()` from its constructor |

A code can restrict the types it accepts: a forbidden combination ends with `FatalException(ExceptionCodeRestriction)`. Many methods take `failsafe` (warn instead of throwing) or `safe_mode` (lock).

```cpp
try {
    server.start();
} catch (const utils::exception::IException& e) {
    if (e.getCode() == utils::exception::InternalCode::AlreadyRunning) return;
    std::cerr << e.formated() << std::endl; // [Error] file:line (origin) -> message / function = info
}
```

## Where the codes come from {#codes}

The codes are generated at the build from `cmake/config/exceptions/**/*.json` (`generated_internal_exception_header.hpp`): `InternalCode` for the library. A project adds its own codes, `ExternalCode`, with the same JSON format and the same generator ([Custom exception codes](page:custom-exceptions)). The generator refuses a code that isn't a C++ identifier, `Undefined` or a duplicate.

| Origin | C++ | C (`utils_exception.origin`) |
|---|---|---|
| a code of libutils | `InternalCode` | `C_API_EXCEPTION_ORIGIN_INTERNAL` |
| a code of your project | `ExternalCode` | `C_API_EXCEPTION_ORIGIN_EXTERNAL` |
| a standard exception | `std::exception` | `C_API_EXCEPTION_ORIGIN_STD` (code `C_API_OTHER_EXCEPTION_CODE`) |
| anything else thrown | - | `C_API_EXCEPTION_ORIGIN_UNKNOWN` |
| a C function (`utils_fail`) | - | `C_API_EXCEPTION_ORIGIN_C` |

## Across the C boundary {#c-boundary}

An exception must never cross into C code (undefined behavior). Each function of the C API runs its C++ code in a **guard**:

1. the call runs in a `try`;
2. on success, only `utils_exception.exception` is set to `false` (no copy of strings on the fast path);
3. on an exception, the guard fills the thread local `utils_exception` (type, code, what, info, file, line, function, module, origin) and the function returns `UTILS_KO`, `NULL` or `0`.

The other way, a C callback that fails returns `UTILS_KO` (or `utils_fail(code, info)`): the C++ side throws an exception with that info, which goes up like any other and ends in the status of the call that ran the callback. Details: [The C API design](page:c-api-design#guard).

:::html
<figure>
<svg viewBox="0 0 760 270" role="img" aria-label="A C program calls the C functions, each one runs the C++ classes in a guard; a failure fills the thread local status utils_exception; a C callback called by the C++ code returns UTILS_OK or utils_fail">
<defs>
<marker id="arrc-fig2-en" viewBox="0 0 10 10" refX="9" refY="5" markerWidth="7" markerHeight="7" orient="auto-start-reverse">
<path class="arrow" d="M0 0L10 5L0 10z"/>
</marker>
<marker id="arrca-fig2-en" viewBox="0 0 10 10" refX="9" refY="5" markerWidth="7" markerHeight="7" orient="auto-start-reverse">
<path class="arrow-accent" d="M0 0L10 5L0 10z"/>
</marker>
</defs>
<rect class="box" x="20" y="40" width="150" height="50" rx="8"/>
<text class="lbl" x="95" y="70" text-anchor="middle">C program</text>
<rect class="box-alt" x="210" y="40" width="160" height="50" rx="8"/>
<text class="lbl" x="290" y="62" text-anchor="middle">utils.h · CApi.h</text>
<text class="small" x="290" y="80" text-anchor="middle">extern "C" functions</text>
<rect class="box-alt" x="410" y="40" width="150" height="50" rx="8"/>
<text class="lbl" x="485" y="62" text-anchor="middle">guard</text>
<text class="small" x="485" y="80" text-anchor="middle">try / catch</text>
<rect class="box" x="600" y="40" width="140" height="50" rx="8"/>
<text class="lbl" x="670" y="62" text-anchor="middle">C++ classes</text>
<text class="small" x="670" y="80" text-anchor="middle">in the handles</text>
<path class="edge" d="M170 65H206" marker-end="url(#arrc-fig2-en)"/>
<path class="edge" d="M370 65H406" marker-end="url(#arrc-fig2-en)"/>
<path class="edge" d="M560 65H596" marker-end="url(#arrc-fig2-en)"/>
<rect class="group" x="330" y="140" width="230" height="44" rx="8"/>
<text class="lbl" x="445" y="167" text-anchor="middle">utils_exception (thread local)</text>
<path class="edge dash" d="M485 90V136" marker-end="url(#arrc-fig2-en)"/>
<text class="small" x="492" y="118">exception</text>
<path class="edge dash" d="M330 162H95V94" marker-end="url(#arrc-fig2-en)"/>
<text class="small" x="105" y="154">UTILS_KO / NULL, read like errno</text>
<rect class="box" x="20" y="210" width="150" height="44" rx="8"/>
<text class="lbl" x="95" y="237" text-anchor="middle">C callback</text>
<path class="edge-accent" d="M690 90V232H174" marker-end="url(#arrca-fig2-en)"/>
<text class="small" x="430" y="224" text-anchor="middle">fn(..., data) → UTILS_OK / utils_fail(code, info)</text>
</svg>

<figcaption>An exception of the C++ code stops at the guard of the C function: it fills <code>utils_exception</code> and the function returns its error value; a failing C callback goes the other way.</figcaption>
</figure>
:::

## Formatted output {#output}

`formated()` in C++, `utils_exception_format()` in C, give the same text, built by `utils::exception::format`:

```text
[<type>] ...<file_path:hyperlink>:<line> (<origin>) -> <message>
-------------------------------------------
<function_demangled> = <info>
-------------------------------------------
```
