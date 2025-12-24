#include <iostream>
#include <vector>
#include <fstream>
#include <cstdint>
#include <algorithm>
#include <set>
#include "libsais.h"

std::string K (std::string&);

std::vector<std::pair<uint32_t, uint32_t>> periods(
	const std::string& T,
	const std::vector<int32_t>& SA,
	const std::vector<int32_t>& LCP 
);

std::string merge(
	const std::string& T,
	const std::vector<std::pair<uint32_t, uint32_t>>& p
);


void process(std::string T, uint32_t k){

	while(k > 0 and T.length()>0){

		T = K(T);
		--k;
	}

	std::cout << T;

}

//usage: stream | kernelize k    ---   where k>=0 is the number of applications of the kernelization function
int main(int argc, char *argv[]) {

	if(argc>2){
		std::cout << "Usage: kernelize [k]  (input/output: stdin/stdout; k>=0 is the number of applications of kernelization function; default: k=1)" << std::endl;
		exit(0);
	}

	uint32_t k = 1;

	if(argc>1)
		k = std::stoi(argv[1]);
	
	std::string T;

	T.assign(
		std::istreambuf_iterator<char>(std::cin),
		std::istreambuf_iterator<char>()
	);

	process(T,k);

}


std::string K(std::string& T){
	
	uint32_t n = T.size();

	// step 1 compute SA, LCP

	std::vector<int32_t> SA(n);
	std::vector<int32_t> LCP(n);
	
	{
	std::vector<int32_t> PLCP(n);
	//std::cout << "Building SA and LCP of T (" << n << " bytes)..." << std::endl;
	libsais((uint8_t*)T.data(), SA.data(), n, 0, nullptr);
	libsais_plcp((uint8_t*)T.data(), SA.data(), PLCP.data(), n);
	libsais_lcp(PLCP.data(), SA.data(), LCP.data(), n);
	//std::cout << "Success!" << std::endl;
	}

	// compute period of each SMR of T

	//std::cout << "Computing periods of SMR of T..." << std::endl;
	auto R = periods(T,SA,LCP);
	//std::cout << "Success! found " << R.size() << " periods." << std::endl;
	
	//std::cout << " *************  Periods **************" << std::endl;
	//for(auto p:R)
		//std::cout << std::string_view(&T[p.first], p.second - p.first + 1) << std::endl << std::endl;

	std::string res;

	// step 3 extract characters avoiding overlaps

	return merge(T, R); 

}

//assumes character 0 does not appear in T
std::string merge(
	const std::string& T,
	const std::vector<std::pair<uint32_t, uint32_t>>& p
	) {

	std::string res;
	if (p.empty()) return res;

	// Pre-allocate pi vector to reuse memory and avoid reallocations
	std::vector<int> pi;

	for (const auto& range : p) {
		uint32_t start = range.first;
		uint32_t end = range.second;
		if (start > end || start >= T.length()) continue;

		size_t m = end - start + 1;
		if (res.empty()) {
			res.append(T, start, m);
			continue;
		}

		size_t n = res.length();
		size_t max_possible_overlap = std::min(n, m);

		// Construct: [New Segment] + 0 + [Suffix of res]
		std::string combine;
		combine.reserve(m + 1 + max_possible_overlap);
		combine.append(T, start, m);
		combine.push_back(0);
		combine.append(res, n - max_possible_overlap, max_possible_overlap);

		// Compute KMP prefix function (linear time)
		pi.assign(combine.length(), 0);
		for (int i = 1, j = 0; i < (int)combine.length(); i++) {
			while (j > 0 && combine[i] != combine[j])
				j = pi[j - 1];
			if (combine[i] == combine[j])
				j++;
			pi[i] = j;
		}

		// The last element in pi gives the longest prefix of segment that is a suffix of res
		size_t overlap = pi.back();
		
		// Append only the non-overlapping suffix
		res.append(T, start + overlap, m - overlap);
	}
	return res;
}


/**
 * Computes the period of every supermaximal repeat.
 * Results are returned as (start_position, end_position) where end is inclusive.
 */
std::vector<std::pair<uint32_t, uint32_t>> periods(
	const std::string& T,
	const std::vector<int32_t>& SA,
	const std::vector<int32_t>& LCP
) {
	uint32_t n = static_cast<uint32_t>(T.size());
	std::vector<std::pair<uint32_t, uint32_t>> R;

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
					// --- KMP-based period detection ---
					uint32_t len = current_lcp;
					uint32_t period_len = len;
					
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
						period_len = len - longest_border;
							
					}

					// Store as (start, end) where end is inclusive
					R.push_back({leftmost_pos, leftmost_pos + period_len - 1});
				}
			}
			i = j - 1; 
		}
	}

	// Sort results by start position
	std::sort(R.begin(), R.end());

	return R;
}