#include "kv_store.hpp"
#include <iostream>
#include <string>

using namespace kvstore;

void print_separator() {
    std::cout << "----------------------------------------" << std::endl;
}

int main() {
    std::cout << "=== Low-Latency Key-Value Store Demo ===" << std::endl << std::endl;
    
    // Create a KV store with capacity of 5 and WAL enabled
    std::cout << "Creating KVStore with capacity=5 and WAL enabled..." << std::endl;
    KVStore store(5, "demo_wal.log");
    print_separator();
    
    // Basic PUT and GET operations
    std::cout << "1. Basic PUT and GET operations:" << std::endl;
    store.put("name", "Alice");
    store.put("age", "30");
    store.put("city", "New York");
    
    std::cout << "   PUT: name=Alice, age=30, city=New York" << std::endl;
    
    auto name = store.get("name");
    auto age = store.get("age");
    auto city = store.get("city");
    
    if (name) std::cout << "   GET name: " << *name << std::endl;
    if (age) std::cout << "   GET age: " << *age << std::endl;
    if (city) std::cout << "   GET city: " << *city << std::endl;
    std::cout << "   Store size: " << store.size() << std::endl;
    print_separator();
    
    // Update operation
    std::cout << "2. Update existing key:" << std::endl;
    store.put("age", "31");
    age = store.get("age");
    if (age) std::cout << "   Updated age: " << *age << std::endl;
    print_separator();
    
    // DELETE operation
    std::cout << "3. DELETE operation:" << std::endl;
    std::cout << "   EXISTS city: " << (store.exists("city") ? "true" : "false") << std::endl;
    store.del("city");
    std::cout << "   After DEL, EXISTS city: " << (store.exists("city") ? "true" : "false") << std::endl;
    std::cout << "   Store size: " << store.size() << std::endl;
    print_separator();
    
    // LRU eviction demonstration
    std::cout << "4. LRU eviction (capacity=5):" << std::endl;
    store.put("key1", "value1");
    store.put("key2", "value2");
    store.put("key3", "value3");
    store.put("key4", "value4");
    std::cout << "   Added 4 keys. Current size: " << store.size() << std::endl;
    std::cout << "   Keys: name, age, key1, key2, key3, key4" << std::endl;
    
    // This should evict "name" (least recently used)
    store.put("key5", "value5");
    std::cout << "   Added key5. Size: " << store.size() << " (capacity reached)" << std::endl;
    std::cout << "   EXISTS name (should be evicted): " << (store.exists("name") ? "true" : "false") << std::endl;
    std::cout << "   EXISTS age (should exist): " << (store.exists("age") ? "true" : "false") << std::endl;
    print_separator();
    
    // Access a key to refresh it in LRU
    std::cout << "5. LRU refresh by accessing:" << std::endl;
    store.get("age"); // Make "age" recently used
    store.put("key6", "value6"); // This should evict key1 instead of age
    std::cout << "   Accessed 'age' then added key6" << std::endl;
    std::cout << "   EXISTS age (should still exist): " << (store.exists("age") ? "true" : "false") << std::endl;
    std::cout << "   EXISTS key1 (should be evicted): " << (store.exists("key1") ? "true" : "false") << std::endl;
    print_separator();
    
    // Show WAL persistence
    std::cout << "6. Write-Ahead Log (WAL):" << std::endl;
    std::cout << "   All operations have been logged to 'demo_wal.log'" << std::endl;
    std::cout << "   The store can be recovered from this log on restart" << std::endl;
    print_separator();
    
    std::cout << std::endl << "Demo completed successfully!" << std::endl;
    std::cout << "Final store size: " << store.size() << std::endl;
    
    return 0;
}
