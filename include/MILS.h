#ifndef MILS_H
#define MILS_h


#include <string>
#include <iostream>
#include <algorithm>
#include "Problem.h"
#include "LocalSearch.h"


using namespace std;

class Mils{

    public:
        double alpha;
        Problem * blankProblem;
        vector<LocalSearch *> localSearch;
        //virtual ~Chromosome();
        virtual ~Mils() {
            // TODO Auto-generated destructor stub
        }
        
        Mils(Problem * problem, double alpha){
            this->blankProblem = problem;
            this->alpha = alpha;
        }

        Problem * start(){
            Problem * p = new Problem(*this->blankProblem);           
            double bestSolValue = 9999999999.0;
            Problem * bestSolution = new Problem(*p);
            string bestSolOrigin = "";
            // bestSolution->createSolution(this->alpha);
            for(int iter = 0; iter < 10; iter++){
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
                    bestSolOrigin = "Constructor";
                }

                bool improvement = true;

                while(improvement){
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

                    if(lsImprovement) improvement = true;
                    lsImprovement = false;

                }
                
                if (p->calculateMakespam() < bestSolValue){
                    delete bestSolution;
                    bestSolution = new Problem(*p);
                    bestSolValue = p->calculateMakespam();
                    bestSolOrigin = "LocalSearch1";
                }

                // cout << "P COST : " << p->calculateMakespam() << endl;
                // cout << "Current State: " << bestSolValue << " Origin: " << bestSolOrigin << endl;

                // cin.get();
                double lastPvalue = p->calculateMakespam();
                for(int mov = 0; mov < 100; mov++){
                    Problem * backupP = new Problem(*p);
                    int pos = rand() % ((p->jobs.size()) / 2) + (p->jobs.size()-1) / 2;
                    // cout << "P COST: " << p->calculateMakespam() << " DESTRUINDO A PARTIR: "  << pos << endl;

                    p->recreateSolution(pos, this->alpha);
                    if(!p->checkFeasible()){
                        cout << "booom1" << endl;
                        cin.get();
                    }
                    // cout << "P COST AFTER PERTURBATION: " << p->calculateMakespam() << endl;
                    improvement = true;
                    while(improvement){
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

                        if(lsImprovement) improvement = true;
                        lsImprovement = false;

                    }

                    if(p->calculateMakespam() < lastPvalue){
                        lastPvalue = p->calculateMakespam();
                        delete backupP;
                        if (p->calculateMakespam() < bestSolValue){
                            // cout << "MOFO GOT BETTAH!" << endl;
                            // cin.get();
                            delete bestSolution;
                            bestSolution = new Problem(*p);
                            bestSolValue = p->calculateMakespam();
                            bestSolOrigin = "perturbation";
                        }
                    } else{
                        delete p;
                        p = backupP;
                    }
                    // cout << "P COST AFTER LOCALSEARCH: " << p->calculateMakespam() << endl;
                    // cin.get();
                    
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