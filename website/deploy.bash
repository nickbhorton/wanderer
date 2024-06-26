#!/bin/bash
fuser 8080/tcp

meadow &
MEADOW_PID=$!
trap "kill $MEADOW_PID" SIGINT
echo "meadow pid: $MEADOW_PID"

echo "Compile tailwindcss"
npx tailwindcss -i src/styles.css -o build/output.css --watch
