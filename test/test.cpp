#include "../utils/C_Thread.h"
#include "../utils/C_MutexLock.h"
#include "../utils/C_Socket.h"
#include "../utils/C_SQueue.h"
#include "../utils/C_Epoll.h"
#include "../utils/Utils.h"
#include "../utils/Notify.h"
#include "../utils/Channel.h"
#include "../HttpRequset.h"
#include "../HandleThread.h"
#include "../HttpRequset.h"
#include <iostream>
#include <memory>
#include <stdio.h>
#include <sstream>
#include <functional>

using namespace std;
using namespace WebServer;

void testSocket()
{
    int port = 10068;
    printf("new\n");
    C_Socket cSocket(port);
    printf("setPort\n");
    cSocket.setPort(port);
    printf("createSocket\n");
    if(!cSocket.createSocket()){
        cout << "createSocket error" << endl;
    }
    cSocket.setSoReuseAddr();
    //cout<< cSocket.getLocalIp()<<endl;
    printf("bindSocket\n");
    if(!cSocket.bindSocket()){
        cout << "bindSocket error" << endl;
    }
    printf("listenSocket\n");
    if(!cSocket.listenSocket()){
        cout << "listenSocket error" << endl;
    }
    C_Socket newSocket;
    int count=0;
    while(true){
        cSocket.acceptConn(newSocket);
        //newSocket.setSocketNodelay();
        string strRead;
        printf("acceptedConn\n");
        if(newSocket.readSocket(strRead)==-1)
        {
            continue;
            newSocket.closeSocket();
        }
        cout << strRead <<endl;
        
        ostringstream rsp;
        rsp << "HTTP/1.1 200 OK\r\n";
        //rsp +=          "Content-Type: text/plain\r\n";
        rsp << "\r\n";
        rsp << "<html>";
        rsp << "index page " << count;
        rsp << "</html>";
        string str(rsp.str());
        if(str.find("/favicon.ico") != -1) 
        {
            newSocket.closeSocket();
            continue;
        }
        newSocket.writeSocket(str);
        newSocket.closeSocket();
        //newSocket.shutDownWR();
        count ++;
        cout << "count " << count << " ip = "<< newSocket.getIp() <<endl << endl;
    }

}

void testQueue()
{
    C_SQueue<int> qu;
    int i=10;
    qu.push(4);
    qu.push(3);
    //int i = 0;
    qu.pop(i);
    cout << i << endl;
    qu.pop(i);
    cout << i << endl;
}
void fun()
    {
        cout << "Test Channel" << endl;
    }
void testChannel()
{
    
    shared_ptr<HttpRequset> requset = make_shared<HttpRequset>();
    ChannelPtr chan = make_shared<Channel>(requset, bind(&HttpRequset::doTest, requset));
    //chan.setHandler(fun);
    //chan.handle();
    cout<< "test start" <<endl;
    ChannelsQueuePtr chansQ = make_shared<C_SQueue<ChannelPtr>>();
    chansQ->push(chan);
    cout<< "chansQ->push(chan);" <<endl;
    HandleThread *handleThread = new HandleThread();
    cout<< "new HandleThread();" <<endl;
    handleThread->setQueuePtr(chansQ);
    handleThread->start();


}
void testHttpRequset()
{
    int port = 10088;
    printf("new\n");
    C_Socket cSocket(port);
    printf("setPort\n");
    cSocket.setPort(port);
    printf("createSocket\n");
    if(!cSocket.createSocket()){
        cout << "createSocket error" << endl;
    }
    cSocket.setSoReuseAddr();
    //cout<< cSocket.getLocalIp()<<endl;
    printf("bindSocket\n");
    if(!cSocket.bindSocket()){
        cout << "bindSocket error" << endl;
    }
    printf("listenSocket\n");
    if(!cSocket.listenSocket()){
        cout << "listenSocket error" << endl;
    }
    C_Socket newSocket;
    ChannelsQueuePtr chansQueue = make_shared<C_SQueue<ChannelPtr>>();
    HandleThread *handleThread = new HandleThread();
    printf("handleThread->setQueuePtr...\n");
    handleThread->setQueuePtr(chansQueue);
    printf("handleThread start...\n");
    handleThread->start();
    while(true){
        printf("cSocket.acceptConn......\n");
        cSocket.acceptConn(newSocket);
        printf("cSocket.acceptConned\n");
        shared_ptr<HttpRequset> newRequeset = make_shared<HttpRequset>(newSocket);
        
        // newRequeset->setReadHandlerQueue(chansQueue);
        // newRequeset->setDoReusetChannelsQueue(chansQueue);
        // newRequeset->setWriteHandleQueue(chansQueue);
        ChannelPtr chan = make_shared<Channel>(newRequeset, bind(&HttpRequset::readHandler, newRequeset));
        chansQueue->push(chan);
        //handleThread
    }
}

