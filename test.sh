current_folder=${2:-./}
base_folder="resources"
passed=true

red=`tput setaf 1`
green=`tput setaf 2`
reset=`tput sgr0`

tmpfile=$(mktemp /tmp/paracl-temp.tmp.XXXXXX)

for file in $current_folder/*.pcl; do
  echo -n "Testing ${green}${file}${reset} ... "
  $1 -i $file < ${file}.in > $tmpfile

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