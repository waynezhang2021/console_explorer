# Console Explorer

A lightweight, keyboard-driven file manager for the Windows console (conhost).

Designed for users who want fast, clutter‑free file navigation without relying on the mouse or heavy GUI.

---

## Features

- **Full keyboard control** – arrows, PgUp/PgDn, Enter, and more.
- **Regex‑powered search** – search for files or folders using regular expressions.
- **Search within search results** (in‑memory recursive filtering) – refine a result set instantly without re‑scanning the disk.
- **Jump to any absolute path** – quickly navigate to a known directory.
- **Open current folder in Explorer** – instantly switch to the graphical file manager.
- **Rename files** – simple inline rename.
- **Multiple windows** – spawn a new independent instance with `Ctrl+N`.
- **Disk selection** – choose from available drives on startup.

---

## Usage

### Navigation

| Key | Action |
|-----|--------|
| `↑` / `↓` | Move up/down one item |
| `←` / `→` | Same as up/down (for convenience) |
| `PgUp` / `PgDn` | Jump one page up/down |
| `Enter` | Open selected file or enter directory |
| `Esc` | Go to parent directory |
| `h` | Go to disk selection screen (home) |

### Operations

| Key | Action |
|-----|--------|
| `e` | Open current folder in Windows Explorer |
| `r` | Rename the selected item (type new name and press Enter) |
| `j` | Jump to an absolute path (type the path and press Enter) |
| `s` | Search – type `file regex` or `folder regex` (see details below) |
| `Ctrl+N` | Open a new Console Explorer window in the same directory |
| `Ctrl+R` | Refresh the current view |

---

## Search

Search uses the syntax:
```
file <regex>
folder <regex>
```

- `file` – search for **files** matching the regex (in the current directory or result set).
- `folder` – search for **folders** matching the regex.

> **Important:** The regex is matched against the **full name** of the item (file or folder) using `regex_match` (C++ standard).  
> This means the entire string must match the pattern. You do **not** need to add `^` or `$` anchors – they are implied.  
> For example:
> - `file .*\.cpp` matches any file name ending with `.cpp` (because `.*` matches any prefix, and the literal dot and `cpp` are required).
> - `folder src.*` matches any folder name that **starts** with `src` (since `src` must appear at the beginning and `.*` consumes the rest).
> - `folder src` matches only folders named exactly `src`.

### Recursive Search (Search within Search Results)

When you are already inside a search result view (i.e., after a previous `file` or `folder` search), a new search will be performed **on the current result set**, not on the entire disk. This allows fast filtering without re‑scanning the filesystem.

The behavior depends on the combination of the **current result type** and the **new search type**:

| Current result type | New search type | Behavior |
|---------------------|-----------------|----------|
| **Files** (`file ...`) | `file ...` | Match file names against the new regex. |
| **Files** (`file ...`) | `folder ...` | Match **the parent directory name** of each file against the regex, and return the files whose parent directory matches. |
| **Folders** (`folder ...`) | `folder ...` | Match folder names against the new regex. |
| **Folders** (`folder ...`) | `file ...` | Recursively traverse **all folders** in the current result set, and collect **files** inside them whose names match the new regex. |

**Examples:**

1. You search for `file .*\.cpp` → result: all `.cpp` files.  
   Then you run `folder src.*` → you get only the `.cpp` files that are inside folders whose name starts with `src`.

2. You search for `folder test` → result: all folders named exactly `test`.  
   Then you run `file .*\.txt` → you get all `.txt` files inside those `test` folders.

This in‑memory recursive filtering is extremely fast and useful for narrowing down large result sets.

---

## Build

You need a Windows compiler with Windows SDK support.

### Using Microsoft Visual C++ (MSVC)

Open a **Developer Command Prompt for Visual Studio** and run:

```cmd
cl /EHsc /std:c++17 /O2 console_explorer.cpp
```
The output will be console_explorer.exe.

### Using MinGW (optional)

If you prefer MinGW, you can compile with:

```bash
g++ -std=c++17 -O2 console_explorer.cpp -o console_explorer.exe -lshlwapi -lole32 -lshell32
(Note: the linker flags may vary depending on your environment.)
```
## Limitations
**Designed exclusively for the classic Windows Console (conhost).**
While it may run in Windows Terminal, visual glitches (especially screen clearing) may occur due to differences in how the console API interacts with the buffer. For best results, use the default cmd.exe or any terminal that emulates the classic conhost behavior.

**No mouse support** – purely keyboard‑driven.

**Do not resize the Window(set it to 120x30)** – or edit the code for your own liking. Configuration file support may be added in future versions, but auto-resizing is impossible because the screen buffer does not resize with the visual window.

No file deletion – the delete function has been intentionally disabled to prevent accidental data loss. It may be fixed in future versions by using transacted file operations.

## Author & Contributing

This project was created by myself. Feel free to fork, modify, and submit pull requests for improvements. Feedback and bug reports are welcome via GitHub Issues.
