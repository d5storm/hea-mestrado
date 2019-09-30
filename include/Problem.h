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

double calculateWritetime(Job * job, int vmId, int write_vm_id){
	double writetime = 0.0;
	if(write_vm_id != -1 && write_vm_id != vmId){
		for(unsigned int i = 0; i < job->output.size(); i++){
			int minBandwidthVm = vmId;
			if(job->output[i]->VMsBandwidth[write_vm_id] < job->output[i]->VMsBandwidth[vmId])
				minBandwidthVm = write_vm_id;
			writetime += job->output[i]->IOTimeCost[minBandwidthVm];
		}
	}
	return writetime;
}

double calculateWritetimeWithChanges(Job * job, int vmId, vector<int> write_vm_ids){
	double writetime = 0.0;
	for(unsigned int i = 0; i < job->output.size(); i++){
		int write_vm_id = write_vm_ids[job->output[i]->id];
		if(write_vm_id != -1 && write_vm_id != vmId){
			int minBandwidthVm = vmId;
			if(job->output[i]->VMsBandwidth[write_vm_id] < job->output[i]->VMsBandwidth[vmId])
				minBandwidthVm = write_vm_id;
			writetime += job->output[i]->IOTimeCost[minBandwidthVm];
		}
	}

	return writetime;
}

