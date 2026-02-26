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

#include <chrono>
#include <condition_variable>
#include <deque>
#include <exception>
#include <mutex>
#include <string>

namespace ares {
/**
 * Queue exceptions.
 */
class queue_exception : public std::exception {
  public:
    /**
     * The queue exception reason.
     */
    enum queue_exception_reason {
        QUEUE_EMPTY,  ///< Queue empty.
        QUEUE_FULL,   ///< Queue full.
        QUEUE_TIMEOUT ///< Queue operation timed out.
    };

    /**
     * .
     * @param exc_reason The reason for the queue exception.
     */
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

    /**
     * Retrieve the error message.
     * @return The error message.
     */
    [[nodiscard]] const char *what() const noexcept override {
        return _what.c_str();
    }

    /**
     * Retrieve the exception reason enum.
     * @return The exception reas as an enum.
     */
    [[nodiscard]] queue_exception_reason reason() const noexcept {
        return _reason;
    }

  private:
    queue_exception_reason _reason;
    std::string _what;
};

/**
 * Thread-safe queue implementation.
 * @tparam Type The queue element type.
 */
template <typename Type> class queue {
  public:
    /**
     * .
     */
    queue() = default;

    /**
     * .
     */
    ~queue() = default;

    /**
     * Place an item into the queue.
     * @param item The item to place into the queue.
     */
    void put(Type &item);

    /**
     * Retrieve and remove an item from the queue.
     * @return The first item in the queue.
     *
     * @note This will block indefinitely if there are no items in the queue.
     */
    Type get();

    /**
     * Retrieve and remove an item from the queue with a timeout.
     * @param timeout_ms The maximum amount of time to wait for item to become
     * ready in the queue. If set to std::chrono::milliseconds::zero(), then
     * this method will become non-blocking.
     * @return The first item in the queue.
     * @throws queue_exception if timeout expired.
     */
    Type get(const std::chrono::milliseconds &timeout_ms);

    /**
     * Retrieve and remove an item from the queue in a non-blocking fashion.
     * @return The first item in the queue.
     * @throws queue_exception if the queue is empty.
     * @note This is the same as calling get(std::chrono::milliseconds::zero())
     */
    Type get_nonblocking();

    /**
     * .
     * @return The number of elements in the queue.
     */
    size_t size();

    /**
     * .
     * @return `true` if the queue is empty. `false` otherwise.
     */
    bool empty();

    /**
     * Clears the queue.
     */
    void clear();

  private:
    std::deque<Type> _buffer;
    std::mutex _lock;
    std::size_t _size = 0;
    std::condition_variable _not_empty;
};

/**
 * Thread-safe, statically allocated queue.
 * @tparam Type The queue element type.
 * @tparam max_size The maximum size of the queue.
 * @tparam overwrite Overwrite old data if full.
 */
template <typename Type, size_t max_size = 1, bool overwrite = false>
class bounded_queue {
  public:
    /**
     * .
     */
    bounded_queue() = default;

    /**
     * .
     */
    ~bounded_queue() = default;

    /**
     * Place an item into the queue.
     * @param item The item to place into the queue.
     * @note This will block indefinitely.
     */
    void put(Type &item);

    /**
     * Place an item in the queue with a timeout.
     * @param item The item to place in the queue.
     * @param timeout_ms The maximum amount of time to block if the queue is
     * full. If set to std::chrono::milliseconds::zero(), then this method
     * becomes non-blocking.
     * @throws queue_exception if timeout expired.
     */
    void put(Type &item, const std::chrono::milliseconds &timeout_ms);

    /**
     * Place an item into the queue in a non-blocking fashion.
     * @param item The item to place in the queue.
     * @throws queue_exception if the queue is full.
     *
     * @note This is the same as calling putt(item,
     * std::chrono::milliseconds::zero())
     */
    void put_nonblocking(Type &item);

    /**
     * Retrieve and remove an item from the queue.
     * @return The first item in the queue.
     *
     * @note This will block indefinitely if there are no items in the queue.
     */
    Type get();

    /**
     * Retrieve and remove an item from the queue with a timeout.
     * @param timeout_ms The maximum amount of time to wait for item to become
     * ready in the queue. If set to std::chrono::milliseconds::zero(), then
     * this method will become non-blocking.
     * @return The first item in the queue.
     * @throws queue_exception if timeout expired.
     */
    Type get(const std::chrono::milliseconds &timeout_ms);

    /**
     * Retrieve and remove an item from the queue in a non-blocking fashion.
     * @return The first item in the queue.
     * @throws queue_exception if the queue is empty.
     * @note This is the same as calling get(std::chrono::milliseconds::zero())
     */
    Type get_nonblocking();

    /**
     * .
     * @return The number of elements in the queue.
     */
    size_t size();

    /**
     * .
     * @return `true` if the queue is empty. `false` otherwise.
     */
    bool empty();

    /**
     * .
     * @return `true` if the queue is full. `false` otherwise.
     */
    bool full();

    /**
     * Clears the queue.
     */
    void clear();

  private:
    Type _buffer[max_size];
    size_t _producer_index = 0;
    size_t _consumer_index = 0;
    std::size_t _size = 0;

