# Trade
README for bank.cpp

Overview
--------
bank.cpp is a C++ program simulating a banking system, designed to manage user accounts, transactions, and query operations. It utilizes a priority queue to handle transaction execution efficiently, based on their scheduled execution time and transaction ID for resolving conflicts. The program reads user data from a file, processes commands interactively, and provides detailed transaction and account summaries.

Features
--------
1. User Registration:
   - Reads user account details (e.g., registration time, PIN, balance) from a specified file.
   - Stores users in an unordered_map for quick access.

2. Transaction Management:
   - Handles transactions with sender, recipient, amount, execution time, and fees.
   - Uses a priority queue to schedule and execute transactions based on the earliest execution time.

3. Account Operations:
   - Login/logout functionalities to manage active sessions.
   - Supports querying user balances and transaction histories.

4. Query Functionality:
   - Summarizes transactions over specific periods.
   - Calculates total fees collected by the bank.
   - Retrieves detailed transaction history for individual users.

5. Command Line Options:
   - -f or --file: Specify the registration file.
   - -v or --verbose: Enable verbose logging.
   - -h or --help: Show usage instructions.

Priority Queue Usage
--------------------
The program employs a priority queue to manage transactions effectively:
- Purpose: Transactions are ordered based on their execution time (execTime) and, if equal, by their transaction ID (id).
- Comparator:
  - Transactions with earlier execution times are given higher priority.
  - If two transactions share the same execution time, the one with a lower id is prioritized.
- Implementation:
  - Defined as std::priority_queue<transact, std::vector<transact>, TransactCompare>.

This ensures that all pending transactions are executed in the correct order, maintaining fairness and accuracy.
