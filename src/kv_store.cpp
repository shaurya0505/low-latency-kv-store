#include "kv_store.hpp"
#include <sstream>
#include <iostream>

namespace kvstore {

KVStore::KVStore(size_t max_capacity, const std::string& wal_path)
    : max_capacity_(max_capacity), wal_path_(wal_path) {
    if (!wal_path_.empty()) {
        wal_file_ = std::make_unique<std::ofstream>(wal_path_, std::ios::app);
        if (!wal_file_->is_open()) {
            std::cerr << "Warning: Failed to open WAL file: " << wal_path_ << std::endl;
            wal_file_.reset();
        }
    }
}

KVStore::~KVStore() {
    if (wal_file_ && wal_file_->is_open()) {
        wal_file_->close();
    }
}

bool KVStore::put(const std::string& key, const std::string& value) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto it = cache_.find(key);
    
    if (it != cache_.end()) {
        // Key exists, update value and move to front
        it->second.value = value;
        lru_list_.erase(it->second.lru_iter);
        lru_list_.push_front(key);
        it->second.lru_iter = lru_list_.begin();
    } else {
        // New key
        if (cache_.size() >= max_capacity_) {
            evict_lru();
        }
        
        lru_list_.push_front(key);
        cache_[key] = CacheEntry{value, lru_list_.begin()};
    }
    
    write_wal("PUT", key, value);
    return true;
}

std::optional<std::string> KVStore::get(const std::string& key) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto it = cache_.find(key);
    if (it == cache_.end()) {
        return std::nullopt;
    }
    
    // Move to front of LRU list
    lru_list_.erase(it->second.lru_iter);
    lru_list_.push_front(key);
    it->second.lru_iter = lru_list_.begin();
    
    return it->second.value;
}

bool KVStore::del(const std::string& key) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto it = cache_.find(key);
    if (it == cache_.end()) {
        return false;
    }
    
    lru_list_.erase(it->second.lru_iter);
    cache_.erase(it);
    
    write_wal("DEL", key);
    return true;
}

bool KVStore::exists(const std::string& key) const {
    std::lock_guard<std::mutex> lock(mutex_);
    return cache_.find(key) != cache_.end();
}

size_t KVStore::size() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return cache_.size();
}

void KVStore::clear() {
    std::lock_guard<std::mutex> lock(mutex_);
    cache_.clear();
    lru_list_.clear();
    write_wal("CLEAR", "");
}

bool KVStore::recover() {
    if (wal_path_.empty()) {
        return false;
    }
    
    std::ifstream wal_in(wal_path_);
    if (!wal_in.is_open()) {
        return false;
    }
    
    // Temporarily disable WAL during recovery to avoid duplicate writes
    auto temp_wal = std::move(wal_file_);
    
    std::string line;
    while (std::getline(wal_in, line)) {
        std::istringstream iss(line);
        std::string op, key, value;
        
        if (!(iss >> op)) {
            continue;
        }
        
        if (op == "PUT") {
            if (!(iss >> key)) {
                continue;
            }
            std::getline(iss, value);
            if (!value.empty() && value[0] == ' ') {
                value = value.substr(1); // Remove leading space
            }
            put(key, value);
        } else if (op == "DEL") {
            if (!(iss >> key)) {
                continue;
            }
            del(key);
        } else if (op == "CLEAR") {
            clear();
        }
    }
    
    wal_in.close();
    
    // Re-enable WAL
    wal_file_ = std::move(temp_wal);
    
    return true;
}

void KVStore::touch(const std::string& key) {
    auto it = cache_.find(key);
    if (it != cache_.end()) {
        lru_list_.erase(it->second.lru_iter);
        lru_list_.push_front(key);
        it->second.lru_iter = lru_list_.begin();
    }
}

void KVStore::evict_lru() {
    if (lru_list_.empty()) {
        return;
    }
    
    const std::string& lru_key = lru_list_.back();
    cache_.erase(lru_key);
    lru_list_.pop_back();
}

void KVStore::write_wal(const std::string& operation, const std::string& key, const std::string& value) {
    if (!wal_file_ || !wal_file_->is_open()) {
        return;
    }
    
    *wal_file_ << operation << " " << key;
    if (!value.empty()) {
        *wal_file_ << " " << value;
    }
    *wal_file_ << std::endl;
    wal_file_->flush(); // Ensure durability
}

} // namespace kvstore
