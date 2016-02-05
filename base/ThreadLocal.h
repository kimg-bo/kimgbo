#ifndef THREADLOCAL_H
#define THREADLOCAL_H

#include<pthread.h>

namespace kimgbo
{
	template<typename T>
	class ThreadLocal
	{
	public:
		ThreadLocal()
		{
			pthread_key_create(&m_pkey, &ThreadLocal::destructor);
		}
		
		~ThreadLocal()
		{
			pthread_key_delete(m_pkey);
		}
		
		T& value()
		{
			T* perThreadValue = static_cast<T*>(pthread_getspecific(m_pkey));
			if(!perThreadValue)
			{
				T* newObj = new T();
				pthread_setspecific(m_pkey, newObj);
				perThreadValue = newObj;
			}
			return *perThreadValue;
		}
		
	private:
		static void destructor(void *arg)
		{
			T* obj = static_cast<T*>(arg);
			typedef char T_must_be_complete_type[sizeof(T) == 0 ? -1 : 1];
			delete obj;
		}
		
	private:
		pthread_key_t m_pkey;
	};
}


#endif