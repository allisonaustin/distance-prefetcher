#include "adapt_distance.h"
#include <algorithm>
#include "cache.h"

/* Function to extract page number and block offset from address */
template <typename T>
auto adapt_distance::page_and_offset(T addr) -> std::pair<champsim::page_number, block_in_page> 
{
    return std::pair{champsim::page_number{addr}, block_in_page{addr}};
}

bool adapt_distance::check_cl_access(champsim::block_number v_addr)
{
  auto [vpn, page_offset] = page_and_offset(v_addr);
  auto region = regions.check_hit(region_type{vpn});

  return (region.has_value() && region->access_map.at(page_offset.to<std::size_t>()));
}

bool adapt_distance::check_cl_prefetch(champsim::block_number v_addr) 
{
    auto [vpn, page_offset] = page_and_offset(v_addr);
    auto region = regions.check_hit(region_type{vpn});

    return (region.has_value() && region->prefetch_map.at(page_offset.to<std::size_t>()));
}

void adapt_distance::update_confidence(champsim::page_number vpn, bool prefetch_success) {
    auto key = vpn.to<uint64_t>();
    // if the page is not in the table, initialize it with a low confidence level
    if (confidence_table.find(key) == confidence_table.end()) {
        confidence_table[key] = 1;  // low confidence initially
    }

    // updating confidence based on prefetch success or failure
    if (prefetch_success) {
        confidence_table[key] = std::min(255, confidence_table[key] + 1);  // increase confidence
    } else {
        confidence_table[key] = std::max(0, confidence_table[key] - 1);  // decrease confidence
    }
}

/* prefetcher logic */ 
uint32_t adapt_distance::prefetcher_cache_operate(champsim::address addr, champsim::address ip, uint8_t cache_hit, bool useful_prefetch, access_type type,
                                                uint32_t metadata_in) 
{
    auto [current_vpn, page_offset] = page_and_offset(addr);
    champsim::block_number block_addr{addr};
    auto demand_region = regions.check_hit(region_type{current_vpn});

    if (!demand_region.has_value()) {
        regions.fill(region_type{current_vpn});
        return metadata_in;
    }
    demand_region->access_map.at(page_offset.to<std::size_t>()) = true;
    regions.fill(demand_region.value());

    // before attempt to prefetch, checking degree of confidence
    auto set = current_vpn.to<uint64_t>();
    uint8_t confidence = confidence_table[set];
    uint8_t max_prefetch_degree = (confidence > 5) ? PREFETCH_DEGREE : 1; // increase prefetch degree if confidence is high

    for (auto direction : {1, -1}) {
        for (int i = 1, prefetches_issued = 0; i <= MAX_DISTANCE && prefetches_issued < max_prefetch_degree; i++) {
        const auto pos_step_addr = block_addr + (direction * i);
        const auto neg_step_addr = block_addr - (direction * i);
        const auto neg_2step_addr = block_addr - (direction * 2 * i);

        if (check_cl_access(neg_step_addr) && check_cl_access(neg_2step_addr) && !check_cl_access(pos_step_addr) && !check_cl_prefetch(pos_step_addr)) {
            // found something that we should prefetch
            if (block_addr != champsim::block_number{pos_step_addr}) {
            champsim::address pf_addr{pos_step_addr};
            if (bool prefetch_success = prefetch_line(pf_addr, (intern_->get_mshr_occupancy_ratio() < 0.5), metadata_in); prefetch_success) {
                // update confidence
                update_confidence(current_vpn, prefetch_success);
                auto [pf_vpn, pf_page_offset] = page_and_offset(pos_step_addr);
                auto pf_region = regions.check_hit(region_type{pf_vpn});

                if (!pf_region.has_value()) {
                    // we're not currently tracking this region, so allocate a new region so we can mark it
                    region_type new_region{pf_vpn};
                    new_region.prefetch_map.at(pf_page_offset.to<std::size_t>()) = true;
                    regions.fill(new_region);
                } else {
                    pf_region.value().prefetch_map.at(pf_page_offset.to<std::size_t>()) = true;
                    regions.fill(pf_region.value());
                }
                prefetches_issued++;
            }
            }
        }
        }
    }

  return metadata_in;
}

uint32_t adapt_distance::prefetcher_cache_fill(champsim::address addr, long set, long way, uint8_t prefetch, champsim::address evicted_addr, uint32_t metadata_in)
{
  return metadata_in;
}
