base_folder="resources"

red=`tput setaf 1`
green=`tput setaf 2`
reset=`tput sgr0`

current_folder=${2:-./}
passed=true

for file in ${current_folder}/${base_folder}/test*.dat
do
    # Total number of test cases found
    count=`echo $file | egrep -o [0-9]+`
    
    echo -n "Testing ${green}${file}${reset} ... "

    # Check if an argument to executable location has been passed to the program
    if [ -z "$1" ]
    then
        bin/hshtend < $file > ${current_folder}/$base_folder/temp.dat
    else
        $1 < $file > ${current_folder}/$base_folder/temp.dat
    fi

    # Compare inputs
    if diff ${current_folder}/${base_folder}/ans${count}.dat ${current_folder}/${base_folder}/temp.dat;
    then
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
    exit 42
fi