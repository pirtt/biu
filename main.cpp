#include "msg_handler.h"
#include <functional>
#include <iostream>

using namespace std;
using namespace biu;

class T : public MsgHandler {
    public:
    T()
    {
        for(int i=1;i<100;i++)
        {
            m_msgPipe.emplace(i, std::bind(&T::OnTest, this, std::placeholders::_1, std::placeholders::_2));
        }
    }
    void OnTest(Msg& msg, int ret)
    {
            cout << msg.GetID() << "[" << __FUNCTION__ << "]"
         << "ret=" << ret << endl;
    }
};

int main()
{

    T H;
    for (int i = 1; i < 100; i++) {
        H.ProcessMsg(std::make_unique<Msg>(i));
    }
    H.Exit();
    cout << Msg::GetCount() << endl;
    cout << Msg::GetDoCount() << endl;
    return 0;
}
