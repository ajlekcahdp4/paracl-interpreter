base_folder="resources"

red=`tput setaf 1`
green=`tput setaf 2`
reset=`tput sgr0`

current_folder=${2:-./}
passed=true

tmpfile=$(mktemp /tmp/paracl-temp.tmp.XXXXXX)

for file in $current_folder/*.pcl; do
  echo -n "Testing ${green}${file}${reset} ... "

  # Check if an argument to executable location has been passed to the program
  $1 -i $file < ${file}.in > $tmpfile

  # Compare inputs
  if diff -Z ${file}.ans $tmpfile; then
      echo "${green}Passed${reset}"
  else
      echo "${red}Failed${reset}"
      passed=false
  fi
done

if ${passed}
  then
    exit 0
  else
    # Exit with the best number for an exit code
    exit 666
fi