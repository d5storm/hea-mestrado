#!/bin/bash

function EXEC {
	$*
	# if [ $? -ne 0 ]
	# then
	# 	#echo "Processo Interrompido"
	# 	#exit 1
	# fi
}

function CTRLC {
	echo "CTRL C ---------------------------------- PRESSIONADO!!!"
	exit 1 
}

declare -a ils=("10" "20" "30" "40" "50" "60" "70" "80" "90" "100" "110" "120" "130" "140" "150")
trap CTRLC SIGINT

## now loop through the above array
for i in "${ils[@]}"
do
	echo  -ne ils$i '' & EXEC ./ils$i.sh &
	# EXEC $execpath -a $a -s $i -c $clusters -w $workflows$file$dag >> ./results/local_search_testing/"GRASP_new_LS.txt"
done

