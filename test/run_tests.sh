#!/usr/bin/env bash
# Offline tests (x86, ASan + UBSan): the framework's host test, then test/plateau_test.cc on the engine itself.
#   test/run_tests.sh              (MPC_VST=../mpc-vst-plugins to use another framework checkout)
#   test/run_tests.sh --wav DIR    also write a few renders to listen to
set -euo pipefail
cd "$(dirname "$0")/.."
MPC_VST="${MPC_VST:-$PWD/third_party/mpc-vst-plugins}"
bash "$MPC_VST/tools/test_port.sh" vst.json
SRCS=$(python3 -c "import json; print(' '.join(json.load(open('vst.json'))['build']['sources']))")
CFLAGS=$(python3 -c "import json; print(' '.join(json.load(open('vst.json'))['build']['cflags']))")
SAN="-fsanitize=address,undefined -fno-omit-frame-pointer -g -O1"
mkdir -p build/dsptest
OBJS=""
for f in $SRCS test/plateau_test.cc; do
  o="build/dsptest/$(echo "$f" | tr / _).o"
  g++ $SAN -std=gnu++11 -Wall -Wno-unused-function $CFLAGS -Ibuild -I"$MPC_VST/wrapper" -c "$f" -o "$o"
  OBJS="$OBJS $o"
done
g++ $SAN $OBJS -lm -ldl -lpthread -o build/dsptest/plateau_test
build/dsptest/plateau_test "$@"
