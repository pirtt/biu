/**
 * @file msg.h
 * @author nyhoo (yoohoo.niu@huawei.com)
 * @brief 消息处理基类
 * @version 0.1
 * @date 2021-09-05
 * 
 * @copyright Copyright (c) 2021
 * 
 */
#ifndef __BIU_MSG__
#define __BIU_MSG__

#include <cstdint>
#include <atomic>

namespace biu {
class MsgHandler;

class Msg {
public:
    Msg(int type)
        : m_type(type)
        , m_id(++m_msgCnt)
        , m_handler(nullptr)
    {
    }
    /**
     * @brief 消息自己的处理函数
     * 
     */
    int DoMsg()
    {
        ++m_msgDoCnt;
        return OnMsg();
    }
    virtual int OnMsg()
    {
        // cout << "OK" << endl;
        return 0;
    }
    /**
     * @brief 获取消息类型,用来标示某一类消息
     * 
     * @return uint32_t 
     */
    uint32_t GetType() const
    {
        return m_type;
    }
    /**
     * @brief 获取消息ID,在一定条件下保证消息的唯一性,以及用来对消息的计数
     * 
     * @return uint64_t 
     */
    uint64_t GetID() const
    {
        return m_id;
    }
    void SetHandler(MsgHandler* handler)
    {
        m_handler = handler;
    }
    MsgHandler* GetHandler() const
    {
        return m_handler;
    }
    /**
     * @brief 获取运行的线程序号,-1时随机轮训,指定时按有效的序号进行执行
     * 
     * @return int 
     */
    virtual int GetThreadIndex() const
    {
        return -1;
    }
    /**
     * @brief 是否需要异步执行
     * 
     * @return true 异步放入线程执行
     * @return false 同步执行
     */
    virtual bool IsAsync() const
    {
        return true;
    }
    /**
     * @brief 获取生产的总消息数量
     * 
     * @return uint64_t 
     */
    static uint64_t GetCount()
    {
        return m_msgCnt;
    }
    /**
     * @brief 获取已经执行了的消息数量
     * 
     * @return uint64_t 
     */
    static uint64_t GetDoCount()
    {
        return m_msgDoCnt;
    }

protected:
    const uint32_t m_type;
    const uint64_t m_id;
    MsgHandler* m_handler;

private:
    static volatile std::atomic_uint64_t m_msgCnt;
    static volatile std::atomic_uint64_t m_msgDoCnt;
};
}
#endif