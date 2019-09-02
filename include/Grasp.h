#ifndef GRASP_H
#define GRASP_H


#include <string>
#include <iostream>
#include <algorithm>
#include "Problem.h"
#include "LocalSearch.h"


using namespace std;

class Grasp{

    public:
        double alpha;
        Problem * blankProblem;
        vector<LocalSearch *> localSearch;
        //virtual ~Chromosome();
        virtual ~Grasp() {
            // TODO Auto-generated destructor stub
        }
        
        Grasp(Problem * problem, double alpha){
            this->blankProblem = problem;
            this->alpha = alpha;
        }

        Problem * startLocalSearch(Problem * p){
            // cout << "Started Local Search" << endl;
            bool improvement = true;
            while(improvement){
                // cout << "LOOP START Cost: " << p->calculateMakespam() << " Elapsed Time: " << double(clock() - begin) / CLOCKS_PER_SEC << endl;
                improvement = false;
                bool lsImprovement = false;
                double moveCost = 0.0;

                moveCost = p->test_swapFileAllocation();
                if(moveCost >= 0){
                    lsImprovement = true;
                }
                if(!p->checkFeasible()){
                    cout << "booom Swap File Allocation" << endl;
                    p->print();
                    cin.get();
                }
                if(lsImprovement){
                    improvement = true;
                    continue;
                }

                for(int i = 0; i < p->alloc.size(); i++){ // RELOCATE LOOP START
                    for (int j = 0; j < p->alloc.size(); j++){
                        if(i == j) continue;
                        Problem * backup = new Problem(*p);
                        if(!p->checkFeasible()){
                                cout << "booom BEFORE Relocate" << endl;
                                cin.get();
                            }
                        bool done = p->realocate(i, j);
                        // delete backup;
                        // bool done = false;
                        if(done){
                            
                            if(!p->checkFeasible()){
                                cout << "booom Relocate" << endl;
                                backup->print();
                                cin.get();
                            }
                            double solValue = p->calculateMakespam();
                            // cout << "A Move was Done! i: " << i << " j: " << j << endl;
                            // cout << "oldValue: " << backup->calculateMakespam() << " newValue: " << solValue << endl;
                            // cin.get();
                            if(solValue >= backup->calculateMakespam()){
                                delete p;
                                p = backup;
                            } else{
                                // cout << "RELOCATE IMPROV! Cost: " << p->calculateMakespam()  << endl;
                                // cin.get();
                                delete backup;
                                lsImprovement = true;
                                break;
                            }                                
                        } else{
                            delete backup;
                        }
                    }
                    if(lsImprovement){
                        break;
                    }
                } // RELOCATE LOOP END

                if(lsImprovement){
                    // cout << "MELHOROU COM A BL1" << endl;
                    // cin.get();
                    improvement = true;
                    continue;
                } 
                
                moveCost = p->test_swapMachinePair();
                if(moveCost >= 0){
                    lsImprovement = true;
                }
                if(!p->checkFeasible()){
                    cout << "booom Swap Machine Pair" << endl;
                    p->print();
                    cin.get();
                }
                if(lsImprovement){
                    improvement = true;
                    continue;
                }

                moveCost = p->test_swapMachine();
                if(moveCost > 0){
                    lsImprovement = true;
                }

                if(!p->checkFeasible()){
                    cout << "booom Swap MAchine" << endl;
                    p->print();
                    cin.get();
                }

                if(lsImprovement){
                    improvement = true;
                    continue;
                }
                
                moveCost = p->test_swapMachineWrite();
                if(moveCost >= 0){
                    lsImprovement = true;
                }
                if(!p->checkFeasible()){
                    cout << "booom Swap Write" << endl;
                    cin.get();
                }
                if(lsImprovement){
                    improvement = true;
                    continue;
                }
            }  
            // cout << "LOOP FINISHED" << endl;
            // cin.get();
            return p;
        }

        Problem * start(){
            Problem * p = new Problem(*this->blankProblem);           
            double bestSolValue = 9999999999.0;
            Problem * bestSolution = new Problem(*p);
            bestSolution->createSolution(this->alpha);
            int localSearchActivated1 = 0;
            int localSearchActivated2 = 0;
            int localSearchActivated3 = 0;
            for(int iter = 0; iter < 100; iter++){
                // cout << "iter= " << iter << endl;
                p = new Problem(*this->blankProblem);
                p->createSolution(this->alpha);
                // p->printAlloc();
                if(!p->checkFeasible()){
                    cout << "booom1" << endl;
                    cin.get();
                }
                if (p->calculateMakespam() < bestSolValue){
                    delete bestSolution;
                    bestSolution = new Problem(*p);
                    bestSolValue = p->calculateMakespam();
                }
                p = startLocalSearch(p);
                if(!p->checkFeasible()){
                    cout << "booom1" << endl;
                    cin.get();
                }
                if (p->calculateMakespam() < bestSolValue){
                    delete bestSolution;
                    bestSolution = new Problem(*p);
                    bestSolValue = p->calculateMakespam();
                }
            }
            bestSolution->checkFeasible();
            // cout << bestSolution->calculateMakespam() / 60 << " " << localSearchActivated1 << " " << localSearchActivated2 << " " << localSearchActivated3 << " " << localSearchActivated1 + localSearchActivated2 + localSearchActivated3 << " ";
            return bestSolution;
        }
};

#endif