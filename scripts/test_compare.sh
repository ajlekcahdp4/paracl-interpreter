#!/bin/sh

current_folder=${2:-./}
passed=0

tmpfile=$(mktemp /tmp/paracl-temp.tmp.XXXXXX)

for file in $current_folder/*.pcl; do
  echo -n "Testing ${green}${file}${reset} ... "
  if [ -f "${file}.in" ]; then
    $1 -i $file < ${file}.in > $tmpfile
  else 
    $1 -i $file > $tmpfile
  fi

  if diff -Z ${file}.ans $tmpfile; then
    echo "${green}Passed${reset}"
  else
    echo "${red}Failed${reset}"
    passed=1
  fi
done

exit $passed