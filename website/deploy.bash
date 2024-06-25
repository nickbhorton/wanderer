#!/bin/bash
fuser 8080/tcp

/home/nick/dev/wanderer/build/profile_server &
PROFILE_SERVER_PID=$!
trap "kill $PROFILE_SERVER_PID" SIGINT
echo "profile server pid: $PROFILE_SERVER_PID"

meadow &
MEADOW_PID=$!
trap "kill $MEADOW_PID" SIGINT
echo "meadow pid: $MEADOW_PID"

echo "Compile tailwindcss"
npx tailwindcss -i src/styles.css -o build/output.css --watch
