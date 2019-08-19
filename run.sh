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


execpath="./bin/HEA"
clusters="./input/clusters/cluster.vcl"
workflows="./input/Workflows/"
dag=".dag"
declare -a wkf=("CyberShake_30.xml" "CyberShake_50.xml" "CyberShake_100.xml" "GENOME.d.351024866.0.dax" "GENOME.d.702049732.0.dax" "Epigenomics_24.xml" "Epigenomics_46.xml" "Epigenomics_100.xml" "Montage_25.xml" "Montage_50.xml" "Montage_100.xml" "Inspiral_30.xml" "Inspiral_50.xml" "Inspiral_100.xml")
# declare -a wkf=("Montage_25.xml" "Montage_50.xml" "Montage_100.xml" "Inspiral_30.xml" "Inspiral_50.xml" "Inspiral_100.xml")
declare -a wkf=("GENOME.d.702049732.0.dax")
# declare -a wkf=("Montage_25.xml" "Montage_50.xml" "Montage_100.xml" "Inspiral_30.xml" "Inspiral_50.xml" "Inspiral_100.xml")
# declare -a wkf=("CYBERSHAKE.n.200.19.dax" "CYBERSHAKE.n.300.19.dax" "CYBERSHAKE.n.400.0.dax" "CYBERSHAKE.n.500.19.dax")
# declare -a wkf=("MONTAGE.n.200.0.dax" "MONTAGE.n.300.0.dax" "SIPHT.n.300.0.dax" "SIPHT.n.400.0.dax" "GENOME.n.300.19.dax" "GENOME.n.400.19.dax")
trap CTRLC SIGINT

## now loop through the above array
for (( a = 2; a <= 2; a++))
do
	for file in "${wkf[@]}"
	do
		for ((  i = 1 ;  i <= 1;  i++  )) 
		do	
			echo  -ne $file '' & EXEC $execpath -x 5 -a $a -s $i -c $clusters -w $workflows$file$dag #>> ./results/local_search_testing/"GRASP_new_LS.txt"
			# EXEC $execpath -a $a -s $i -c $clusters -w $workflows$file$dag >> ./results/local_search_testing/"GRASP_new_LS.txt"
		done			
	done
	# echo "Creating .csv ...\n" & EXEC ./results/script ./results/local_search_testing/"GRASP_new_LS.txt" ./results/local_search_testing/"GRASP_new_LS.csv" 10
done
