#ifndef SINGLETON_H
#define SINGLETON_H
#include <mutex>
#include <memory>

template<typename T>
class MagicSingleton {
public:

	//获取全局单例对象
	template<typename ...Args>
	static std::shared_ptr<T> GetInstance(Args&&... args) {
		if (!_pSington) {
			std::lock_guard<std::mutex> gLock(_mutex);
			if (nullptr == _pSington) {
				_pSington = std::make_shared<T>(std::forward<Args>(args)...);
			}
		}
		return _pSington;
	}

	//主动析构单例对象（一般不需要主动析构，除非特殊需求）
	static void DesInstance() {
		if (_pSington) {
			_pSington.reset();
			_pSington = nullptr;
		}
	}

private:
	explicit MagicSingleton() = default;
	MagicSingleton(const MagicSingleton&) = delete;
	MagicSingleton& operator=(const MagicSingleton&) = delete;
	~MagicSingleton() = default;

private:
	static std::shared_ptr<T> _pSington;
	static std::mutex _mutex;
};

template<typename T>
std::shared_ptr<T> MagicSingleton<T>::_pSington = nullptr;

template<typename T>
std::mutex MagicSingleton<T>::_mutex;
#endif // !SINGLETON_H
