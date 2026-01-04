#ifndef KV_STORE_HPP
#define KV_STORE_HPP

#include <string>
#include <unordered_map>
#include <list>
#include <mutex>
#include <optional>
#include <fstream>
#include <memory>

namespace kvstore {

/**
 * @brief A thread-safe, in-memory key-value store with LRU cache eviction and WAL support.
 * 
 * Features:
 * - O(1) get/put/delete operations using hash-based indexing
 * - LRU (Least Recently Used) eviction policy with configurable capacity
 * - Thread-safe operations using mutex-based locking
 * - Write-Ahead Logging (WAL) for durability
 */
class KVStore {
public:
    /**
     * @brief Construct a new KVStore object
     * 
     * @param max_capacity Maximum number of key-value pairs to store (LRU eviction when exceeded)
     * @param wal_path Path to the write-ahead log file (empty string disables WAL)
     */
    explicit KVStore(size_t max_capacity = 1000, const std::string& wal_path = "");

    /**
     * @brief Destroy the KVStore object and close WAL file
     */
    ~KVStore();

    /**
     * @brief Insert or update a key-value pair
     * 
     * @param key The key to store
     * @param value The value to associate with the key
     * @return true if operation succeeded
     */
    bool put(const std::string& key, const std::string& value);

    /**
     * @brief Retrieve a value by key
     * 
     * @param key The key to look up
     * @return std::optional<std::string> The value if found, std::nullopt otherwise
     */
    std::optional<std::string> get(const std::string& key);

    /**
     * @brief Delete a key-value pair
     * 
     * @param key The key to delete
     * @return true if the key was found and deleted, false otherwise
     */
    bool del(const std::string& key);

    /**
     * @brief Check if a key exists in the store
     * 
     * @param key The key to check
     * @return true if the key exists, false otherwise
     */
    bool exists(const std::string& key) const;

    /**
     * @brief Get the current number of key-value pairs stored
     * 
     * @return size_t Number of entries
     */
    size_t size() const;

    /**
     * @brief Clear all key-value pairs from the store
     */
    void clear();

    /**
     * @brief Recover data from the write-ahead log
     * 
     * @return true if recovery succeeded, false otherwise
     */
    bool recover();

private:
    // LRU cache node: stores key in a doubly-linked list
    using KeyList = std::list<std::string>;
    using KeyListIterator = KeyList::iterator;

    // Cache entry: stores value and iterator to position in LRU list
    struct CacheEntry {
        std::string value;
        KeyListIterator lru_iter;
    };

    // Hash map for O(1) lookups
    std::unordered_map<std::string, CacheEntry> cache_;
    
    // LRU list: most recently used at front, least recently used at back
    KeyList lru_list_;
    
    // Maximum capacity before eviction
    size_t max_capacity_;
    
    // Thread safety
    mutable std::mutex mutex_;
    
    // Write-ahead log
    std::string wal_path_;
    std::unique_ptr<std::ofstream> wal_file_;
    
    /**
     * @brief Touch a key to mark it as recently used (move to front of LRU list)
     * 
     * @param key The key to touch
     */
    void touch(const std::string& key);
    
    /**
     * @brief Evict the least recently used entry
     */
    void evict_lru();
    
    /**
     * @brief Write an operation to the write-ahead log
     * 
     * @param operation The operation type ("PUT", "DEL", "CLEAR")
     * @param key The key (empty for CLEAR)
     * @param value The value (empty for DEL and CLEAR)
     */
    void write_wal(const std::string& operation, const std::string& key, const std::string& value = "");
};

} // namespace kvstore

#endif // KV_STORE_HPP
