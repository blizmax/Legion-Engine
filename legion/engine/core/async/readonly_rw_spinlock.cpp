#include <core/async/readonly_rw_spinlock.hpp>

namespace legion::core::async
{
    std::atomic_uint readonly_rw_spinlock::m_lastId = 1;

    thread_local std::unordered_map<uint, int>* readonly_rw_spinlock::m_localWritersPtr = new std::unordered_map<uint, int>();
    thread_local std::unordered_map<uint, int>* readonly_rw_spinlock::m_localReadersPtr = new std::unordered_map<uint, int>();
    thread_local std::unordered_map<uint, lock_state>* readonly_rw_spinlock::m_localStatePtr = new std::unordered_map<uint, lock_state>();

    thread_local std::unordered_map<uint, int>& readonly_rw_spinlock::m_localWriters = *m_localWritersPtr;
    thread_local std::unordered_map<uint, int>& readonly_rw_spinlock::m_localReaders = *m_localReadersPtr;
    thread_local std::unordered_map<uint, lock_state>& readonly_rw_spinlock::m_localState = *m_localStatePtr;
}
