# MIDD
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
```sh
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
```sh
midd -i source_code.cpp -r 10:12 -l -1 -q
```
Output:
```
10 | // Tenth line matching your IDE
11 | int main() {
12 |     setup_variables();
```

# Midd is also capable of taking input from stdin and output to stdout
    -if -i is not specified it assumes the input is from stdin
    -if -o is not specified it will output results to stdout
```sh
cat large_file.log | midd -r 100:200 -l > another_file.txt
```


# CORE ARCHITECTURE & VALIDATION
**Fail-Fast Range Policy**
To prevent wasteful compute cycles or script cascading failures, **midd** validates the range integrity before processing. If a range is provided where the start parameter is greater than the end parameter (e.g., `-r 50:20`), the tool will immediately print an explicit error to STDERR and exit with status code 1.

**Data Stream Sanitization (-p)**
When running in printable-only modes, the utility filters incoming byte streams to sanitize binary noise while preserving structural text alignment. Standard layout control characters (`\n`, `\r`, `\t`) are explicitly allowed through the filter, while raw control characters and non-printable binary junk are dropped.

## License

This project is licensed under the terms of the GNU General Public License version 3 (GPL-3.0). See the [LICENSE](LICENSE) file for the full license text.

## Download and build

You can find the latest release at https://github.com/Did-Nikole/Midd/releases/latest



## Requirements

### To Build
- **Compiler**: A C++20 compatible compiler (e.g., GCC 11+ or Clang 13+)
- **Build System**: Make
- **Documentation (Optional)**: Pandoc (needed to build/generate the man page file `midd.1` from markdown)

### To Package (Optional)
- **Debian (`.deb`)**: `dpkg-deb` (standard on Debian/Ubuntu)
- **Red Hat (`.rpm`)**: `rpmbuild`


## Download, Build, & Installation
You can find pre-compiled releases and packages at the [GitHub Releases Page](https://github.com/Did-Nikole/Midd/releases/latest).

### Building from Source
1. Clone the repository:
   ```bash
   git clone https://github.com/Did-Nikole/Midd.git
   cd Midd
   ```
2. Compile the binary and the man page:
   ```bash
   make
   ```
3. Install on your system:
   ```bash
   sudo make install
   ```
   *This installs the `midd` binary to `/usr/local/bin` and the man page to `/usr/local/share/man/man1/midd.1`.*
4. To uninstall:
   ```bash
   sudo make uninstall
   ```
---
## Packaging
You can compile standalone packages natively without installing any files directly to your host:
### Debian Package (`.deb`)
```bash
make deb
```
*Creates the Debian package in the `dist/` folder. You can install it on Debian/Ubuntu/Mint with `sudo dpkg -i dist/midd_*.deb`.*
### RPM Package (`.rpm`)
```bash
make rpm
```
*Creates the RPM package in the `dist/` folder. You can install it on Fedora/CentOS/RHEL with `sudo rpm -i dist/midd-*.rpm` or `sudo dnf install dist/midd-*.rpm`.*
