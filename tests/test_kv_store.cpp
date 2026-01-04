#include "kv_store.hpp"
#include <iostream>
#include <cassert>
#include <thread>
#include <vector>
#include <chrono>

using namespace kvstore;

// Test basic put and get operations
void test_basic_operations() {
    std::cout << "Running test_basic_operations..." << std::endl;
    
    KVStore store(100);
    
    // Test put and get
    assert(store.put("key1", "value1") == true);
    auto value = store.get("key1");
    assert(value.has_value());
    assert(value.value() == "value1");
    
    // Test update
    assert(store.put("key1", "value2") == true);
    value = store.get("key1");
    assert(value.has_value());
    assert(value.value() == "value2");
    
    // Test non-existent key
    value = store.get("nonexistent");
    assert(!value.has_value());
    
    std::cout << "✓ test_basic_operations passed" << std::endl;
}

// Test delete operation
void test_delete() {
    std::cout << "Running test_delete..." << std::endl;
    
    KVStore store(100);
    
    store.put("key1", "value1");
    assert(store.exists("key1") == true);
    assert(store.size() == 1);
    
    assert(store.del("key1") == true);
    assert(store.exists("key1") == false);
    assert(store.size() == 0);
    
    // Delete non-existent key
    assert(store.del("nonexistent") == false);
    
    std::cout << "✓ test_delete passed" << std::endl;
}

// Test LRU eviction
void test_lru_eviction() {
    std::cout << "Running test_lru_eviction..." << std::endl;
    
    KVStore store(3); // Small capacity for testing
    
    store.put("key1", "value1");
    store.put("key2", "value2");
    store.put("key3", "value3");
    assert(store.size() == 3);
    
    // Add a 4th key, should evict key1 (least recently used)
    store.put("key4", "value4");
    assert(store.size() == 3);
    assert(!store.exists("key1"));
    assert(store.exists("key2"));
    assert(store.exists("key3"));
    assert(store.exists("key4"));
    
    // Access key2 to make it recently used
    store.get("key2");
    
    // Add another key, should evict key3 (now LRU)
    store.put("key5", "value5");
    assert(store.size() == 3);
    assert(!store.exists("key3"));
    assert(store.exists("key2"));
    assert(store.exists("key4"));
    assert(store.exists("key5"));
    
    std::cout << "✓ test_lru_eviction passed" << std::endl;
}

// Test clear operation
void test_clear() {
    std::cout << "Running test_clear..." << std::endl;
    
    KVStore store(100);
    
    store.put("key1", "value1");
    store.put("key2", "value2");
    store.put("key3", "value3");
    assert(store.size() == 3);
    
    store.clear();
    assert(store.size() == 0);
    assert(!store.exists("key1"));
    assert(!store.exists("key2"));
    assert(!store.exists("key3"));
    
    std::cout << "✓ test_clear passed" << std::endl;
}

// Test thread safety
void test_thread_safety() {
    std::cout << "Running test_thread_safety..." << std::endl;
    
    KVStore store(1000);
    const int num_threads = 10;
    const int operations_per_thread = 100;
    
    auto worker = [&store](int thread_id) {
        for (int i = 0; i < operations_per_thread; ++i) {
            std::string key = "key_" + std::to_string(thread_id) + "_" + std::to_string(i);
            std::string value = "value_" + std::to_string(thread_id) + "_" + std::to_string(i);
            
            store.put(key, value);
            auto result = store.get(key);
            assert(result.has_value());
            assert(result.value() == value);
        }
    };
    
    std::vector<std::thread> threads;
    for (int i = 0; i < num_threads; ++i) {
        threads.emplace_back(worker, i);
    }
    
    for (auto& thread : threads) {
        thread.join();
    }
    
    assert(store.size() <= 1000); // Should not exceed capacity
    
    std::cout << "✓ test_thread_safety passed" << std::endl;
}

// Test WAL recovery
void test_wal_recovery() {
    std::cout << "Running test_wal_recovery..." << std::endl;
    
    const std::string wal_path = "test_wal.log";
    
    // Remove old WAL file if it exists
    std::remove(wal_path.c_str());
    
    {
        KVStore store(100, wal_path);
        store.put("key1", "value1");
        store.put("key2", "value2");
        store.put("key3", "value3");
    } // Store goes out of scope, WAL file should be written
    
    // Create new store and recover
    KVStore store2(100, wal_path);
    store2.recover();
    
    assert(store2.exists("key1"));
    assert(store2.exists("key2"));
    assert(store2.exists("key3"));
    
    auto value1 = store2.get("key1");
    assert(value1.has_value());
    assert(value1.value() == "value1");
    
    // Clean up
    std::remove(wal_path.c_str());
    
    std::cout << "✓ test_wal_recovery passed" << std::endl;
}

// Test performance (basic benchmark)
void test_performance() {
    std::cout << "Running test_performance..." << std::endl;
    
    KVStore store(10000);
    const int num_operations = 10000;
    
    // Measure PUT performance
    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < num_operations; ++i) {
        store.put("key" + std::to_string(i), "value" + std::to_string(i));
    }
    auto end = std::chrono::high_resolution_clock::now();
    auto put_duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    
    // Measure GET performance
    start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < num_operations; ++i) {
        store.get("key" + std::to_string(i));
    }
    end = std::chrono::high_resolution_clock::now();
    auto get_duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    
    std::cout << "  PUT: " << num_operations << " operations in " 
              << put_duration.count() << " μs ("
              << (double)put_duration.count() / num_operations << " μs/op)" << std::endl;
    std::cout << "  GET: " << num_operations << " operations in " 
              << get_duration.count() << " μs ("
              << (double)get_duration.count() / num_operations << " μs/op)" << std::endl;
    
    std::cout << "✓ test_performance passed" << std::endl;
}

int main() {
    std::cout << "=== Running KVStore Tests ===" << std::endl << std::endl;
    
    try {
        test_basic_operations();
        test_delete();
        test_lru_eviction();
        test_clear();
        test_thread_safety();
        test_wal_recovery();
        test_performance();
        
        std::cout << std::endl << "=== All tests passed! ===" << std::endl;
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Test failed with exception: " << e.what() << std::endl;
        return 1;
    }
}
