### Bank_sim
Bank_sim is a simple project, where you can log in, sign in or out of your accounts, deposit and withdraw fake money, send it to friends and view recent transactions. I make simple projects like this and you can do whatever you'd like to with it, read LICENSE to find out more.
Note that this likely won't work on Windows.

### Compiling
#### GCC
```g++ src/main.cpp -o build/bank -lsqlite3 -lssl -lcrypto```
#### CLANG
```clang++ src/main.cpp -o build/bank -lsqlite3 -lssl -lcrypto```
### Dependencies
You'll need to install two dependencies to compile it: sqlite3 for the database and OpenSSL for the password hashing.
Ubuntu/Desbian : ```sudo apt-get install libsqlite3-dev libssl-dev```
Fedora: ```sudo dnf install sqlite-devel openssl-devel```
Arch: ```sudo pacman -S sqlite openssl```

I'm sorry for not including how to install dependencies or the compile command for different OS's, I don't want to give someone a command that likely won't work.

### Features
1. Create a new account or log into an existing one.
2. Deposit money
3. Withdraw money
4. Send money to a different account
5. View latest or all transactions
6. Edit account information
7. View balance
8. Log out of your account
