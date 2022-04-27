base_folder="./resources"

red=`tput setaf 1`
green=`tput setaf 2`
reset=`tput sgr0`

echo "Testing ${green}${executable}${reset}"

for file in $base_folder/test*.dat
do
  count=`echo $file | egrep -o [0-9]+`
  
  echo -n "Testing ${green}${file}${reset} ... "
  
  ./bin/hshtend -i $file > $base_folder/temp.dat
  if diff -Z ${base_folder}/ans${count}.dat ${base_folder}/temp.dat; then
    echo "${green}Passed${reset}"
  else
    echo "${red}Failed${reset}"

    result=`cat ${base_folder}/temp.dat`
    expected=`cat ${base_folder}/ans${count}.dat`

    echo -e "Result: \t$result"
    echo "--------------------------------"
    echo -e "Expected: \t$expected"
  fi
done