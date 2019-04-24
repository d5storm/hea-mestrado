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
                // cout << "ITERATION: " << iter << " CURRENT BEST SOL VALUE: " << bestSolValue << endl;
                while(improvement){
                    improvement = false;
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
                                    improvement = true;
                                }                                
                            } else{
                                delete backup;
                            }
                        }
                        if(improvement){
                            break;
                        }
                    } // RELOCATE LOOP END
 
                    // cout << "EXAUSTED RELOCATE!" << endl;
                    // for(int i = 0; i < p->alloc.size(); i++){ // SWAP MACHINE LOOP START
                    //     Problem * backup = new Problem(*p);
                    //     improvement = p->swapMachine(i);
                    //     if(improvement){
                    //         // cout << "SWAP IMPROV! Cost: " << p->calculateMakespam()  << endl;
                    //         // cin.get();
                    //         delete backup;
                    //         break;
                    //     } else{
                    //         delete p;
                    //         p = backup;
                    //     }
                    // } // END SWAP MACHINE LOOP

                    // // cout << "EXAUSTED SWAP MACHINE!" << endl;
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
                    if(improvement){
                        if (p->calculateMakespam() < bestSolValue){
                            delete bestSolution;
                            bestSolution = new Problem(*p);
                            bestSolValue = bestSolution->calculateMakespam();
                        }
                    }
                }  
                delete p;              
            }
            // exit(1);
            // cin.get();
            return bestSolution;
        }
};

#endif