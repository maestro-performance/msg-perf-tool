#!/bin/bash

[[ -d src ]] && find src -type f \( -iname '*.c' -or -iname '*.h' \) -exec clang-format -i -style=file {} \;
[[ -d test ]] && find test -type f \( -iname '*.c' -or -iname '*.h' \) -exec clang-format -i -style=file {} \;