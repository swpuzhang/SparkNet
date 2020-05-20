#ifndef SINGLETON_H
#define SINGLETON_H
#include <mutex>
#include <memory>

template<typename T>
class MagicSingleton {
public:

	//��ȡȫ�ֵ�������
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

	//����������������һ�㲻��Ҫ����������������������
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
