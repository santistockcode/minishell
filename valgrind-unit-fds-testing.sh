#!/bin/bash

# Define the target directory
directory="tests/unit/execution/bin"

# Check if the target is not a directory
if [ ! -d "$directory" ]; then
  exit 1
fi


# Loop through files in the target directory
for file in "$directory"/*; do
  if [ -f "$file" ]; then
    valgrind --leak-check=full --show-leak-kinds=all --trace-children=yes --child-silent-after-fork=no --track-fds=yes --track-fds=all --log-file="$directory/valgrind-fds-results/valgrind_$(basename "$file").log" "$file"
  fi
done

# print important lines : cat tests/unit/execution/bin/valgrind-fds-results/* | grep FILE
cat "$directory/valgrind-fds-results/"* | grep FILE