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
            for(int j = 0; j < 100; j++){
                p = new Problem(*this->blankProblem);
                p->createSolution(this->alpha);
                p->printAlloc();
                continue;
                if (p->calculateMakespam() < bestSolValue){
                    cout << "NEW BEST@@@@: " << p->calculateMakespam() / 60.0<< endl;
                    delete bestSolution;
                    bestSolution = new Problem(*p);
                    bestSolValue = p->calculateMakespam();
                }
                bool improvement = true;
                while(improvement){
                    improvement = false;
                    for(int i = 0; i < p->alloc.size(); i++){
                        for (int j = 0; j < p->alloc.size(); j++){
                            if(i == j) continue;
                            Problem * backup = new Problem(*p);
                            bool done = p->realocate(i, j);
                            if(done){
                                if(!p->checkFeasible()){
                                    cout << "booom1" << endl;
                                    cin.get();
                                }

                                double solValue = p->calculateMakespam();
                                if(solValue > backup->calculateMakespam()){
                                    delete p;
                                    p = backup;
                                } else{
                                    delete backup;
                                }

                                if (solValue < bestSolValue){
                                    cout << "NEW BEST!!!!: " << p->calculateMakespam() / 60.0<< endl;
                                    delete bestSolution;
                                    bestSolution = new Problem(*p);
                                    bestSolValue = p->calculateMakespam();
                                }
                            }
                        }
                    }
                }

                // cout << "FirstSolution: " << endl;
                // p->printAlloc();
                // for(int i = 0; i < 100; i++){
                //     // int perturbationId = random() % 3;
                //     // int perturbationSize = random() % p->alloc.size();
                //     // cout << "TotalPerturbations: " << perturbationSize << endl;
                //     // cout << "Cost Before Perturbation: " << p->calculateMakespam() << endl;
                //     double solValue;
                //     // while(perturbationSize > 0){
                //         // p->printAlloc();
                //         int posPerturbation = random() % p->alloc.size();
                //         bool done = p->forceRelocate(posPerturbation);
                //         if(!p->checkFeasible()){
                //             cout << "booom1" << endl;
                //             cin.get();
                //         }
                //         solValue = p->calculateMakespam();
                //         if (solValue < bestSolValue){
                //             cout << "NEW BEST!!!!: " << solValue / 60.0<< endl;
                //             delete bestSolution;
                //             bestSolution = new Problem(*p);
                //             bestSolValue = solValue;
                //         }
                //         // perturbationSize--;
                //     // }
                
                //     // if(!p->checkFeasible()){
                //     //     cout << "booom2" << endl;
                //     //     cin.get();
                //     // }

                //     // solValue = p->calculateMakespam();
                //     // if (solValue < bestSolValue){
                //     //     delete bestSolution;
                //     //     cout << "NEW BEST!!!!: " << solValue / 60.0<< endl;
                //     //     bestSolution = new Problem(*p);
                //     //     bestSolValue = solValue;
                //     // }
                // }
                delete p;
                
                // cin.get();
            }
            // exit(1);
            // cin.get();
            return bestSolution;
        }
};

#endif