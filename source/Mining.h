#ifndef MiningH
#define MiningH

#include <sstream>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <vector>

using namespace std;

// Forward declaration
// Data Structure for DM operations
class Solucao{
	public:
		vector<int> transaction;
		double cost;
		Solucao(){cost =0;};
		~Solucao(){};
		Solucao &operator=(const Solucao &outro) {
			transaction = outro.transaction;
			cost = outro.cost;
			return *this;
		};
};


class Mining{
	// Forward declaration
	class Pattern;

	private:	
		int nPatterns,sizeES,maxSizeES, min_sup, min_sup_orig, worstCostPos, currentPattern, gamma,nJobs;
		double worstCostES;
		Pattern** listOfPatterns;		// Lista de padroes minerados pelo FPmax
		Solucao** ES;    		// lista das melhores solucoes
		bool eschanged,mined;
		int iterWNC, numberMine;
		
		class Pattern{
			public:	
				vector<pair<int,int>> elements;	//
				int support,size;
				Pattern(){  elements = std::vector<pair<int,int>>(); support = 0; size = 0;};
				~Pattern(){ elements.clear();};
				// Assignment operator overload 
				Pattern &operator=(const Pattern &other) {
					support = other.support;
					size = other.size;
					elements = other.elements;
					return *this;
				}
		};
	
	
	public:
		Mining();
		Mining(int _tamCE, int _suporte, int _gamma);
		~Mining();
		void mine(int = 10);	
		void map_file();
		void unmapall_file();
		void printPatterns();
		
		///// Manipula Conjunto Elite
		void printES();
		bool updateES(Solucao* s);
		int getSizeCE(){ return sizeES;};
		bool isStableES() { return itersWithoutChange() >= gamma;};
		double getWorstCostES() { return worstCostES;};
		int itersWithoutChange() {return iterWNC;};
		void oneMoreIWC() { iterWNC++; eschanged = false;};
		bool EShasChanged(){ return eschanged;};
		void resetESConfig() { eschanged = false;};
		bool hasMined() {return mined;};
		int getNumberOfMine() {return numberMine;};
		int getNumberOfPatterns() {return nPatterns;};
		int getSupport() {return min_sup_orig;};
		
		///// Manipula padrões
		void nextPattern(){(currentPattern < nPatterns -1 ) ? (currentPattern++) : (currentPattern = 0); }
		Pattern* getCurrentPattern() { return listOfPatterns[currentPattern];};
		
};


//---------------------------------------------------------------------------
#endif
