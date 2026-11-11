# Heliogrim code style (agent notes)

Derived from the owner's refactor pass over `feat/actor-archetypes` (2026-11-07). Formatting itself is owned by
ReSharper/CLion (`.resharper-format.xml`): reformat, do not hand-format against it. This guide wins over
`.clang-format`, which only approximates ReSharper; it is aligned on everything it can express, and the known
residue is not a finding: a lambda passed as an argument stays on its own line with `);` closing on its own line,
`template <...> requires (...)` shares one line, a constructor's `:` stays after a trailing `requires`, and ReSharper
breaks some arguments and template-argument lists that still fit in 140 columns.

## Formatter-owned (just reformat the file)
- Include order: one sorted block of `<...>` (std first, then `<Bundle/...>`), blank line, then `"..."` local includes.
  No extra "Engine.Common first" block above the std headers.
- Column limit 140; a signature that fits goes on one line, otherwise one parameter per line.
- Ternaries break after `?` and `:`; trailing comments have no space before `//`.
- Every file ends with a newline.

## Includes
- Include what you use and nothing else. Drop includes that are unused (`<vector>`, `<memory>`, `<string_view>`, `<cstring>`, ...).
- Prefer engine wrappers (`Engine.Common/Collection/Vector.hpp`, `Wrapper.hpp`) over the raw std header.
- A header the `.hpp` does not need moves into the `.cpp` (e.g. `Make.hpp` lives in `Registry.cpp`, not `Registry.hpp`).
- Headers are self-contained: never rely on include order in the consumer (e.g. `ActorScheme.hpp` includes `SerializationModule.hpp`).
- Tests: `#include "pch.h"`, blank line, `/**/`, blank line, then the sorted block.

## Types and parameters
- Spell raw pointers `ptr<T>` everywhere: members, parameters, function-pointer signatures, casts
  (`static_cast<ptr<std::byte>>`, `reinterpret_cast<ptr<Type_>>`). `auto*` locals are fine.
- New signatures take `ref<const T>`, not `cref<T>` (const stays explicit on the handled type).
- Structs larger than a couple of words (e.g. `ArchetypeOps`) go by `ref<const T>`, not by value.
- `const` on by-value parameters is kept in declarations as well as definitions.
- Build an `Opt<T>` by value with `SomeVal<T>(args...)` instead of an implicit braced conversion.
- order is `inline constexpr static`

## Class layout
- One access label per group, repeated even when the access does not change. Order:
  1. friends (under their own label, usually `private:`),
  2. constants and nested types,
  3. constructors / destructor (private constructors right after the public ones),
  4. operators, grouped by family: all `==`/`!=` (incl. `= delete` overloads), then all `<=>`,
  5. data members (`private:`),
  6. accessors and methods, mutation before queries; test-only/debug API in its own trailing `public:` group,
  7. static factories (e.g. `Handle::make`) last.
- Private section order: nested types, then data members, then private helper methods, each under its own `private:`.
- One blank line between consecutive declarations: struct members with comments, enum declarations,
  macro invocations, explicit instantiations.

## Source files
- Definitions in the `.cpp` follow the declaration order of the `.hpp`; private helpers go last.
- File-local helpers sit in an anonymous namespace: forward-declared after `using namespace ...;` and defined
  at the end of the file behind a `/**/` separator.
- `/**/` separates logical sections inside long functions and files.
- Exhaustive `if / else if` chains over an enum end in `else { std::unreachable(); }` rather than falling through.
- One-statement lambdas stay on one line: `[this] { _x = make(); }`.

## Serialization files
- `X.hpp` holds only the type. No `access::Layout<X>` / `access::Structure<X>` code goes in it, apart from the
  `ACCESS_LAYOUT` / `ACCESS_STRUCTURE` friend macros.
- `X.Structured.cpp`: explicit specializations of `access::Structure<X>::serialize/hydrate`.
- `X.Layout.hpp` + `X.Layout.cpp`: the `access::Layout<X>` specialization (version, size, dump/load declarations)
  and its definitions. Only archetype-defining `.cpp` files (the ones that include `ArchetypeDefinition.inl.hpp`)
  and tests include `X.Layout.hpp`. If it is missing, the `dumpable` static_assert fires.

## Formatter artifacts seen in the pass (do not copy)
- A scope block glued onto the previous statement: `Handle stale; {` and `auto archive = BufferArchive {}; {`.
- A member access split across lines: `desc_.\n\tstride`.
These come from ReSharper's brace/wrap settings. Write the block on its own line, and shorten the condition instead.

## Guidance for classes and structs

```c++
namespace xyz {
    class/struct PascalCaseClassName :
        public InheritedClass {
    <<{ if friend classes }>>
    public/private/protected:
        template <...>
        friend class MyFriendClass;
        
        friend struct Abc;
    <<{ end friend block}>>

    <<{ if usings or constexpr (static) constants }>>
    public/private/protected:
        using this_type = PascalCaseClassName;
        using alloc_trait = ...;
        
        constexpr static TypeId myTypeId ...;
    <<{ end using block }>>

    public/private/protected:
        <<{ Constructors and Destructors }>>
        PascalCaseClassName() noexcept = default;
        
        PascalCaseClassName(mref<PascalCaseClassName> other_) noexcept ...
        
        ~PascalCaseClassName() ...
        
    public/private/protected:
        <<{ Assignments }>>
        ref<PascalCaseClassName> operator=(mref<PascalCaseClassName> other_) noexcept ...
        
        ref<PascalCaseClassName> operator=(ref<const PascalCaseClassName> other_) noexcept ...
    
    <<{ Repeated semantic member blocks }>>
    <<{ Start Block }>>
    private/protected/public:
        float _myFloat;
        ptr<T> _myPtr;
        ref<Abc> _helloWorldAbc;

    private/protected/public:
        <<{ Semantically batched and sorted }>>
        Opt<ref<Abc>> acquire(...);
        
        Opt<span<Abc>> acquireMany(...);
        
        bool release(ref<const Abc> ...);
        
        bool releaseMany(ref<const span<Abc>> ...);
        
    private/protected/public:
        <<{ Second semantic block }>>
        size_t traverse(FnRef<...> ...) const;
        
        void unwind(...);
        
    private/protected/public:
        <<{ Third semantic block }>>
        size_t acquireCount() const;
        
        ...
    <<{ End Block }>>
    };
}
```

## Guidance for functions

```c++
bool myFormattedFunction(auto&&... args_) {
    
    if (not isValidConsumption(args_...)) {
        doReconcile(...);
        return false;
    }
    <<{ space after guard block }>>
    
    if (isConsumptionA(args_...)) {
        doWorkPackage...
        fn(...)
        return true;
        <<{ if followed by else add space before next control block }>>
        
    } else if (isConsumptionB(args_...)) {
        doWhatever...;
    }
    
    <<{ start semantic block / batch }>>
    auto t = deltaTime(...);
    auto speed = getAccel() * t;
    workWithSpeed(speed, ...);
    <<{ end semantic block/batch with follow-up space }>>
    
    <<{ if phase changes or bit semantic breaks or visual breaks use divider }>>
    bigBlock...
    ...
    
    /**/
    
    nextBlock...
    ...
    <<{ end }>>
    
    const auto hasSucceeded = ...;
    doAnything...;
    return hasSucceeded;
}
```