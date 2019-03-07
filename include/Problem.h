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
	vector<double> VMsBandwidth;
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
	vector<Item*> input, output;

	bool checkJobFeasibleOnMachine(int vm_id){
		// cout << "JobNameTested: " << this->name << endl;
		// cout << "testing feasible on Machine: " << vm_id << endl;
		if(rootJob){
		// cout << "IsRoot" << endl;
			for(int f = 0; f < input.size(); f++){
				if(input[f]->is_static){
					bool feasible = false;
					// cout << "StaticVMs( " << input[f]->static_vms.size() << "):";
					if(input[f]->alocated_vm_id < 0){
						for(int vm = 0; vm < input[f]->static_vms.size(); vm++){
							// cout << input[f]->static_vms[vm] << " "; 
							if(input[f]->static_vms[vm] == vm_id){
								feasible = true;
								break;
							}
						}
						// cout << endl;
						if(!feasible)
							return false;
					}
				}
			}
		}
		return true;
	}

	bool checkInputNeed(int item_id){
		for(int i = 0; i < input.size(); i++){
			if(input[i]->id == item_id)
				return true;
		}
		return false;
	}

	bool checkOutputNeed(int item_id){
		for(int i = 0; i < output.size(); i++){
			if(output[i]->id == item_id)
				return true;
		}
		return false;
	}

	Job(){};
	Job (string name, int id, double base_time, bool rootJob, vector<Item*> input, vector<Item*> output):
		name(name), id(id), base_time(base_time), rootJob(rootJob), input(input), output(output){}
};

class Machine{
public:
	string name;
	int id;
	double slowdown, storage, cost, bandwidth;
	vector<Job*> timelineJobs;
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

	bool popJob(int jobId){
		for(int i = 0; i < timelineJobs.size(); i++){
			if (timelineJobs[i]->id == jobId){
				timelineJobs[i]->alocated = false;
				for(int f = 0; f < timelineJobs[i]->output.size(); f++){
					timelineJobs[i]->output[f]->alocated_vm_id = -1;
				}
				if(timelineJobs[i]->rootJob){
					for(int f = 0; f < timelineJobs[i]->input.size(); f++){
						if(timelineJobs[i]->input[f]->is_static)
							timelineJobs[i]->input[f]->alocated_vm_id = -1;
					}	
				}
				timelineJobs.erase(timelineJobs.begin() + i);
				timelineStartTime.erase(timelineStartTime.begin() + i);
				timelineFinishTime.erase(timelineFinishTime.begin() + i);
				return true;
			}
		}
		return false;
	}

	bool pushJob(Job * job, int write_vm_id, double minSpam){
		if(!job->checkJobFeasibleOnMachine(this->id)){
			cout << "not feasible on machine" << endl;
			return false;
		}
		// cout << "feasible on machine" << endl;
		// cout << "MinSpam: " << minSpam << endl;
		cout << "JobID: " << job->id << endl;
		for(int i = 0; i < job->input.size(); i++){ // checando se todos os arquivos de input que nao sao estaticos ja foram produzidos
			// cout << "inputFileID: " << job->input[i]->id << " isStatic: " << job->input[i]->is_static << endl;
			if(job->input[i]->is_static == false && job->input[i]->alocated_vm_id < 0){
				cout << "JOB NOT AVAILABLE: FILES NOT READY" << endl; 
				return false;
			}
		}


		double startTime; // calculando tempo de inicio.
		if (timelineJobs.size() == 0)
			startTime = 0.0;
		else
			startTime = (timelineFinishTime[timelineFinishTime.size() - 1]);

		if(startTime < minSpam)
			startTime = minSpam;

		// cout << "StartTime: " << startTime << endl;
		// cin.get();
		

		
		double readtime = 0.0; // calculando o tempo de leitura de todos os arquivos de input necessarios caso nao estejam alocados na Maquina
		for(int i = 0; i < job->input.size(); i++){
			if(job->input[i]->alocated_vm_id == this->id)
				continue;
			int minBandwidthVm = this->id;
			if(job->input[i]->VMsBandwidth[job->input[i]->alocated_vm_id] < job->input[i]->VMsBandwidth[this->id])
				minBandwidthVm = job->input[i]->alocated_vm_id;
			readtime += job->input[i]->IOTimeCost[minBandwidthVm];
		}

		// cout << "ReadTime: " << readtime << endl;
		// cin.get();

		double writetime = 0.0;
		if(write_vm_id != -1 && write_vm_id != this->id){
			for(int i = 0; i < job->output.size(); i++){
				int minBandwidthVm = this->id;
				if(job->output[i]->VMsBandwidth[write_vm_id] < job->output[i]->VMsBandwidth[this->id])
					minBandwidthVm = job->output[i]->alocated_vm_id;
				writetime += job->output[i]->IOTimeCost[minBandwidthVm];
			}
		}
		double processtime = ceil(job->base_time * this->slowdown);
		double finishTime = readtime + writetime + processtime + startTime;

		timelineStartTime.push_back(startTime);
		timelineFinishTime.push_back(finishTime);
		timelineJobs.push_back(job);

		job->alocated = true;
		job->alocated_vm_id = this->id;
		for(int i = 0; i < job->output.size(); i++){
			int final_vm = this->id;
			if(write_vm_id != -1)
				final_vm = write_vm_id;
			job->output[i]->alocated_vm_id = final_vm;
		}

		for(int i = 0; i < job->input.size(); i++){
			if(job->input[i]->is_static){
				if(job->input[i]->alocated_vm_id < 0)
					job->input[i]->alocated_vm_id = this->id;
			}
		}

		return true;
	}

