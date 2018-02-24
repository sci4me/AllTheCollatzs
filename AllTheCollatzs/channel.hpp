// Credit: https://st.xorian.net/blog/2012/08/go-style-channel-in-c/

#include <thread>
#include <mutex>
#include <list>

template<typename T>
class channel {
    std::list<T>            _queue;
    std::mutex              _mutex;
    std::condition_variable _cv;
    bool                    _closed;
public:
    channel() : _closed(false) { };

    void close() {
        std::unique_lock<std::mutex> lock(_mutex);
        _closed = true;
        _cv.notify_all();
    }

    bool is_closed() {
        std::unique_lock<std::mutex> lock(_mutex);
        return _closed;
    }

    void send(T const& i) {
        send(std::move(i));
    }

    void send(T const&& i) {
        std::unique_lock<std::mutex> lock(_mutex);
        if (_closed)
            throw std::logic_error("put to closed channel");
        _queue.push_back(i);
        _cv.notify_one();
    }

    bool recv(T& out, bool wait = true) {
        std::unique_lock<std::mutex> lock(_mutex);
        if (wait) _cv.wait(lock, [&] { return _closed || !_queue.empty(); });
        if (_queue.empty()) return false;
        out = _queue.front();
        _queue.pop_front();
        return true;
    }
};