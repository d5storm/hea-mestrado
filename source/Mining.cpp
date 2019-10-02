#include "Mining.h"

using namespace std;


Mining::Mining(int _sizeES, int _suporte, int _gamma){
	currentPattern = nPatterns = sizeES = 0;
	maxSizeES = _sizeES;
	min_sup_orig = min_sup = _suporte;
	gamma = _gamma;
	worstCostES = -999999999;
	worstCostPos = -1;
	listOfPatterns = NULL;
	mined = eschanged = false;
	iterWNC = numberMine = 0;
	ES = new Solucao*[_sizeES];
	for(int i = 0; i < _sizeES ;i++){
//		criarSolucao(ES[i]);
	}
}

Mining::~Mining(){
	for(int i = 0; i < sizeES; i++)
		delete (ES[i]);
	delete(ES);	
	for(int i = 0; i < nPatterns; i++){
		//listOfPatterns[i]->~Pattern();
		delete (listOfPatterns[i]);
	}
	delete(listOfPatterns);
}

void Mining::mine(int nMaxPatterns){
	
	if(listOfPatterns == NULL){
		listOfPatterns = new Pattern*[nMaxPatterns];
		for(int i = 0; i < nMaxPatterns ;i++){
			listOfPatterns[i] = new Pattern();
		}
	}
	
	// New patterns to be mined
	nPatterns = 0;
	
	ostringstream buffer;
	buffer.str("");		
/*
   Exemplo de utilização:
   	   fpmax_hnmp <semente> <id_arq_tmp> <banco de dados> <tam. do banco> <suporte minimo> <qtd de padroes> <arq. saida> 
*/
	buffer 
			<< "../miner/fpmax_hnmp " 
			<< "1 " 
			<< ( random() % 100)
			<< " ../miner/bd.txt " 
			<<  sizeES
			<< " " << min_sup 
			<< " " << nMaxPatterns 
			<< " ../miner/padroes.txt" ;
	int s = system(buffer.str().c_str());
	if(s != 0){ cout << "ERROR: function system cannot be performed." << endl; exit(1);}
	buffer.str("");
	iterWNC = 0;
	numberMine++;
	eschanged = false;
	mined = true;
}

// Mapeia soluções elite e printa em um arquivo.
void Mining::map_file(){
	ofstream in("../miner/bd.txt"); 
	if(!in) { 
		cout << "Cannot open file."; 
		return ; 
	}
	
	for(int i=0 ; i< sizeES ; i++){
		//for(int j=0 ; j< ES[i]->numPts ; j++){
		//	in << numPosCand*j + ES[i]->vetPosCand[j] << " ";
		//}
		in << "\n";
	}
	in.close(); 
	return;
}


// Lê saída do FPmax e remapeia os padrões em arestas.

void Mining::unmapall_file(){
	FILE *fp = fopen("../miner/padroes.txt", "r");
	if(!fp) { cout << "ERROR: Could not open padroes.txt file." << endl;  exit(1);}
	
	int r, supp, tam;
	// Ler o tamanho e o suporte do padrão corrente
	r = fscanf(fp, "%d;%d;", &tam, &supp);
	//cout << "tam e suporte " << tam  << " " << supp << endl;
//	Pattern *p = new Pattern();
//	p->support = supp;
//	p->size = tam;
//	if(!p->support && !p->size){
//		numberMine--;
//		eschanged = true;
//		mined = false;
//		//cout << "Sem Padrões Minerados!" << min_sup << endl;
//		return;
//		min_sup--;
//		cout << "Re-MINE: " << min_sup << endl;
//		mine();
//
//
//	}else{
//		min_sup = min_sup_orig;
//	}
//
//	while(r == 2){
//		int elem;
//		// -1 indica que não existe aresta com aquele indice como origem no padrão
//		for(int i=0; i < numPts ; i++){
//			p->elements[i] = -1;
//			p->usage[i] = 0;
//		}
//		for(int i = 0; i < tam; i++){
//			int l = fscanf(fp, "%d", &elem);
//			if (l < 1){ cout << "ERROR: fscanf. nothing read." << endl; exit(1);}
//			int pos = elem/numPosCand;
//			p->elements[pos] = elem%numPosCand;
//		}
//		*listOfPatterns[nPatterns++] = *p;
//		r = fscanf(fp, "%d;%d;", &tam, &supp);
//		delete(p);
//		p = new Pattern();
//		p->support = supp;
//		p->size = tam;
//	}
	fclose(fp);
}

bool Mining::updateES(Solucao *&s){
	//if (s->custoTotal > worstCostES || sizeES < maxSizeES) {
		for(int i = 0 ; i < sizeES ; i++){
//			if(ES[i]->custoTotal == s->custoTotal ){
//				if(IsEqualVectorInt(ES[i]->vetPosCand, ES[i]->numPts, s->vetPosCand, s->numPts)){
//					// Se solucao já existir no CE, é descartada.
//					if(iterWNC > 0 || eschanged)
//						oneMoreIWC();
//					cout << "Solution already in ES" << endl;
//					return false;
//				}
//			}
		}
		eschanged = true;
		mined = false;
		iterWNC = 0;
//		if(sizeES < maxSizeES){
//			clonarSolucao(s, ES[sizeES++]);
//		}else{
//			worstCostES = ES[0]->custoTotal;
//			worstCostPos = 0;
//			for(int i = 1 ; i < sizeES ; i++){
//				if(ES[i]->custoTotal < worstCostES ){
//					worstCostES = ES[i]->custoTotal;
//					worstCostPos = i;
//				}
//			}
//			clonarSolucao(s, ES[worstCostPos]);
//
//		}
		// ES worst cost must be updated
//		worstCostES = ES[0]->custoTotal;
//		worstCostPos = 0;
//		for(int i = 1 ; i < sizeES ; i++){
//			if(ES[i]->custoTotal < worstCostES ){
//				worstCostES = ES[i]->custoTotal;
//				worstCostPos = i;
//			}
//		}
		return true;
	//}
	if(iterWNC > 0 || eschanged)
		oneMoreIWC();
	return false;
}

void Mining::printES(){
//	for(int i=0 ; i< sizeES ; i++){
//		cout << "Solução " << i << ":" << endl << "\t";
//		for(int j = 0 ; j < ES[i]->numPts; j++){
//			cout << numPosCand*j+ES[i]->vetPosCand[j] << " ";
//		}
//		cout << "Custo " << ES[i]->custoTotal << endl;
//	}
}

void Mining::printPatterns(){
//	for(int i=0 ; i< nPatterns ; i++){
//		//cout << "Padrão " << i << ":" << endl << "\t";
//		for(int j = 0 ; j < numPts; j++){
//			if(listOfPatterns[i]->elements[j] == -1)
//				cout << "- ";
//			else
//				cout << numPosCand*j+listOfPatterns[i]->elements[j]<< " ";
//		}
//		cout << endl;
//	}
}

//---------------------------------------------------------------------------
