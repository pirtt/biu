/**
 * @file msg_handler.h
 * @author nyhoo (yoohoo.niu@huawei.com)
 * @brief 
 * @version 0.1
 * @date 2021-09-05
 * 
 * @copyright Copyright (c) 2021
 * 
 */
#ifndef __BIU_HANDLER__
#define __BIU_HANDLER__
#include "msg.h"
#include <condition_variable>
#include <functional>
#include <memory>
#include <mutex>
#include <queue>
#include <thread>
#include <unordered_map>

namespace biu {
// 错误码
constexpr int ERR_ASYNC_PROCESS = -1;
constexpr int ERR_NULL_POINTER = -2;

/**
 * @brief 消息处理器,不同的处理器可以继承此实现
 * 
 */
class MsgHandler {
public:
    explicit MsgHandler(int threadCnt = 4);
    virtual ~MsgHandler();
    /**
     * @brief 退出处理器
     * 
     */
    void Exit();
    /**
     * @brief 获取当前消息处理器处理消息后的错误信息,默认使用全局唯一的错误映射表,如果不同请
     * 自行实现即可
     * @param err 错误码
     * @return std::string 错误描述
     */
    static std::string GetErrInfo(int err);
    /**
     * @brief 处理消息入口
     * 
     * @param msg 
     * @return int 
     */
    int ProcessMsg(std::unique_ptr<Msg> msg);

protected:
    /**
     * @brief 异步处理消息
     * 
     * @param msg 
     */
    void AsyncProcessMsg(Msg& msg);
    /**
     * @brief 异步消息处理回调函数
     * 
     * @param msg 
     */
    virtual void OnMsgProcessed(Msg& msg, int);
    virtual void OnInit(Msg& msg, int);

protected:
    /**
     * @brief 初始化消息处理线程
     * 
     * @param threadCnt 线程数,实际内部会控制在[2,MaxThread]之间
     */
    static void Init(int threadCnt,MsgHandler& handler);
    /**
     * @brief 销毁消息处理线程
     * 
     */
    static void Destroy();
    /**
     * @brief 添加消息到线程
     * 
     * @param msg 
     */
    static void AddThreadMsg(std::unique_ptr<Msg> msg);
    // 线程处理函数
    static void DispatchMsg(int threadIndex);

protected:
    // 消息管道用于记录不同消息异步处理后的回调,类似命令模式
    std::unordered_map<uint32_t, std::function<void(Msg&, int)>> m_msgPipe;

protected:
    // 保证处理器全局资源互斥
    static std::mutex m_safeMutex;
    // 处理器最大线程数
    static constexpr uint32_t m_MaxThreadCount = 64;
    // 处理器当前线程数
    static volatile uint32_t m_curThreadCount;
    // 处理器是否已被初始化
    static volatile bool m_isInited;
    // 处理器状态 0:已退出,>0:运行中 <0:挂起中
    static volatile int m_status;
    // 消息处理线程整个处理器共用
    static std::thread m_msgThread[m_MaxThreadCount];
    static std::condition_variable m_msgCV[m_MaxThreadCount];
    static std::mutex m_msgMutex[m_MaxThreadCount];
    static std::queue<std::unique_ptr<Msg>> m_msgQueue[m_MaxThreadCount];
    // 用于映射处理器对应的错误码,用该映射表时最好是全局唯一的错误码,>=0为系统错误码,<0为自定义错误码
    static const std::unordered_map<int, std::string> m_handleErrors;
};
}
#endif