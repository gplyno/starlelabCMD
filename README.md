# Starlelab CMD

starlelabCMD - A command-line tool that assists with development and downloading new versions of the game. WARNING! If you are a developer, use this tool only if you know what you are doing.

## Commands

| Command    | Description                                  |
|------------|----------------------------------------------|
| `help`     | list available commands                      |
| `about`    | info about starlelab cmd                     |
| `gi`       | game info                                    |
| `dlg`      | open the download page in browser            |
| `ws`       | open the studio website in browser           |
| `cs`       | list saves in `%LOCALAPPDATA%/Zombotanic/saves` |
| `ds`       | delete a save (asks for the name)            |
| `ds <name>`| delete the given save immediately            |
| `blog`     | open the blog page in browser                |
| `thoughts` | open the thoughts page in browser            |
| `cv`       | open version pages (itch.io + history)       |
| `color`    | toggle dark / light theme                    |
| `hints`    | show/hide bottom hint bar                    |
| `clear`    | clear the screen                             |
| `exit`     | quit                                         |

## Hotkeys

| Key       | Action                    |
|-----------|---------------------------|
| `Enter`   | run the command           |
| `↑` / `↓` | command history           |
| `Tab`     | autocomplete              |
| `Ctrl+L`  | clear the screen          |
| `F1`      | show/hide hint bar        |

## Build

Requires **Qt 6** with the `Widgets` module and **CMake 3.16+**.

```bash
cmake -B build -DCMAKE_PREFIX_PATH=/path/to/Qt/6.x.x/gcc_64
cmake --build build
```

### Windows notes

- The target is built as a GUI app (`WIN32`), so no console window appears.
- To run the produced `starlelab cmd.exe` outside Qt Creator, deploy the Qt runtime:

```cmd
C:\Qt\6.x.x\msvc2019_64\bin\windeployqt.exe "starlelab cmd.exe"
```

Or leave it — the `CMakeLists.txt` already runs `windeployqt` automatically after build if it can find the tool.
