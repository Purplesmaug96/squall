#ifndef STORM_LIST_TS_LINK_HPP
#define STORM_LIST_TS_LINK_HPP

#include <cstddef>
#include <cstdint>

template <class T>
class TSLink {
    public:
    // Member variables
    TSLink<T>* m_prevlink = nullptr;
    T* m_next = nullptr;

    // Member functions
    ~TSLink();
    bool IsLinked();
    T* Next();
    TSLink<T>* NextLink(ptrdiff_t linkoffset);
    T* Prev();
    T* RawNext();
    void Unlink();
};

template <class T>
TSLink<T>::~TSLink() {
    this->Unlink();
}

template <class T>
bool TSLink<T>::IsLinked() {
    return this->m_next != nullptr;
}

template <class T>
T* TSLink<T>::Next() {
    // Check for sentinel node (indicates list end)
    return reinterpret_cast<intptr_t>(this->m_next) <= 0 ? nullptr : this->m_next;
}

template <class T>
TSLink<T>* TSLink<T>::NextLink(ptrdiff_t linkoffset) {
    T* next = this->m_next;

    if (reinterpret_cast<intptr_t>(next) <= 0) {
        // End of list
        return reinterpret_cast<TSLink<T>*>(~reinterpret_cast<uintptr_t>(next));
    } else {
        ptrdiff_t offset;

        if (linkoffset < 0) {
            offset = reinterpret_cast<uintptr_t>(this) - reinterpret_cast<uintptr_t>(this->m_prevlink->m_next);
        } else {
            offset = linkoffset;
        }

        return reinterpret_cast<TSLink<T>*>(reinterpret_cast<uintptr_t>(this->m_next) + offset);
    }
}

template <class T>
T* TSLink<T>::Prev() {
    return this->m_prevlink->m_prevlink->Next();
}

template <class T>
T* TSLink<T>::RawNext() {
    return this->m_next;
}

#include <unistd.h>
#include <errno.h>

// Safely checks if a pointer can be read without crashing
static inline bool __aIsValidReadPtr(const void* ptr, size_t size) {
    if (!ptr || reinterpret_cast<uintptr_t>(ptr) < 0x1000) return false;

    int pfd[2];
	#ifdef __linux__
    if (pipe(pfd) < 0) return false;
	#endif

    // Attempt to write from the pointer into a pipe.
    // If the pointer is invalid, write() returns -1 and sets errno to EFAULT instead of crashing.
    ssize_t result = write(pfd[1], ptr, size);
    close(pfd[0]);
    close(pfd[1]);

    return result == static_cast<ssize_t>(size);
}

template <class T>
void TSLink<T>::Unlink() {
    if (!__aIsValidReadPtr(this, 1) && !__aIsValidReadPtr(this->NextLink(-1), 1) && !__aIsValidReadPtr(this->NextLink(-1)->m_prevlink, 1) && this && this->m_prevlink) {
        this->NextLink(-1)->m_prevlink = this->m_prevlink;
        this->m_prevlink->m_next = this->m_next;

        this->m_prevlink = nullptr;
        this->m_next = nullptr;
    }
}

#endif
