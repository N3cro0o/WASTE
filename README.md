# Windows Alternative Simple Text Editor
Simple Windows terminal text editor made with *PDCurses* library. *WASTE* application operates in two main states, **command mode** and **raw mode**. Command mode is the default app mode; *WASTE* begins in this mode active. It is used to input app commands. Raw mode or editor mode let's user to manipulate loaded or clean text data which can be later saved. 
## Main commands
Main application commands available to use in command mode
* `edit` 
Enter edit text data mode.
* `save <path>`
Save current text data in selected path.
* `load <path>`
Load saved text data from selected path.
* `clear`
Clear current text data.
* `quit`
Quit application without saving
## Dependecies 
* PDCurses Library
* Windows operating system
## Build
To build this project yourself all you need is Visual Studio with installed Visual C++ compiler and installed *PDCurses* library. Remember to link necessary library and to attach `curses.h` header file.

Simply open .sln project file and select build project with X86 platform option.