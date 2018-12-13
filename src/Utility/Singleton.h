#ifndef Singleton_h__
#define Singleton_h__
namespace DDRFramework
{
	template<class T>
	class CSingleton
	{
	public:
		static T* Instance()
		{
			if (nullptr == m_pInstance)
			{
				m_pInstance = new T;
				atexit(Destory);

			}

			return m_pInstance;
		}

	protected:
		CSingleton() {} //防止实例
		CSingleton(const CSingleton&) {} //防止拷贝构造一个实例
		CSingleton& operator=(const CSingleton&) {} //防止赋值出另一个实例

		virtual ~CSingleton()
		{
		}

		static void Destory()
		{
			if (m_pInstance)
			{
				delete m_pInstance;
				m_pInstance = nullptr;
			}
		}

	private:
		static T* m_pInstance;
	};

	template<class T> T* CSingleton<T>::m_pInstance = nullptr;
}
#endif // Singleton_h