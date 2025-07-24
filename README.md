# Concurrent-Banking-System-in-Linux
A concurrent banking system implementation in Linux that processes financial transactions (deposits, withdrawals, transfers) using multiple child processes with proper synchronization to ensure data consistency.

![C](https://img.shields.io/badge/-C-00599C?logo=c&logoColor=white)
![Linux](https://img.shields.io/badge/-Linux-FCC624?logo=linux&logoColor=black)
![Banking](https://img.shields.io/badge/%F0%9F%92%B5-Concurrent_Banking-important)
![IPC](https://img.shields.io/badge/IPC-Shared_Memory%2BSemaphores-blueviolet)
![OS](https://img.shields.io/badge/OS-Linux-9cf)

🚀 Features


Multi-process Architecture: Each transaction handled by separate child process

Thread-safe Operations: Semaphore-protected account access

Deadlock Prevention: Ordered locking strategy for transfers

Transaction Logging: Detailed audit trail with timestamps

Automatic Retry: One retry attempt for failed transactions

Shared Memory: Efficient inter-process communication

🛠️ Technical Implementation


Language: C

Synchronization: POSIX semaphores

IPC: Shared memory segments

Error Handling: Comprehensive system call checks

Logging: Transaction status tracking

