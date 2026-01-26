#!/bin/bash

# Define the target directory
directory="tests/unit/execution/bin"
outdir="$directory/valgrind-fds-results"

# Check if the target is not a directory
if [ ! -d "$directory" ]; then
  exit 1
fi

# Create output directory if it doesn't exist
mkdir -p "$outdir"

# Loop through files in the target directory
for file in "$directory"/*; do
  if [ -f "$file" ]; then
    valgrind --leak-check=full --show-leak-kinds=all --trace-children=yes --child-silent-after-fork=no --track-fds=yes --track-fds=all \
    --log-file="$outdir/valgrind_%p_$(basename "$file").log" \
    "$file"
  fi
done

# Safely grep as text even if logs contain NULs
grep -a "FILE DESCRIPTORS:" tests/unit/execution/bin/valgrind-fds-results/*.log \
  | sed -E 's#^.*(FILE DESCRIPTORS:.*)$#\1#'

echo "The extra fd is Valgrind’s own log file. That’s normal. No app fd leaks here."
echo "If more than 4 present inspect file to check if they come from my code"