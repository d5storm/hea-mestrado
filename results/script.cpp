#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <cmath>

using namespace std;


int main(int argc, char **argv){
	if(argc < 4){
		cerr << "\033[1;31mINSERT FILE PATH ... \033[0m" << endl;
		exit(1);
	}
	string filePath(argv[1]);
	string resultPath(argv[2]);
	int seedNumber = atoi(argv[3]);
	
	ifstream File;
	File.open(filePath);
	ofstream Out;
	
	string line;
	
	string instanceName;
	double fo, time;
	
	double timeSum, foSum, foMin;
	
	Out.open(resultPath);
	Out << "instance,best,average,cv,time" << endl;
	
	while (getline(File, line)) {
		foMin  = 9999999999.0;
		vector<double> Fos;
		istringstream iss(line);
		if (!(iss >> instanceName >> fo >> time)) { break; }
		Fos.push_back(fo);
		timeSum += time;
		foSum += fo;
		if(fo < foMin) foMin = fo;
		for(int i = 1; i < seedNumber; i++){
			getline(File, line);
			istringstream iss(line);
			if (!(iss >> instanceName >> fo >> time)) { break; }
			timeSum += time;
			foSum += fo;
			if(fo < foMin) foMin = fo;
		}
		double E;
		double foAVG = foSum / (double)seedNumber;
		for( int i = 0; i < seedNumber; i++){
			E += pow((Fos[i] - foAVG), 2) / (double)seedNumber;
		}
		double dp = sqrt(E);
		double cv = dp / foAVG;
		ostringstream oss;
		oss << instanceName << "," << foMin << "," << foSum/seedNumber << "," << cv << "," << timeSum/seedNumber << "\n";
		Out << oss.str();
		//exit(1);
	}
	
	File.close();
	Out.close();
	
}