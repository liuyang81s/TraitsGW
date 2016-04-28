#ifndef _UNLOCK_RINGBUFFER_H
#define _UNLOCK_RINGBUFFER_H

#include <stdint.h>
  
class UnlockRingBuffer
{
public:
     UnlockRingBuffer(int nSize);
     virtual ~UnlockRingBuffer();
  
     bool init();
  
     uint32_t put(const uint8_t *pBuffer, uint32_t nLen);
     uint32_t get(uint8_t *pBuffer, uint32_t nLen);
  
     inline void clean() { m_nIn = m_nOut = 0; }
     inline uint32_t data_len() const { return  m_nIn - m_nOut; }
	 uint8_t* get_data();

private:
     inline bool is_power_of_2(uint32_t n) { return (n != 0 && ((n & (n - 1)) == 0)); };
     inline uint32_t roundup_power_of_two(uint32_t val);
  
private:
     uint8_t *m_pBuffer;    /* the buffer holding the data */
     uint32_t   m_nSize;        /* the size of the allocated buffer */
     uint32_t   m_nIn;        /* data is added at offset (in % size) */
     uint32_t   m_nOut;        /* data is extracted from off. (out % size) */
};
  
#endif

