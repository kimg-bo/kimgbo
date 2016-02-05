#ifndef THREADLOCALSINGLETON_H
#define THREADLOCALSINGLETON_H

#include<pthread.h>
#include<assert.h>

namespace kimgbo
{
	template<typename T>
	class ThreadLocalSingleton
	{
	public:
		static T& instance()
		{
			if(!m_t_value)
			{
				m_t_value = new T();
				m_deleter.set(m_t_value);
			}
			return *m_t_value;
		}
		
		T* pointer()
		{
			return m_t_value;
		}
		
	private:
		static void destructor(void* arg)
		{
			assert(arg == m_t_value);
			typedef char T_must_be_complete_type[sizeof(T) == 0 ? -1 : 1];
			delete m_t_value;
			m_t_value = 0;
		}
		
		//delete Singleton object
		class Deleter
		{
		public:
			Deleter()
			{
				pthread_key_create(&m_key, &ThreadLocalSingleton::destructor);
			}
			
			~Deleter()
			{
				pthread_key_delete(m_key);
			}
			
			void set(T* newObj)
			{
				assert(pthread_getspecific(m_key) == NULL);
				pthread_setspecific(m_key, newObj);
			}
			
			pthread_key_t m_key;
		};
		
		static __thread T* m_t_value;
		static Deleter m_deleter;
	};
	
	template<typename T>
	__thread T* ThreadLocalSingleton<T>::m_t_value = 0;
		
	template<typename T>
	typename ThreadLocalSingleton<T>::Deleter  ThreadLocalSingleton<T>::m_deleter;
	
}


#endif