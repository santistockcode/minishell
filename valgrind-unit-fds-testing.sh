#!/bin/bash

# Define the target directory
directory="./tests/unit/execution/bin"

# Check if the target is not a directory
if [ ! -d "$directory" ]; then
  exit 1
fi

# Loop through files in the target directory
for file in "$directory"/*; do
  if [ -f "$file" ]; then
    valgrind --leak-check=full --show-leak-kinds=all --trace-children=yes --child-silent-after-fork=no --track-fds=yes --track-fds=all "$file"
  else
    echo "No files, did you 'tox -e unit-val-script'?"
  fi
done