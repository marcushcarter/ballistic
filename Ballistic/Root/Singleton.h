#pragma once

namespace ballistic
{
    template<typename T>
    class Singleton
    {
    protected:
        static T* msInstance;

        Singleton() { msInstance = static_cast<T*>(this); }
        virtual ~Singleton() { msInstance = nullptr; }

    public:
        Singleton(const Singleton&) = delete;
        Singleton& operator=(const Singleton&) = delete;

        static T& getSingleton() {
            return *msInstance;
        }

        static T* getSingletonPtr() {
            return msInstance;
        }
    };

    template<typename T>
    T* Singleton<T>::msInstance = nullptr;
    
} // namespace ballistic
