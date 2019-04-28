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

        Problem * start(){
            Problem * p = new Problem(*this->blankProblem);           
            double bestSolValue = 9999999999.0;
            Problem * bestSolution = new Problem(*p);
            bestSolution->createSolution(this->alpha);
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
                bool improvement = true;
                while(improvement){
                    // cout << "LOOP START" << endl;
                    improvement = false;
                    bool lsImprovement = false;
                    for(int i = 0; i < p->alloc.size(); i++){ // RELOCATE LOOP START
                        for (int j = 0; j < p->alloc.size(); j++){
                            if(i == j) continue;
                            Problem * backup = new Problem(*p);
                            bool done = p->realocate(i, j);
                            // delete backup;
                            // bool done = false;
                            if(done){
                                
                                if(!p->checkFeasible()){
                                    cout << "booom1" << endl;
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
                    lsImprovement = false;
                    // cout << "EXAUSTED RELOCATE! p->cost was: " << p->calculateMakespam() << endl;
                    for(int i = 0; i < p->alloc.size(); i++){ // SWAP MACHINE LOOP START
                        Problem * backup = new Problem(*p);
                        lsImprovement = p->swapMachine(i);
                        if(!p->checkFeasible()){
                            cout << "booom1" << endl;
                            cin.get();
                        }
                        if(lsImprovement){
                            // cout << "SWAP IMPROV! Cost: " << p->calculateMakespam()  << endl;
                            // cin.get();
                            delete backup;
                            break;
                        } else{
                            delete p;
                            p = backup;
                        }
                    } // END SWAP MACHINE LOOP

                    if(lsImprovement){
                        // cout << "MELHOROU COM A BL2" << endl;
                        // cin.get();
                        improvement = true;
                        continue;
                    }
                    lsImprovement = false;
                    // cout << "EXAUSTED SWAP MACHINE! p->cost was: " << p->calculateMakespam() << endl;
                    // for(int i = 0; i < p->alloc.size(); i++){ // SWAP MACHINE LOOP START
                    //     for (int j = 0; j < p->alloc.size(); j++){
                    //         Problem * backup = new Problem(*p);
                    //         improvement = p->swapMachinePair(i, j);
                    //         if(improvement){
                    //             // cout << "SWAP PAIR IMPROV! Cost: " << p->calculateMakespam() << endl;
                    //             // cin.get();
                    //             delete backup;
                    //             break;
                    //         } else{
                    //             delete p;
                    //             p = backup;
                    //         }
                    //     }
                    // } // END SWAP MACHINE LOOP
                    // cout << "EXAUSTED SWAP PAIR!" << endl;
                    if (p->calculateMakespam() < bestSolValue){
                        delete bestSolution;
                        bestSolution = new Problem(*p);
                        bestSolValue = bestSolution->calculateMakespam();
                    }
                }  
                delete p;
                // cout << "ITERATION: " << iter << " CURRENT BEST SOL VALUE: " << bestSolValue << endl;
                // cin.get();
            }
            // exit(1);
            // cin.get();
            return bestSolution;
        }
};

#endif