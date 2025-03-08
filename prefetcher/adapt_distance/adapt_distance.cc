// #include "adapt_distance.h"
// #include <unordered_map>

// // Correlation history table (maps pages to most frequently following blocks)
// std::unordered_map<champsim::page_number, champsim::block_number> correlation_table;
// std::unordered_map<champsim::page_number, uint8_t> confidence_table;

// // Function to extract page number and block offset from address
// template <typename T>
// auto adapt_distance::page_and_offset(T addr) -> std::pair<champsim::page_number, block_in_page> {
//     champsim::page_number page = addr >> LOG2_PAGE_SIZE; //extract page number by shifting R by page_size
//     block_in_page offset = addr & (PAGE_SIZE - 1); // get the block within the page
//     return {page, offset};
// }

// // Prefetcher logic
// uint32_t adapt_distance::prefetcher_cache_operate(champsim::address addr, champsim::address ip, uint8_t cache_hit, bool useful_prefetch, access_type type,
//                                                 uint32_t metadata_in) {
//     auto [page, block] = page_and_offset(addr); //extract page number and block offset

//     // If block access is observed, update correlation history
//     if (correlation_table.find(page) != correlation_table.end()) {
//         if (correlation_table[page] == block) { // page exists
//             confidence_table[page] = std::min(confidence_table[page] + 1, 10u);  // Increase confidence if correlation repeats
//         } else {  // increase the confidence for the page 
//             confidence_table[page] = std::max(confidence_table[page] - 1, 0u);  // Decrease confidence if pattern is inconsistent
//         }
//     } else {
//         correlation_table[page] = block;
//         confidence_table[page] = 5;  // Start with medium confidence
//     }

//     // If confidence is high, issue a prefetch
//     if (confidence_table[page] > 3) {
//         champsim::address pf_address = champsim::to_address(page, correlation_table[page]);
//         if (!check_cl_prefetch(pf_address)) {
//             return pf_address;  // Prefetch the correlated address
//         }
//     }

//     return 0;  // No prefetch issued
// }

// // Adaptive prefetch filtering
// bool adapt_distance::check_cl_prefetch(champsim::block_number v_addr) {
//     auto [page, block] = page_and_offset(v_addr);

//     // If confidence is low, do not prefetch
//     if (confidence_table.find(page) != confidence_table.end()) {
//         if (confidence_table[page] < 3) {
//             return false;  // Ignore low-confidence prefetches
//         }
//     }

//     return true;  // Allow prefetch if confidence is high
// }
