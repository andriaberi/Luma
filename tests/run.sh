#!/usr/bin/env bash
# Runs every tests/**/*.luma. A test is Luma source followed by its expectations:
#
#   fn main() -> int {
#       int x = 10
#   }
#
#   // args: {file} -d tokens     optional; arguments to luma, {file} = this file (default: {file})
#   // exit: 1                    optional; expected exit status (default: 0)
#   // expect:                    expected stdout+stderr, one "// " line each, to end of file
#   // 2:15: error: expected ';' after declaration
#
#   tests/run.sh                 run all tests          (make test)
#   tests/run.sh lex             run one folder         (make test lex)
#   tests/run.sh lex/strings     run one test           (make test lex/strings)
#   tests/run.sh update [...]    rewrite exit/expect from current output, then review with git diff
#                                                       (make test update, make test update lex)

cd "$(dirname "$0")/.." || exit 1
LUMA=${LUMA:-build/luma}
MAX_DIFF=15

# Color only on a terminal, and not when NO_COLOR is set.
if [ -t 1 ] && [ -z "$NO_COLOR" ]; then
    G=$'\e[32m' R=$'\e[31m' D=$'\e[2m' B=$'\e[1m' X=$'\e[0m'
fi

TRAILER='^// (args|exit|expect):'
source_of() { awk -v t="$TRAILER" '$0 ~ t {exit} {print}' "$1"; }
field()     { grep -m1 -E "^// $2:" "$1" | sed -E "s|^// $2: *||"; }
expect_of() { awk '/^\/\/ expect:/{e=1; next} e{sub(/^\/\/ ?/, ""); print}' "$1"; }
name()      { local n=${1#tests/}; echo "${n%.luma}"; }   # tests/lex/strings.luma -> lex/strings

# Sets $got and $code.
run() {
    local args='{file}'
    grep -q '^// args:' "$1" && args=$(field "$1" args)
    # shortcut: args split on whitespace, no quoting; fine until a test needs spaces in an arg
    got=$("$LUMA" ${args//\{file\}/$1} 2>&1); code=$?
}

update() {
    local src
    run "$1"
    src=$(source_of "$1")   # $(...) also drops the blank lines before the trailer
    {
        [ -n "$src" ] && printf '%s\n\n' "$src"
        grep -m1 '^// args:' "$1"
        [ "$code" -ne 0 ] && echo "// exit: $code"
        echo "// expect:"
        [ -n "$got" ] && printf '%s\n' "$got" | sed 's|^|// |; s|^// $|//|'
    } > "$1.new" && mv "$1.new" "$1"
}

# Expected (-, red) vs got (+, green), 2 lines of dim context, hunks split by "…", cut at MAX_DIFF lines.
showdiff() {
    diff -U2 <(printf '%s\n' "$1") <(printf '%s\n' "$2") | tail -n +3 | sed '1{/^@@/d}' | sed \
        -e "s/^@@.*/${D}…${X}/" -e "s/^-\(.*\)/${R}- \1${X}/" \
        -e "s/^+\(.*\)/${G}+ \1${X}/" -e "s/^ \(.*\)/${D}  \1${X}/" |
    awk -v max=$MAX_DIFF -v d="$D" -v x="$X" \
        'NR <= max {print "    " $0} END {if (NR > max) print "    " d "… " NR - max " more lines" x}'
}

paths=()
for a in "$@"; do
    a=${a#tests/}; a=${a%/}
    if   [ "$a" = update ];           then UPDATE=1
    elif [ -d "tests/$a" ];           then paths+=("tests/$a")
    elif [ -f "tests/${a%.luma}.luma" ]; then paths+=("tests/${a%.luma}.luma")
    else echo "no test or folder named '$a'"; exit 2
    fi
done
tests=$(find "${paths[@]:-tests}" -name '*.luma' | sort)
[ -z "$tests" ] && { echo "no tests"; exit 0; }

if [ -n "$UPDATE" ]; then
    while IFS= read -r t; do update "$t"; done <<< "$tests"
    echo "updated $(wc -l <<< "$tests") tests; review with git diff tests/"
    exit 0
fi

# Folder totals first, then a "failures" section with each failure under its folder.
groups=() details= fgroup= pass=0 fail=0
declare -A gpass gtotal
while IFS= read -r t; do
    g=$(name "$t"); n=${g##*/}; g=${g%/*}
    [ -z "${gtotal[$g]}" ] && groups+=("$g")
    gtotal[$g]=$(( ${gtotal[$g]:-0} + 1 ))

    run "$t"
    want_code=$(field "$t" exit); want_code=${want_code:-0}
    want=$(expect_of "$t")

    if [ "$code" = "$want_code" ] && [ "$got" = "$want" ]; then
        gpass[$g]=$(( ${gpass[$g]:-0} + 1 )); pass=$((pass + 1)); continue
    fi
    fail=$((fail + 1))

    [ "$g" != "$fgroup" ] && { details+="${B}$g${X}"$'\n'; fgroup=$g; }
    details+="  ${R}$n${X}"
    [ "$code" != "$want_code" ] && details+="  ${D}exit $code, expected $want_code${X}"
    details+=$'\n'
    [ "$got" != "$want" ] && details+="$(showdiff "$want" "$got")"$'\n'
done <<< "$tests"

w=0; for g in "${groups[@]}"; do [ ${#g} -gt $w ] && w=${#g}; done
for g in "${groups[@]}"; do
    p=${gpass[$g]:-0} n=${gtotal[$g]}
    [ "$p" -eq "$n" ] && c=$G || c=$R
    printf "%-${w}s  %s\n" "$g" "$c$p/$n$X"
done

if [ $fail -gt 0 ]; then
    echo; echo "${R}${B}failures${X}"; echo
    printf '%s' "$details"
fi

echo
if [ $fail -eq 0 ]; then
    echo "${G}${B}$pass passed${X}"
else
    echo "${R}${B}$fail failed${X}, ${G}$pass passed${X}"
fi
[ $fail -eq 0 ]
