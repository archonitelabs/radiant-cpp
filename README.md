# 😎 Radiant Library (C++)

Radiant is a C++ library that is cross platform, kernel safe, and exception free.

## Guiding Principles

* Exception free and safe
* Kernel compatible
* Platform portable
* Bring your own allocator

## Contributing

Radiant is an open-source project, and we warmly welcome contributions. Please
refer to the [Contributing Guide](CONTRIBUTING.md) for more information.

## Strategic Doctrine

Radiant takes security seriously. C++ is a powerful language that can introduce
vulnerabilities if not used correctly. Radiant cannot prevent this, but it can
help minimize the likelihood of mistakes. Wherever possible, Radiant will
encourage safe practices.

Radiant takes performance seriously. Radiant is designed to be fast and
efficient. Radiant will be regularly profiled and optimized to ensure maximum
performance.

Radiant is designed for use in kernel or other low level environments where the
C++ Standard Library (STL) may not be suitable. Radiant is not intended to be a
drop in replacement for the STL. Where possible, Radiant adopts naming
conventions similar to those of the STL. Radiant will intentionally deviate from
the STL naming conventions when necessary.

Radiant is not restricted to kernel or low level environments. Radiant can be
used in any C++ project (C++ 14 and above). Radiant is designed to be portable
across platforms.

Radiant emphasizes the importance of control over allocations. Any sufficiently
complex system must be capable of reasoning about allocations. Radiant is
designed to provide users with full control over memory allocations.
* Users are required to implement their own allocators.
* Allocators can be scoped globally, per-file, or per-object.
* Implemented allocators must adhere to the Radiant allocator contract.

Radiant will never throw an exception. Radiant will provide error codes and
error handling mechanisms. Radiant is safe from exceptions, guaranteeing that it
will not leak memory, resources, or be left in an undefined state in the event
of an exception.

Radiant will break backwards compatibility and application binary interfaces
when necessary and justified. Radiant is not intended for use with foreign
function interfaces. [Semantic Versioning](https://semver.org/) will be used to
indicate breaking changes.

Radiant acknowledges the fact that C++ is incumbent in many industries. Radiant
is designed to be a modern C++ library that can be used in conjunction with
existing code bases. The success of alternate systems programming languages has
shown that C++ is not the only option. But for many industries and code bases,
C++ is the incumbent and displacing it means an alternative must interoperate
with it seamlessly. While Radiant helps to fill this gap, it does not aim to
displace or discourage the development of alternative libraries or languages.
