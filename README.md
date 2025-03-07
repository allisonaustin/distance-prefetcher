# Distance Prefetcher

This project evaluates an adaptive distance prefetcher on the following sparse-matrix and graph processing workloads + inputs: 
- SpMV on mc2depi
- BFS on graph500
- DFS on Higgs social network graph

# Download dependencies

ChampSim uses [vcpkg](https://vcpkg.io) to manage its dependencies. In this repository, vcpkg is included as a submodule. You can download the dependencies with
```
git submodule update --init
vcpkg/bootstrap-vcpkg.sh
vcpkg/vcpkg install
```

**Compile and test**
Add the prefetcher to the configuration file.
```
{
    "L2C": {
        "prefetcher": "mypref"
    }
}
```
Note that the example prefetcher is an L2 prefetcher. You might design a prefetcher for a different level.

# Run simulation
```
$ ./config.sh ./pref_configs/<configuration file>
$ make
$ bin/champsim --warmup_instructions 20000 --simulation_instructions 1000000 /traces/crono/dfs.champsimtrace.xz
```

# References
Gober, N., Chacon, G., Wang, L., Gratz, P. V., Jimenez, D. A., Teran, E., Pugsley, S., & Kim, J. (2022). The Championship Simulator: Architectural Simulation for Education and Competition. https://doi.org/10.48550/arXiv.2210.14324
