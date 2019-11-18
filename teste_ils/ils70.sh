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


execpath="../bin/HEA"
clusters="../input/clusters/cluster.vcl"
workflows="../input/Workflows/"
dag=".dag"
# declare -a wkf=("CyberShake_30.xml" "CyberShake_50.xml")
# declare -a times=("3" "3")
declare -a wkf=("Montage_25.xml" "Montage_50.xml" "Montage_100.xml" "Inspiral_30.xml" "Inspiral_50.xml" "Inspiral_100.xml")
declare -a times=("2665" "2746" "3355" "3382" "3490" "4021")
# declare -a wkf=("CyberShake_30.xml" "CyberShake_50.xml" "CyberShake_100.xml" "GENOME.d.351024866.0.dax" "GENOME.d.702049732.0.dax" "Epigenomics_24.xml" "Epigenomics_46.xml" "Epigenomics_100.xml" "Montage_25.xml" "Montage_50.xml" "Montage_100.xml" "Inspiral_30.xml" "Inspiral_50.xml" "Inspiral_100.xml")
# declare -a times=("73" "212" "1022" "1420" "2245" "2196" "2269" "2661" "2665" "2746" "3355" "3382" "3490" "4021")
# declare -a times=("2746" "3355" "3382" "3490" "4021")
# declare -a wkf=("Montage_50.xml" "Montage_100.xml" "Inspiral_30.xml" "Inspiral_50.xml" "Inspiral_100.xml")
# declare -a wkf=("GENOME.d.702049732.0.dax")
# declare -a wkf=("Inspiral_30.xml" "Inspiral_50.xml" "Inspiral_100.xml")
# declare -a wkf=("CYBERSHAKE.n.200.19.dax" "CYBERSHAKE.n.300.19.dax" "CYBERSHAKE.n.400.0.dax" "CYBERSHAKE.n.500.19.dax")
# declare -a wkf=("MONTAGE.n.200.0.dax" "MONTAGE.n.300.0.dax" "SIPHT.n.300.0.dax" "SIPHT.n.400.0.dax" "GENOME.n.300.19.dax" "GENOME.n.400.19.dax")
declare -a ils=("70")
trap CTRLC SIGINT

## now loop through the above array
for (( a = 2; a <= 2; a++))
do
	
	for i in "${ils[@]}"
	do
		time=0
		for file in "${wkf[@]}"
		do
			for ((  s = 1 ;  s <= 10;  s++  )) 
			do	
				echo  -ne $file '' & EXEC $execpath -g 100 -i $i -t ${times[$time]} -x 5 -a $a -s $s -c $clusters -w $workflows$file$dag --timelimit >> ../results/grasp_ils_new/dm/grasp_time_ils_$i.txt
				# EXEC $execpath -a $a -s $i -c $clusters -w $workflows$file$dag >> ./results/local_search_testing/"GRASP_new_LS.txt"
			done
			time=$((time+1))			
		done
	done
	# echo "Creating .csv ...\n" & EXEC ./results/script ./results/local_search_testing/"GRASP_new_LS.txt" ./results/local_search_testing/"GRASP_new_LS.csv" 10
done
