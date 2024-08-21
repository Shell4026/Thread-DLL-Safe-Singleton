# Thread-DLL-Safe-Singleton
c++17 Thread-safe, Across dll singleton

같은 DLL내 멀티 스레드 환경에서는 lock을 쓰지 않고 memory_order를 이용하여 lock-free로 구현합니다.

다른 DLL내 멀티 스레드 환경에서는 ISingleton클래스의 CreateInstance 함수 내부에서 락을 써서 구현합니다.

Windows와 Ubuntu 환경에서 정상 작동 확인했습니다.