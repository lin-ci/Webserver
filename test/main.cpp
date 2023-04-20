#include"EventLoop.h"
#include"TcpServer.h"
#include"Common.h"
#include<iostream>
#include"Thread.h"
#include<memory>
int main(int argc, char** argv) {
    try {
        if (argc < 3) {
            std::cout << "./main ip port\n";
            return 0;
        }
        //std::cout << argv[1] << ' ' << atoi(argv[2]) << '\n';
        std::shared_ptr<TcpServer> server_ptr = std::make_shared<TcpServer>(argv[1], atoi(argv[2]));
        std::cout << "Server Start\n";
        server_ptr->start();
    }
    catch (...) {
        std::cout << "errror\n";
        exit(0);
    }
}