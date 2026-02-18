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
#include <condition_variable>
#include <deque>
#include <mutex>

namespace ares {
template <typename Type> class queue {
  public:
    queue();
    ~queue();

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

template <typename Type> queue<Type>::queue() { _size = 0; }

template <typename Type> queue<Type>::~queue() {}

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
} // namespace ares

#endif // ARES_QUEUE_HPP