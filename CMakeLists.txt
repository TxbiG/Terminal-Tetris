cmake_minimum_required(VERSION 3.20)

project(TerminalTetris
    VERSION 0.1.0
    DESCRIPTION "A small terminal-based Tetris game"
    LANGUAGES CXX
)

# The supplied source uses _kbhit() and _getch() from <conio.h>.
# Those functions are provided by the Windows toolchains, so this
# version of the game is Windows-only until POSIX input is implemented.
if(NOT WIN32)
    message(FATAL_ERROR
        "This TerminalTetris source currently requires Windows because it uses "
        "_kbhit() and _getch() from <conio.h>."
    )
endif()

add_executable(TerminalTetris
    main.cpp
)

target_compile_features(TerminalTetris PRIVATE cxx_std_17)

set_target_properties(TerminalTetris PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO
    OUTPUT_NAME "TerminalTetris"
)

if(MSVC)
    target_compile_options(TerminalTetris PRIVATE /W4 /permissive-)
else()
    target_compile_options(TerminalTetris PRIVATE -Wall -Wextra -Wpedantic)
endif()

# Do not use add_executable(... WIN32 ...): this is a console game and
# needs its terminal window for rendering and keyboard input.
