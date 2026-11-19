#!/bin/sh
set -u
cd "$(dirname "$0")"
make || exit 1
TMP=$(mktemp -d)
trap 'rm -rf "$TMP"' EXIT HUP INT TERM
cd "$TMP" || exit 1
printf 'Hello, pipe!\n' > hello.txt
: > empty.txt
head -c 10000 /dev/urandom > binary.dat
printf 'Test 1: unnamed pipes\n'
"$OLDPWD/filecopy" hello.txt empty.txt binary.dat || exit 1
cmp hello.txt hello.txt.copy && cmp empty.txt empty.txt.copy && cmp binary.dat binary.dat.copy || exit 1
printf 'Test 2: named pipes\n'
"$OLDPWD/filecopy" -p channel hello.txt binary.dat || exit 1
cmp hello.txt hello.txt.copy && cmp binary.dat binary.dat.copy || exit 1
test ! -e channel.p2c && test ! -e channel.c2p || exit 1
printf 'Test 3: nonexistent file (expect failure)\n'
if "$OLDPWD/filecopy" missing.txt; then exit 1; fi
printf 'Test 4: missing arguments (expect failure)\n'
if "$OLDPWD/filecopy"; then exit 1; fi
printf 'All tests passed.\n'
