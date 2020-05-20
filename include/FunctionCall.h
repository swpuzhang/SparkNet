#ifndef FUNCTION_CALL_H
#define FUNCTION_CALL_H
#include <functional>
#include <memory>
#include <chrono>

class Buffer;
class TcpConnection;
using TcpConnectionPtr = std::shared_ptr<TcpConnection>;
using ConnectionCallback = std::function<void(const TcpConnectionPtr&)>;
using CloseCallback = std::function<void(const TcpConnectionPtr&)>;
using MessageCallback = std::function<void(const TcpConnectionPtr&, 
    std::chrono::system_clock::time_point)>;
using WriteCompleteCallback = std::function<void(const TcpConnectionPtr&)>;
using DisconnectCallback = std::function<void()>;
#endif // !FUNCTION_CALL_H
