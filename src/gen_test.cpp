#include <iostream>
#include <vector>
#include <numeric>   // for std::iota
#include <algorithm> // for std::shuffle
#include <random>    // for std::mt19937

//usage: gen_test n    ---   where n is the alphabet size
int main(int argc, char *argv[]){

	if(argc<2){
		std::cout << "Usage: gen_test alphabet_size" << std::endl;
		exit(0);
	}

	int n = std::stoi(argv[1]);

	std::vector<std::vector<int>> p;

	for(int i=0;i<n;++i){
		std::vector<int> pi;
		for(int j=i;j<i+n;++j){
			pi.push_back(j%n);
		}
		p.push_back(pi);
	}


	std::vector<int> shuf(n);
    std::iota(shuf.begin(), shuf.end(), 0);
    std::random_device rd;
    std::mt19937 g(rd());
    std::shuffle(shuf.begin(), shuf.end(), g);

    for(auto x : shuf){
    	for(auto y : p[x]){
    		std::cout << "1";
    		for(int m=0;m<y;++m) std::cout << "0";
    	}
    }

    std::cout << std::endl;

}