﻿/*MIT License

Copyright (c) 2024 Shell

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.*/

#pragma once

#include "Export.h"

#include <atomic>
#include <mutex>
#include <unordered_map>
#include <utility>

namespace sh::core
{
	/// @brief DLL간 공유를 위해 쓰이는 인터페이스.
	class ISingleton
	{
	private:
		static std::mutex mu;
		static std::unordered_map<uint64_t, void*> instance;
	protected:
		struct Result
		{
			void* ptr;
			bool needInit;
		};

		SH_CORE_API static auto CreateInstance(uint64_t hash, std::size_t size) -> Result;
		SH_CORE_API static void DeleteInstance(uint64_t hash);
	};

	/// @brief 스레드에 안전한 싱글톤 클래스.
	/// @tparam T 타입
	/// @tparam shareDLL DLL간 메모리 영역을 공유할지
	template<typename T, bool shareDLL = true>
	class Singleton : private ISingleton
	{
	private:
		static std::atomic<T*> instance;
		static std::mutex mu;
	protected:
		Singleton() = default;
	public:
		static auto GetInstance()->T*;
		static void Destroy();
	};

	template<typename T>
	class Singleton<T, false>
	{
	private:
		static std::atomic<T*> instance;
	protected:
		Singleton() = default;
	public:
		static auto GetInstance() -> T*;
		static void Destroy();
	};

	template<typename T, bool shareDLL>
	std::atomic<T*> Singleton<T, shareDLL>::instance{};
	template<typename T, bool shareDLL>
	std::mutex Singleton<T, shareDLL>::mu{};

	template<typename T>
	std::atomic<T*> Singleton<T, false>::instance{};
	/// @brief 싱글톤 인스턴스를 생성하거나 가져온다.
	/// 
	/// @details 같은 dll내 멀티 스레드 환경에서는 lock을 쓰지 않고 memory_order를 이용하여 lock-free로 구현.
	/// @details 다른 dll내 멀티 스레드 환경에서는 ISingleton의 CreateInstance내부에서 락을 써서 구현.
	/// 
	/// @tparam T 타입
	/// @tparam shareDLL DLL간 메모리 영역을 공유할지
	/// @return 객체 포인터
	template<typename T, bool shareDLL>
	auto Singleton<T, shareDLL>::GetInstance() -> T*
	{
		T* instancePtr = instance.load(std::memory_order::memory_order_acquire);
		if (instancePtr == nullptr)
		{
			std::lock_guard<std::mutex> lockGuard{ mu };
			instancePtr = instance.load(std::memory_order::memory_order_relaxed);
			if (instancePtr == nullptr)
			{
				if constexpr (shareDLL)
				{
					uint64_t hash = typeid(T).hash_code();
					Result result = CreateInstance(hash, sizeof(T));
					void* resultPtr = result.ptr;
					if (result.needInit)
						new (resultPtr) T();
					instancePtr = reinterpret_cast<T*>(resultPtr);
				}
				else
					instancePtr = new T{};
				instance.store(instancePtr, std::memory_order::memory_order_release);
			}	
		}
		return instancePtr;
	}

	/// @brief 생성 돼 있는 싱글톤 인스턴스의 메모리를 해제한다.
	/// @tparam T 타입
	template<typename T, bool shareDLL>
	void Singleton<T, shareDLL>::Destroy()
	{
		T* instancePtr = instance.load(std::memory_order::memory_order_acquire);
		if (instancePtr != nullptr)
		{
			std::lock_guard<std::mutex> guard{ mu };
			instancePtr = instance.load(std::memory_order::memory_order_relaxed);
			if (instancePtr != nullptr)
			{
				if constexpr (shareDLL)
				{
					instancePtr->~T();
					DeleteInstance(typeid(T).hash_code());
				}
				else
					delete instancePtr;
				instance.store(nullptr, std::memory_order::memory_order_release);
			}
		}
	}
}//namespace