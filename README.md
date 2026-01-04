# Low-Latency Key-Value Store

An in-memory key-value storage engine in C++ using hash-based indexing and LRU cache management to achieve constant-time operations, with thread-safe concurrency control and write-ahead logging.

## Features

- **O(1) Operations**: Hash-based indexing provides constant-time get, put, and delete operations
- **LRU Cache Management**: Automatic eviction of least recently used entries when capacity is reached
- **Thread-Safe**: Mutex-based locking ensures safe concurrent access from multiple threads
- **Write-Ahead Logging (WAL)**: Optional persistence layer for crash recovery
- **Low Latency**: Optimized for performance with minimal overhead

## Architecture

### Components

1. **Hash Map**: `std::unordered_map` provides O(1) average-case lookup, insert, and delete
2. **LRU List**: Doubly-linked list (`std::list`) tracks access order for eviction
3. **Mutex Lock**: `std::mutex` ensures thread-safe operations
4. **WAL**: Optional append-only log file for durability

### Design Decisions

- **In-Memory Storage**: All data kept in RAM for maximum performance
- **LRU Eviction**: When capacity is reached, least recently used entries are removed
- **WAL Format**: Simple text-based format for easy debugging and recovery

## Building

### Prerequisites

- CMake 3.10 or higher
- C++17 compatible compiler (GCC 7+, Clang 5+, MSVC 2017+)
- pthread library (on Unix systems)

### Build Instructions

```bash
# Create build directory
mkdir build && cd build

# Configure with CMake
cmake ..

# Build
make

# Run tests
./kv_tests

# Run example
./kv_example
```

## Usage

### Basic Example

```cpp
#include "kv_store.hpp"
#include <iostream>

int main() {
    // Create store with capacity of 1000 entries
    kvstore::KVStore store(1000);
    
    // Put key-value pairs
    store.put("username", "alice");
    store.put("email", "alice@example.com");
    
    // Get values
    auto username = store.get("username");
    if (username) {
        std::cout << "Username: " << *username << std::endl;
    }
    
    // Check existence
    if (store.exists("email")) {
        std::cout << "Email exists" << std::endl;
    }
    
    // Delete a key
    store.del("email");
    
    // Get store size
    std::cout << "Store size: " << store.size() << std::endl;
    
    return 0;
}
```

### With WAL (Write-Ahead Logging)

```cpp
#include "kv_store.hpp"

int main() {
    // Create store with WAL enabled
    kvstore::KVStore store(1000, "data.wal");
    
    // Operations are logged to WAL
    store.put("key1", "value1");
    store.put("key2", "value2");
    
    // On restart, recover from WAL
    kvstore::KVStore recovered_store(1000, "data.wal");
    recovered_store.recover();
    
    return 0;
}
```

## API Reference

### Constructor

```cpp
KVStore(size_t max_capacity = 1000, const std::string& wal_path = "")
```

- `max_capacity`: Maximum number of entries before LRU eviction
- `wal_path`: Path to WAL file (empty string disables WAL)

### Methods

#### `bool put(const std::string& key, const std::string& value)`

Insert or update a key-value pair. Returns true on success.

#### `std::optional<std::string> get(const std::string& key)`

Retrieve a value by key. Returns `std::nullopt` if key doesn't exist.

#### `bool del(const std::string& key)`

Delete a key-value pair. Returns true if key was found and deleted.

#### `bool exists(const std::string& key) const`

Check if a key exists. Returns true if key is present.

#### `size_t size() const`

Get the current number of key-value pairs.

#### `void clear()`

Remove all key-value pairs from the store.

#### `bool recover()`

Recover data from the write-ahead log. Returns true on success.

## Performance

Typical performance on modern hardware:

- **PUT**: ~0.5-2 μs per operation
- **GET**: ~0.3-1 μs per operation
- **DEL**: ~0.5-2 μs per operation

All operations maintain O(1) average-case complexity.

## Thread Safety

The KVStore is fully thread-safe. Multiple threads can safely perform concurrent operations. Internal mutex locking ensures data consistency.

## Testing

Run the test suite:

```bash
cd build
./kv_tests
```

Tests cover:
- Basic operations (put, get, delete)
- LRU eviction behavior
- Thread safety with concurrent access
- WAL recovery
- Performance benchmarks

## License

MIT License - See LICENSE file for details

## Contributing

Contributions are welcome! Please feel free to submit a Pull Request.
