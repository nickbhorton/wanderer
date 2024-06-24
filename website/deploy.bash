#!/bin/bash
killall meadow
fuser 8080/tcp
meadow &
MEADOW_PID=$!
echo $MEADOW_PID
echo "Compile tailwindcss"
npx tailwindcss -i src/styles.css -o build/output.css --watch
kill $MEADOW_PID
