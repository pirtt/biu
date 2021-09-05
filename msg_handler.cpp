#include "msg_handler.h"
#include <cstring>
#include <iostream>
namespace biu {
volatile std::atomic_uint64_t Msg::m_msgCnt(0);
volatile std::atomic_uint64_t Msg::m_msgDoCnt(0);

MsgHandler::MsgHandler(int threadCnt)
{
    // 添加绑定信息
    m_msgPipe.emplace(0, std::bind(&MsgHandler::OnInit, this, std::placeholders::_1, std::placeholders::_2));
    if (!m_isInited) {
        Init(threadCnt, *this);
    }
}
MsgHandler::~MsgHandler()
{
}
void MsgHandler::Exit()
{
    m_status = 0;
    for (auto& cv : m_msgCV) {
        cv.notify_all();
    }
    Destroy();
}
std::string MsgHandler::GetErrInfo(int err)
{
    if (err < 0) {
        const auto& curErr = m_handleErrors.find(err);
        if (curErr != m_handleErrors.end()) {
            return curErr->second;
        }
    }
    return strerror(err);
}

int MsgHandler::ProcessMsg(std::unique_ptr<Msg> msg)
{
    if (msg) {
        msg->SetHandler(this);
        if (msg->IsAsync()) {
            AddThreadMsg(std::move(msg));
            return ERR_ASYNC_PROCESS;
        }
        return msg->DoMsg();
    }
    return ERR_NULL_POINTER;
}

void MsgHandler::AsyncProcessMsg(Msg& msg)
{
    int ret = msg.DoMsg();
    // 有自定义回调时,调用自定义回调
    auto curPipe = m_msgPipe.find(msg.GetType());
    if (curPipe != m_msgPipe.end()) {
        return curPipe->second(msg, ret);
    }
    //没有时调默认回调函数
    return OnMsgProcessed(msg, ret);
}
void MsgHandler::OnMsgProcessed(Msg& msg, int ret)
{
    using std::cout;
    using std::endl;
    cout << msg.GetID() << "[" << __FUNCTION__ << "]"
         << "ret=" << ret << endl;
}
void MsgHandler::OnInit(Msg& msg, int ret)
{
    using std::cout;
    using std::endl;
    cout << __FUNCTION__ << endl;
}
void MsgHandler::Init(int threadCnt, MsgHandler& handler)
{
    std::lock_guard<std::mutex> lk(m_safeMutex);
    if (m_isInited) {
        return;
    }
    m_isInited = true;
    m_status = 1;
    static_assert(m_MaxThreadCount > 2, "m_MaxThreadCount must grater than 2");
    for (int i = 0; (i < threadCnt && i < m_MaxThreadCount) || i < 2; i++) {
        m_msgThread[i] = std::move(std::thread(DispatchMsg, i));
        ++m_curThreadCount;
    }
    using InitMsg = Msg;
    (void)handler.ProcessMsg(std::make_unique<InitMsg>(0));
}
void MsgHandler::Destroy()
{
    std::lock_guard<std::mutex> lk(m_safeMutex);
    m_isInited = false;
    for (auto& th : m_msgThread) {
        if (th.joinable()) {
            th.join();
        }
    }
}
void MsgHandler::AddThreadMsg(std::unique_ptr<Msg> msg)
{
    int threadIndex = msg->GetThreadIndex();
    if (threadIndex == -1) {
        static int index = 0;
        threadIndex = ++index;
    }
    threadIndex %= m_curThreadCount;
    std::unique_lock<std::mutex> lk(m_msgMutex[threadIndex]);
    m_msgQueue[threadIndex].push(std::move(msg));
    m_msgCV[threadIndex].notify_one();
}
void MsgHandler::DispatchMsg(int threadIndex)
{
    while (m_status != 0) {
        std::unique_lock<std::mutex> lk(m_msgMutex[threadIndex]);
        auto status = m_msgCV[threadIndex].wait_for(lk, std::chrono::seconds(1));
        if (status == std::cv_status::timeout) {
            continue;
        }
        while (!m_msgQueue[threadIndex].empty()) {
            auto& msg = m_msgQueue[threadIndex].front();
            MsgHandler* msg_handler = msg->GetHandler();
            try {
                msg_handler->AsyncProcessMsg(*msg);
            } catch (const std::exception& e) {
                // std::cerr << e.what() << std::endl;
            } catch (...) {
            }
            m_msgQueue[threadIndex].pop();
        }
    }

    // std::cout << threadIndex << std::endl;
}
std::mutex MsgHandler::m_safeMutex;

volatile uint32_t MsgHandler::m_curThreadCount = 0;

volatile bool MsgHandler::m_isInited = false;
volatile int MsgHandler::m_status = 0;

std::thread MsgHandler::m_msgThread[MsgHandler::m_MaxThreadCount] = {};
std::condition_variable MsgHandler::m_msgCV[MsgHandler::m_MaxThreadCount] = {};
std::mutex MsgHandler::m_msgMutex[MsgHandler::m_MaxThreadCount] = {};
std::queue<std::unique_ptr<Msg>> MsgHandler::m_msgQueue[MsgHandler::m_MaxThreadCount] = {};
// 错误信息
const std::unordered_map<int, std::string> MsgHandler::m_handleErrors = {
    { ERR_ASYNC_PROCESS, "async process" },
    { ERR_NULL_POINTER, "null pointer" },
};
}