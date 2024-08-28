#!/bin/bash

# Directory to watch
WATCH_DIR="/var/log"

# Monitor the directory and its subdirectories for changes using inotifywait
inotifywait -m -r -e create -e modify --format '%w%f %e' "$WATCH_DIR" | while read FILE EVENT
do
  if [[ "$EVENT" == "CREATE" ]]; then
    TIMESTAMP=$(date '+%Y-%m-%d %H:%M:%S')
    echo "$TIMESTAMP: $FILE has been created"
  elif [[ "$EVENT" == "MODIFY" ]]; then
    TIMESTAMP=$(date '+%Y-%m-%d %H:%M:%S')
    echo "$TIMESTAMP: $FILE has been modified"
  fi
done