	int jobPosOnTimeline(int id){
		for(int i = 0; i < this->timelineJobs.size(); i++){
			if(this->timelineJobs[i]->id == id)
				return i;
		}
		return -1;
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
	vector<Item*> files;
	vector<Job*> jobs;
	vector<Machine*> vms;

	void undoSolution(){

	}

	double createSolution(double alpha){
		int totalJobs = jobs.size();
		while(totalJobs > 0){
			// cout << "************************** TOTAL JOBS: " << totalJobs << endl;
			// cin.get();
			vector<Job*> CL;
			vector<int> jobVmDestination;
			vector<int> outputVmDestination;
			vector<double> cost;
			for(int i = 0; i < jobs.size(); i++){
				// cout << "i: " << i  << " JobName: "<< jobs[i]->name << endl;
				if (jobs[i]->alocated)
					continue;
				for(int m = 0; m < vms.size(); m++){
					for(int d = 0; d < vms.size(); d++){

						double minSpam = getJobConflictMinSpam(jobs[i]);
						if(minSpam < 0) break; 
						bool pushed = vms[m]->pushJob(jobs[i], d, minSpam);
						// cin.get();
						if(!pushed){
							// cout << "COULD NOT PUSH!" << endl;
							// cin.get();
							break;
						}
						double insertionCost = calculateMakespam();
						// cout << "JOB: " << jobs[i]->id << " vmsID: " << vms[m]->id << " InsertionID: " << vms[d]->id <<  " COST: " << insertionCost << endl;
						// cin.get();
						if(CL.size() == 0){
							CL.push_back(jobs[i]);
							jobVmDestination.push_back(m);
							cost.push_back(insertionCost);
							outputVmDestination.push_back(d);
						} else{
							bool inserted = false;
							for(int j = 0; j < cost.size(); j++){
								if(cost[j] >= insertionCost){
									cost.insert(cost.begin() + j, insertionCost);
									jobVmDestination.insert(jobVmDestination.begin() + j, m);
									CL.insert(CL.begin() + j, jobs[i]);
									outputVmDestination.insert(outputVmDestination.begin() + j, d);
									inserted = true;
									break;
								}
							}
							if (!inserted){
								cost.push_back(insertionCost);
								jobVmDestination.push_back(m);
								CL.push_back(jobs[i]);
								outputVmDestination.push_back(d);
							}
						}
						// cout << "tested!" << endl;
						vms[m]->popJob(jobs[i]->id);
						// cout << "Spam After Removal: " << vms[m]->calculateLocalspam() << endl;
						// cin.get();
					}
				}
			}

			// cout << "CL size: " << CL.size() << endl;
			// for(int c = 0; c < CL.size(); c++){
			// 	cout << CL[c]->id << " " ;
			// }
			// cout << endl;

			int chosenMovement;
			int maxClPos = (int)(CL.size() * alpha);
			if(maxClPos == 0)
				chosenMovement = 0;
			else
				chosenMovement = random() % maxClPos;

			// cout << "Chosen Movement: " << chosenMovement << endl;
			// cout << "CLSIZE: " << CL.size() << endl;
			bool moveDone = doMovement(jobVmDestination[chosenMovement], outputVmDestination[chosenMovement], CL[chosenMovement]);
			if(moveDone){
				// cout << "JobID: " << CL[chosenMovement]->id << " Was Inserted!" << endl;
				totalJobs--;
			}
			else
				// cout << "JobID: " << CL[chosenMovement]->id << " Was NOT Inserted!" << endl;
			
			// cout << "Spam: " << calculateMakespam() << endl;
			cin.get();

		}
		// cout << "Testing solution..." << endl;
		// cout << "[";
		// for(int i = 0; i < jobs.size();i++){
		// 	cout << jobs[i]->alocated_vm_id << ",";
		// }
		// cout << "]" << endl;
		// cout << "[";
		// for(int i = 0; i < jobs.size();i++){
		// 	cout << jobs[i]->alocated << ",";
		// }
		// cout << "]" << endl;
		// cin.get();
		return calculateMakespam();
	}

	bool doMovement(int vm, int output, Job* job){
		return vms[vm]->pushJob(job, output, getJobConflictMinSpam(job));
	}

	void preSetStaticFile(Item * file, int vm_id){
		file->alocated_vm_id = vm_id;
	}

	void print(){
		cout << "VMs:" << endl;
		for(int i = 0; i < vms.size(); i++){
			cout << vms[i]->id << ": " ;
			for(int j = 0; j < vms[i]->timelineJobs.size(); j++){
				cout << vms[i]->timelineJobs[j]->name << "( " << vms[i]->timelineStartTime[j] << "," << vms[i]->timelineFinishTime[j] << " )" << " ";
			}
			cout << endl;
		}

		for(int i = 0; i < files.size(); i++){
			cout << "file: " << files[i]->id << " VM: " << files[i]->alocated_vm_id << endl;
		}
	}

	double calculateMakespam(){
		double makespam = 0.0;
		for(int i = 0; i < vms.size(); i++){
			double localspam = vms[i]->calculateLocalspam();
			if (localspam > makespam) makespam = localspam;
		}
		return makespam;
	}

	bool checkFeasible(){
		for(int i = 0; i < files.size(); i++){  // checando se os arquivos static estao alocados nas máquinas possíveis.
			if(files[i]->is_static){
				bool feasible_alocation = false;
				for(int m = 0; m < files[i]->static_vms.size(); m++){
					if(files[i]->alocated_vm_id == files[i]->static_vms[m]){
						feasible_alocation = true;
						break;
					}
				}
				if(!feasible_alocation){
					cout << "Arquivo: " << i << " nao alocado numa static_vms!" << endl;
					return false;
				}
			}
		}

		for(int i = 0; i < files.size(); i++){  // checando se todos os arquivos estao alocados a alguma maquina
			if(files[i]->alocated_vm_id < 0){
				cout << "Arquivo: " << i << " nao alocado!" << endl;
				return false;
			}
		}

		for(int i = 0; i < jobs.size(); i++){ // checando se todos os jobs foram alocados
			if(!jobs[i]->alocated){
				cout << "Job: " << i << " nao alocado!" << endl;
				return false;
			}
			if(jobs[i]->alocated_vm_id < 0){
				cout << "Job: " << i << " alocado, mas sem vm_id!" << endl;
				return false;
			}

			//checando se topologia foi respeitada
			for(int j = 0; j < jobs.size();j++){
				if(conflicts[i][j] == 0)
					continue;
				if(jobs[j]->alocated == false){
					cout << "Job: " << i << " nao teve topologia respeitada! Pre-Job: " << j << endl;
					return false;
				}

				double preJobFinishtime = 0.0;
				Machine * preJobMachine = vms[jobs[j]->alocated_vm_id];
				int pos = preJobMachine->jobPosOnTimeline(jobs[j]->id);
				preJobFinishtime = preJobMachine->timelineFinishTime[pos];
				
				double jobStartTime = 0.0;
				Machine * JobMachine = vms[jobs[i]->alocated_vm_id];
				pos = JobMachine->jobPosOnTimeline(jobs[i]->id);
				jobStartTime = JobMachine->timelineStartTime[pos];
				// cout << "Job: " << i << " comecou no tempo: " << jobStartTime << ", mas preJob:" << j << " terminou no tempo: " << preJobFinishtime << endl;
				if(jobStartTime < preJobFinishtime){
					cout << "Job: " << i << " comecou no tempo: " << jobStartTime << ", mas preJob:" << j << " terminou no tempo: " << preJobFinishtime << endl;
					return false;
				}

			}

		}	


		return true;

	}

	double getJobConflictMinSpam(Job * job){
		int id = job->id;
		double minSpam = 0.0;
		for(int i = 0; i < conflicts[id].size(); i++){ // checando tempo minimo baseado na topologia até o arquivo.
			if(conflicts[id][i] == 0)
				continue;
			if(jobs[i]->alocated == false)
				return -1.0;
			int machineID = jobs[i]->alocated_vm_id;
			Machine * vm = vms[machineID];
			double spam = vm->calculateLocalspam();
			if(spam > minSpam)
				minSpam = spam;
		}
		// cout << "MinSpam: " << minSpam << endl;
		// for(int i = 0; i < notParallelable[id].size(); i++){ // checando tempo minimo baseado na possivel paralelização
		// 	cout << "i: " << i << " notParallelable: " << notParallelable[id][i] << endl;
		// 	if(notParallelable[id][i] == 0)
		// 		continue;
		// 	if(jobs[i]->alocated == false){
		// 		cout << "Conflictuous job not yet assigned" << endl;
		// 		continue;
		// 	}
		// 	int machineID = jobs[i]->alocated_vm_id;
		// 	Machine * vm = vms[machineID];
		// 	double spam = vm->calculateLocalspam();
		// 	if(spam > minSpam)
		// 		minSpam = spam;
		// }
		return minSpam;
	}

	double Simulate(){

		// createSolution(0.3);

		cout << "Simulations..." << endl;
		Machine * aux = vms[0];
		double minSpam;
		minSpam = getJobConflictMinSpam(jobs[0]);
		cout << "Job[0] name: " << jobs[0]->name << endl;
		aux->pushJob(jobs[0], 2, minSpam);
		aux = vms[2];
		minSpam = getJobConflictMinSpam(jobs[1]);
		cout << "Job[1] name: " << jobs[1]->name << endl;
		aux->pushJob(jobs[1], -1, minSpam);
		minSpam = getJobConflictMinSpam(jobs[2]);
		cout << "Job[2] name: " << jobs[2]->name << endl;
		aux->pushJob(jobs[2], -1, minSpam);
		double makespam = calculateMakespam();
		cout << "My Makespam: " << makespam << endl;
		this->print();
		return makespam;
		// cin.get();
	}

	Item * getFileByName(string name){
		for(int i = 0; i < this->files.size(); i++){
			if(this->files[i]->name == name)
				return this->files[i];
		}
		return NULL;
	}

	Job * getJobByName(string name){
		for(int i = 0; i < this->jobs.size(); i++){
			if(this->jobs[i]->name == name)
				return this->jobs[i];
		}
		return NULL;
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
			
			Item * afile;
			if(i < sfile_size){
				int n_static_vms = stoi(strs[2]);
				// cout << "n_static: " << n_static_vms << endl;
				vector<int> static_vms;
				for(int j = 3; j < 3 + n_static_vms; j++){
					static_vms.push_back(stoi(strs[j]));
					// cout << strs[j] << endl;
				}
				afile = new Item(file_name, i, file_size, static_vms);
			} else{
				afile = new Item(file_name, i, file_size);
				
			}
			this->files.push_back(afile);
		}
		// cout << this->files.size() << endl;
		// for(int i = 0; i < this->files.size();i++){
		// 	cout << this->files[i]->name << endl;
		// }
		getline(in_file, line); //reading blank line
		// cout << "!!!" << endl;

		for(int i = 0; i < job_size; i++){
			getline(in_file, line);
			vector<string> strs;
			boost::split(strs, line, boost::is_any_of(" "));
			// get job info
			string job_name = strs[0];
			auto job_time = stod(strs[2]);
			auto n_input_files = stoi(strs[3]);
			auto n_output_files = stoi(strs[4]);
			vector<Item*> input;
			vector<Item*> output;
			//reading input files
			bool is_root_job = true;
			int total_not_roots = 0;
			for(int j = 0; j < n_input_files; j++){
				getline(in_file, line);
				Item * inputItem = getFileByName(line);
				if(!inputItem->is_static) total_not_roots++;
				input.push_back(inputItem);
			}
			if(total_not_roots == n_input_files)
				is_root_job = false;
			//reading output files
			for(int j = 0; j < n_output_files; j++){
				getline(in_file, line);
				Item * outputItem = getFileByName(line);
				output.push_back(outputItem);
			}

			Job * ajob = new Job(job_name, i, job_time, is_root_job, input, output);
			this->jobs.push_back(ajob);
		}
		// cout << "!!!" << endl;
		getline(in_file, line); //reading blank line

		// for(int i = 0; i < jobs.size(); i++){
		// 	cout << "NAME: " << jobs[i]->name << " ISROOT:" << jobs[i]->rootJob << " BASETIME: " << jobs[i]->base_time << " [";
		// 	for(int j = 0; j < jobs[i]->input.size(); j++){
		// 		cout << jobs[i]->input[j]->name << ", ";
		// 	}
		// 	cout << "] [";
		// 	for(int j = 0; j < jobs[i]->output.size(); j++){
		// 		cout << jobs[i]->output[j]->name << ", ";
		// 	}
		// 	cout << "]" << endl;
		// }
		// cin.get();

		// reading topology
		for(int i = 0; i < job_size; i++){
			getline(in_file, line);
			vector<string> strs;
			boost::split(strs, line, boost::is_any_of(" "));

			string job_name = strs[0];
			Job * aux = getJobByName(job_name);
			int father_id = aux->id;
			int children = stoi(strs[1]);
			for(int j = 0; j < children; j++){
				getline(in_file, line);
				vector<string> strs;
				boost::split(strs, line, boost::is_any_of(" "));
				string name = strs[0];
				aux = getJobByName(name);
				int id = aux->id;
				conflicts[id][father_id] = 1;
			}
		}

		// setting if jobs are parallelable

		for(int i = 0; i < job_size-1; i++){
			for(int j = i+1; j < job_size; j++){
				Job * job1 = jobs[i];
				Job * job2 = jobs[j];
				for(int k = 0; k < total_file; k++){
					if(job1->checkInputNeed(files[k]->id) && job2->checkInputNeed(files[k]->id)){
						notParallelable[job1->id][job2->id] = 1;
						notParallelable[job2->id][job1->id] = 1;
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
		in_file.close();

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

			Machine * avm = new Machine(vm_name, type_id, slowdown, storage, cost, bandwidth);
			this->vms.push_back(avm);
		}

		// for(int i = 0; i < this->vms.size(); i++){
		// 	cout << "NAME: " << this->vms[i]->name << endl;
		// }
		// cin.get();

		in_cfile.close();

		// pre-calculating READ/WRITE times for files

		for(int i = 0; i < this->files.size(); i++){
			this->files[i]->IOTimeCost = vector<double>(this->vms.size());
			for(int j = 0; j < this->vms.size(); j++){
				double IO = ceil(this->files[i]->size / this->vms[j]->bandwidth);
				int vm_id = this->vms[j]->id;
				this->files[i]->IOTimeCost[vm_id] = IO;
			}	
		}

		for(int i = 0; i < this->files.size(); i++){
			this->files[i]->VMsBandwidth = vector<double>(this->vms.size());
			for(int j = 0; j < this->vms.size(); j++){
				this->files[i]->VMsBandwidth[j] = this->vms[j]->bandwidth;
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