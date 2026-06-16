---
title: MIDD
section: 1
date: June 2026
header: Midd User Manual
footer: Version 1.0.2
author: Nikole Smith
---

# NAME
midd - extracts precise, bounded intervals of lines from a stream or file

# SYNOPSIS
**midd** [**-i** *input*] [**-o** *output*] [**-r** *range*] [**-l**] [**-1**] [**-q**] [**-p**]

# DESCRIPTION
**midd** extracts precise, bounded intervals of lines from a stream or file. Unlike traditional tools that load entire datasets into memory or continue reading files long after the target range has passed, **midd** utilizes a fail-fast validation layer and terminates execution the exact moment its upper-bound constraint is satisfied.

# OPTIONS
**-i** *input*
: Path to the input file. Default is STDIN.

**-o** *output*
: Path to the output file. Default is STDOUT.

**-r** *range*
: The target line range formatted strictly as *start*:*end*. Default is `$:$` (full file).

**-l**
: Enables visual line numbering prefixes in the output. Default is disabled.

**-1**
: Shifts range parsing and line numbering to 1-indexed mode. Default is 0-indexed.

**-q**
: Quiet mode. Suppresses informational slice boundary markers. Default is disabled.

**-p**
: Printable-only mode. Strips non-printable characters. Default is disabled.

# RANGE SYNTAX
The **-r** flag accepts flexible bounding parameters using an integer or the `$` symbol (representing End-of-File):

* **10:50**
: Extracts from line 10 to line 50.

* **100:$** or **100:**
: Extracts from line 100 to the end of the file.

* **$:25** or **:25**
: Extracts from the beginning of the file up to line 25.

# EXAMPLES
**Extracting a Specific Slice (0-Indexed, with Line Numbers)**
```
midd -i raw_data.txt -r 10:15 -l
```
Output:
```
[ SLICE START: 10 ]
10 | Eleventh line of the file
11 | Twelfth line of the file
12 | Thirteenth line of the file
13 | Fourteenth line of the file
14 | Fifteenth line of the file
15 | Sixteenth line of the file
[ SLICE END: 15 ]
```

**Matching Text Editor Coordinates (1-Indexed, Quiet Output)**
```
midd -i source_code.cpp -r 10:12 -l -1 -q
```
Output:
```
10 | // Tenth line matching your IDE
11 | int main() {
12 |     setup_variables();
```

# CORE ARCHITECTURE & VALIDATION
**Fail-Fast Range Policy**
To prevent wasteful compute cycles or script cascading failures, **midd** validates the range integrity before processing. If a range is provided where the start parameter is greater than the end parameter (e.g., `-r 50:20`), the tool will immediately print an explicit error to STDERR and exit with status code 1.

**Data Stream Sanitization (-p)**
When running in printable-only modes, the utility filters incoming byte streams to sanitize binary noise while preserving structural text alignment. Standard layout control characters (`\n`, `\r`, `\t`) are explicitly allowed through the filter, while raw control characters and non-printable binary junk are dropped.

## License

This project is licensed under the terms of the GNU General Public License version 3 (GPL-3.0). See the [LICENSE](file://LICENSE) file for the full license text.

