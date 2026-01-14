#!/bin/bash
set -e

PICO_PATH="/Volumes/CIRCUITPY"

if [ ! -d "$PICO_PATH" ]; then
    echo "Pico not found at $PICO_PATH"
    exit 1
fi

cp main.py "$PICO_PATH/code.py"

if [ -d "lib" ]; then
    cp -r lib "$PICO_PATH/"
fi

echo "Done"
