/**
 * BetterSave - Rate Limiter
 * Prevents DDOS and abuse
 * Created by: sidastuff
 */

#include "RateLimiter.hpp"
#include "BetterSaveLogger.hpp"

RateLimiter* RateLimiter::s_instance = nullptr;

bool RateLimiter::checkLimit(const std::string& action, int maxRequests, int windowSeconds) {
    auto now = std::chrono::system_clock::now();
    
    // Check if we have a limit for this action
    if (m_limits.find(action) == m_limits.end()) {
        // First request, create new limit
        m_limits[action] = {1, now};
        return true;
    }
    
    auto& limit = m_limits[action];
    auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - limit.windowStart);
    
    // Check if window has expired
    if (elapsed.count() >= windowSeconds) {
        // Reset window
        limit.count = 1;
        limit.windowStart = now;
        return true;
    }
    
    // Within window, check count
    if (limit.count >= maxRequests) {
        BetterSaveLogger::get()->error("RateLimit", 
            fmt::format("Rate limit exceeded for {}: {} requests in {} seconds", 
                action, limit.count, elapsed.count()));
        return false;
    }
    
    // Increment and allow
    limit.count++;
    return true;
}

void RateLimiter::reset(const std::string& action) {
    m_limits.erase(action);
}

void RateLimiter::clearAll() {
    m_limits.clear();
}

