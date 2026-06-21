CC := gcc
CFLAGS := -fno-stack-protector -z execstack

SHELL := /usr/bin/env bash

GDB := gdb
GDB_INIT := debug-prepared.cmd

SRC := stsmash.c
TARGET := build/stack-smash
INPUT := input.txt

$(TARGET): $(SRC)
	mkdir -p $(dir $@)
	$(CC) -o $@ $(SRC)

clean:
	rm -rf $(dir $(TARGET)) $(INPUT)

run: $(TARGET)
	./$(TARGET)

debug: $(TARGET)
	$(GDB) -q $(TARGET)

debug-prepared: $(TARGET) generate-input
	$(GDB) -x $(GDB_INIT) -q $(TARGET)

generate-input:
	for i in {1..700}; do printf 'A'; done | tee $(INPUT)

.PHONY: clean run debug debug-prepared
