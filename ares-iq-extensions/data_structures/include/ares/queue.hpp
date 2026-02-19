/**
 * @file queue.hpp
 *
 * @brief
 *
 * @date 2/17/26
 *
 * @author Tom Schmitz \<tschmitz@andrew.cmu.edu\>
 */

#ifndef ARES_QUEUE_HPP
#define ARES_QUEUE_HPP

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <deque>
#include <exception>
#include <mutex>
#include <string>

namespace ares {
template <typename Type> class queue {
  public:
    queue() : _size(0) {}
    ~queue() = default;

    void put(Type &item);

    Type get();

    size_t size();

    bool empty() const;

    void clear();

  private:
    std::deque<Type> _buffer;
    std::mutex _lock;
    std::atomic_size_t _size;
    std::condition_variable _not_empty;
};

class queue_exception : public std::exception {
  public:
    enum queue_exception_reason { QUEUE_EMPTY, QUEUE_FULL, QUEUE_TIMEOUT };

    explicit queue_exception(const queue_exception_reason &exc_reason)
        : _reason(exc_reason) {
        switch (exc_reason) {
        case QUEUE_EMPTY: {
            _what = "Queue Empty";
            break;
        }
        case QUEUE_FULL: {
            _what = "Queue Full";
            break;
        }
        case QUEUE_TIMEOUT: {
            _what = "Queue Timed Out";
            break;
        }
        default: {
            _what = "Unknown Queue Error";
            break;
        }
        }
    }

    [[nodiscard]] const char *what() const noexcept override {
        return _what.c_str();
    }

    [[nodiscard]] queue_exception_reason reason() const noexcept {
        return _reason;
    }

  private:
    queue_exception_reason _reason;
    std::string _what;
};

template <typename Type, size_t max_size = 1, bool overwrite = false>
class bounded_queue {
  public:
    bounded_queue() : _size(0) {}
    ~bounded_queue() = default;

    void put(Type &item, bool block = true,
             const std::chrono::milliseconds &timeout_ms =
                 std::chrono::milliseconds::zero());
    Type get(bool block = true, const std::chrono::milliseconds &timeout_ms =
                                    std::chrono::milliseconds::zero());
    size_t size() const;
    bool empty() const;
    bool full() const;
    void clear();

  private:
    Type _buffer[max_size];
    size_t _producer_index = 0;
    size_t _consumer_index = 0;
    std::atomic_size_t _size;

    std::mutex _lock;
    std::condition_variable _not_empty;
    std::condition_variable _space_available;
};

template <typename Type> void queue<Type>::put(Type &item) {
    std::unique_lock<std::mutex> guard(_lock);

    _buffer.emplace_back(item);
    _size.fetch_add(1);
    _not_empty.notify_one();
}

template <typename Type> Type queue<Type>::get() {
    std::unique_lock<std::mutex> guard(_lock);

    _not_empty.wait(guard, [this]() { return !empty(); });
    Type ret = _buffer.front();
    _buffer.pop_front();
    _size.fetch_sub(1);

    return ret;
}

template <typename Type> size_t queue<Type>::size() { return _size; }

template <typename Type> bool queue<Type>::empty() const {
    size_t size = _size;
    return size == 0;
}

template <typename Type> void queue<Type>::clear() {
    std::unique_lock<std::mutex> guard(_lock);
    _size.store(0);
    _buffer.clear();
}

template <typename Type, size_t max_size, bool overwrite>
void bounded_queue<Type, max_size, overwrite>::put(
    Type &item, bool block, const std::chrono::milliseconds &timeout_ms) {
    std::unique_lock<std::mutex> guard(_lock);

    if (!block) {
        if (!_space_available.wait_for(guard, timeout_ms,
                                       [this]() { return !full(); })) {
            if (!overwrite) {
                throw queue_exception(timeout_ms ==
                                              std::chrono::milliseconds::zero()
                                          ? queue_exception::QUEUE_FULL
                                          : queue_exception::QUEUE_TIMEOUT);
            }
        }
    } else {
        _space_available.wait(guard, [this]() { return !full(); });
    }

    _buffer[_producer_index] = item;
    _producer_index = (_producer_index + 1) % max_size;
    _size.fetch_add(1);
    if (_size >= max_size) {
        _size = max_size;
        if (overwrite) {
            _consumer_index = (_consumer_index + 1) % max_size;
        }
    }
    _not_empty.notify_one();
}

template <typename Type, size_t max_size, bool overwrite>
Type bounded_queue<Type, max_size, overwrite>::get(
    bool block, const std::chrono::milliseconds &timeout_ms) {
    std::unique_lock<std::mutex> guard(_lock);
    Type ret;

    if (!block) {
        if (!_not_empty.wait_for(guard, timeout_ms,
                                 [this]() { return !empty(); })) {
            throw queue_exception(timeout_ms ==
                                          std::chrono::milliseconds::zero()
                                      ? queue_exception::QUEUE_EMPTY
                                      : queue_exception::QUEUE_TIMEOUT);
        }
    } else {
        _not_empty.wait(guard, [this]() { return !empty(); });
    }

    ret = _buffer[_consumer_index];
    _consumer_index = (_consumer_index + 1) % max_size;
    _size.fetch_sub(1);
    _space_available.notify_one();
    return ret;
}

template <typename Type, size_t max_size, bool overwrite>
size_t bounded_queue<Type, max_size, overwrite>::size() const {
    size_t size = _size;
    return size;
}

template <typename Type, size_t max_size, bool overwrite>
bool bounded_queue<Type, max_size, overwrite>::empty() const {
    size_t size = _size;
    return size == 0;
}

template <typename Type, size_t max_size, bool overwrite>
bool bounded_queue<Type, max_size, overwrite>::full() const {
    size_t size = _size;
    return size == max_size;
}

template <typename Type, size_t max_size, bool overwrite>
void bounded_queue<Type, max_size, overwrite>::clear() {
    std::unique_lock<std::mutex> guard(_lock);
    _size = 0;
    _producer_index = 0;
    _consumer_index = 0;
}
} // namespace ares

#endif // ARES_QUEUE_HPP