// void connHandle(C_Socket acCSocket){
//     C_Socket newSocket;
//     acCSocket.acceptConn(newSocket);
// }
void readhandle(C_EpollPtr ep ,ChannelPtr newChan){
    string msg;
    C_Socket newSocket(newChan->getFd());
    cout << "readhandle" << endl;
    bool zero = false;
    setFdNonBlock(newSocket.getSocket());
    newSocket.readnSocket(msg, zero);
    if(zero){
        newSocket.closeSocket();
        ep->delChannelPtr(newChan);
        return ;
    }
    cout << " -------zero------ " << zero << " -----------------" << endl;
    cout << msg << endl;
    cout << "----------------Http Msg---------------- "<< endl;

    ostringstream rsp;
    static int count = 0;
    rsp << "HTTP/1.1 200 OK\r\n";
    //rsp +=          "Content-Type: text/plain\r\n";
    rsp << "\r\n";
    rsp << "<html>";
    rsp << "index page " << count;
    count ++;
    rsp << "</html>";
    string str(rsp.str());

    newSocket.writenSocket(str);
    newSocket.closeSocket();
    ep->delChannelPtr(newChan);
}
void connHandle(C_Socket acCSocket, C_EpollPtr ep)
{
    C_Socket newSocket;
    acCSocket.acceptConn(newSocket);
    cout << "acceptConn from " << newSocket.getIp() <<endl; 
    ChannelPtr newChan = make_shared<Channel>();
    newChan->setFd(newSocket.getSocket());
    newChan->setSetEvents(EPOLLIN|EPOLLET);
    cout << " setReadhanedler " <<endl;
    newChan->setReadhanedler(bind(readhandle, ep, newChan));
    cout << " addChannelPtr " <<endl;
    ep->addChannelPtr(newChan, newChan->getSetEvents());
    // string msg;
    // bool zero = false;
    // setFdNonBlock(newSocket.getSocket());
    // newSocket.readnSocket(msg, zero);
    // cout << " ---zero--- " << zero <<endl;
    // cout << msg << endl;
    // cout << endl;
    // newSocket.closeSocket();
}
void testEpoll()
{
    //void connHandle();
    int port = 10088;
    printf("new\n");
    C_Socket cSocket(port);
    printf("setPort\n");
    cSocket.setPort(port);
    printf("createSocket\n");
    if(!cSocket.createSocket()){
        cout << "createSocket error" << endl;
    }
    cSocket.setSoReuseAddr();
    //cout<< cSocket.getLocalIp()<<endl;
    printf("bindSocket\n");
    if(!cSocket.bindSocket()){
        cout << "bindSocket error" << endl;
    }
    printf("listenSocket\n");
    if(!cSocket.listenSocket()){
        cout << "listenSocket error" << endl;
    }
    //shared_ptr<HttpRequset> 
    ChannelPtr chanPtr = make_shared<Channel>();
    chanPtr->setFd(cSocket.getSocket());
    C_EpollPtr cEpollPtr = make_shared<C_Epoll>();
    chanPtr->setReadhanedler(bind(connHandle, cSocket, cEpollPtr));
    chanPtr->setSetEvents(EPOLLIN|EPOLLET);
    cEpollPtr->addChannelPtr(chanPtr, chanPtr->getSetEvents());
    ///cEpoll.poll();
    while(true)
    {
        vector<ChannelPtr> accChan = cEpollPtr->poll();
        cout << "accChan size = " << accChan.size() <<endl;
        for(int i = 0; i < accChan.size(); i++){
            accChan[i]->handleEvent();
        }
    }


    
}

void runloop(Notify notify){
    while(true)
    {
        notify.wait();
        cout << "Test Notify" << endl;
    }
}
void testNotify(){
    Notify notify;
    C_Thread thread(bind(runloop,notify));
    notify.notify();
    notify.notify();
    notify.notify();
    thread.start();
    char c;
    // while(true)
    // {
    //     cin >> c;
    //     notify.notify();
    // }
    //notify.notify();
    //notify.notify();
    //notify.notify();
    thread.join();
}

int main(){
    //testSocket();
    //testQueue();
    //testChannel();
    //testHttpRequset();
    testEpoll();
    //testNotify();
    return 0;
}