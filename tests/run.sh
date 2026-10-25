#!/bin/sh
set -eu

root=$(pwd)
make -C "$root" >/dev/null

passed=0
failed=0
for case in "$root"/tests/cases/*; do
    [ -d "$case" ] || continue
    name=$(basename "$case")
    [ -z "$(ls "$case"/*.png 2>/dev/null)" ] && continue
    rm -rf "/tmp/conv testcase_$name"
    mkdir -p "/tmp/conv testcase_$name"
    for ext in png pal txt; do cp "$case"/*.$ext "/tmp/conv testcase_$name"/ 2>/dev/null || true; done
    args_file="$case/args"
    args=""
    if [ -f "$args_file" ]; then args=$(cat "$args_file"); fi
    cd "/tmp/conv testcase_$name"
    /home/roudoudou/code/beam/convgeneric $args >/dev/null 2>&1 || true
    if [ -d "$case/expected" ]; then
        ok=1
        for exp in "$case"/expected/*; do
            [ -e "$exp" ] || continue
            bn=$(basename "$exp")
            if ! cmp -s "$exp" "$bn"; then
                echo "DIFF: $bn"; cmp "$exp" "$bn" 2>/dev/null || true
                ok=0
            fi
        done
        if [ -f "$case/absent.txt" ]; then
            while IFS= read -r bn; do
                [ -z "$bn" ] && continue
                if [ -e "$bn" ]; then
                    echo "UNEXPECTED: $bn"
                    ok=0
                fi
            done < "$case/absent.txt"
        fi
        if [ "$ok" = "1" ]; then
            echo "PASS $name"
            passed=$((passed+1))
        else
            echo "FAIL $name"
            failed=$((failed+1))
        fi
    else
        echo "SKIP $name (no expected/)"
    fi
    cd - >/dev/null
done

echo "passed=$passed failed=$failed"
test "$failed" -eq 0
