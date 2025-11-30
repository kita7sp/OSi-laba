#ifndef BUFFERED_CHANNEL_H
#define BUFFERED_CHANNEL_H

#include <queue>
#include <mutex>
#include <condition_variable>
#include <stdexcept>

template<typename T>
class BufferedChannel {
private:
    size_t bufferSize;
    std::queue<T> queue;
    std::mutex mtx;
    std::condition_variable sendCv;
    std::condition_variable recvCv;
    bool closed;

public:
    explicit BufferedChannel(size_t bufferSize) 
        : bufferSize(bufferSize), closed(false) {}

    void send(T value) {
        std::unique_lock<std::mutex> lock(mtx);
        
        if (closed) {
            throw std::runtime_error("Канал закрыт");
        }
        
        sendCv.wait(lock, [this]() { 
            return queue.size() < bufferSize || closed; 
        });
        
        if (closed) {
            throw std::runtime_error("Канал закрыт");
        }
        
        queue.push(std::move(value));
        recvCv.notify_one();
    }

    std::pair<T, bool> recv() {
        std::unique_lock<std::mutex> lock(mtx);
        
        recvCv.wait(lock, [this]() { 
            return !queue.empty() || closed; 
        });
        
        if (!queue.empty()) {
            T value = std::move(queue.front());
            queue.pop();
            sendCv.notify_one();
            return {std::move(value), true};
        }
        
        return {T(), false};
    }

    void close() {
        std::unique_lock<std::mutex> lock(mtx);
        closed = true;
        sendCv.notify_all();
        recvCv.notify_all();
    }

    ~BufferedChannel() {
        close();
    }
};

#endif