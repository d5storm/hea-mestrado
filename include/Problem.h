#ifndef PROBLEM_H_
#define PROBLEM_H_

#include <iostream>
#include <vector>
#include <list>
#include <limits>
#include <algorithm>
#include <random>
#include <regex>


using namespace std;

class Item{
public:
	int id = -1;
	string name;
	double size;
	int alocated_vm_id = -1;
	vector<int> static_vms;
	bool is_static = false;
	vector<double> IOTimeCost;
	Item(){};
	Item(string name, int id, double size) : name(name), id(id), size(size), is_static(false) {}
	Item(string name, int id, double size, vector<int> static_vms) : name(name), id(id), size(size), is_static(true), static_vms(static_vms) {}
};

class Job{
public:
	string name;
	int id;
	double base_time;
	bool rootJob;
	bool alocated = false;
	int alocated_vm_id = -1;
	vector<Item> input, output;

	bool checkInputNeed(int item_id){
		for(int i = 0; i < input.size(); i++){
			if(input[i].id == item_id)
				return true;
		}
		return false;
	}

	bool checkOutputNeed(int item_id){
		for(int i = 0; i < output.size(); i++){
			if(output[i].id == item_id)
				return true;
		}
		return false;
	}

	Job(){};
	Job (string name, int id, double base_time, bool rootJob, vector<Item> input, vector<Item> output):
		name(name), id(id), base_time(base_time), rootJob(rootJob), input(input), output(output){}
};

class Machine{
public:
	string name;
	int id;
	double slowdown, storage, cost, bandwidth;
	vector<Job> timelineJobs;
	vector<double> timelineStartTime;
	vector<double> timelineFinishTime;
	double makespam = 0.0;

	double calculateLocalspam(){
		if(timelineFinishTime.size() == 0){
			this->makespam = 0.0;
			return 0.0;
		}
		this->makespam = timelineFinishTime[timelineFinishTime.size() - 1];
		return this->makespam; 
	}

	void pushJob(Job job){
		timelineJobs.push_back(job);
		timelineStartTime.push_back(timelineStartTime[timelineStartTime.size() - 1]);
		double readtime = 0.0;
		for(int i = 0; i < job.input.size(); i++){
			if(job.input[i].alocated_vm_id != this->id){
				readtime += min(job.input[i].IOTimeCost[this->id], job.input[i].IOTimeCost[job.input[i].alocated_vm_id]);
			}
		}
		double writetime = 0.0;
		for(int i = 0; i < job.output.size(); i++){
			writetime += job.output[i].IOTimeCost[this->id];
		}
		double processtime = job.base_time * slowdown;

		double totaltime = readtime + writetime + processtime;
		timelineFinishTime.push_back(job.base_time * slowdown);
	}

	int jobPosOnTimeline(int id){
		for(int i = 0; i < this->timelineJobs.size(); i++){
			if(this->timelineJobs[i].id == id)
				return i;
		}
		return -1;
	}

	void addJob(Job job){

	}

	bool timeLineFeasible(){
		for(int i = 1; i < this->timelineJobs.size(); i++){
			if(this->timelineFinishTime[i - 1] > this->timelineStartTime[i])
				return false;
		}
		return true;
	}

	bool checkFeasible(){
		return timeLineFeasible();
	}



	Machine(){}
	Machine (string name, int id, double slowdown, double storage, double cost, double bandwidth):
		name(name), id(id), slowdown(slowdown), storage(storage), cost(cost), bandwidth(bandwidth){}
};

class Problem{

public:
	vector<vector<int>> conflicts;
	vector<vector<int>> notParallelable;
	vector<Item> files;
	vector<Job> jobs;
	vector<Machine> vms;

	double calculateMakespam(){
		double makespam = 0.0;
		for(int i = 0; i < vms.size(); i++){
			double localspam = vms[i].calculateLocalspam();
			if (localspam > makespam) makespam = localspam;
		}
		return makespam;
	}

	double Simulate(){
		cout << "Simulations..." << endl;
		Machine aux = vms[0];
		aux.pushJob(jobs[0]);
		vms[0] = aux;
		aux = vms[2];
		aux.pushJob(jobs[1]);
		aux.pushJob(jobs[2]);
		vms[2] = aux;

		return calculateMakespam();
	}

	Item getFileByName(string name){
		for(int i = 0; i < this->files.size(); i++){
			if(this->files[i].name == name)
				return this->files[i];
		}
		return Item();
	}

	Job getJobByName(string name){
		for(int i = 0; i < this->jobs.size(); i++){
			if(this->jobs[i].name == name)
				return this->jobs[i];
		}
		return Job();
	}