    std::mutex _lock;
    std::condition_variable _not_empty;
    std::condition_variable _space_available;
};

template <typename Type> void queue<Type>::put(Type &item) {
    std::unique_lock<std::mutex> guard(_lock);

    _buffer.emplace_back(item);
    _size += 1;
    _not_empty.notify_one();
}

template <typename Type> Type queue<Type>::get() {
    std::unique_lock<std::mutex> guard(_lock);

    _not_empty.wait(guard, [this]() { return _size != 0; });
    Type ret = _buffer.front();
    _buffer.pop_front();
    _size -= 1;

    return ret;
}

template <typename Type>
Type queue<Type>::get(const std::chrono::milliseconds &timeout_ms) {
    std::unique_lock<std::mutex> guard(_lock);
    if (!_not_empty.wait_for(guard, timeout_ms,
                             [this]() { return _size != 0; })) {
        throw queue_exception(timeout_ms == std::chrono::milliseconds::zero()
                                  ? queue_exception::QUEUE_EMPTY
                                  : queue_exception::QUEUE_TIMEOUT);
    }
    Type ret = _buffer.front();
    _buffer.pop_front();
    _size -= 1;

    return ret;
}

template <typename Type> Type queue<Type>::get_nonblocking() {
    return get(std::chrono::milliseconds::zero());
}

template <typename Type> size_t queue<Type>::size() {
    std::unique_lock<std::mutex> guard(_lock);
    return _size;
}

template <typename Type> bool queue<Type>::empty() {
    std::unique_lock<std::mutex> guard(_lock);
    return _size == 0;
}

template <typename Type> void queue<Type>::clear() {
    std::unique_lock<std::mutex> guard(_lock);
    _size = 0;
    _buffer.clear();
}

template <typename Type, size_t max_size, bool overwrite>
void bounded_queue<Type, max_size, overwrite>::put(Type &item) {
    std::unique_lock<std::mutex> guard(_lock);

    _space_available.wait(guard, [this]() { return _size != max_size; });

    _buffer[_producer_index] = item;
    _producer_index = (_producer_index + 1) % max_size;
    _size += 1;
    _not_empty.notify_one();
}

template <typename Type, size_t max_size, bool overwrite>
void bounded_queue<Type, max_size, overwrite>::put(
    Type &item, const std::chrono::milliseconds &timeout_ms) {
    std::unique_lock<std::mutex> guard(_lock);
    if (!_space_available.wait_for(guard, timeout_ms,
                                   [this]() { return _size != max_size; })) {
        if (!overwrite) {
            throw queue_exception(timeout_ms ==
                                          std::chrono::milliseconds::zero()
                                      ? queue_exception::QUEUE_FULL
                                      : queue_exception::QUEUE_TIMEOUT);
        }
    }

    _buffer[_producer_index] = item;
    _producer_index = (_producer_index + 1) % max_size;
    _size += 1;
    if (_size >= max_size) {
        _size = max_size;
        if (overwrite) {
            _consumer_index = (_consumer_index + 1) % max_size;
        }
    }
    _not_empty.notify_one();
}

template <typename Type, size_t max_size, bool overwrite>
void bounded_queue<Type, max_size, overwrite>::put_nonblocking(Type &item) {
    put(item, std::chrono::milliseconds::zero());
}

template <typename Type, size_t max_size, bool overwrite>
Type bounded_queue<Type, max_size, overwrite>::get() {
    std::unique_lock<std::mutex> guard(_lock);
    Type ret;

    _not_empty.wait(guard, [this]() { return _size != 0; });

    ret = _buffer[_consumer_index];
    _consumer_index = (_consumer_index + 1) % max_size;
    _size -= 1;
    _space_available.notify_one();
    return ret;
}

template <typename Type, size_t max_size, bool overwrite>
Type bounded_queue<Type, max_size, overwrite>::get(
    const std::chrono::milliseconds &timeout_ms) {
    std::unique_lock<std::mutex> guard(_lock);
    Type ret;

    if (!_not_empty.wait_for(guard, timeout_ms,
                             [this]() { return _size != 0; })) {
        throw queue_exception(timeout_ms == std::chrono::milliseconds::zero()
                                  ? queue_exception::QUEUE_EMPTY
                                  : queue_exception::QUEUE_TIMEOUT);
    }

    ret = _buffer[_consumer_index];
    _consumer_index = (_consumer_index + 1) % max_size;
    _size -= 1;
    _space_available.notify_one();
    return ret;
}

template <typename Type, size_t max_size, bool overwrite>
Type bounded_queue<Type, max_size, overwrite>::get_nonblocking() {
    return get(std::chrono::milliseconds::zero());
}

template <typename Type, size_t max_size, bool overwrite>
size_t bounded_queue<Type, max_size, overwrite>::size() {
    std::unique_lock<std::mutex> guard(_lock);
    return _size;
}

template <typename Type, size_t max_size, bool overwrite>
bool bounded_queue<Type, max_size, overwrite>::empty() {
    std::unique_lock<std::mutex> guard(_lock);
    return _size == 0;
}

template <typename Type, size_t max_size, bool overwrite>
bool bounded_queue<Type, max_size, overwrite>::full() {
    std::unique_lock<std::mutex> guard(_lock);
    return _size == max_size;
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