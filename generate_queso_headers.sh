#!/bin/bash
set -e

# Find all teh headers
headers=`find src -name "*.h" -a -not -name all.h -type f | grep -v 'ANN'`

# Find the thing we want to conduct wizardry on
quesoh=`find src -name "all.h"`

# Create temporary file to hold output of sed
tmp=`mktemp "${TMPDIR-/tmp}/tmp.XXXXXXXXXX"`
trap "rm -f $tmp" EXIT

# Nuke all previous includes
sed "/#include/d" $quesoh > $tmp
mv $tmp $quesoh

for header_with_path in $headers; do
  header=`basename $header_with_path`;

  # We need the backslash because this is going to be used with sed
  header="#include<queso/$header>"

  echo $header >> $quesoh
done