	Problem(string workflow, string cluster){

		//Reading workflow file
		ifstream in_file(workflow);
		string line;

		// Get number of jobs and number of items
		getline(in_file, line);
		vector<string> tokens;
		boost::split(tokens, line, boost::is_any_of(" "));

		int sfile_size = stoi(tokens[0]);
		int dfile_size = stoi(tokens[1]);
		int job_size = stoi(tokens[2]);

		for(int i = 0; i < job_size; i++){
			vector<int> aux1(job_size);
			conflicts.push_back(aux1);
			vector<int> aux2(job_size);
			notParallelable.push_back(aux2);
		}

		int total_file = sfile_size + dfile_size;
		
		getline(in_file, line); //reading blank line

		//Reading items
		for(int i = 0; i < total_file; i++){
			getline(in_file, line);
			vector<string> strs;
			boost::split(strs, line, boost::is_any_of(" "));
			string file_name = strs[0];
			double file_size = stod(strs[1]);

			// std::regex rgx("^file(.*)$");
			// std::smatch matches;
			// std::regex_search(file_name, matches, rgx);
			// int id = stoi(matches[1]);

			// cout << "NAME: " << file_name << endl;
			// cin.get();
			
			Item afile;
			if(i < sfile_size){
				int n_static_vms = stoi(strs[3]);
				vector<int> static_vms;
				for(int j = 4; j <= 3 + n_static_vms; j++)
					static_vms.push_back(stoi(strs[j]));
				afile = Item(file_name, i, file_size, static_vms);
			} else{
				afile = Item(file_name, i, file_size);
				
			}
			this->files.push_back(afile);
		}

		getline(in_file, line); //reading blank line

		// for(int i = 0; i < files.size(); i++){
		// 	cout << "NAME: " << files[i].name << " ID: " << files[i].id << endl;
		// }
		// cin.get();

		for(int i = 0; i < job_size; i++){
			getline(in_file, line);
			vector<string> strs;
			boost::split(strs, line, boost::is_any_of(" "));
			// get job info
			string job_name = strs[0];
			auto job_time = stod(strs[2]);
			auto n_input_files = stoi(strs[3]);
			auto n_output_files = stoi(strs[4]);
			vector<Item> input;
			vector<Item> output;

			//reading input files
			bool is_root_job = true;
			for(int j = 0; j < n_input_files; j++){
				getline(in_file, line);
				Item inputItem = getFileByName(line);
				if(!inputItem.is_static) is_root_job = false;
				input.push_back(inputItem);
			}

			//reading output files
			for(int j = 0; j < n_output_files; j++){
				getline(in_file, line);
				Item outputItem = getFileByName(line);
				output.push_back(outputItem);
			}


			Job ajob(job_name, i, job_time, is_root_job, input, output);
			this->jobs.push_back(ajob);
		}

		getline(in_file, line); //reading blank line

		for(int i = 0; i < jobs.size(); i++){
			cout << "NAME: " << jobs[i].name << " ISROOT:" << jobs[i].rootJob << " BASETIME: " << jobs[i].base_time << " [";
			for(int j = 0; j < jobs[i].input.size(); j++){
				cout << jobs[i].input[j].name << ", ";
			}
			cout << "] [";
			for(int j = 0; j < jobs[i].output.size(); j++){
				cout << jobs[i].output[j].name << ", ";
			}
			cout << "]" << endl;
		}
		cin.get();

		// reading topology
		for(int i = 0; i < job_size; i++){
			getline(in_file, line);
			vector<string> strs;
			boost::split(strs, line, boost::is_any_of(" "));

			string job_name = strs[0];
			Job aux = getJobByName(job_name);
			int father_id = aux.id;
			int children = stoi(strs[1]);
			for(int j = 0; j < children; j++){
				getline(in_file, line);
				vector<string> strs;
				boost::split(strs, line, boost::is_any_of(" "));
				string name = strs[0];
				aux = getJobByName(name);
				int id = aux.id;
				conflicts[id][father_id] = 1;
			}
		}

		// setting if jobs are parallelable

		for(int i = 0; i < job_size-1; i++){
			for(int j = i+1; j < job_size; j++){
				Job job1 = jobs[i];
				Job job2 = jobs[j];
				for(int k = 0; k < total_file; k++){
					if(job1.checkInputNeed(files[k].id) && job2.checkInputNeed(files[k].id)){
						notParallelable[job1.id][job2.id] = 1;
						notParallelable[job2.id][job1.id] = 1;
					}
				}
			}
		}

		// for(int i = 0; i < job_size; i++){
		// 	for(int j = 0; j < job_size; j++){
		// 		cout << notParallelable[i][j] << " ";
		// 	}
		// 	cout << endl;
		// }
		// cin.get();
		// in_file.close();

		ifstream in_cfile(cluster);



		// Reading Cluster's info
		getline(in_cfile, line);//ignore first line
		getline(in_cfile, line);

		vector<string> strs1;
		boost::split(strs1, line, boost::is_any_of(" "));

		int vm_size = stoi(strs1[4]);

		//reading vms
		for(int i = 0; i < vm_size; i++){
			getline(in_cfile, line);
			vector<string> strs;
			boost::split(strs, line, boost::is_any_of(" "));


			int type_id = stoi(strs[0]);
			string vm_name = strs[1];
			double slowdown = stod(strs[2]);
			double storage = stod(strs[3]) * 1024; // GB to MB
			double bandwidth = stod(strs[4]);
			double cost = stod(strs[5]);

			Machine avm(vm_name, type_id, slowdown, storage, cost, bandwidth);
			this->vms.push_back(avm);
		}

		for(int i = 0; i < this->vms.size(); i++){
			cout << "NAME: " << this->vms[i].name << endl;
		}
		cin.get();

		in_cfile.close();

		// pre-calculating READ/WRITE times for files

		for(int i = 0; i < this->files.size(); i++){
			this->files[i].IOTimeCost = vector<double>(this->vms.size());
			for(int j = 0; j < this->vms.size(); j++){
				double IO = this->files[i].size / this->vms[j].bandwidth;
				int vm_id = this->vms[j].id;
				this->files[i].IOTimeCost[vm_id] = IO;
			}	
		}

		// for(int i = 0; i < this->files.size(); i++){
		// 	for(int j = 0; j < this->vms.size(); j++){
		// 		cout << this->files[i].IOTimeCost[j] << " ";
		// 	}
		// 	cout << endl;
		// }
		// cin.get();

	}
};

#endif