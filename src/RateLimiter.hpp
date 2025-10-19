/**
 * BetterSave - Rate Limiter
 * Prevents DDOS and abuse
 * Created by: sidastuff
 */

#pragma once
#include <Geode/Geode.hpp>
#include <chrono>
#include <unordered_map>

using namespace geode::prelude;

class RateLimiter {
private:
    static RateLimiter* s_instance;
    
    struct RateLimit {
        int count = 0;
        std::chrono::system_clock::time_point windowStart;
    };
    
    std::unordered_map<std::string, RateLimit> m_limits;
    
public:
    static RateLimiter* get() {
        if (!s_instance) {
            s_instance = new RateLimiter();
        }
        return s_instance;
    }
    
    // Check if action is allowed
    // Returns true if allowed, false if rate limited
    bool checkLimit(const std::string& action, int maxRequests, int windowSeconds);
    
    // Reset a specific action's limit
    void reset(const std::string& action);
    
    // Clear all limits
    void clearAll();
};

