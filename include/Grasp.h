#ifndef GRASP_H
#define GRASP_H


#include <string>
#include <iostream>
#include <algorithm>
#include <tclap/CmdLine.h>
#include "MinMin.h"


using namespace TCLAP;
using namespace std;

class Grasp{

    public:
        Data* data;
        double alpha;
        double lambda;

        //virtual ~Chromosome();
        virtual ~Grasp() {
            // TODO Auto-generated destructor stub
        }

        // Random Constructor
        
        Grasp(Data* data, double alpha, double lambda) : 
            data(data), alpha(alpha), lambda(lambda) {

        }

        void Construct(){
            cout << "Random Constructor" << endl;
            unordered_map<int, list<int>> task_map;
            int maxHeight = 0;
            for (auto task : data->task_map){
                auto id = task.second.id;
                auto height = data->height[id];
                if(height > maxHeight) maxHeight = height;
                auto list = task_map[height];
                list.push_back(id);
                task_map[height] = list;
            }
            
            for(int i = 1; i <= maxHeight; i++){
                vector<double> ft_vector(data->size, 0);
                vector<double> queue(data->vm_size, 0);
                vector<int> file_place(data->size, 0);
                list<int> task_ordering(0);

                scheduleTask(data, task_map[i], ft_vector, queue, file_place, task_ordering,lambda);

            }

            cout << "MAX HEIGHT: " << maxHeight << endl;

            // for (auto tuple : task_map){
            //     cout << "First: " << tuple.first << " Second: [ ";
            //     for (auto seconds : task_map[tuple.first])
            //         cout << "," << seconds;
            //     cout << "]" << endl;
            // }


            cout << "end" << endl;

        }
    private:
        void scheduleTask(Data *data, list<int> avail_tasks, vector<double> &ft_vector, vector<double> &queue, vector<int> &file_place,
         list<int> &task_ordering, double lambda){

        }
};

#endif