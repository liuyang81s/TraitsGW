#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <algorithm>

#include "unlock_ringbuffer.h"
    
UnlockRingBuffer::UnlockRingBuffer(int nSize)
        :m_pBuffer(NULL) 
        ,m_nSize(nSize)
        ,m_nIn(0)
        ,m_nOut(0)
{
    //round up to the next power of 2
    if (!is_power_of_2(nSize))
    {
        m_nSize = roundup_power_of_two(nSize);
    }
}
 
UnlockRingBuffer::~UnlockRingBuffer()
{
    if(NULL != m_pBuffer)
    {
        delete[] m_pBuffer;
        m_pBuffer = NULL;
    }
}
 
bool UnlockRingBuffer::Init()
{
    m_pBuffer = new uint8_t[m_nSize];
    if (!m_pBuffer)
    {
        return false;
    }
 
    m_nIn = m_nOut = 0;
 
    return true;
}
 
uint32_t UnlockRingBuffer::roundup_power_of_two(uint32_t val)
{
    if((val & (val-1)) == 0)
        return val;
 
    uint32_t maxulong = (uint32_t)((uint32_t)~0);
    uint32_t andv = ~(maxulong&(maxulong>>1));
    while((andv & val) == 0)
        andv = andv>>1;
 
    return andv<<1;
}


uint32_t UnlockRingBuffer::Put(const uint8_t *buffer, uint32_t len)
{
    uint32_t l;
 
    len = std::min(len, m_nSize - m_nIn + m_nOut);
 
    /*
     * Ensure that we sample the m_nOut index -before- we
     * start putting bytes into the UnlockRingBuffer.
     */
    __sync_synchronize();
 
    /* first put the data starting from fifo->in to buffer end */
    l = std::min(len, m_nSize - (m_nIn  & (m_nSize - 1)));
    memcpy(m_pBuffer + (m_nIn & (m_nSize - 1)), buffer, l);
 
    /* then put the rest (if any) at the beginning of the buffer */
    memcpy(m_pBuffer, buffer + l, len - l);
 
    /*
     * Ensure that we add the bytes to the kfifo -before-
     * we update the fifo->in index.
     */
    __sync_synchronize();
 
    m_nIn += len;
 
    return len;
}  

uint32_t UnlockRingBuffer::Get(uint8_t *buffer, uint32_t len)
{
    uint32_t l;
 
    len = std::min(len, m_nIn - m_nOut);
 
    /*
     * Ensure that we sample the fifo->in index -before- we
     * start removing bytes from the kfifo.
     */
    __sync_synchronize();
 
    /* first get the data from fifo->out until the end of the buffer */
    l = std::min(len, m_nSize - (m_nOut & (m_nSize - 1)));
    memcpy(buffer, m_pBuffer + (m_nOut & (m_nSize - 1)), l);
 
    /* then get the rest (if any) from the beginning of the buffer */
    memcpy(buffer + l, m_pBuffer, len - l);
 
    /*
     * Ensure that we remove the bytes from the kfifo -before-
     * we update the fifo->out index.
     */
    __sync_synchronize();
 
    m_nOut += len;
 
    return len;
}

