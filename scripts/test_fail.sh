#!/bin/sh

current_folder=${2:-./}
passed=0

for file in $current_folder/*.pcl; do
  echo -n "Testing ${green}${file}${reset} ... "
  
  # If return code is not 0, then in means errors were detected
  if $1 -i $file > /dev/null; then
    echo "${red}Failed${reset}"
    passed=1
  else
    echo "${green}Passed${reset}"
  fi
done

exit $passed