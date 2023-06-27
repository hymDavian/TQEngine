#include "MemoryManager.hpp"
#include <malloc.h>

using namespace My;

namespace My {
    //�����������Щ��С�Ŀ飬��Ӧ����Щ������
    static const uint32_t kBlockSizes[] = {
        // 4-����ֵ
        4,  8, 12, 16, 20, 24, 28, 32, 36, 40, 44, 48,
        52, 56, 60, 64, 68, 72, 76, 80, 84, 88, 92, 96,

        // 32
        128, 160, 192, 224, 256, 288, 320, 352, 384,
        416, 448, 480, 512, 544, 576, 608, 640,

        // 64
        704, 768, 832, 896, 960, 1024
    };

    //��ҳ��
    static const uint32_t kPageSize = 8192;
    //����ֵ
    static const uint32_t kAlignment = 4;

    // ���ж���Ŀ�ɷ����С����ɵ�����ĳ���
    static const uint32_t kNumBlockSizes =
        sizeof(kBlockSizes) / sizeof(kBlockSizes[0]);

    // ����Ķ����С
    static const uint32_t kMaxBlockSize =
        kBlockSizes[kNumBlockSizes - 1];
}

int My::MemoryManager::Initialize()
{
    // ��ʼ��һ��
    static bool s_bInitialized = false;
    if (!s_bInitialized) {
        //��ʼ�����С���ұ�
        m_pBlockSizeLookup = new size_t[kMaxBlockSize + 1];
        size_t j = 0;
        //����һ�����֣�0000 1111 2222 3333......
        for (size_t i = 0; i <= kMaxBlockSize; i++) {
            if (i > kBlockSizes[j]) ++j;
            m_pBlockSizeLookup[i] = j;
        }

        //��ʼ��������
        m_pAllocators = new Allocator[kNumBlockSizes];
        for (size_t i = 0; i < kNumBlockSizes; i++) {
            m_pAllocators[i].Reset(kBlockSizes[i], kPageSize, kAlignment);
        }

        s_bInitialized = true;
    }

    return 0;
}

void My::MemoryManager::Finalize()
{
    delete[] m_pAllocators;
    delete[] m_pBlockSizeLookup;
}

void My::MemoryManager::Tick()
{
}

Allocator* My::MemoryManager::LookUpAllocator(size_t size)
{

    // check eligibility for lookup
    if (size <= kMaxBlockSize)
        return m_pAllocators + m_pBlockSizeLookup[size];
    else
        return nullptr;
}

void* My::MemoryManager::Allocate(size_t size)
{
    Allocator* pAlloc = LookUpAllocator(size);
    if (pAlloc)
        return pAlloc->Allocate();
    else
        return malloc(size);
}

void My::MemoryManager::Free(void* p, size_t size)
{
    Allocator* pAlloc = LookUpAllocator(size);
    if (pAlloc)
        pAlloc->Free(p);
    else
        free(p);
}

