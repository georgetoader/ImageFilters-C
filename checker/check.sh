# By Stefan Laurentiu 343C3

function compare_files {
    diff $1 $2 &> /dev/null
    return $?
}

pad=$(printf '%.1s' "."{1..70})
padlength=71

function print_result {
    printf "%s" "$1"
    printf "%*.*s" 0 $((padlength - ${#1} - ${#2} )) "$pad"
    printf "%s\n" "$2"
}

function check_prerequisites {
    # Check if makefile exists
    ls . | grep -i "makefile" &> /dev/null

    if [ $? -ne 0 ]; then
        echo -e "Makefile file not found! Stopping testing!"
        exit 1
    fi

    makefile_name=$(ls . | grep -i makefile | cut -f 1 -d " ")

    # Check for makefile build, run, clean rules
    cat $makefile_name | grep -i "build:" &> /dev/null
    if [ $? -ne 0 ]; then
        echo -e "[MAKEFILE]: BUILD rule not found! Stopping testing!"
        exit 1
    fi

    cat $makefile_name | grep -i "run:" &> /dev/null
    if [ $? -ne 0 ]; then
        echo -e "[MAKEFILE]: RUN rule not found! Stopping testing!"
        exit 1
    fi

    cat $makefile_name | grep -i "clean:" &> /dev/null
    if [ $? -ne 0 ]; then
        echo -e "[MAKEFILE]: CLEAN rule not found! Stopping testing!"
        exit 1
    fi
}

function check_tasks {
    total_points=0
    tasks_weight=(1 2 2 1 3)
    tasks_endings=("black_white" "nocrop" "filter" "pooling" "clustered")
    no_tasks=5

    touch input.txt
    rm output/* &> /dev/null
    rm *.bmp &> /dev/null
    make clean &> /dev/null
    make build &> /dev/null

    if [ $? -ne 0 ]; then
        echo "[MAKEFILE] Makefile build rule failed! Stopping testing!"
        exit 1
    fi

    for i in $(seq 0 9)
    do
        echo "==============================TEST $i==================================="

        # Copying the current image
        cp input/images/test$i.bmp .

        # Creating the input.txt file
        echo test$i.bmp > input.txt
        echo input/filters/filter$i.txt >> input.txt
        echo input/pooling/pooling$i.txt >> input.txt
        echo input/clustering/cluster$i.txt >> input.txt

        # Running the executable
        make run &> /dev/null

        if [ $? -ne 0 ]; then
            echo "[MAKEFILE] Makefile run rule failed!"
            echo -e "\n\n\n"
            # Don't check at all the test
            continue
        fi

        current_test_points=0

        for j in $(seq 0 4)
        do
            current_task_file="test${i}_${tasks_endings[j]}.bmp"
            ref_file="ref/test${i}_${tasks_endings[j]}.bmp"

            # Search the current file
            ls . | grep -i $current_task_file &> /dev/null

            if [ $? -ne 0 ]; then
                echo "$current_task_file image was not found!"
            fi

            cp $current_task_file output/ &> /dev/null

            # Compare the output with the reference
            compare_files $current_task_file $ref_file

            if [ $? -eq 0 ]; then
                print_result "Task $(expr $j + 1)" "OK (${tasks_weight[j]}p)"
                ((current_test_points=current_test_points+${tasks_weight[j]}))
            else
                print_result "Task $(expr $j + 1)" "FAILED"
            fi
        done

        echo ""
        print_result "Total TEST $i" "[${current_test_points}p / 9p]"
        ((total_points=total_points+current_test_points))

        rm *.bmp
        echo "======================================================================="
        echo ""
    done

    make clean &> /dev/null
    rm input.txt
}

function check_readme {
    echo "=============================README===================================="

    ls . | grep -i "readme" &> /dev/null

    if [ $? -eq 0 ]; then
        print_result "Checking for README file" "OK (10p for now)"
        readme_points=10
        ((total_points=total_points+10))
    else
        readme_points=0
        print_result "Checking for README file" "FAILED"
    fi

    echo ""
    print_result "Total README" "[${readme_points}p / 10p]"
    echo "======================================================================="
    echo ""
}

function check_bonus {
    echo "==============================BONUS===================================="

    touch input.txt
    make build &> /dev/null

    # The biggest image is "test8.bmp"
    cp input/images/test8.bmp .

    # Creating the input.txt file
    echo test8.bmp > input.txt
    echo input/filters/filter8.txt >> input.txt
    echo input/pooling/pooling8.txt >> input.txt
    echo input/clustering/cluster8.txt >> input.txt

    # Running valgrind over the executable
    valgrind --tool=memcheck --leak-check=full --error-exitcode=1 ./bmp &>  /dev/null

    if [ $? -eq 0 ]; then
        print_result "Test Valgrind memory checking" "OK (20p)"
        ((total_points=total_points+20))
    else
        print_result "Test Valgrind memory checking" "FAILED"
    fi

    echo ""
    print_result "Total BONUS " "[$(expr $total_points - 100)p / 20p]"

    rm *.bmp
    make clean &> /dev/null
    rm input.txt

    echo "======================================================================="
}

function bonus_reward {
    iterations=0
    trap "tput reset; tput cnorm; exit" 2
    clear
    tput civis
    lin=2
    col=$(($(tput cols) / 2))
    c=$((col-1))
    est=$((c-2))
    color=0
    tput setaf 2; tput bold

    # Tree
    for ((i=1; i<20; i+=2))
    {
        tput cup $lin $col
        for ((j=1; j<=i; j++))
        {
            echo -n \*
        }
        let lin++
        let col--
    }

    tput sgr0; tput setaf 3

    # Trunk
    for ((i=1; i<=2; i++))
    {
        tput cup $((lin++)) $c
        echo 'mWm'
    }
    new_year=$(date +'%Y')
    let new_year++
    tput setaf 1; tput bold
    tput cup $lin $((c - 6)); echo MERRY CHRISTMAS
    tput cup $((lin + 2)) $((c - 17)); echo YOU GOT A MAXIMUM SCORE ON THE HOMEWORK
    tput cup $((lin + 1)) $((c - 10)); echo And lots of CODE in $new_year
    let c++
    k=1

    # Lights and decorations
    while [ $iterations -lt 3 ]; do
        for ((i=1; i<=35; i++)) {
            # Turn off the lights
            [ $k -gt 1 ] && {
                tput setaf 2; tput bold
                tput cup ${line[$[k-1]$i]} ${column[$[k-1]$i]}; echo \*
                unset line[$[k-1]$i]; unset column[$[k-1]$i]  # Array cleanup
            }

            li=$((RANDOM % 9 + 3))
            start=$((c-li+2))
            co=$((RANDOM % (li-2) * 2 + 1 + start))
            tput setaf $color; tput bold   # Switch colors
            tput cup $li $co
            echo o
            line[$k$i]=$li
            column[$k$i]=$co
            color=$(((color+1)%8))

            # Flashing text
            sh=1
            for l in C O D E
            do
                tput cup $((lin+1)) $((c+sh))
                echo $l
                let sh++
                sleep 0.01
            done
        }

        k=$((k % 2 + 1))
        iterations=$((iterations+1))
    done
}


# "Main" code starts here
check_prerequisites

check_tasks

check_readme

if [ $total_points -eq 100 ]; then
    check_bonus
else
    echo "You don't have maximum points! Not checking the bonus!"
fi

echo ""
echo "TOTAL: ${total_points}/120"

if [ $total_points -eq 120 ]; then
    bonus_reward
fi
