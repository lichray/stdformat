<!-- maruku -o format.html format.md -->

<style type="text/css">
pre code { display: block; margin-left: 2em; }
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

    ```
    "%1$s=%2$*3$d,%4$*3$d\n"
    ```

  with

    ```
    "{1:s}={2:*3d},{4:*3d}\n"
    ```

  The traditional `printf` (as specified in the C standard) does not support
  positional arguments; POSIX added them.  However, without explicit
  boundaries (`{` and `}` in the example above), it's hard for a human to
  figure out the meaning of those numbers.

- Limited extensibility.

  You may want to write

    ```
    format("{:h} past!", 3h + 30min);  // not specified in this paper
    ```

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

## Design Decisions

### Syntax

### Extensibility

## Technical Specifications

### Header `<format>`

    namespace std {
    namespace experimental {

    }}

### Wording

This is an initial report; a wording can be prepared after a further
discussion.

## Sample Implementation

A sample implementation is available at
<https://github.com/lichray/stdformat>

## Future Issues

## References
