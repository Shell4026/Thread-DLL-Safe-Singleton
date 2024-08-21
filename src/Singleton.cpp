/*MIT License

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

#include "Singleton.hpp"

#include <cstring>
namespace sh::core
{
	std::mutex ISingleton::mu{};
	std::unordered_map<uint64_t, void*> ISingleton::instance{};

	auto ISingleton::CreateInstance(uint64_t hash, std::size_t size) -> Result
	{
		std::lock_guard<std::mutex> lock{ mu };

		auto it = instance.find(hash);
		if (it == instance.end())
		{
			void* ptr = ::operator new(size);
			std::memset(ptr, 0, size);
			instance.insert({ hash, ptr });

			return Result{ ptr, true };
		}
		return Result{ it->second, false };
	}

	void ISingleton::DeleteInstance(uint64_t hash)
	{
		std::lock_guard<std::mutex> lock{ mu };

		auto it = instance.find(hash);
		if (it == instance.end())
			return;

		::operator delete(it->second);
		instance.erase(it);
	}
}//namespace