double calculateReadtime(Job* job, int vmId){
	double readtime = 0.0; // calculando o tempo de leitura de todos os arquivos de input necessarios caso nao estejam alocados na Maquina
	for(unsigned int i = 0; i < job->input.size(); i++){
		int minBandwidthVm = vmId;

		if(job->input[i]->is_static){
			bool transferNeed = true;
			double maxBandwidth = 0.0;
			int id;
			for(int j = 0; j < job->input[i]->static_vms.size(); j++){
				if(vmId == job->input[i]->static_vms[j]){
					transferNeed = false;
					break;
				}
				if(job->input[i]->VMsBandwidth[job->input[i]->static_vms[j]] > maxBandwidth){
					maxBandwidth = job->input[i]->VMsBandwidth[job->input[i]->static_vms[j]];
					id = job->input[i]->static_vms[j];
				}
			}

			if(maxBandwidth < job->input[i]->VMsBandwidth[vmId])
				minBandwidthVm = id;
			if(transferNeed) readtime += job->input[i]->IOTimeCost[minBandwidthVm];
		} else{
			if(job->input[i]->alocated_vm_id == vmId)
				continue;
			if(job->input[i]->VMsBandwidth[job->input[i]->alocated_vm_id] < job->input[i]->VMsBandwidth[vmId])
				minBandwidthVm = job->input[i]->alocated_vm_id;
			readtime += job->input[i]->IOTimeCost[minBandwidthVm];			
		}
	}
	return readtime;
}


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

	bool popJobKeepAllocation(int jobId){
		for(unsigned int i = 0; i < timelineJobs.size(); i++){
			if (timelineJobs[i]->id == jobId){
				timelineJobs[i]->alocated = false;
				// for(unsigned int f = 0; f < timelineJobs[i]->output.size(); f++){
				// 	timelineJobs[i]->output[f]->alocated_vm_id = -1;
				// }
				// if(timelineJobs[i]->rootJob){
				// 	for(unsigned int f = 0; f < timelineJobs[i]->input.size(); f++){
				// 		if(timelineJobs[i]->input[f]->is_static)
				// 			timelineJobs[i]->input[f]->alocated_vm_id = -1;
				// 	}	
				// }
				timelineJobs.erase(timelineJobs.begin() + i);
				timelineStartTime.erase(timelineStartTime.begin() + i);
				timelineFinishTime.erase(timelineFinishTime.begin() + i);
				return true;
			}
		}
		return false;
	}

	bool pushJobKeepAllocation(Job * job, int write_vm_id, double minSpam, vector<int> allocations){
		for(unsigned int i = 0; i < job->input.size(); i++){ // checando se todos os arquivos de input que nao sao estaticos ja foram produzidos
			// cout << "inputFileID: " << job->input[i]->id << " isStatic: " << job->input[i]->is_static << endl;
			if(job->input[i]->is_static == false && job->input[i]->alocated_vm_id < 0){
				// cout << "JOB NOT AVAILABLE: FILES NOT READY" << endl; 
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
		
		
		
		double readtime = calculateReadtime(job, this->id);

		// cout << "ReadTime: " << readtime << endl;
		// cin.get();

		
		double writetime = calculateWritetimeWithChanges(job, this->id, allocations);

		double processtime = ceil(job->base_time * this->slowdown);
		double finishTime = readtime + writetime + processtime + startTime;

		// if(job->id == 0)
			// cout  << "JobId: " << job->id << " readtime: " << readtime << " processtime: " << processtime << " writetime: " << writetime << endl;
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
			// job->output[i]->alocated_vm_id = final_vm;
		}

		for(unsigned int i = 0; i < job->input.size(); i++){
			if(job->input[i]->is_static){
				if(job->input[i]->alocated_vm_id < 0)
					job->input[i]->alocated_vm_id = this->id;
			}
		}

		return true;
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
				// cout << "JOB NOT AVAILABLE: FILES NOT READY" << endl; 
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
		
		
		
		double readtime = calculateReadtime(job, this->id);

		// cout << "ReadTime: " << readtime << endl;
		// cin.get();

		
		double writetime = calculateWritetime(job, this->id, write_vm_id);

		double processtime = ceil(job->base_time * this->slowdown);
		double finishTime = readtime + writetime + processtime + startTime;

		// if(job->id == 0)
			// cout  << "JobId: " << job->id << " readtime: " << readtime << " processtime: " << processtime << " writetime: " << writetime << endl;
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

	double test_swapFileAllocation(){
		vector<double> newFinishTimes;
		vector<double> newStartTimes;
		vector<double> BestnewFinishTimes;
		vector<double> BestnewStartTimes;
		double originalCost = this->calculateMakespam();

		for(int f = 0; f < this->files.size(); f++){
			Item * file = files[f];
			// cout << "File: " << file->id << " is_Static: " << file->is_static << endl;
			if(file->is_static) continue;

			double bestMove = originalCost;
			int bestId = -1;
			for(unsigned int i = 0; i < this->vms.size(); i++){
				Machine * testVm = this->vms[i];
				// cout << "Tested VM: " << testVm->id << endl;

				if(testVm->id == file->alocated_vm_id) continue;
				
				newFinishTimes = vector<double>(this->jobs.size(), 0.0);
				newStartTimes = vector<double>(this->jobs.size(), 0.0);

				double newSpan = calculate_swapFileAllocation_Effect(file, testVm->id, newStartTimes, newFinishTimes);
				// cout << "JOBID: " << job->id << " i: " << i << " span: " << newSpan << " originalSpan: " << originalCost<< " readtime: " << readtime << " processtime: " << processtime << " writetime: " << writetime << endl;

				// cout << "originalCost: " << originalCost << " newSpan: " << newSpan << " machinePos: " << i << endl;
				// cin.get();
				if(newSpan < bestMove){
					// cout << "The new Span is: " << newSpan << " was: " << originalCost << endl;
					// cout << "File: " << file->id << " is_Static: " << file->is_static << " Tested VM: " << testVm->id << endl;
					// cin.get();
					bestMove = newSpan;
					bestId = testVm->id;
					BestnewFinishTimes = newFinishTimes;
					BestnewStartTimes = newStartTimes;
				}
			}
			if(bestId >= 0){
				// cin.get();
				return this->execSwapFileAllocation(file, bestId, BestnewStartTimes, BestnewFinishTimes);
			}
		}
		// cout << "Nao achou melhora!" << endl;
		return -1.0;
	}
	double calculate_swapFileAllocation_Effect(Item * file, int writeTo, vector<double>& newStartTimes, vector<double>& newFinishTimes){
		
		for(int a = 0; a < alloc.size(); a++){ // preenchendo inicio e fim original
			int jobId = alloc[a]->job->id;
			Machine * vm = alloc[a]->vms;
			int posOnVm = vm->jobPosOnTimeline(jobId);
			newStartTimes[jobId] = vm->timelineStartTime[posOnVm];
			newFinishTimes[jobId] = vm->timelineFinishTime[posOnVm];
		}

		vector<int> newAlocations = vector<int>(files.size(), -1);
		vector<int> oldAlocations = vector<int>(files.size(), -1);
		for(int f = 0; f < files.size(); f++){
			int id = files[f]->alocated_vm_id;
			oldAlocations[files[f]->id] = id;
			if(files[f]->id == file->id) id = writeTo;
			newAlocations[files[f]->id] = id;
		}
		// for(int i = 0; i < newStartTimes.size(); i++){
		// 	// if(jobs[i]->alocated_vm_id == job->alocated_vm_id)
		// 		cout << "Id: " << i << " Start: " << newStartTimes[i] << " Finish: " << newFinishTimes[i] << endl;
		// }

		double latestJobConflictFinish = 0.0;
		double latestJobVmFinish = 0.0;

		for(int a = 0; a < alloc.size(); a++){ // recalculando start e finish seguindo a ordem de allocation
			latestJobConflictFinish = 0.0;
			latestJobVmFinish = 0.0;
			int aPosOnVm = alloc[a]->vms->jobPosOnTimeline(alloc[a]->job->id);
			// double oldWriteTime = calculateWritetimeWithChanges(alloc[a]->job, alloc[a]->vms->id, oldAlocations);
			// double oldReadTime = calculateReadtime(alloc[a]->job, alloc[a]->vms->id);
			// double execTime = alloc[a]->vms->timelineFinishTime[aPosOnVm] - alloc[a]->vms->timelineStartTime[aPosOnVm] - oldReadTime - oldWriteTime;
			double execTime = ceil(alloc[a]->vms->slowdown * alloc[a]->job->base_time);
			// cout << "********* calculated execTime: " << execTime << endl;
			double readTime = 0.0;

			for(int f = 0; f < alloc[a]->job->input.size(); f++){
				int minBandwidthVm = alloc[a]->vms->id;
				if(alloc[a]->job->input[f]->is_static){
					bool transferNeed = true;
					double maxBandwidth = 0.0;
					int id;
					for(int j = 0; j < alloc[a]->job->input[f]->static_vms.size(); j++){
						if(alloc[a]->vms->id == alloc[a]->job->input[f]->static_vms[j]){
							transferNeed = false;
							break;
						}
						if(alloc[a]->job->input[f]->VMsBandwidth[alloc[a]->job->input[f]->static_vms[j]] > maxBandwidth){
							maxBandwidth = alloc[a]->job->input[f]->VMsBandwidth[alloc[a]->job->input[f]->static_vms[j]];
							id = alloc[a]->job->input[f]->static_vms[j];
						}
					}

					if(maxBandwidth < alloc[a]->job->input[f]->VMsBandwidth[alloc[a]->vms->id])
						minBandwidthVm = id;
					if(transferNeed) readTime += alloc[a]->job->input[f]->IOTimeCost[minBandwidthVm];
				} else{
					// cout << "Not Static!" << endl;
					Item * auxFile = alloc[a]->job->input[f];
					int origin = auxFile->alocated_vm_id;
					
					if(file->id == auxFile->id) origin = writeTo;

					if(origin == alloc[a]->vms->id) continue;

					if(alloc[a]->job->input[f]->VMsBandwidth[origin] < alloc[a]->job->input[f]->VMsBandwidth[minBandwidthVm])
						minBandwidthVm = origin;
					// cout << "ReadTime += " <<  alloc[a]->job->input[f]->IOTimeCost[minBandwidthVm] << endl;
					readTime += alloc[a]->job->input[f]->IOTimeCost[minBandwidthVm];
				}
			}

			double writeTime = calculateWritetimeWithChanges(alloc[a]->job, alloc[a]->vms->id, newAlocations);

			// cout << "execTime: " << execTime + calculateReadtime(alloc[a]->job, alloc[a]->vms->id) << " oldRead: " << calculateReadtime(alloc[a]->job, alloc[a]->vms->id);			

			execTime += readTime + writeTime;

			// cout << "FileID: " << file->id << " newWriteTo: " << writeTo << " newRead: " << readTime << " oldRead: " << oldReadTime <<  " newWrite: " << writeTime << " oldWrite: " << oldWriteTime <<" newExec: " << execTime << endl;


			for(int b = 0; b < a; b++){
				int bPosOnVm = alloc[b]->vms->jobPosOnTimeline(alloc[b]->job->id);
				int bVmId = alloc[b]->vms->id;
				if(conflicts[alloc[a]->job->id][alloc[b]->job->id] == 0){ // nao tem conflito
					if(alloc[a]->vms->id == bVmId){ // esta na mesma maquina
						if(newFinishTimes[alloc[b]->job->id] > latestJobVmFinish) // terminou mais tarde do que o ultimo da mesma vm
							latestJobVmFinish = newFinishTimes[alloc[b]->job->id];
					}
				} else { // tem conflito
					if(newFinishTimes[alloc[b]->job->id] > latestJobConflictFinish) // terminou mais tarde do que o ultimo que tenha conflito
						latestJobConflictFinish = newFinishTimes[alloc[b]->job->id];
				}
			}
			newStartTimes[alloc[a]->job->id] = max(latestJobVmFinish, latestJobConflictFinish); // tempo de comeco
			newFinishTimes[alloc[a]->job->id] = newStartTimes[alloc[a]->job->id] + execTime; // tempo de fim
		}


		// cout << "times after recalculation" << endl;
		// for(int i = 0; i < newStartTimes.size(); i++){
		// 	// if(jobs[i]->alocated_vm_id == job->alocated_vm_id)
		// 		cout << "Id: " << i << " Start: " << newStartTimes[i] << " Finish: " << newFinishTimes[i] << endl;
		// }
		// cin.get();

		double biggestSpan = 0.0;
		for(int i = 0; i < newFinishTimes[i]; i++){
			if(newFinishTimes[i] > biggestSpan)
				biggestSpan = newFinishTimes[i];
		}

		return biggestSpan;
	}

	double execSwapFileAllocation(Item * file, int writeTo, vector<double>& newStartTimes, vector<double>& newFinishTimes){
		file->alocated_vm_id = writeTo;

		for(int a = 0; a < this->alloc.size(); a++){
			Job * job = this->alloc[a]->job;
			Machine * vm = this->alloc[a]->vms;
			int posOnTimeline = vm->jobPosOnTimeline(job->id);

			vm->timelineFinishTime[posOnTimeline] = newFinishTimes[job->id];
			vm->timelineStartTime[posOnTimeline] = newStartTimes[job->id];
		}
		// cout << "^^^^" << endl;
		// this->print();
		// cin.get();

		for(int vm = 0; vm < vms.size(); vm++){
			this->fixMachineTimelineOrder(vm);
		}

		return this->calculateMakespam();
	}
	

	double test_swapMachinePair(){
		vector<double> newFinishTimes;
		vector<double> newStartTimes;

		double originalCost = this->calculateMakespam();

		vector<int> allocations = vector<int>(files.size(), -1);
		for(int f = 0; f < files.size(); f++){
			int id = files[f]->alocated_vm_id;
			allocations[files[f]->id] = id;
		}

		for(int pos = 0; pos < this->alloc.size(); pos++){ // SWAP MACHINE LOOP START
			Allocation * swap = this->alloc[pos];
			Job * job = swap->job;
			Machine * originalAllocationMachine = swap->vms;

			int posOnTimeline = originalAllocationMachine->jobPosOnTimeline(job->id);
			double oldExecTime = originalAllocationMachine->timelineFinishTime[posOnTimeline] - originalAllocationMachine->timelineStartTime[posOnTimeline];

			int write_vm_id = swap->writeTo;

			for(int pos2 = pos + 1; pos2 < this->alloc.size(); pos2++){
				
				Allocation * swap2 = this->alloc[pos2];
				Job * job2 = swap2->job;
				Machine * originalAllocationMachine2 = swap2->vms;

				if(originalAllocationMachine2->id == originalAllocationMachine->id) continue;
				// cout << "JobID: " << job->id << " Job2ID: " << job2->id << endl;

				int posOnTimeline2 = originalAllocationMachine2->jobPosOnTimeline(job2->id);
				double oldExecTime2 = originalAllocationMachine2->timelineFinishTime[posOnTimeline2] - originalAllocationMachine2->timelineStartTime[posOnTimeline2];

				int write_vm_id2 = swap2->writeTo;

				// times for pos job
				Machine * testVm = swap2->vms;
				double readtime = calculateReadtime(job, testVm->id);

				double writetime = calculateWritetimeWithChanges(job, testVm->id, allocations);
				// double writetime = calculateWritetime(job, testVm->id, write_vm_id);

				double processtime = ceil(job->base_time * testVm->slowdown);

				double newTime = readtime + processtime + writetime;

				// times for pos2 job
				testVm = swap->vms;
				readtime = calculateReadtime(job2, testVm->id);

				writetime = calculateWritetimeWithChanges(job2, testVm->id, allocations);

				// writetime = calculateWritetime(job, testVm->id, write_vm_id2);

				processtime = ceil(job2->base_time * testVm->slowdown);

				double newTime2 = readtime + processtime + writetime;


				newFinishTimes = vector<double>(this->jobs.size(), 0.0);
				newStartTimes = vector<double>(this->jobs.size(), 0.0);
				double newSpan = calculate_swapMachinePair_effect(pos, pos2, job, job2, newTime, newTime2, newStartTimes, newFinishTimes);
				// cout << "JOBID: " << job->id << " i: " << i << " span: " << newSpan << " originalSpan: " << originalCost<< " readtime: " << readtime << " processtime: " << processtime << " writetime: " << writetime << endl;

				// cout << "originalCost: " << originalCost << " newSpan: " << newSpan << " oldExecTime: " << oldExecTime << " newTime: " << newTime << " machinePos: " << i << endl;
				// cin.get();
				if(newSpan < originalCost){
					// cout << "The new Span is: " << originalCost - oldExecTime + newTime << endl;
					
					// cin.get();
					// this->print();
					// this->printAlloc();
					// cout << "#################" << endl;
					// cout << "JobID: " << job->id << " Job2ID: " << job2->id << endl;
					
					return this->execSwapMachinePair(pos, pos2, newStartTimes, newFinishTimes);
					// if(job->id == 8 && job2->id == 13 || job2->id == 8 && job->id == 13) {
					// 	this->print();
					// 	this->printAlloc();
					// 	cin.get();
					// }
				}
				
			}
		}
		return -1;
	}

	void fixMachineTimelineOrder(int vmId){
		Machine * vm = vms[vmId];
		int moves = 9999;
		bool moved = true;
		// cout << "Before fix!" << endl;
		// this->print();
		// cout << "$$$$$$$$$$$$$4" << endl;
		while(moved){
			if(moves < 0){
				cout << "LOOP" << endl;
			}
			moved = false;
			for(int j = 1; j < vm->timelineJobs.size(); j++){
				if(vm->timelineStartTime[j] < vm->timelineFinishTime[j - 1]){ // ordem errada na timeline!
					moves--;
					moved = true;
					Job * job = vm->timelineJobs[j];
					double startTime = vm->timelineStartTime[j];
					double finishTime = vm->timelineFinishTime[j];
					
					// cout << "Ordem Errada!" << endl;

					vm->timelineJobs.erase(vm->timelineJobs.begin() + j);
					vm->timelineStartTime.erase(vm->timelineStartTime.begin() + j);
					vm->timelineFinishTime.erase(vm->timelineFinishTime.begin() + j);
					bool found = false;
					for(int k = 0; k < vm->timelineJobs.size(); k++){
						if(vm->timelineStartTime[k] >= finishTime){
							found = true;
							vm->timelineJobs.insert(vm->timelineJobs.begin() + k, job);
							vm->timelineStartTime.insert(vm->timelineStartTime.begin() + k, startTime);
							vm->timelineFinishTime.insert(vm->timelineFinishTime.begin() + k, finishTime);
							break;
						}
					}
					if (!found){
						vm->timelineJobs.push_back(job);
						vm->timelineStartTime.push_back(startTime);
						vm->timelineFinishTime.push_back(finishTime);
					}
					break;
				}
			}
			if(moved){
				// this->print();
				// cin.get();
			}
		}
	}

	double execSwapMachinePair(int pos, int pos2, vector<double>& newStartTimes, vector<double>& newFinishTimes){
		Job * changedJob = this->alloc[pos]->job;
		Machine * changedVm = this->alloc[pos2]->vms;
		Job * changedJob2 = this->alloc[pos2]->job;
		Machine * changedVm2 = this->alloc[pos]->vms;
		for(int a = 0; a < this->alloc.size(); a++){
			Job * job = this->alloc[a]->job;
			Machine * vm = this->alloc[a]->vms;
			int posOnTimeline = vm->jobPosOnTimeline(job->id);

			if(job->id == changedJob->id){
				// cout << "Is Changed Job!" << endl;
				job->alocated_vm_id = changedVm->id;
				vm->timelineJobs.erase(vm->timelineJobs.begin() + posOnTimeline);
				vm->timelineStartTime.erase(vm->timelineStartTime.begin() + posOnTimeline);
				vm->timelineFinishTime.erase(vm->timelineFinishTime.begin() + posOnTimeline);
				// cout << "Erased" << endl;

				// this->print();
				// cin.get();

				bool inserted = false;

				// cout << "Changed VM ID: " << changedVm->id << endl;
				for(int j = 0; j < changedVm->timelineJobs.size(); j++){
					// cout << "j: " << j << endl;
					int jId = changedVm->timelineJobs[j]->id;
					if(newStartTimes[jId] >= newFinishTimes[job->id]){
						// cout << "Found possition: " << j << endl;
						changedVm->timelineJobs.insert(changedVm->timelineJobs.begin() + j, job);
						changedVm->timelineStartTime.insert(changedVm->timelineStartTime.begin() + j, newStartTimes[job->id]);
						changedVm->timelineFinishTime.insert(changedVm->timelineFinishTime.begin() + j, newFinishTimes[job->id]);
						inserted=true;
						break;
					}
				}
				if(!inserted){
					changedVm->timelineJobs.push_back(job);
					changedVm->timelineStartTime.push_back(newStartTimes[job->id]);
					changedVm->timelineFinishTime.push_back(newFinishTimes[job->id]);
				}
			

			} else if(job->id == changedJob2->id){
				job->alocated_vm_id = changedVm2->id;
				vm->timelineJobs.erase(vm->timelineJobs.begin() + posOnTimeline);
				vm->timelineStartTime.erase(vm->timelineStartTime.begin() + posOnTimeline);
				vm->timelineFinishTime.erase(vm->timelineFinishTime.begin() + posOnTimeline);
				// cout << "Erased" << endl;

				// this->print();
				// cin.get();

				bool inserted = false;

				// cout << "Changed VM ID: " << changedVm->id << endl;
				for(int j = 0; j < changedVm2->timelineJobs.size(); j++){
					// cout << "j: " << j << endl;
					int jId = changedVm2->timelineJobs[j]->id;
					if(newStartTimes[jId] >= newFinishTimes[job->id]){
						// cout << "Found possition: " << j << endl;
						changedVm2->timelineJobs.insert(changedVm2->timelineJobs.begin() + j, job);
						changedVm2->timelineStartTime.insert(changedVm2->timelineStartTime.begin() + j, newStartTimes[job->id]);
						changedVm2->timelineFinishTime.insert(changedVm2->timelineFinishTime.begin() + j, newFinishTimes[job->id]);
						inserted=true;
						break;
					}
				}
				if(!inserted){
					changedVm2->timelineJobs.push_back(job);
					changedVm2->timelineStartTime.push_back(newStartTimes[job->id]);
					changedVm2->timelineFinishTime.push_back(newFinishTimes[job->id]);
				}
			
			} else{
				// cout << "Regular Job" << endl;
				vm->timelineFinishTime[posOnTimeline] = newFinishTimes[job->id];
				vm->timelineStartTime[posOnTimeline] = newStartTimes[job->id];
			}
		}

		this->alloc[pos]->vms = changedVm;
		this->alloc[pos2]->vms = changedVm2;

		// fixing order on each VM
		// cout << "Fixing order on VMs" << endl;
		for(int vm = 0; vm < vms.size(); vm++){
			this->fixMachineTimelineOrder(vm);
		}

		// this->calculateMakespam();
		// this->print();

		// this->checkFeasible();
		// cin.get();

		return this->calculateMakespam();
	}

	double calculate_swapMachinePair_effect(int allocPos, int allocPos2, Job* job, Job* job2, double newTime, double newTime2, vector<double> &newStartTimes, vector<double> &newFinishTimes){
		for(int a = 0; a < alloc.size(); a++){ // preenchendo inicio e fim original
			int jobId = alloc[a]->job->id;
			Machine * vm = alloc[a]->vms;
			int posOnVm = vm->jobPosOnTimeline(jobId);
			newStartTimes[jobId] = vm->timelineStartTime[posOnVm];
			newFinishTimes[jobId] = vm->timelineFinishTime[posOnVm];
		}

		int vmId = alloc[allocPos2]->vms->id;
		int vmId2 = alloc[allocPos]->vms->id;

		// procurar latest job na vmId e latest job que tenha conflito 
		double latestJobVmFinish =  0.0;
		double latestJobConflictFinish = 0.0;
		for(int a = 0; a < allocPos; a++){
			if(conflicts[job->id][alloc[a]->job->id] == 0){ // nao tem conflito
				if(alloc[a]->vms->id == vmId){ // esta na mesma maquina
					if(newFinishTimes[alloc[a]->job->id] > latestJobVmFinish) // terminou mais tarde do que o ultimo da mesma vm
						latestJobVmFinish = newFinishTimes[alloc[a]->job->id];
				}
			} else{ // tem conflito
				if(newFinishTimes[alloc[a]->job->id] > latestJobConflictFinish) // terminou mais tarde do que o ultimo que tenha conflito
					latestJobConflictFinish = newFinishTimes[alloc[a]->job->id];
			}
		}
		newStartTimes[alloc[allocPos]->job->id] = max(latestJobVmFinish, latestJobConflictFinish); // tempo de comeco
		newFinishTimes[alloc[allocPos]->job->id] = newStartTimes[alloc[allocPos]->job->id] + newTime; // tempo de fim

		// printAlloc();

		// cout << "######################" << endl;
		// for(int i = 0; i < newStartTimes.size(); i++){
		// 	// if(jobs[i]->alocated_vm_id == job->alocated_vm_id)
		// 		cout << "Id: " << i << " Start: " << newStartTimes[i] << " Finish: " << newFinishTimes[i] << endl;
		// }


		for(int a = allocPos + 1; a < alloc.size(); a++){ // recalculando start e finish seguindo a ordem de allocation
			if(a == allocPos2){
				latestJobVmFinish =  0.0;
				latestJobConflictFinish = 0.0;
				for(int b = 0; b < allocPos2; b++){
					if(conflicts[job2->id][alloc[b]->job->id] == 0){ // nao tem conflito
						int id = alloc[b]->vms->id;
						if (a == allocPos) id = vmId;
						if(id == vmId2){ // esta na mesma maquina
							// cout << "Mesma Maquina! ID: " << alloc[a]->job->id << endl;
							if(newFinishTimes[alloc[b]->job->id] > latestJobVmFinish) // terminou mais tarde do que o ultimo da mesma vm
								latestJobVmFinish = newFinishTimes[alloc[b]->job->id];
						}
					} else{ // tem conflito
						if(newFinishTimes[alloc[b]->job->id] > latestJobConflictFinish) // terminou mais tarde do que o ultimo que tenha conflito
							latestJobConflictFinish = newFinishTimes[alloc[b]->job->id];
					}
				}
				newStartTimes[alloc[allocPos2]->job->id] = max(latestJobVmFinish, latestJobConflictFinish); // tempo de comeco
				newFinishTimes[alloc[allocPos2]->job->id] = newStartTimes[alloc[allocPos2]->job->id] + newTime2; // tempo de fim
			}
			else{
				latestJobConflictFinish = 0.0;
				latestJobVmFinish = 0.0;
				int aPosOnVm = alloc[a]->vms->jobPosOnTimeline(alloc[a]->job->id);
				
				double execTime = alloc[a]->vms->timelineFinishTime[aPosOnVm] - alloc[a]->vms->timelineStartTime[aPosOnVm];
				for(int b = 0; b < a; b++){
					int bPosOnVm = alloc[b]->vms->jobPosOnTimeline(alloc[b]->job->id);
					int bVmId = alloc[b]->vms->id;
					if(b == allocPos){ // eh o modificado
						bVmId = vmId;
					} else if(b == allocPos2){
						bVmId = vmId2;
					}
					if(conflicts[alloc[a]->job->id][alloc[b]->job->id] == 0){ // nao tem conflito
						if(alloc[a]->vms->id == bVmId){ // esta na mesma maquina
							if(newFinishTimes[alloc[b]->job->id] > latestJobVmFinish) // terminou mais tarde do que o ultimo da mesma vm
								latestJobVmFinish = newFinishTimes[alloc[b]->job->id];
						}
					} else { // tem conflito
						if(newFinishTimes[alloc[b]->job->id] > latestJobConflictFinish) // terminou mais tarde do que o ultimo que tenha conflito
							latestJobConflictFinish = newFinishTimes[alloc[b]->job->id];
					}
				}
				newStartTimes[alloc[a]->job->id] = max(latestJobVmFinish, latestJobConflictFinish); // tempo de comeco
				newFinishTimes[alloc[a]->job->id] = newStartTimes[alloc[a]->job->id] + execTime; // tempo de fim
			}
		}


		// cout << "**************************" << endl;
		// for(int i = 0; i < newStartTimes.size(); i++){
		// 	// if(jobs[i]->alocated_vm_id == job->alocated_vm_id)
		// 		cout << "Id: " << i << " Start: " << newStartTimes[i] << " Finish: " << newFinishTimes[i] << endl;
		// }
		// cin.get();

		double biggestSpan = 0.0;
		for(int i = 0; i < newFinishTimes[i]; i++){
			if(newFinishTimes[i] > biggestSpan)
				biggestSpan = newFinishTimes[i];
		}

		return biggestSpan;
	}

	double calculate_swapMachine_effect(int allocPos, Job* job, double newTime, int vmId, vector<double> &newStartTimes, vector<double> &newFinishTimes){
		// print();
		// cout << "newTime: " << newTime  << " vmId: " << vmId << endl;
		
		for(int a = 0; a < alloc.size(); a++){ // preenchendo inicio e fim original
			int jobId = alloc[a]->job->id;
			Machine * vm = alloc[a]->vms;
			int posOnVm = vm->jobPosOnTimeline(jobId);
			newStartTimes[jobId] = vm->timelineStartTime[posOnVm];
			newFinishTimes[jobId] = vm->timelineFinishTime[posOnVm];
		}

		// for(int i = 0; i < newStartTimes.size(); i++){
		// 	if(jobs[i]->alocated_vm_id == job->alocated_vm_id)
		// 		cout << "Id: " << i << " Start: " << newStartTimes[i] << " Finish: " << newFinishTimes[i] << endl;
		// }
		// cin.get();

		// procurar latest job na vmId e latest job que tenha conflito 
		double latestJobVmFinish =  0.0;
		double latestJobConflictFinish = 0.0;
		for(int a = 0; a < allocPos; a++){
			if(conflicts[job->id][alloc[a]->job->id] == 0){ // nao tem conflito
				if(alloc[a]->vms->id == vmId){ // esta na mesma maquina
					if(newFinishTimes[alloc[a]->job->id] > latestJobVmFinish) // terminou mais tarde do que o ultimo da mesma vm
						latestJobVmFinish = newFinishTimes[alloc[a]->job->id];
				}
			} else{ // tem conflito
				if(newFinishTimes[alloc[a]->job->id] > latestJobConflictFinish) // terminou mais tarde do que o ultimo que tenha conflito
					latestJobConflictFinish = newFinishTimes[alloc[a]->job->id];
			}
		}
		newStartTimes[alloc[allocPos]->job->id] = max(latestJobVmFinish, latestJobConflictFinish); // tempo de comeco
		newFinishTimes[alloc[allocPos]->job->id] = newStartTimes[alloc[allocPos]->job->id] + newTime; // tempo de fim


		for(int a = allocPos + 1; a < alloc.size(); a++){ // recalculando start e finish seguindo a ordem de allocation
			latestJobConflictFinish = 0.0;
			latestJobVmFinish = 0.0;
			int aPosOnVm = alloc[a]->vms->jobPosOnTimeline(alloc[a]->job->id);

			double execTime = alloc[a]->vms->timelineFinishTime[aPosOnVm] - alloc[a]->vms->timelineStartTime[aPosOnVm];
			for(int b = 0; b < a; b++){
				int bPosOnVm = alloc[b]->vms->jobPosOnTimeline(alloc[b]->job->id);
				int bVmId = alloc[b]->vms->id;
				if(b == allocPos){ // eh o modificado
					bVmId = vmId;
				} 
				if(conflicts[alloc[a]->job->id][alloc[b]->job->id] == 0){ // nao tem conflito
					if(alloc[a]->vms->id == bVmId){ // esta na mesma maquina
						if(newFinishTimes[alloc[b]->job->id] > latestJobVmFinish) // terminou mais tarde do que o ultimo da mesma vm
							latestJobVmFinish = newFinishTimes[alloc[b]->job->id];
					}
				} else { // tem conflito
					if(newFinishTimes[alloc[b]->job->id] > latestJobConflictFinish) // terminou mais tarde do que o ultimo que tenha conflito
						latestJobConflictFinish = newFinishTimes[alloc[b]->job->id];
				}
			}
			newStartTimes[alloc[a]->job->id] = max(latestJobVmFinish, latestJobConflictFinish); // tempo de comeco
			newFinishTimes[alloc[a]->job->id] = newStartTimes[alloc[a]->job->id] + execTime; // tempo de fim
		}



		// for(int i = 0; i < newStartTimes.size(); i++){
		// 	// if(jobs[i]->alocated_vm_id == job->alocated_vm_id)
		// 		cout << "Id: " << i << " Start: " << newStartTimes[i] << " Finish: " << newFinishTimes[i] << endl;
		// }
		// cin.get();

		double biggestSpan = 0.0;
		for(int i = 0; i < newFinishTimes[i]; i++){
			if(newFinishTimes[i] > biggestSpan)
				biggestSpan = newFinishTimes[i];
		}

		return biggestSpan;
	}

	double test_swapMachine(){

		vector<double> newFinishTimes;
		vector<double> newStartTimes;
		double originalCost = this->calculateMakespam();

		vector<int> allocations = vector<int>(files.size(), -1);
		for(int f = 0; f < files.size(); f++){
			int id = files[f]->alocated_vm_id;
			allocations[files[f]->id] = id;
		}
		for(int pos = 0; pos < this->alloc.size(); pos++){ // SWAP MACHINE LOOP START
		
			Allocation * swap = this->alloc[pos];
			Job * job = swap->job;
			Machine * originalAllocationMachine = swap->vms;

			// cout << "JobID: " << job->id << endl;

			// int posOnTimeline = originalAllocationMachine->jobPosOnTimeline(job->id);
			// double oldExecTime = originalAllocationMachine->timelineFinishTime[posOnTimeline] - originalAllocationMachine->timelineStartTime[posOnTimeline];

			// cout << "WRITE_TO: " << write_vm_id << endl;

			for(unsigned int i = 0; i < this->vms.size(); i++){
				Machine * testVm = this->vms[i];
				if(testVm->id == originalAllocationMachine->id) continue;
				double readtime = calculateReadtime(job, testVm->id);

				double writetime = calculateWritetimeWithChanges(job, testVm->id, allocations);
				// double writetime = calculateWritetime(job, testVm->id, alloc[pos]->writeTo);

				double processtime = ceil(job->base_time * testVm->slowdown);

				double newTime = readtime + processtime + writetime;

				
				
				// cout << "Calculating cascade effect" << endl;
				newFinishTimes = vector<double>(this->jobs.size(), 0.0);
				newStartTimes = vector<double>(this->jobs.size(), 0.0);
				double newSpan = calculate_swapMachine_effect(pos, job, newTime, testVm->id, newStartTimes, newFinishTimes);
				// cout << "JOBID: " << job->id << " i: " << i << " span: " << newSpan << " originalSpan: " << originalCost<< " readtime: " << readtime << " processtime: " << processtime << " writetime: " << writetime << endl;

				// cout << "originalCost: " << originalCost << " newSpan: " << newSpan << " oldExecTime: " << oldExecTime << " newTime: " << newTime << " machinePos: " << i << endl;
				// cin.get();
				if(newSpan < originalCost){
					// cout << "The new Span is: " << originalCost - oldExecTime + newTime << endl;
					
					// cin.get();
					return this->execSwapMachine(pos, i, newStartTimes, newFinishTimes);
				}
			}
		}
		return -1;

	}

	double execSwapMachine(int pos, int vmId, vector<double>& newStartTimes, vector<double>& newFinishTimes){
		Job * changedJob = this->alloc[pos]->job;
		Machine * changedVm = this->vms[vmId];

		for(int a = 0; a < this->alloc.size(); a++){
			Job * job = this->alloc[a]->job;
			Machine * vm = this->alloc[a]->vms;
			int posOnTimeline = vm->jobPosOnTimeline(job->id);

			if(job->id == changedJob->id){
				// cout << "Is Changed Job!" << endl;
				job->alocated_vm_id = vmId;
				vm->timelineJobs.erase(vm->timelineJobs.begin() + posOnTimeline);
				vm->timelineStartTime.erase(vm->timelineStartTime.begin() + posOnTimeline);
				vm->timelineFinishTime.erase(vm->timelineFinishTime.begin() + posOnTimeline);
				// cout << "Erased" << endl;

				// this->print();
				// cin.get();

				bool inserted = false;

				// cout << "Changed VM ID: " << changedVm->id << endl;
				for(int j = 0; j < changedVm->timelineJobs.size(); j++){
					// cout << "j: " << j << endl;
					int jId = changedVm->timelineJobs[j]->id;
					if(newStartTimes[jId] >= newFinishTimes[job->id]){
						// cout << "Found possition: " << j << endl;
						changedVm->timelineJobs.insert(changedVm->timelineJobs.begin() + j, job);
						changedVm->timelineStartTime.insert(changedVm->timelineStartTime.begin() + j, newStartTimes[job->id]);
						changedVm->timelineFinishTime.insert(changedVm->timelineFinishTime.begin() + j, newFinishTimes[job->id]);
						inserted=true;
						break;
					}
				}
				if(!inserted){
					changedVm->timelineJobs.push_back(job);
					changedVm->timelineStartTime.push_back(newStartTimes[job->id]);
					changedVm->timelineFinishTime.push_back(newFinishTimes[job->id]);
				}

			} else{
				// cout << "Regular Job" << endl;
				vm->timelineFinishTime[posOnTimeline] = newFinishTimes[job->id];
				vm->timelineStartTime[posOnTimeline] = newStartTimes[job->id];
			}
		}

		this->alloc[pos]->vms = changedVm;

		for(int vm = 0; vm < vms.size(); vm++){
			this->fixMachineTimelineOrder(vm);
		}
		// this->calculateMakespam();
		// this->print();

		// this->checkFeasible();
		// cin.get();

		return this->calculateMakespam();
	}


	double calculate_swapMachineWrite_effect(int allocPos, Job* job, double newTime, vector<int>& output, int vmId, vector<double> &newStartTimes, vector<double> &newFinishTimes){
		// print();
		// cout << "newTime: " << newTime  << " vmId: " << vmId << endl;
		
		for(int a = 0; a < alloc.size(); a++){ // preenchendo inicio e fim original
			int jobId = alloc[a]->job->id;
			Machine * vm = alloc[a]->vms;
			int posOnVm = vm->jobPosOnTimeline(jobId);
			newStartTimes[jobId] = vm->timelineStartTime[posOnVm];
			newFinishTimes[jobId] = vm->timelineFinishTime[posOnVm];
		}

		// for(int i = 0; i < newStartTimes.size(); i++){
		// 	if(jobs[i]->alocated_vm_id == job->alocated_vm_id)
		// 		cout << "Id: " << i << " Start: " << newStartTimes[i] << " Finish: " << newFinishTimes[i] << endl;
		// }
		// cin.get();

		newFinishTimes[alloc[allocPos]->job->id] = newStartTimes[alloc[allocPos]->job->id] + newTime; // tempo de fim

		double latestJobConflictFinish = 0.0;
		double latestJobVmFinish = 0.0;

		for(int a = allocPos + 1; a < alloc.size(); a++){ // recalculando start e finish seguindo a ordem de allocation
			latestJobConflictFinish = 0.0;
			latestJobVmFinish = 0.0;
			int aPosOnVm = alloc[a]->vms->jobPosOnTimeline(alloc[a]->job->id);

			double execTime = alloc[a]->vms->timelineFinishTime[aPosOnVm] - alloc[a]->vms->timelineStartTime[aPosOnVm] - calculateReadtime(alloc[a]->job, alloc[a]->vms->id);
			// double writeTime = calculateWritetime(alloc[a]->job, alloc[a]->vms->id, alloc[a]->writeTo);
			// double processTime = ceil(alloc[a]->job->base_time * alloc[a]->vms->slowdown);
			
			double readTime = 0.0;

			for(int f = 0; f < alloc[a]->job->input.size(); f++){
				int minBandwidthVm = alloc[a]->vms->id;
				if(alloc[a]->job->input[f]->is_static){
					bool transferNeed = true;
					double maxBandwidth = 0.0;
					int id;
					for(int j = 0; j < alloc[a]->job->input[f]->static_vms.size(); j++){
						if(alloc[a]->vms->id == alloc[a]->job->input[f]->static_vms[j]){
							transferNeed = false;
							break;
						}
						if(alloc[a]->job->input[f]->VMsBandwidth[alloc[a]->job->input[f]->static_vms[j]] > maxBandwidth){
							maxBandwidth = alloc[a]->job->input[f]->VMsBandwidth[alloc[a]->job->input[f]->static_vms[j]];
							id = alloc[a]->job->input[f]->static_vms[j];
						}
					}

					if(maxBandwidth < alloc[a]->job->input[f]->VMsBandwidth[alloc[a]->vms->id])
						minBandwidthVm = id;
					if(transferNeed) readTime += alloc[a]->job->input[f]->IOTimeCost[minBandwidthVm];
				} else{
					// cout << "Not Static!" << endl;
					Item * file = alloc[a]->job->input[f];
					int origin =  file->alocated_vm_id;
					for(int i = 0; i < output.size(); i++){
						if(file->id == output[i]){
							// cout << "Changed VM!" << endl;
							origin = vmId;						
							break;			
						}
					}				
					if(origin == alloc[a]->vms->id) continue;

					if(alloc[a]->job->input[f]->VMsBandwidth[origin] < alloc[a]->job->input[f]->VMsBandwidth[minBandwidthVm])
						minBandwidthVm = origin;
					// cout << "ReadTime += " <<  alloc[a]->job->input[f]->IOTimeCost[minBandwidthVm] << endl;
					readTime += alloc[a]->job->input[f]->IOTimeCost[minBandwidthVm];
				}
			}
			// cout << "execTime: " << execTime + calculateReadtime(alloc[a]->job, alloc[a]->vms->id) << " oldRead: " << calculateReadtime(alloc[a]->job, alloc[a]->vms->id);			

			execTime += readTime;

			// cout << " newRead: " << readTime << " newExec: " << execTime << endl;

			for(int b = 0; b < a; b++){
				int bPosOnVm = alloc[b]->vms->jobPosOnTimeline(alloc[b]->job->id);
				int bVmId = alloc[b]->vms->id;
				if(conflicts[alloc[a]->job->id][alloc[b]->job->id] == 0){ // nao tem conflito
					if(alloc[a]->vms->id == bVmId){ // esta na mesma maquina
						if(newFinishTimes[alloc[b]->job->id] > latestJobVmFinish) // terminou mais tarde do que o ultimo da mesma vm
							latestJobVmFinish = newFinishTimes[alloc[b]->job->id];
					}
				} else { // tem conflito
					if(newFinishTimes[alloc[b]->job->id] > latestJobConflictFinish) // terminou mais tarde do que o ultimo que tenha conflito
						latestJobConflictFinish = newFinishTimes[alloc[b]->job->id];
				}
			}
			newStartTimes[alloc[a]->job->id] = max(latestJobVmFinish, latestJobConflictFinish); // tempo de comeco
			newFinishTimes[alloc[a]->job->id] = newStartTimes[alloc[a]->job->id] + execTime; // tempo de fim
		}



		// for(int i = 0; i < newStartTimes.size(); i++){
		// 	// if(jobs[i]->alocated_vm_id == job->alocated_vm_id)
		// 		cout << "Id: " << i << " Start: " << newStartTimes[i] << " Finish: " << newFinishTimes[i] << endl;
		// }
		// cin.get();

		double biggestSpan = 0.0;
		for(int i = 0; i < newFinishTimes[i]; i++){
			if(newFinishTimes[i] > biggestSpan)
				biggestSpan = newFinishTimes[i];
		}

		return biggestSpan;

	}

	double test_swapMachineWrite(){
		vector<double> newFinishTimes;
		vector<double> newStartTimes;
		for(int pos = 0; pos < this->alloc.size(); pos++){

			Allocation * swap = this->alloc[pos];
			Job * job = swap->job;
			Machine * originalAllocationMachine = swap->vms;
			double originalCost = this->calculateMakespam();

			// cout << "JobID: " << job->name << endl;

			int posOnTimeline = originalAllocationMachine->jobPosOnTimeline(job->id);
			double oldExecTime = originalAllocationMachine->timelineFinishTime[posOnTimeline] - originalAllocationMachine->timelineStartTime[posOnTimeline];
			// cout << "WRITE_TO: " << write_vm_id << endl;

			vector<int> outputs;
			for(int i = 0; i < job->output.size(); i++){
				outputs.push_back(job->output[i]->id);
			}

			for(unsigned int i = 0; i < this->vms.size(); i++){
				Machine * testVm = this->vms[i];
				if(testVm->id == swap->writeTo) continue;
				double readtime = calculateReadtime(job, originalAllocationMachine->id);

				double writetime = calculateWritetime(job, originalAllocationMachine->id, testVm->id);

				double processtime = ceil(job->base_time * originalAllocationMachine->slowdown);

				double newTime = readtime + processtime + writetime;

				
				
				// cout << "Calculating cascade effect" << endl;
				newFinishTimes = vector<double>(this->jobs.size(), 0.0);
				newStartTimes = vector<double>(this->jobs.size(), 0.0);
				double newSpan = calculate_swapMachineWrite_effect(pos, job, newTime, outputs, testVm->id, newStartTimes, newFinishTimes);
				// cout << "JOBID: " << job->id << " i: " << i << " span: " << newSpan << " originalSpan: " << originalCost<< " readtime: " << readtime << " processtime: " << processtime << " writetime: " << writetime << endl;

				// cout << "originalCost: " << originalCost << " newSpan: " << newSpan << " oldExecTime: " << oldExecTime << " newTime: " << newTime << " machinePos: " << i << endl;
				// cin.get();
				if(newSpan < originalCost){
					// cout << "The new Span is: " << newSpan << endl;
					
					// cin.get();
					 return this->execSwapMachineWrite(pos, i, newStartTimes, newFinishTimes);
				}
			}
		}
		return -1.0;
	}

	double execSwapMachineWrite(int pos, int vmId, vector<double>& newStartTimes, vector<double>& newFinishTimes){
		Job * changedJob = this->alloc[pos]->job;
		Machine * changedVm = this->vms[vmId];

		for(int a = 0; a < this->alloc.size(); a++){
			Job * job = this->alloc[a]->job;
			Machine * vm = this->alloc[a]->vms;
			int posOnTimeline = vm->jobPosOnTimeline(job->id);

			if(job->id == changedJob->id){
				// cout << "Is Changed Job!" << endl;
				for(int f = 0; f < job->output.size(); f++){
					job->output[f]->alocated_vm_id = vmId;
				}
			} 
			vm->timelineFinishTime[posOnTimeline] = newFinishTimes[job->id];
			vm->timelineStartTime[posOnTimeline] = newStartTimes[job->id];			
		}

		this->alloc[pos]->writeTo = vmId;

		for(int vm = 0; vm < vms.size(); vm++){
			this->fixMachineTimelineOrder(vm);
		}
		// this->calculateMakespam();
		// this->print();

		// this->checkFeasible();
		// cin.get();

		return this->calculateMakespam();
	}


	bool test_reallocate_valid(int pos, int newPos){
		Job * job = alloc[pos]->job;

		//esquerda filho do da direita
		if(newPos > pos){
			for(int i = 0; i < newPos; i++){
				
				Job * aux = alloc[i]->job;
				if (i == pos) aux = alloc[newPos]->job;

				if(conflicts[aux->id][job->id] == 1){
					return false;
				}
			}
		} else{
			for(int i = newPos + 1; i <= pos; i++){				
				Job * aux = alloc[i]->job;

				if(conflicts[job->id][aux->id] == 1){
					return false;
				}
			}
		}

		return true;
	}

	double exec_reallocate(int pos1, int pos2, vector<double> newStartTimes, vector<double> newFinishTimes){
		for(int a = 0; a < this->alloc.size(); a++){
			Job * job = this->alloc[a]->job;
			Machine * vm = this->alloc[a]->vms;
			int posOnTimeline = vm->jobPosOnTimeline(job->id);
			vm->timelineFinishTime[posOnTimeline] = newFinishTimes[job->id];
			vm->timelineStartTime[posOnTimeline] = newStartTimes[job->id];			
		}
		Allocation * aux = alloc[pos1];
		alloc[pos1] = alloc[pos2];
		alloc[pos2] = aux;

		this->calculateMakespam();
		// this->print();
		// this->printAlloc();
		// cin.get();

		for(int vm = 0; vm < vms.size(); vm++){
			this->fixMachineTimelineOrder(vm);
		}
		

		// this->checkFeasible();
		// cin.get();

		return this->calculateMakespam();
	}

	double test_reallocate(){
		// this->print();
		// this->printAlloc();
		// cin.get();

		vector<double> newFinishTimes;
		vector<double> newStartTimes;
		double originalCost = this->calculateMakespam();
		for(int pos1 = 0; pos1 < alloc.size() - 1; pos1++){
			for(int pos2 = pos1 + 1; pos2 < alloc.size(); pos2++){
				
				if(!test_reallocate_valid(pos1, pos2) || !test_reallocate_valid(pos2, pos1)) {
					// cout << "Pos1: " << pos1 << " pos2: " << pos2 << " Nao podem ser trocados!" << endl;
					continue;
				}

				// cout << "Pos1: " << pos1 << " pos2: " << pos2 << " PODEM ser trocados!" << endl;
				newFinishTimes = vector<double>(this->jobs.size(), 0.0);
				newStartTimes = vector<double>(this->jobs.size(), 0.0);

				double newSpan = calculate_reallocate_effect(pos1, pos2, newStartTimes, newFinishTimes);
				if(newSpan < originalCost){
					// cout << "newSpan: " << newSpan << " oldSpam: " << originalCost << endl;
					// cin.get();
					// cout << "Pos1: " << pos1 << " pos2: " << pos2 <<  endl;
					// return newSpan;
					return exec_reallocate(pos1, pos2, newStartTimes, newFinishTimes);
				}
			}
		}

		return -1.0;
	}

	double calculate_reallocate_effect(int pos1, int pos2, vector<double>& newStartTimes, vector<double>& newFinishTimes){
		
		for(int a = 0; a < alloc.size(); a++){ // preenchendo inicio e fim original
			int jobId = alloc[a]->job->id;
			Machine * vm = alloc[a]->vms;
			int posOnVm = vm->jobPosOnTimeline(jobId);
			newStartTimes[jobId] = vm->timelineStartTime[posOnVm];
			newFinishTimes[jobId] = vm->timelineFinishTime[posOnVm];
		}

		//  for(int i = 0; i < newStartTimes.size(); i++){
		// 	// if(jobs[i]->alocated_vm_id == job->alocated_vm_id)
		// 		cout << "Id: " << i << " Start: " << newStartTimes[i] << " Finish: " << newFinishTimes[i] << endl;
		// }
		// cin.get();

		for(int a = pos1; a < alloc.size(); a++){ // recalculando start e finish seguindo a ordem de allocation
		
			double latestJobConflictFinish = 0.0;
			double latestJobVmFinish = 0.0;
			int usedPos = a;
			if(a == pos1) usedPos = pos2;
			if(a == pos2) usedPos = pos1;

			int aPosOnVm = alloc[usedPos]->vms->jobPosOnTimeline(alloc[usedPos]->job->id);

			double execTime = alloc[usedPos]->vms->timelineFinishTime[aPosOnVm] - alloc[usedPos]->vms->timelineStartTime[aPosOnVm];
			for(int b = 0; b < a; b++){

				int usedPos2 = b;
				if(b == pos1) usedPos2 = pos2;
				if(b == pos2) usedPos2 = pos1;

				int bPosOnVm = alloc[usedPos2]->vms->jobPosOnTimeline(alloc[usedPos2]->job->id);
				int bVmId = alloc[usedPos2]->vms->id;
				
				if(conflicts[alloc[usedPos]->job->id][alloc[usedPos2]->job->id] == 0){ // nao tem conflito
					if(alloc[usedPos]->vms->id == bVmId){ // esta na mesma maquina
						if(newFinishTimes[alloc[usedPos2]->job->id] > latestJobVmFinish) // terminou mais tarde do que o ultimo da mesma vm
							latestJobVmFinish = newFinishTimes[alloc[usedPos2]->job->id];
					}
				} else { // tem conflito
					if(newFinishTimes[alloc[usedPos2]->job->id] > latestJobConflictFinish) // terminou mais tarde do que o ultimo que tenha conflito
						latestJobConflictFinish = newFinishTimes[alloc[usedPos2]->job->id];
				}
			}
			// cout << "Recalculando Job: " << alloc[usedPos]->job->id << " ExecTime: " << execTime << " latestJobVmFinish: " << latestJobVmFinish << " latestJobConflictFinish: " << latestJobConflictFinish << endl;

			newStartTimes[alloc[usedPos]->job->id] = max(latestJobVmFinish, latestJobConflictFinish); // tempo de comeco
			newFinishTimes[alloc[usedPos]->job->id] = newStartTimes[alloc[usedPos]->job->id] + execTime; // tempo de fim
		}

		// for(int i = 0; i < newStartTimes.size(); i++){
		// 	// if(jobs[i]->alocated_vm_id == job->alocated_vm_id)
		// 		cout << "Id: " << i << " Start: " << newStartTimes[i] << " Finish: " << newFinishTimes[i] << endl;
		// }
		// cin.get();

		double biggestSpan = 0.0;
		for(int i = 0; i < newFinishTimes[i]; i++){
			if(newFinishTimes[i] > biggestSpan)
				biggestSpan = newFinishTimes[i];
		}

		return biggestSpan;

	}

	bool realocate(int pos1, int pos2){
		Allocation * relocation = this->alloc[pos1];
		// this->printAlloc();
		// cout << "********" << endl;
		this->alloc.erase(this->alloc.begin() + pos1);

		vector<int> allocations = vector<int>(files.size(), -1);
		for(int f = 0; f < files.size(); f++){
			int id = files[f]->alocated_vm_id;
			allocations[files[f]->id] = id;
		}

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
			// teste = this->alloc[j]->vms->popJob(this->alloc[j]->job->id);
			teste = this->alloc[j]->vms->popJobKeepAllocation(this->alloc[j]->job->id);
			if(!teste){
				cout << "NAO CONSEGUIU POP!" << endl;
				cin.get();
			}
		}

		for(unsigned int j = pos1; j < this->alloc.size(); j++){
			double minSpam = this->getJobConflictMinSpam(this->alloc[j]->job);
			// this->alloc[j]->vms->pushJob(this->alloc[j]->job, this->alloc[j]->writeTo, minSpam);
			this->alloc[j]->vms->pushJobKeepAllocation(this->alloc[j]->job, this->alloc[j]->writeTo, minSpam, allocations);
		}

		// this->printAlloc();

		return true;
	}

	void printAlloc(){
		// cout << "Allocation order that created solution: " << endl;
		// for(unsigned int i = 0; i < alloc.size(); i++){
		// 	cout << "JobID: " << alloc[i]->job->name << " to MachineID: " << alloc[i]->vms->id << " Writing to MachineID: " << alloc[i]->writeTo << endl;
		// }
		for(unsigned int i = 0; i < alloc.size(); i++){
			cout << alloc[i]->job->id << ", ";
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
			else{
				cout << "JobID: " << CL[chosenMovement]->id << " Was NOT Inserted!" << endl;
			
			// cout << "Spam: " << calculateMakespam() << endl;
			cin.get();
			}

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

	bool perturbateOrder(int pos, int max){
		int end = pos + max;
		if(end >= this->alloc.size() - 1) end = this->alloc.size() - 1;

		// cout << "Starting PerturbateOrder Pos: " << pos << " max: " << max << endl;
		// cout << "Cost: " << this->calculateMakespam() << endl;
		// this->printAlloc();
		// cin.get();

		vector<Allocation*> shufflePool;
		vector<Allocation*> tail;
		for(int i = this->alloc.size() - 1; i >= pos; i--){
			if(i <= end){
				shufflePool.push_back(this->alloc[i]);
			}
			if(i > end){
				tail.push_back(this->alloc[i]);
			}
			this->alloc[i]->vms->popJob(this->alloc[i]->job->id);
		}
		
		for(int i = this->alloc.size() - 1; i >= pos; i--){
			this->alloc.pop_back();
		}
		
		// this->printAlloc();
		// cin.get();
		
		// cout << "Shuffle Result" << endl;

		// for(int i = 0; i < shufflePool.size(); i++){
		// 	cout << shufflePool[i]->job->name << " ";
		// }
		// cout << endl;

		random_shuffle(shufflePool.begin(), shufflePool.end());

		// for(int i = 0; i < shufflePool.size(); i++){
		// 	cout << shufflePool[i]->job->name << " ";
		// }
		// cout << endl;

		// cin.get();

		// cout << "Inserting shuffledPool" << endl;
		while(shufflePool.size() > 0){
			Allocation * aux = shufflePool.front();
			shufflePool.erase(shufflePool.begin());
			bool inserted = aux->vms->pushJob(aux->job, aux->writeTo, getJobConflictMinSpam(aux->job));
			if(!inserted){
				shufflePool.push_back(aux);
			} else {
				// cout << "Inserted!" << endl;
				this->alloc.push_back(aux);
			}
		}

		// cin.get();

		// cout << "Inserting old Tail" << endl;
		// cout << "Tail Result:" << endl;

		// for(int i = 0; i < tail.size(); i++){
		// 	cout << tail[i]->job->name << " ";
		// }
		// cout << endl;


		while(tail.size() > 0){
			Allocation * aux = tail.back();
			tail.pop_back();
			bool inserted = aux->vms->pushJob(aux->job, aux->writeTo, getJobConflictMinSpam(aux->job));
			if(!inserted){
				// cout << "Deu merda" << endl;
			} else {
				// cout << "Inserted!" << endl;
				this->alloc.push_back(aux);
			}
		}
		
		// cout << "Shuffled!" << endl;
		// cout << "Cost: " << this->calculateMakespam() << endl;
		// this->printAlloc();
		// cin.get();

		// re-add tail jobs maintaining the order
	}

	bool perturbateWriteTo(int pos){
		int newVM = rand() % vms.size();
		Job* job = alloc[pos]->job;
		
		Machine * testVm = this->vms[newVM];
		
		vector<int> newAlocations = vector<int>(files.size(), -1);
		for(int f = 0; f < files.size(); f++){
			int id = files[f]->alocated_vm_id;
			newAlocations[files[f]->id] = id;
		}

		for(int f = 0; f < job->output.size(); f++){
			newAlocations[job->output[f]->id] = testVm->id;
		}

		double readtime = calculateReadtime(job, alloc[pos]->vms->id);

		double writetime = calculateWritetimeWithChanges(job, alloc[pos]->vms->id, newAlocations);

		double processtime = ceil(job->base_time * alloc[pos]->vms->slowdown);

		double newTime = readtime + processtime + writetime;

		vector<int> outputs;
		for(int i = 0; i < job->output.size(); i++){
			outputs.push_back(job->output[i]->id);
		}
		
		// cout << "Calculating cascade effect" << endl;
		vector<double> newFinishTimes = vector<double>(this->jobs.size(), 0.0);
		vector<double> newStartTimes = vector<double>(this->jobs.size(), 0.0);
		double newSpan = calculate_swapMachineWrite_effect(pos, job, newTime, outputs, testVm->id, newStartTimes, newFinishTimes);

		this->execSwapMachineWrite(pos, testVm->id, newStartTimes, newFinishTimes);

		return true;
	}

	bool perturbateMachine(int pos){

		int newVM = rand() % vms.size();

		Job* job = alloc[pos]->job;

		Machine * testVm = this->vms[newVM];

		double readtime = calculateReadtime(job, testVm->id);
		double writetime = calculateWritetime(job, testVm->id, alloc[pos]->writeTo);
		double processtime = ceil(job->base_time * testVm->slowdown);

		double newTime = readtime + processtime + writetime;

		vector<double> newFinishTimes = vector<double>(this->jobs.size(), 0.0);
		vector<double> newStartTimes = vector<double>(this->jobs.size(), 0.0);
		double newSpan = calculate_swapMachine_effect(pos, job, newTime, testVm->id, newStartTimes, newFinishTimes);

		this->execSwapMachine(pos, testVm->id, newStartTimes, newFinishTimes);

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

	void print(){
		cout << "VMs:" << endl;
		for(unsigned int i = 0; i < vms.size(); i++){
			cout << "ID: " << vms[i]->id << " Name:" << vms[i]->name << ": " ;
			for(unsigned int j = 0; j < vms[i]->timelineJobs.size(); j++){
				cout << vms[i]->timelineJobs[j]->id << "( " << vms[i]->timelineStartTime[j] << "," << vms[i]->timelineFinishTime[j] << " )" << " ";
			}
			cout << endl;
		}

		// for(unsigned int i = 0; i < files.size(); i++){
		// 	cout << "file: " << files[i]->id << " VM: " << files[i]->alocated_vm_id << endl;
		// }
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
			Machine * JobMachine = vms[jobs[i]->alocated_vm_id];
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
				pos = JobMachine->jobPosOnTimeline(jobs[i]->id);
				jobStartTime = JobMachine->timelineStartTime[pos];
				// cout << "Job: " << i << " comecou no tempo: " << jobStartTime << ", mas preJob:" << j << " terminou no tempo: " << preJobFinishtime << endl;
				if(jobStartTime < preJobFinishtime){
					cout << "Job: " << i << " comecou no tempo: " << jobStartTime << ", mas preJob:" << j << " terminou no tempo: " << preJobFinishtime << endl;
					return false;
				}

			}
			int jobPos = JobMachine->jobPosOnTimeline(jobs[i]->id);
			if(jobPos > 0){
				if(JobMachine->timelineFinishTime[jobPos - 1] > JobMachine->timelineStartTime[jobPos]){
					cout << "Job: " << JobMachine->timelineJobs[jobPos - 1]->id << " terminou depois do Job: " << jobs[i]->id << " Comecar!" << endl;
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