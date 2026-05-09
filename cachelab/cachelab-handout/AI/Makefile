# Makefile for the AI/ cache simulator.
#
# Builds csim_claude.c against the parent's cachelab.c (for printSummary)
# and provides a `test` target that runs the 8 official trace cases and
# diffs the output against csim-ref.

CC      := gcc
CFLAGS  := -g -Wall -Werror -std=c99 -m64
ROOT    := ..
SRC     := csim_claude.c
BIN     := csim
TRACES  := $(ROOT)/traces

# (s, E, b, tracefile, points) for the 8 official test cases.
CASES := \
  1:1:1:yi2.trace:3   \
  4:2:4:yi.trace:3    \
  2:1:4:dave.trace:3  \
  2:1:3:trans.trace:3 \
  2:2:3:trans.trace:3 \
  2:4:3:trans.trace:3 \
  5:1:5:trans.trace:3 \
  5:1:5:long.trace:6

.PHONY: all test test-csim clean

all: $(BIN)

$(BIN): $(SRC) $(ROOT)/cachelab.c $(ROOT)/cachelab.h
	$(CC) $(CFLAGS) -I$(ROOT) -o $@ $(SRC) $(ROOT)/cachelab.c -lm

# Diff our simulator's summary against csim-ref's for every official case.
test: $(BIN)
	@pass=0; fail=0; \
	for case in $(CASES); do \
	  s=$${case%%:*}; rest=$${case#*:}; \
	  E=$${rest%%:*}; rest=$${rest#*:}; \
	  b=$${rest%%:*}; rest=$${rest#*:}; \
	  trace=$${rest%%:*}; pts=$${rest#*:}; \
	  mine=$$(./$(BIN)            -s $$s -E $$E -b $$b -t $(TRACES)/$$trace); \
	  ref=$$($(ROOT)/csim-ref     -s $$s -E $$E -b $$b -t $(TRACES)/$$trace); \
	  if [ "$$mine" = "$$ref" ]; then \
	    printf "  [PASS] (%s,%s,%s) %-12s %s  (%s pts)\n" $$s $$E $$b $$trace "$$mine" $$pts; \
	    pass=$$((pass+1)); \
	  else \
	    printf "  [FAIL] (%s,%s,%s) %-12s mine=[%s] ref=[%s]\n" $$s $$E $$b $$trace "$$mine" "$$ref"; \
	    fail=$$((fail+1)); \
	  fi; \
	done; \
	echo; echo "  $$pass passed, $$fail failed"; \
	[ $$fail -eq 0 ]

# Convenience: drop our binary in as the parent ./csim and run the
# official test-csim autograder.
test-csim: $(BIN)
	cp $(BIN) $(ROOT)/csim
	cd $(ROOT) && ./test-csim

clean:
	rm -f $(BIN)
