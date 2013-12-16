<!-- maruku -o format.html format.md -->

<style type="text/css">
pre>code { display: block; margin-left: 2em; }
code { white-space: pre-wrap; }
ins { text-decoration: none; font-weight: bold; background-color: #A0FFA0 }
del { text-decoration: line-through; background-color: #FFA0A0 }
</style>

<table><tbody>
<tr><th>Doc. no.:</th>	<td>Dnnnn</td></tr>
<tr><th>Date:</th>	<td>2013-10-4</td></tr>
<tr><th>Project:</th>	<td>Programming Language C++, Library Evolution Working Group</td></tr>
<tr><th>Reply-to:</th>	<td>Zhihao Yuan &lt;zy at miator dot net&gt;</td></tr>
</tbody></table>

# C++ String Formatting

* TOC
{:toc}

## Overview

The advantage of output text by substituting a compact format string
(domain-specific language, if you must) with multiple arguments compared to
mixing `<<` operators, `"string literals"`, manipulators, and variables, is now
well-understood, and that is why, I believe, LEWG showed great interests on
standardizing a type-safe `printf``[1]`.  However, despite of its
backward-compatibility advantage, printf-style text formatting syntax has its
weakness which cannot be overcome:

- Cryptical positional specification.

  Compare

  ``"%1$s=%2$*3$d,%4$*3$d\n"``

  with

  ``"{1:s}={2:*3d},{4:*3d}\n"``

  The traditional `printf` (as specified in the C standard) does not support
  positional arguments; POSIX added them.  However, without explicit
  boundaries (`{` and `}` in the example above), it's hard for a human to
  figure out the meaning of those numbers.

- Limited extensibility.

  You may want to write

  ``format("{:ms} past!", 1s + 100ms);  // not specified in this paper``

  , but `printf` has no such extensibility.  GNU`[2]`'s version has some --
  you can register new conversion specifications (the last character) globaly,
  while 12 of 26 are already occupied.  N3716`[1]` accepts user-defined types,
  but flags and type hints are all predefined.

  Due to the lack of type information of `varargs`, `printf` is born to be
  format driven, and this property is inherited by N3716 to achieve
  backward-compatibility to both `printf` and C++ streams.  However, the
  successes of C#`[3]`, Python`[4]`, plus the exploration done by Folly`[5]`
  already showed that the type-specific formatting is the key to achieve
  greatest extensibility.

This paper demonstrate a modern string formatting design for C++, based on
the design of the existing extensible solutions mentioned above.


## Impact on the Standard

Two formatting interfaces are proposed here: an overload set of a `format`
function, and a function object type, `ostream_format`, to generate the
`format`-like function objects which write to the output streams.

The most significant change is a new framework to customize the formatting of
the fundamental types, the standard library types, and the user-defined types.
The framework is consisted of:

- `formatter<T>`, a class template to be specialized by the users for a decayed
  type `T`.  A specialization may be constructed with a piece of the format
  string, and an object of the specialization will be used to perform the
  actual formatting.

- `format_writter<StringType>`, a class template as a proxy connecting the
  formatting result (an object of `basic_string`) and the output method of
  a `formatter<T>` specialization.

The proposed library depends on N3762`[5]`; `basic_string_view` is explicitly
required by both the formatting interfaces and the `formatter<T>`
specifications as the format strings.


## Design Decisions

### Syntax

- Ordered flags.

  `printf` format flags (**`+`**, **`#`**, **`0`**, etc.) can appear in any
  order, more than once, and need a set of overriding rules to work.  This
  gives the same formatting instruction multiple representations, which is not
  intuitive for code review, and adds unnecessary complexity to the parser.

  The practice of ordered flags is succeeded in Python:
  `[[fill]align][sign][#][0][width][,][.precision][type]`

- Support both sequential and positional access to arguments.

  Positional access is usually more explicit,

  ``"Error: {0}: {1} {2}"``

  , and the feature itself is necessary for internationalization.  But
  sequential access also has its market since `printf`.  I try not to over
  design here.

  Note that the syntax proposed in this paper uses count from _1_ instead of
  _0_, so the example above will be

  ``"Error: {1}: {2} {3}"``

  , and `0` is just not a valid character to start a decimal integer in a
  format string to avoid `00` (and many other complexities).  For the same
  reason, C++ the language category `0` as an octal literal instead of decimal.

### Interface

The formatting facilities proposed in this paper target `basic_string`.  This
design clearly separates I/O and formatting with data, and brings the following
advantages:

1. No locale overhead, or, the locale support, if being added, can be optional.
   A side-effect of doing this is that the supported character types are
   limited to those with implied encodings (`char`, `wchar_t`, `char16_t`, and
   `char32_t`); we see no issue with it in `<filesystem>` `[6]`.

2. No virtual calls.

3. Zero overhead, for whatever "overhead" means, when the user wants a string.
   Construct a `stringstream`, output to it, and copy the result out, is both
   costly and nonintuitive, and brings inexplicable error handling.

4. Works with any third-party I/O solutions.  I/O is a big topic in real-world
   applications.  A formatting solution engaged with a specific I/O library
   is rather limited.

5. Output integrity.  What happens if a formatting error occurs in the middle
   of a chain of `<<` calls?  To avoid incomplete lines to be left in the log,
   log4cxx`[7]` has to buffer the result with an additional string stream.
   Having a format buffer in front of an I/O buffer solves the problems like
   this, may help solving the multi-thread problem with streams`[8]`, and
   brings trivial strong exception safety.

The last question is how this design affects the performance.  Some early
benchmarks show that this depends on the ratio between the fixed content and
the formatted content, and the complexity of the formatting.  More work need
to be done.

### Customizability

As mentioned in [Impact on the Standard](#impact_on_the_standard), the
proposed formatting facilities have maximized the extensibility -- imagine
having a `formatter<locale_sensitive<T>>`.  But a `formatter` is not
omnipotent:

1. Justification is not customizable; it's done automatically and enforced.

   Consider the following `operator<<` which formats a fraction number:

   ``template <typename CharT, template Traits>
   auto operator<<(basic_ostream<CharT, Traits>& out, Fraction const& v)
   {
       return out << v.den << '/' << v.num;
   }``

   The function above is intuitive, but it's incorrectly sensible to `fmtflags`
   (plus locale), and `width()`.  The first issue just doesn't exist in other
   formatting solutions, and the later can be forgot in the proposed
   solution.  Because the justification specifications, for example,
   `"<10"` in `"{1:<10#d}"`, are not observable to a `formatter`, and a
   `formatter` has no access to the underlying format buffer.  Justification
   is then performed by the `format` machinery.

2. Format specification is exposed to `formatter`.

   There is no predefined formatting information like `fmtflags`.  They are
   either error-prone (as in `printf`, lots of undefined behaviors), or add
   parsing overhead to all `formatter`s (as in Folly).  The proposed
   solution passes the format specifications, for example, `"#d"` in
   `"{1:<10#d}"`, as `basic_string_view`s, to `formatter<T>`'s constructor.

   The practice of type-specific parsing can be found in Python.

3. Format specification is optional to a `formatter` class.

   There are lots of interests of formatting objects with the "default" format,
   and a user may furtherly want a `formatter` which respond to no format
   specification.  The proposed solution always default constructs
   `formatter<T>` when the format specification is empty, and throws an
   exception when it's not but the `formatter<T>` cannot be constructed with
   a `basic_string_view`.  Thus, the user can simply forget about everything
   explained in 2) and 3) to get the precise behavior that he/she wants.

## Technical Specifications

### Header `<format>`

    namespace std {
    namespace experimental {

    }}

## Sample Implementation

A sample implementation is available at
<https://github.com/lichray/stdformat>; many `formatter`s are not implemented
yet. 

## Future Issues

## References
