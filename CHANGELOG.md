# @ekx/unit

## 0.0.6

### Patch Changes

- 5636fc4: change all macro identifiers to UPPER_CASE style #16
- 5636fc4: fix disabled mode compilation

## 0.0.5

### Patch Changes

- 7388751: Fix release script, rebuild library with new version 0.0.5

## 0.0.4

### Patch Changes

- 34b4a33: add _doctest-like_ reporter for **CLion IDE** via `-r=xml` command-line option
- 34b4a33: add command-line option `--trace` or `-t` sets tracing printer for extra verbose output mode (for debugging)
- 7b61b83: add `UNIT_IMPLEMENT` and `UNIT_MAIN` compile options
- e30f22c: add `(anonymous)` label for nodes with empty-string name
- 92d2903: add command-line option `--no-colors` to disable terminal ascii codes and unicode characters used for icons
- 92d2903: add command-line option `--silent` or `-s` to turn off reporting to `stdout`
- 92d2903: add command-line option `--animate` or `-a` waits between printed messages (just for fun)

## 0.0.3

### Patch Changes

- 99bd315: change internal state structure
- 80bac54: improve default output
- 4e82b1e: add extra generics: const char*, const void*
- 80bac54: add UNIT_VERBOSE option
- 99bd315: add splitted source code and amalgamation step to provide `unit.h`
