#pragma once
#include "Interface.hpp"

namespace My
{
    Interface IRuntimeModule
    {
        public:
            virtual ~IRuntimeModule(){};
            //初始化
            virtual int Initialize()=0;
            //结束
            virtual void Finalize() = 0;
            //更新
            virtual void Tick() = 0;
    };
}
