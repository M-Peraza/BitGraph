# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

BITSCAN_plus is a refactored C++ framework with three main libraries:
- **BITSCAN**: High-performance bitset manipulation library (1.0)
- **GRAPH**: Efficient graph operations using BITSCAN for adjacency representation (1.0) 
- **UTILS**: Common utilities, benchmarking, and logging framework

This is academic research code resulting from 20+ years of combinatorial optimization research, primarily focused on exact algorithms for NP-hard problems.

## Project Status & Build Issues

⚠️ **CURRENT STATUS**: The project has recently undergone a major merge that brought significant structural changes. While merge conflicts have been resolved, there are compilation issues that need attention.

### Recent Changes (After Merge)
- **Structure Change**: Libraries moved from root level to `src/` directory (new nested structure)
- **New Files Added**: Multiple new components including `info_analyser`, `result`, `test_analyser`, and additional test files
- **Namespace Updates**: Fixed `bitgraph::bit_ops` namespace in `bitscan/bbconfig.h`
- **Header Guards**: Fixed missing `#endif` statements in multiple header files

### Current Build Issues
The build system is partially working but has several compilation errors:

1. **Missing Type Definitions**: `BITBOARD` type not found in `bitgraph` namespace
2. **Duplicate Function Definitions**: Multiple definitions in `bbset_sparse.cpp`
3. **Constructor/Destructor Issues**: Missing or deleted constructors in `BBScan` class
4. **Missing Base Classes**: `infoBase` class not found for `infoCLQ` template

### Build System

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

**Note**: Current build will fail with compilation errors. See "Current Build Issues" above.

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

**Updated Structure (Post-Merge):**
```
src/
├── utils/ (common utilities, logging, benchmarking)
├── bitscan/ (bitset operations)
└── graph/ (graph algorithms using bitsets)
```

**Dependencies**: `utils` ← `bitscan` ← `graph` (utils is base dependency)

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

## Known Issues & TODO

### Post-Merge Compilation Issues
1. **Type System**: Need to restore `BITBOARD` and other core type definitions that may have been lost during merge
2. **Class Hierarchies**: Some base classes like `infoBase` are missing, breaking inheritance
3. **Duplicate Implementations**: Some functions are defined both in header files and source files
4. **Constructor/Destructor Chain**: Copy constructors and move constructors need review

### Immediate Action Items
- [ ] Locate and restore missing type definitions (especially `BITBOARD`)
- [ ] Review class inheritance hierarchies in `utils/info/` directory
- [ ] Resolve duplicate function definitions in `bbset_sparse.cpp`
- [ ] Verify all header include dependencies are correct

### Build Verification Status
- ✅ CMake configuration works
- ✅ Merge conflicts resolved
- ✅ Header guard issues fixed
- ❌ Compilation fails due to missing types and duplicate definitions

**Estimated Fix Time**: 2-4 hours of systematic debugging focusing on type definitions and class hierarchies.