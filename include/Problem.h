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
			for(unsigned int f = 0; f < input.size(); f++){
				if(input[f]->is_static){
					bool feasible = false;
					// cout << "StaticVMs( " << input[f]->static_vms.size() << "):";
					if(input[f]->alocated_vm_id < 0){
						for(unsigned int vm = 0; vm < input[f]->static_vms.size(); vm++){
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
		for(unsigned int i = 0; i < input.size(); i++){
			if(input[i]->id == item_id)
				return true;
		}
		return false;
	}

	bool checkOutputNeed(int item_id){
		for(unsigned int i = 0; i < output.size(); i++){
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
		for(unsigned int i = 0; i < timelineJobs.size(); i++){
			if (timelineJobs[i]->id == jobId){
				timelineJobs[i]->alocated = false;
				for(unsigned int f = 0; f < timelineJobs[i]->output.size(); f++){
					timelineJobs[i]->output[f]->alocated_vm_id = -1;
				}
				if(timelineJobs[i]->rootJob){
					for(unsigned int f = 0; f < timelineJobs[i]->input.size(); f++){
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
		// if(!job->checkJobFeasibleOnMachine(this->id)){
		// 	cout << "not feasible on machine" << endl;
		// 	return false;
		// }
		// cout << "feasible on machine" << endl;
		// cout << "MinSpam: " << minSpam << endl;
		// cout << "JobID: " << job->id << endl;
		for(unsigned int i = 0; i < job->input.size(); i++){ // checando se todos os arquivos de input que nao sao estaticos ja foram produzidos
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
		for(unsigned int i = 0; i < job->input.size(); i++){
			if(job->input[i]->alocated_vm_id == this->id)
				continue;
			int minBandwidthVm = this->id;

			if(job->input[i]->is_static){
				bool transferNeed = true;
				double maxBandwidth = 0.0;
				int id;
				for(int j = 0; j < job->input[i]->static_vms.size(); j++){
					if(this->id == job->input[i]->static_vms[j])
						transferNeed = false;
						break;
					if(job->input[i]->VMsBandwidth[job->input[i]->static_vms[j]] > maxBandwidth){
						maxBandwidth = job->input[i]->VMsBandwidth[job->input[i]->static_vms[j]];
						id = job->input[i]->static_vms[j];
					}
				}

				if(job->input[i]->VMsBandwidth[id] < job->input[i]->VMsBandwidth[this->id])
					minBandwidthVm = id;
				if(transferNeed) readtime += job->input[i]->IOTimeCost[minBandwidthVm];
			} else{
				if(job->input[i]->VMsBandwidth[job->input[i]->alocated_vm_id] < job->input[i]->VMsBandwidth[this->id])
					minBandwidthVm = job->input[i]->alocated_vm_id;
				readtime += job->input[i]->IOTimeCost[minBandwidthVm];			
			}
		}

		// cout << "ReadTime: " << readtime << endl;
		// cin.get();

		double writetime = 0.0;
		if(write_vm_id != -1 && write_vm_id != this->id){
			for(unsigned int i = 0; i < job->output.size(); i++){
				int minBandwidthVm = this->id;
				if(job->output[i]->VMsBandwidth[write_vm_id] < job->output[i]->VMsBandwidth[this->id])
					minBandwidthVm = job->output[i]->alocated_vm_id;
				writetime += job->output[i]->IOTimeCost[minBandwidthVm];
			}
		}
		double processtime = ceil(job->base_time * this->slowdown);
		double finishTime = readtime + writetime + processtime + startTime;

		// if(job->name == "ID00002"){
		// 	cout << "ProcessMachineID: " <<this->id << " ProcessMAchineName: " << this->name << " writeID: " << write_vm_id << " minSpam: " << minSpam << endl;
		// 	cout << "start: " << startTime << " read: " << readtime << " process: " << processtime << " write: " << writetime << " fulltime: " << finishTime << endl;
		// 	cin.get();
		// }

		timelineStartTime.push_back(startTime);
		timelineFinishTime.push_back(finishTime);
		timelineJobs.push_back(job);

		job->alocated = true;
		job->alocated_vm_id = this->id;
		for(unsigned int i = 0; i < job->output.size(); i++){
			int final_vm = this->id;
			if(write_vm_id != -1)
				final_vm = write_vm_id;
			job->output[i]->alocated_vm_id = final_vm;
		}

		for(unsigned int i = 0; i < job->input.size(); i++){
			if(job->input[i]->is_static){
				if(job->input[i]->alocated_vm_id < 0)
					job->input[i]->alocated_vm_id = this->id;
			}
		}

		return true;
	}

	int jobPosOnTimeline(int id){
		for(unsigned int i = 0; i < this->timelineJobs.size(); i++){
			if(this->timelineJobs[i]->id == id)
				return i;
		}
		return -1;
	}

	bool timeLineFeasible(){
		for(unsigned int i = 1; i < this->timelineJobs.size(); i++){
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

class Allocation{
public:
	Job * job;
	Machine * vms;
	int writeTo;
};

class Problem{

public:
	vector<vector<int>> conflicts;
	vector<vector<int>> notParallelable;
	vector<Item*> files;
	vector<Job*> jobs;
	vector<Machine*> vms;
	vector<Allocation*> alloc;

	bool changeAllocVm(int pos, int vm_id){
		// cout << "!!!" << endl;

		Job* job = alloc[pos]->job;
		for(int i = 0; i < job->input.size(); i++){
			if(job->input[i]->is_static){
				bool found = false;
				for(int j = 0; j < job->input[i]->static_vms.size(); j++){
					if(job->input[i]->static_vms[j] == vm_id){
						found = true;
						break;
					}
				}
				if(!found)
					return false;
			}

		}

		for(int i = alloc.size() - 1; i >= pos; i--){
			alloc[i]->vms->popJob(alloc[i]->job->id);
		}
		// this->print();
		// cin.get();
		alloc[pos]->vms = this->vms[vm_id];
		for(unsigned int i = pos; i < alloc.size(); i++){
			double minSpam = getJobConflictMinSpam(alloc[i]->job);
			// cout << "MinSpam: " << minSpam << endl;
			// cin.get();
			alloc[i]->vms->pushJob(alloc[i]->job, alloc[i]->writeTo, minSpam);
		}
		return true;
	}

	

	bool changeAllocOrder(int pos1, int pos2){
		Job * job1 = alloc[pos1]->job;
		Job * job2 = alloc[pos2]->job;
		for(unsigned int i = pos1; i < pos2; i++){
			if(conflicts[job2->id][alloc[i]->job->id] == 1)
				return false;
		}
		
		

		for(int i = (alloc.size() - 1); i >= pos1; i--){
			alloc[i]->vms->popJob(alloc[i]->job->id);
		}

		this->print();

		Allocation * aux = alloc[pos1];
		alloc[pos1] = alloc[pos2];
		alloc[pos2] = aux;

		for(unsigned int i = pos1; i < alloc.size(); i++){
			double minSpam = getJobConflictMinSpam(alloc[i]->job);
			alloc[i]->vms->pushJob(alloc[i]->job, alloc[i]->writeTo, minSpam);
		}
		return true;
	}

	bool forceRelocate(int pos){
		vector<int> possibleMoves;
		Allocation * relocation = this->alloc[pos];
		// this->printAlloc();
		// cout << "********" << endl;
		this->alloc.erase(this->alloc.begin() + pos);
		for(int i = 0; i < this->alloc.size(); i++){
			if(i == pos) continue;
			bool tryMove = true;
			if(pos > i){
				for(unsigned int j = i; j < this->alloc.size(); j++){ // vendo se topologia é respeitada para a direita
					if(this->conflicts[relocation->job->id][this->alloc[j]->job->id] == 1){
						tryMove = false;
						break;
					}
				}
			} else{
				for(unsigned int j = 0; j < i; j++){ // vendo se topologia é respeitada para a esquerda
					if(this->conflicts[this->alloc[j]->job->id][relocation->job->id] == 1){
						tryMove = false;
						break;
					}
				}
			}

			if(!tryMove){ // topologia nao foi respeitada, nao tente movimento
				// cout << "quebra topologia" << endl;
				continue;
			}
			possibleMoves.push_back(i);
			
			
		}
		if(possibleMoves.empty()){
			this->alloc.insert(this->alloc.begin() + pos, relocation);
			return false;
		}
		int posRand = random() % possibleMoves.size();
		int insertionPos = possibleMoves.at(posRand);
		if(possibleMoves.size() > 1){
			while(insertionPos == pos){
				posRand = random() % possibleMoves.size();
				insertionPos = possibleMoves.at(posRand);
			}
		}

			
		this->alloc.insert(this->alloc.begin() + insertionPos, relocation);
		
		// this->printAlloc();
		// cin.get();
		// cout << "PosRemove: " << pos << " PosInsert: " << insertionPos << endl;

		for(int j = this->alloc.size() - 1; j >= pos; j--)
			this->alloc[j]->vms->popJob(this->alloc[j]->job->id);


		for(unsigned int j = pos; j < this->alloc.size(); j++){
			double minSpam = this->getJobConflictMinSpam(this->alloc[j]->job);
			this->alloc[j]->vms->pushJob(this->alloc[j]->job, this->alloc[j]->writeTo, minSpam);
		}

		// this->printAlloc();

		return true;
	}

	bool swapMachine(int pos){
		Allocation * swap = this->alloc[pos];
		Machine * originalAllocationMachine = swap->vms;
		double originalCost = this->calculateMakespam();

		if(!this->checkFeasible()){
			cout << "At first it was not feasible" << endl;
			cin.get();
		}

		for(unsigned int i = 0; i < this->vms.size(); i++){
			
			for(int j = this->alloc.size() - 1; j >= pos; j--)
				this->alloc[j]->vms->popJob(this->alloc[j]->job->id);

			swap->vms = this->vms[i];

			for(unsigned int j = pos; j < this->alloc.size(); j++){
				double minSpam = this->getJobConflictMinSpam(this->alloc[j]->job);
				this->alloc[j]->vms->pushJob(this->alloc[j]->job, this->alloc[j]->writeTo, minSpam);
			}

			if(!this->checkFeasible()){
				cout << "machine swap was not feasible" << endl;
				cin.get();
			}

			if(this->calculateMakespam() < originalCost){
				return true;
			}
		}
		swap->vms = originalAllocationMachine;
		for(int j = this->alloc.size() - 1; j >= pos; j--)
				this->alloc[j]->vms->popJob(this->alloc[j]->job->id);
		for(unsigned int j = pos; j < this->alloc.size(); j++){
			double minSpam = this->getJobConflictMinSpam(this->alloc[j]->job);
			this->alloc[j]->vms->pushJob(this->alloc[j]->job, this->alloc[j]->writeTo, minSpam);
		}

		if(!this->checkFeasible()){
			cout << "undo of move was not feasible" << endl;
			cin.get();
		}

		return false;
	}

	bool swapMachineWrite(int pos){
		Allocation * swap = this->alloc[pos];
		int originalAllocationMachine = swap->writeTo;
		double originalCost = this->calculateMakespam();
		for(unsigned int i = 0; i < this->vms.size(); i++){
			swap->writeTo = this->vms[i]->id;
			for(int j = this->alloc.size() - 1; j >= pos; j--)
				this->alloc[j]->vms->popJob(this->alloc[j]->job->id);

			for(unsigned int j = pos; j < this->alloc.size(); j++){
				double minSpam = this->getJobConflictMinSpam(this->alloc[j]->job);
				this->alloc[j]->vms->pushJob(this->alloc[j]->job, this->alloc[j]->writeTo, minSpam);
			}

			if(this->calculateMakespam() < originalCost){
				return true;
			}
		}
		swap->writeTo = originalAllocationMachine;
		for(int j = this->alloc.size() - 1; j >= pos; j--)
				this->alloc[j]->vms->popJob(this->alloc[j]->job->id);
		for(unsigned int j = pos; j < this->alloc.size(); j++){
			double minSpam = this->getJobConflictMinSpam(this->alloc[j]->job);
			this->alloc[j]->vms->pushJob(this->alloc[j]->job, this->alloc[j]->writeTo, minSpam);
		}
		return false;
	}

	bool realocate(int pos1, int pos2){
		Allocation * relocation = this->alloc[pos1];
		// this->printAlloc();
		// cout << "********" << endl;
		this->alloc.erase(this->alloc.begin() + pos1);

		bool tryMove = true;
		if(pos1 > pos2){
			for(unsigned int j = pos2; j < this->alloc.size(); j++){ // vendo se topologia é respeitada para a direita
				if(this->conflicts[relocation->job->id][this->alloc[j]->job->id] == 1){
					tryMove = false;
					break;
				}
			}
		} else{
			for(unsigned int j = 0; j < pos2; j++){ // vendo se topologia é respeitada para a esquerda
				if(this->conflicts[this->alloc[j]->job->id][relocation->job->id] == 1){
					tryMove = false;
					break;
				}
			}
		}

		if(!tryMove){ // topologia nao foi respeitada, nao tente movimento
			// cout << "quebra topologia" << endl;
			this->alloc.insert(this->alloc.begin() + pos1, relocation);
			return false;
		}

			
		this->alloc.insert(this->alloc.begin() + pos2, relocation);
		
		// this->printAlloc();
		// cin.get();
		// cout << "PosRemove: " << pos << " PosInsert: " << insertionPos << endl;
		bool teste = true;
		for(int j = this->alloc.size() - 1; j >= pos1; j--){
			teste = this->alloc[j]->vms->popJob(this->alloc[j]->job->id);
			if(!teste){
				cout << "NAO CONSEGUIU POP!" << endl;
				cin.get();
			}
		}

		for(unsigned int j = pos1; j < this->alloc.size(); j++){
			double minSpam = this->getJobConflictMinSpam(this->alloc[j]->job);
			this->alloc[j]->vms->pushJob(this->alloc[j]->job, this->alloc[j]->writeTo, minSpam);
		}

		// this->printAlloc();

		return true;
	}

	void changeAllocWrite(int pos, int newWriteTo){
		for(int i = alloc.size() - 1; i >= pos; i--){
			alloc[i]->vms->popJob(alloc[i]->job->id);
		}
		// this->print();
		// cin.get();
		alloc[pos]->writeTo = newWriteTo;
		for(unsigned int i = pos; i < alloc.size(); i++){
			double minSpam = getJobConflictMinSpam(alloc[i]->job);
			// cout << "MinSpam: " << minSpam << endl;
			// cin.get();
			alloc[i]->vms->pushJob(alloc[i]->job, alloc[i]->writeTo, minSpam);
		}
	}

	void printAlloc(){
		// cout << "Allocation order that created solution: " << endl;
		// for(unsigned int i = 0; i < alloc.size(); i++){
		// 	cout << "JobID: " << alloc[i]->job->name << " to MachineID: " << alloc[i]->vms->id << " Writing to MachineID: " << alloc[i]->writeTo << endl;
		// }
		for(unsigned int i = 0; i < alloc.size(); i++){
			cout << alloc[i]->job->name << ", ";
		}
		cout << endl;
	}

	Problem(const Problem &p){
		this->conflicts = p.conflicts;
		this->notParallelable = p.notParallelable;

		for(unsigned int i = 0; i < p.files.size(); i++){			
				Item * copiedItem = p.files[i];
				Item * newItem = new Item();
				newItem->id = copiedItem->id;
				newItem->name = copiedItem->name;
				newItem->size = copiedItem->size;
				newItem->alocated_vm_id = copiedItem->alocated_vm_id;
				newItem->static_vms = copiedItem->static_vms;
				newItem->is_static = copiedItem->is_static;
				newItem->IOTimeCost = copiedItem->IOTimeCost;
				newItem->VMsBandwidth = copiedItem->VMsBandwidth;
				this->files.push_back(newItem);
		}

		for(unsigned int i = 0; i < p.jobs.size(); i++){
			Job * copiedJob = p.jobs[i];
			Job * newJob = new Job();
			newJob->name = copiedJob->name;
			newJob->id = copiedJob->id;
			newJob->base_time = copiedJob->base_time;
			newJob->rootJob = copiedJob->rootJob;
			newJob->alocated = copiedJob->alocated;
			newJob->alocated_vm_id = copiedJob->alocated_vm_id;
			for(unsigned int input = 0; input < copiedJob->input.size(); input++){
				for(unsigned int item = 0; item < this->files.size(); item++){
					if(this->files[item]->id == copiedJob->input[input]->id){
						newJob->input.push_back(this->files[item]);
						break;
					}
				}
			}
			for(unsigned int output = 0; output < copiedJob->output.size(); output++){
				for(unsigned int item = 0; item < this->files.size(); item++){
					if(this->files[item]->id == copiedJob->output[output]->id){
						newJob->output.push_back(this->files[item]);
						break;
					}
				}
			}
			this->jobs.push_back(newJob);
		}

		for(unsigned int i = 0; i < p.vms.size(); i++){
			Machine * copiedMachine = p.vms[i];
			Machine * newMachine = new Machine();
			newMachine->name = copiedMachine->name;
			newMachine->id = copiedMachine->id;
			newMachine->slowdown = copiedMachine->slowdown;
			newMachine->storage = copiedMachine->storage;
			newMachine->cost = copiedMachine->cost;
			newMachine->bandwidth = copiedMachine->bandwidth;
			newMachine->timelineStartTime = copiedMachine->timelineStartTime;
			newMachine->timelineFinishTime = copiedMachine->timelineFinishTime;
			newMachine->makespam = copiedMachine->makespam;
			for(unsigned int job = 0; job < copiedMachine->timelineJobs.size(); job++){
				for(unsigned int pos = 0; pos < this->jobs.size(); pos++){
					if(this->jobs[pos]->id == copiedMachine->timelineJobs[job]->id){
						newMachine->timelineJobs.push_back(this->jobs[pos]);
						break;
					}
				}
			}
			this->vms.push_back(newMachine);
		}

		for(unsigned int i = 0; i < p.alloc.size(); i++){
			Allocation * copiedAllocation = p.alloc[i];
			Allocation * newAllocation = new Allocation();
			newAllocation->writeTo = copiedAllocation->writeTo;
			int copiedJobId = copiedAllocation->job->id;
			newAllocation->job = this->jobs[copiedJobId];
			int copiedVmsId = copiedAllocation->vms->id;
			newAllocation->vms = this->vms[copiedVmsId];
			this->alloc.push_back(newAllocation);
		}

	}

	double recreateSolution(int pos, double alpha){
		// cout << "recreateSolution" << endl;
		// cin.get();
		bool teste = true;
		
		// this->print();
		// this->printAlloc();
		// cout << "initial alloc size: " << this->alloc.size() << endl;

		for(int i = this->alloc.size() - 1; i >= pos; i--){
			teste = this->alloc[i]->vms->popJob(this->alloc[i]->job->id);
			if(!teste){
				cout << "NAO CONSEGUIU POP JOB!" << endl;
				cin.get();
			}
			delete this->alloc[i];
			this->alloc.pop_back();
		}
		// cout << "middle alloc size: " << this->alloc.size() << endl;
		int totalJobs = jobs.size() - pos;
		while(totalJobs > 0){
			vector<Job*> CL;
			vector<int> jobVmDestination;
			vector<int> outputVmDestination;
			for(unsigned int i = 0; i < jobs.size(); i++){
				// cout << "i: " << i  << " JobName: "<< jobs[i]->name << endl;
				if (jobs[i]->alocated)
					continue;
				for(unsigned int m = 0; m < vms.size(); m++){
					for(unsigned int d = 0; d < vms.size(); d++){

						double minSpam = getJobConflictMinSpam(jobs[i]);
						if(minSpam < 0) break; 
						bool pushed = vms[m]->pushJob(jobs[i], d, minSpam);
						// cin.get();
						if(!pushed){
							// cout << "COULD NOT PUSH!" << endl;
							// cin.get();
							break;
						}
						// if(jobs[i]->name == "ID00002"){
						// 	cout << "JOB: " << jobs[i]->id << " vmsID: " << vms[m]->name << " InsertionID: " << vms[d]->id <<  " COST: " << insertionCost << endl;
						// 	cin.get();
						// }
						CL.push_back(jobs[i]);
						jobVmDestination.push_back(m);
						outputVmDestination.push_back(d);
						// cout << "tested!" << endl;
						vms[m]->popJob(jobs[i]->id);
						// cout << "Spam After Removal: " << vms[m]->calculateLocalspam() << endl;
						// cin.get();
					}
				}
			}

			int chosenMovement;
			int maxClPos = (int)(CL.size() * alpha);
			if(maxClPos == 0)
				chosenMovement = 0;
			else
				chosenMovement = random() % maxClPos;
			bool moveDone = doMovement(jobVmDestination[chosenMovement], outputVmDestination[chosenMovement], CL[chosenMovement]);
			if(moveDone){
				// cout << "JobID: " << CL[chosenMovement]->id << " Was Inserted!" << endl;
				totalJobs--;
			}
			else{
				cout << "JobID: " << CL[chosenMovement]->id << " Was NOT Inserted!" << endl;
				cin.get();
			}
		}
		// cout << "final alloc size: " << this->alloc.size() << endl;
		// this->print();
		// this->printAlloc();
		// cin.get();
		return this->calculateMakespam();

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
			for(unsigned int i = 0; i < jobs.size(); i++){
				// cout << "i: " << i  << " JobName: "<< jobs[i]->name << endl;
				if (jobs[i]->alocated)
					continue;
				for(unsigned int m = 0; m < vms.size(); m++){
					for(unsigned int d = 0; d < vms.size(); d++){

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
						// if(jobs[i]->name == "ID00002"){
						// 	cout << "JOB: " << jobs[i]->id << " vmsID: " << vms[m]->name << " InsertionID: " << vms[d]->id <<  " COST: " << insertionCost << endl;
						// 	cin.get();
						// }
						if(CL.size() == 0){
							CL.push_back(jobs[i]);
							jobVmDestination.push_back(m);
							cost.push_back(insertionCost);
							outputVmDestination.push_back(d);
						} else{
							bool inserted = false;
							for(unsigned int j = 0; j < cost.size(); j++){
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

			
			if(CL.size() == 0){
				cout << "NO POSSIBLE MOVEMENTS!" << endl;
				cin.get();
			}

			// cout << "CL size: " << CL.size() << endl;
			// for(unsigned int c = 0; c < CL.size(); c++){
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
		// for(unsigned int i = 0; i < jobs.size();i++){
		// 	cout << jobs[i]->alocated_vm_id << ",";
		// }
		// cout << "]" << endl;
		// cout << "[";
		// for(unsigned int i = 0; i < jobs.size();i++){
		// 	cout << jobs[i]->alocated << ",";
		// }
		// cout << "]" << endl;
		// cin.get();
		// vector<int> vmUsage(vms.size());
		// for(unsigned int f = 0; f < files.size(); f++){
		// 	vmUsage[files[f]->alocated_vm_id] += files[f]->size;
		// }
		// for(unsigned int vm = 0; vm < vms.size(); vm++){
		// 	cout << "vmName: " << vms[vm]->name << " vmTotalSize: " << vms[vm]->storage << " usage: " << vmUsage[vm] << endl; 
		// }
		// cin.get();
		return calculateMakespam();
	}

	bool perturbateWriteTo(int pos){
		int newVM = rand() % vms.size();



		for(int i = this->alloc.size() - 1; i >= pos; i--){
			bool teste = this->alloc[i]->vms->popJob(this->alloc[i]->job->id);
			if(!teste){
				cout << "nao pop job!" << endl;
				cin.get();
			}
		}

		alloc[pos]->writeTo = vms[newVM]->id;


		for(int i = pos; i < this->alloc.size(); i++){
			bool teste = alloc[i]->vms->pushJob(alloc[i]->job, alloc[i]->writeTo, getJobConflictMinSpam(alloc[i]->job));
			if(!teste){
				cout << "nao push job!" << endl;
				cin.get();
			}
		}

		return true;
	}

	bool perturbateMachine(int pos){
		int newVM = rand() % vms.size();



		for(int i = this->alloc.size() - 1; i >= pos; i--){
			bool teste = this->alloc[i]->vms->popJob(this->alloc[i]->job->id);
			if(!teste){
				cout << "nao pop job!" << endl;
				cin.get();
			}
		}

		alloc[pos]->vms = vms[newVM];

		for(int i = pos; i < this->alloc.size(); i++){
			bool teste = alloc[i]->vms->pushJob(alloc[i]->job, alloc[i]->writeTo, getJobConflictMinSpam(alloc[i]->job));
			if(!teste){
				cout << "nao push job!" << endl;
				cin.get();
			}
		}

		return true;
	}

	bool doMovement(int vm, int output, Job* job){
		Allocation * newAlloc = new Allocation();
		newAlloc->job = job;
		newAlloc->vms = vms[vm];
		newAlloc->writeTo = output;
		alloc.push_back(newAlloc);
		return vms[vm]->pushJob(job, output, getJobConflictMinSpam(job));
	}

	void preSetStaticFile(Item * file, int vm_id){
		file->alocated_vm_id = vm_id;
	}

	void print(){
		cout << "VMs:" << endl;
		for(unsigned int i = 0; i < vms.size(); i++){
			cout << "ID: " << vms[i]->id << " Name:" << vms[i]->name << ": " ;
			for(unsigned int j = 0; j < vms[i]->timelineJobs.size(); j++){
				cout << vms[i]->timelineJobs[j]->name << "( " << vms[i]->timelineStartTime[j] << "," << vms[i]->timelineFinishTime[j] << " )" << " ";
			}
			cout << endl;
		}

		for(unsigned int i = 0; i < files.size(); i++){
			cout << "file: " << files[i]->id << " VM: " << files[i]->alocated_vm_id << endl;
		}
	}

	double calculateMakespam(){
		double makespam = 0.0;
		for(unsigned int i = 0; i < vms.size(); i++){
			double localspam = vms[i]->calculateLocalspam();
			if (localspam > makespam) makespam = localspam;
		}
		return makespam;
	}

	bool checkFeasible(){
		// for(unsigned int i = 0; i < files.size(); i++){  // checando se os arquivos static estao alocados nas máquinas possíveis.
		// 	if(files[i]->is_static){
		// 		bool feasible_alocation = false;
		// 		for(unsigned int m = 0; m < files[i]->static_vms.size(); m++){
		// 			if(files[i]->alocated_vm_id == files[i]->static_vms[m]){
		// 				feasible_alocation = true;
		// 				break;
		// 			}
		// 		}
		// 		if(!feasible_alocation){
		// 			cout << "Arquivo: " << files[i]->name << " nao alocado numa static_vms!" << endl;
		// 			return false;
		// 		}
		// 	}
		// }

		for(unsigned int i = 0; i < files.size(); i++){  // checando se todos os arquivos estao alocados a alguma maquina
			if(files[i]->alocated_vm_id < 0){
				cout << "Arquivo: " << i << " nao alocado!" << endl;
				return false;
			}
		}

		for(unsigned int i = 0; i < jobs.size(); i++){ // checando se todos os jobs foram alocados
			if(!jobs[i]->alocated){
				cout << "Job: " << i << " nao alocado!" << endl;
				return false;
			}
			if(jobs[i]->alocated_vm_id < 0){
				cout << "Job: " << i << " alocado, mas sem vm_id!" << endl;
				return false;
			}

			//checando se topologia foi respeitada
			for(unsigned int j = 0; j < jobs.size();j++){
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
		// cout << "JOBID: " << job->id << endl;
		int id = job->id;
		double minSpam = 0.0;
		for(unsigned int i = 0; i < conflicts[id].size(); i++){ // checando tempo minimo baseado na topologia até o arquivo.
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
		// for(unsigned int i = 0; i < notParallelable[id].size(); i++){ // checando tempo minimo baseado na possivel paralelização
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

		vector<string> order = {"ID00013", "ID00002", "ID00026", "ID00014", "ID00009", "ID00018", "ID00024", "ID00016", "ID00005", "ID00028", 
		"ID00011", "ID00022", "ID00023", "ID00012", "ID00025", "ID00015", "ID00020", "ID00027", "ID00029", "ID00006", "ID00019", "ID00010", 
		"ID00017", "ID00003", "ID00007", "ID00008", "ID00004", "ID00021", "ID00000", "ID00001"};

		vector<int> vm_order = {0, 0, 0, 2};


		Job * job;
		Machine * aux;
		double minSpam;

		job = this->getJobByName("ID00013");
		aux = vms[0];
		minSpam = getJobConflictMinSpam(job);
		aux->pushJob(job, -1, minSpam);

		job = this->getJobByName("ID00002");
		aux = vms[0];
		minSpam = getJobConflictMinSpam(job);
		aux->pushJob(job, -1, minSpam);

		job = this->getJobByName("ID00016");
		aux = vms[3];
		minSpam = getJobConflictMinSpam(job);
		aux->pushJob(job, -1, minSpam);

		job = this->getJobByName("ID00014");
		aux = vms[0];
		minSpam = getJobConflictMinSpam(job);
		aux->pushJob(job, -1, minSpam);

		job = this->getJobByName("ID00018");
		aux = vms[0];
		minSpam = getJobConflictMinSpam(job);
		aux->pushJob(job, -1, minSpam);

		job = this->getJobByName("ID00024");
		aux = vms[0];
		minSpam = getJobConflictMinSpam(job);
		aux->pushJob(job, -1, minSpam);

		job = this->getJobByName("ID00028");
		aux = vms[0];
		minSpam = getJobConflictMinSpam(job);
		aux->pushJob(job, -1, minSpam);

		job = this->getJobByName("");
		aux = vms[0];
		minSpam = getJobConflictMinSpam(job);
		aux->pushJob(job, -1, minSpam);

		job = this->getJobByName("");
		aux = vms[0];
		minSpam = getJobConflictMinSpam(job);
		aux->pushJob(job, -1, minSpam);

		job = this->getJobByName("");
		aux = vms[0];
		minSpam = getJobConflictMinSpam(job);
		aux->pushJob(job, -1, minSpam);

		job = this->getJobByName("");
		aux = vms[0];
		minSpam = getJobConflictMinSpam(job);
		aux->pushJob(job, -1, minSpam);

		job = this->getJobByName("");
		aux = vms[0];
		minSpam = getJobConflictMinSpam(job);
		aux->pushJob(job, -1, minSpam);

		job = this->getJobByName("");
		aux = vms[0];
		minSpam = getJobConflictMinSpam(job);
		aux->pushJob(job, -1, minSpam);

		job = this->getJobByName("");
		aux = vms[0];
		minSpam = getJobConflictMinSpam(job);
		aux->pushJob(job, -1, minSpam);

		job = this->getJobByName("");
		aux = vms[0];
		minSpam = getJobConflictMinSpam(job);
		aux->pushJob(job, -1, minSpam);

		job = this->getJobByName("");
		aux = vms[0];
		minSpam = getJobConflictMinSpam(job);
		aux->pushJob(job, -1, minSpam);

		job = this->getJobByName("");
		aux = vms[0];
		minSpam = getJobConflictMinSpam(job);
		aux->pushJob(job, -1, minSpam);

		job = this->getJobByName("");
		aux = vms[0];
		minSpam = getJobConflictMinSpam(job);
		aux->pushJob(job, -1, minSpam);

		job = this->getJobByName("");
		aux = vms[0];
		minSpam = getJobConflictMinSpam(job);
		aux->pushJob(job, -1, minSpam);

		job = this->getJobByName("");
		aux = vms[0];
		minSpam = getJobConflictMinSpam(job);
		aux->pushJob(job, -1, minSpam);

		job = this->getJobByName("");
		aux = vms[0];
		minSpam = getJobConflictMinSpam(job);
		aux->pushJob(job, -1, minSpam);

		job = this->getJobByName("");
		aux = vms[0];
		minSpam = getJobConflictMinSpam(job);
		aux->pushJob(job, -1, minSpam);

		job = this->getJobByName("");
		aux = vms[0];
		minSpam = getJobConflictMinSpam(job);
		aux->pushJob(job, -1, minSpam);

		job = this->getJobByName("");
		aux = vms[0];
		minSpam = getJobConflictMinSpam(job);
		aux->pushJob(job, -1, minSpam);

		job = this->getJobByName("");
		aux = vms[0];
		minSpam = getJobConflictMinSpam(job);
		aux->pushJob(job, -1, minSpam);

		job = this->getJobByName("");
		aux = vms[0];
		minSpam = getJobConflictMinSpam(job);
		aux->pushJob(job, -1, minSpam);

		job = this->getJobByName("");
		aux = vms[0];
		minSpam = getJobConflictMinSpam(job);
		aux->pushJob(job, -1, minSpam);

		job = this->getJobByName("");
		aux = vms[0];
		minSpam = getJobConflictMinSpam(job);
		aux->pushJob(job, -1, minSpam);

		job = this->getJobByName("");
		aux = vms[0];
		minSpam = getJobConflictMinSpam(job);
		aux->pushJob(job, -1, minSpam);

		job = this->getJobByName("");
		aux = vms[0];
		minSpam = getJobConflictMinSpam(job);
		aux->pushJob(job, -1, minSpam);


		double makespam = calculateMakespam();
		cout << "My Makespam: " << makespam << endl;
		this->print();
		return makespam;
		// cin.get();
	}

	Item * getFileByName(string name){
		for(unsigned int i = 0; i < this->files.size(); i++){
			// cout << "FileName: |" << this->files[i]->name << "|" << name << "|" << endl;
			// if(this->files[i]->name == name) cout << "EQUAL" << endl;
			if(this->files[i]->name == name)
				return this->files[i];
		}
		return NULL;
	}

	Job * getJobByName(string name){
		for(unsigned int i = 0; i < this->jobs.size(); i++){
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

		for(unsigned int i = 0; i < job_size; i++){
			vector<int> aux1(job_size);
			conflicts.push_back(aux1);
			vector<int> aux2(job_size);
			notParallelable.push_back(aux2);
		}

		int total_file = sfile_size + dfile_size;
		
		getline(in_file, line); //reading blank line

		//Reading items
		for(unsigned int i = 0; i < total_file; i++){
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
				for(unsigned int j = 3; j < 3 + n_static_vms; j++){
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
		// for(unsigned int i = 0; i < this->files.size();i++){
		// 	cout << this->files[i]->name << endl;
		// }
		getline(in_file, line); //reading blank line
		// cout << "!!!" << endl;

		for(unsigned int i = 0; i < job_size; i++){
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
			// cout << "!!!" << endl;
			for(unsigned int j = 0; j < n_input_files; j++){
				getline(in_file, line);
				Item * inputItem = getFileByName(line);
				// cout << line << endl;
				// cout << "name: " << inputItem->name << endl;
				if(!inputItem->is_static) total_not_roots++;
				input.push_back(inputItem);
			}
			if(total_not_roots == n_input_files)
				is_root_job = false;
			//reading output files
			for(unsigned int j = 0; j < n_output_files; j++){
				getline(in_file, line);
				Item * outputItem = getFileByName(line);
				output.push_back(outputItem);
			}

			Job * ajob = new Job(job_name, i, job_time, is_root_job, input, output);
			this->jobs.push_back(ajob);
		}
		// cout << "!!!" << endl;
		getline(in_file, line); //reading blank line

		// for(unsigned int i = 0; i < jobs.size(); i++){
		// 	cout << "NAME: " << jobs[i]->name << " ISROOT:" << jobs[i]->rootJob << " BASETIME: " << jobs[i]->base_time << " [";
		// 	for(unsigned int j = 0; j < jobs[i]->input.size(); j++){
		// 		cout << jobs[i]->input[j]->name << ", ";
		// 	}
		// 	cout << "] [";
		// 	for(unsigned int j = 0; j < jobs[i]->output.size(); j++){
		// 		cout << jobs[i]->output[j]->name << ", ";
		// 	}
		// 	cout << "]" << endl;
		// }
		// cin.get();

		// reading topology
		for(unsigned int i = 0; i < job_size; i++){
			getline(in_file, line);
			vector<string> strs;
			boost::split(strs, line, boost::is_any_of(" "));

			string job_name = strs[0];
			Job * aux = getJobByName(job_name);
			int father_id = aux->id;
			int children = stoi(strs[1]);
			for(unsigned int j = 0; j < children; j++){
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

		for(unsigned int i = 0; i < job_size-1; i++){
			for(unsigned int j = i+1; j < job_size; j++){
				Job * job1 = jobs[i];
				Job * job2 = jobs[j];
				for(unsigned int k = 0; k < total_file; k++){
					if(job1->checkInputNeed(files[k]->id) && job2->checkInputNeed(files[k]->id)){
						notParallelable[job1->id][job2->id] = 1;
						notParallelable[job2->id][job1->id] = 1;
					}
				}
			}
		}

		// for(unsigned int i = 0; i < job_size; i++){
		// 	for(unsigned int j = 0; j < job_size; j++){
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
		for(unsigned int i = 0; i < vm_size; i++){
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

		// for(unsigned int i = 0; i < this->vms.size(); i++){
		// 	cout << "NAME: " << this->vms[i]->name << endl;
		// }
		// cin.get();

		in_cfile.close();

		// pre-calculating READ/WRITE times for files

		for(unsigned int i = 0; i < this->files.size(); i++){
			this->files[i]->IOTimeCost = vector<double>(this->vms.size());
			for(unsigned int j = 0; j < this->vms.size(); j++){
				double IO = ceil(this->files[i]->size / this->vms[j]->bandwidth);
				int vm_id = this->vms[j]->id;
				this->files[i]->IOTimeCost[vm_id] = IO;
			}	
		}

		for(unsigned int i = 0; i < this->files.size(); i++){
			this->files[i]->VMsBandwidth = vector<double>(this->vms.size());
			for(unsigned int j = 0; j < this->vms.size(); j++){
				this->files[i]->VMsBandwidth[j] = this->vms[j]->bandwidth;
			}	
		}

		// for(unsigned int i = 0; i < this->files.size(); i++){
		// 	for(unsigned int j = 0; j < this->vms.size(); j++){
		// 		cout << this->files[i].IOTimeCost[j] << " ";
		// 	}
		// 	cout << endl;
		// }
		// cin.get();

	}

	~Problem() { 
		for(int i = 0; i < this->jobs.size(); i++)
			delete jobs[i];
		for(int i = 0; i < this->vms.size(); i++)
			delete vms[i];
		for(int i = 0; i < this->alloc.size(); i++)
			delete alloc[i];
		for(int i = 0; i < this->files.size(); i++)
			delete files[i];
	}
};

#endif