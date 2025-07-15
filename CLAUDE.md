# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

BITSCAN_plus is a refactored C++ framework with three main libraries:
- **BITSCAN**: High-performance bitset manipulation library (1.0)
- **GRAPH**: Efficient graph operations using BITSCAN for adjacency representation (1.0) 
- **UTILS**: Common utilities, benchmarking, and logging framework

This is academic research code resulting from 20+ years of combinatorial optimization research, primarily focused on exact algorithms for NP-hard problems.

## Build System

Uses CMake with C++11 standard. Build commands:

```bash
# Configure (from repository root)
cmake -B build

# Build all libraries and tests
cmake --build build

# Build specific targets  
cmake --build build --target bitscan
cmake --build build --target graph
cmake --build build --target utils
```

### Running Tests

```bash
# Run all tests (executables in build/bin/tests/)
./build/bin/tests/test_bitscan
./build/bin/tests/test_graph  
./build/bin/tests/test_utils

# Run examples (executables in build/bin/)
./build/bin/gen_random_benchmark
./build/bin/graph_formats
./build/bin/kcore
```

### CMake Configuration Options

- `LOG_LEVEL`: Set to "debug", "verbose", or "error" (configures utils/logger.h)
- `*_build_tests`: Enable/disable test building for each library
- `*_build_examples`: Enable/disable example building

## Architecture

### Library Hierarchy

```
utils (common utilities, logging, benchmarking)
├── bitscan (bitset operations)
│   └── graph (graph algorithms using bitsets)
```

### Key Type Aliases

**BITSCAN types:**
- `bitarray` (BBScan): Main bitset type
- `sparse_bitarray` (BBScanSp): For large sparse sets
- `simple_bitarray` (BitSet): Basic bitset operations
- `watched_bitarray` (BBSentinel): For low-density populations

**GRAPH types:**
- `ugraph`: Undirected graphs using bitarray
- `sparse_ugraph`: Undirected graphs using sparse_bitarray  
- `ugraph_w`/`ugraph_wi`: Vertex-weighted graphs (double/int)
- `ugraph_ew`/`ugraph_ewi`: Edge-weighted graphs (double/int)

### Core Headers

Include these main headers in your code:
- `#include "bitscan/bitscan.h"` - All BITSCAN functionality
- `#include "graph/graph.h"` - All GRAPH functionality  
- `#include "utils/common.h"` - Utilities and logging

### Data Paths

Test data locations are configured via CMake:
- Graph test data: `PATH_GRAPH_TESTS_CMAKE_SRC_CODE` → `graph/data/`
- Uses absolute paths defined at build time

## Key Patterns

### Bitscanning Operations

```cpp
bitarray bba(100);
bba.set_bit(10);

// Standard enumeration
bba.init_scan(bbo::NON_DESTRUCTIVE);
int nBit;
while((nBit = bba.next_bit()) != BBObject::noBit) {
    // Process nBit
}

// Destructive scan (removes bits as scanned)
bba.init_scan(bbo::DESTRUCTIVE);
while((nBit = bba.next_bit_del()) != BBObject::noBit) {
    // Process and remove nBit
}
```

### Graph Operations

```cpp
ugraph g(100);           // 100-vertex undirected graph
g.add_edge(0, 1);        // Add edges
g.remove_edge(0, 1);     // Remove edges
g.make_bidirected();     // Convert directed to undirected

// File I/O (supports DIMACS, MTX, EDGES formats)
ugraph g2("graph/data/brock200_1.clq");
```

### Logging

```cpp
#include "utils/logger.h"

LOGG_DEBUG("Debug message: ", value);
LOGG_INFO("Info message");  
LOGG_ERROR("Error: ", error_details);
```

## File Formats

Supports standard graph formats:
- **DIMACS** (.clq, .col): Standard format for clique/coloring problems
- **Matrix Market** (.mtx): Sparse matrix format
- **Edge list** (.edges): Simple text format with edge pairs

## Development Notes

- All output binaries go to `build/bin/` (examples) or `build/bin/tests/` (tests)
- Libraries built as static libraries in `build/lib/`
- Platform support: Windows 64-bit and Linux (64-bit mandatory for BITSCAN)
- Hardware requirements: Supports POPCOUNT_64 assembly instructions (configurable in bbconfig.h)