#include <iostream>
#include <vector>
#include <fstream>
#include <cstdint>
#include <algorithm>
#include <set>
#include "libsais.h"

void K (std::string&,std::string&);

void rts(
	const std::string& T,
	const std::vector<int32_t>& SA,
	const std::vector<int32_t>& LCP,
	std::vector<std::pair<uint32_t, uint32_t>>& R 
);

//version reading stdin
int main() {
	
	std::string T;

	T.assign(
		std::istreambuf_iterator<char>(std::cin),
		std::istreambuf_iterator<char>()
	);

	std::string ker;
	K(T,ker);

	std::cout << "string and its kernel:" << std::endl;
	std::cout << T << std::endl;
	std::cout << ker << std::endl;

	return 0;
}

//version reading a file
/*
int main(int argc, char* argv[]) {
	if (argc < 2) {
		std::cerr << "Usage: " << argv[0] << " <filename>" << std::endl;
		return 1;
	}

	std::ifstream file(argv[1], std::ios::binary | std::ios::ate);
	std::streamsize n = file.tellg();
	file.seekg(0, std::ios::beg);
	std::string T(n);
	std::string ker;
	
	file.read(reinterpret_cast<char*>(T.data()), n);

	K(T,ker);

	return 0;
}
*/



void K(std::string& T, std::string& ker){
	
	uint32_t n = T.size();

	// step 1 compute SA, LCP

	std::vector<int32_t> SA(n);
	std::vector<int32_t> LCP(n);
	
	{
	std::vector<int32_t> PLCP(n);
	std::cout << "Building SA and LCP of T (" << n << " bytes)..." << std::endl;
	libsais((uint8_t*)T.data(), SA.data(), n, 0, nullptr);
	libsais_plcp((uint8_t*)T.data(), SA.data(), PLCP.data(), n);
	libsais_lcp(PLCP.data(), SA.data(), LCP.data(), n);
	std::cout << "Success!" << std::endl;
	}

	// compute SMR of T

	std::vector<std::pair<uint32_t, uint32_t>> R;


	std::cout << "Computing roots of SMR of T..." << std::endl;
	rts(T,SA,LCP,R);
	std::cout << "Success! found " << R.size() << " roots." << std::endl;

	for(auto p:R) std::cout << p.first << "," << p.second << std::endl;

	std::string res;

	// step 3 extract characters avoiding overlaps

	ker.clear();

	uint32_t j = 0; //last extracted character was j-1

	for(auto p : R){

		for(uint32_t i = std::max(j,p.first); i<=p.second; ++i)
			ker.push_back(T[i]);

		j = p.second+1;

	}
	
}


/**
 * Computes the primitive root of every supermaximal repeat.
 * Results are stored as (start_position, end_position) where end is inclusive.
 */
void rts(
	const std::string& T,
	const std::vector<int32_t>& SA,
	const std::vector<int32_t>& LCP,
	std::vector<std::pair<uint32_t, uint32_t>>& R
) {
	uint32_t n = static_cast<uint32_t>(T.size());
	R.clear();

	for (uint32_t i = 0; i < n; ++i) {
		// Identify an LCP-interval [i, j]
		if (i + 1 < n && LCP[i + 1] > 0) {
			uint32_t j = i + 1;
			uint32_t current_lcp = LCP[j];
			
			while (j + 1 < n && LCP[j + 1] == current_lcp) {
				j++;
			}

			// Local Maximum Check (Right-maximality)
			bool left_boundary = (i == 0 || LCP[i] < current_lcp);
			bool right_boundary = (j + 1 == n || LCP[j + 1] < current_lcp);

			if (left_boundary && right_boundary) {
				std::set<uint8_t> left_chars;
				bool all_distinct = true;
				uint32_t leftmost_pos = n;

				// Verify Left-Maximality and find leftmost position
				for (uint32_t k = i; k <= j; ++k) {
					uint32_t pos = SA[k];
					if (pos < leftmost_pos) leftmost_pos = pos;

					uint8_t c = (pos == 0) ? 0x00 : T[pos - 1];
					
					if (left_chars.count(c)) {
						all_distinct = false;
						break;
					}
					left_chars.insert(c);
				}

				if (all_distinct && left_chars.size() > 1) {
					// --- KMP-based Primitive Root Detection ---
					uint32_t len = current_lcp;
					uint32_t root_len = len;
					
					if (len > 1) {
						std::vector<uint32_t> pi(len, 0);
						for (uint32_t k = 1; k < len; k++) {
							uint32_t m = pi[k - 1];
							while (m > 0 && T[leftmost_pos + k] != T[leftmost_pos + m]) {
								m = pi[m - 1];
							}
							if (T[leftmost_pos + k] == T[leftmost_pos + m]) {
								m++;
							}
							pi[k] = m;
						}
						
						uint32_t longest_border = pi[len - 1];
						uint32_t period = len - longest_border;
						if (len % period == 0) {
							root_len = period;
						}
					}

					// Store as (start, end) where end is inclusive
					R.push_back({leftmost_pos, leftmost_pos + root_len - 1});
				}
			}
			i = j - 1; 
		}
	}

	// Sort results by start position
	std::sort(R.begin(), R.end());
}