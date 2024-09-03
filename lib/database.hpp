#pragma once
#include <iostream>
#include <vector>
#include <string>

#include "io.hpp"
#include "sqlite3.h"

// Account struct for the account database.
struct account {
   std::string name, pass;
   int id, age, balance;

   account(): id(-1), name(""), pass(""), age(-1), balance(-1) {}

   account(std::string name, std::string pass, int age, int balance) 
   : id(-1), name(name), pass(pass), age(age), balance(balance) {}

   account(int id, std::string name, std::string pass, int age, int balance) 
   : id(id), name(name), pass(pass), age(age), balance(balance) {}

   account& operator=(const account& other) {
      if (this == &other) return *this;

      id = other.id;
      pass = other.pass;
      name = other.name;
      age = other.age;
      balance = other.balance;

      return *this;
   }

   // Convert to formal string.
   std::string string() const {
      return name + " Id: " + str(id);
   }
};

// Transactions struct for the transactions database.
struct transaction {
   std::string date, from, to;
   int id, fromId, toId, amount;

   transaction(): date(""), id(-1), amount(-1), fromId(-1), toId(-1) {}

   transaction(int amount, int fromId, int toId)
   : date(""), id(-1), amount(amount), fromId(fromId), toId(toId) {}

   transaction(int id, int fromId, int toId, int amount, std::string date,
   std::string from, std::string to): date(date), id(id), amount(amount), 
   fromId(fromId), toId(toId), from(from), to(to) {}

   // Convert to formal string.
   std::string string() const {
      return str(amount) + "$ From '" + from + "' To '" + to + "' at " + date;
   }
};

// Database class for creating databases.
class Database {
public:
   // Create a new db file and create a new table if they do not exist yet.
   Database(std::string fileName, std::string sql) {
      int failed = sqlite3_open(fileName.c_str(), &db);

      if (failed) {
         println(str("Fatal error: Database could not be opened, ") 
         + sqlite3_errmsg(db), RED);
         exit(-1);
      }

      // Enable foreign key support for transactions database.
      sqlite3_exec(db, "PRAGMA foreign_keys = ON;", nullptr, nullptr, nullptr);

      int status = sqlite3_exec(db, sql.c_str(), nullptr, 0, &errorMsg);
      if (status != SQLITE_OK) {
         println(str("Fatal Error: Could not create table: ") + errorMsg, RED);
         sqlite3_free(errorMsg);
         exit(-2);
      }
   }
   
   // Close the database when the class is deleted to save memory.
   ~Database() {
      sqlite3_close(db);
   };

protected:
   sqlite3* db;
   char* errorMsg;

   // Handle SQL insertion errors.
   bool handleInsertion(int status, std::string error) {
      if (status != SQLITE_DONE) {
         println(error + sqlite3_errmsg(db), RED);
      }
      return status == SQLITE_DONE;
   }
};

// Account database used to store all of the id's, names, ages and balances
// of users.
class Accounts : public Database {
public:
   // Create a new database with the file name and an ACCOUNTS table.
   Accounts() : Database("database/database.db", 
      "CREATE TABLE IF NOT EXISTS ACCOUNTS("
      "ID INTEGER PRIMARY KEY AUTOINCREMENT, "
      "NAME TEXT NOT NULL, "
      "PASS TEXT NOT NULL, "
      "AGE INT NOT NULL, "
      "BALANCE INT NOT NULL);"
   ) {}

   // Create a new account if the given information is valid and it doesn't
   // exist yet.
   bool createAccount(const account& acc) {
      // Username already exists.
      if (selectByName(acc.name).id != -1) {
         println("Could not create account with username '" + acc.name + "' as "
         "an account with that name already exists, try a different name!", RED);
         return false;
      }

      // Username too long or too short.
      if (acc.name.size() > 24 || acc.name.size() < 3) {
         println("Username is either too long or too short. Lower limit is 3 "
         "and upper limit is 24.", RED);
         return false;
      }

      // User too young or too old.
      if (acc.age < 18 || acc.age > 99) {
         println("Age is either too low or too high. You must be atleast 18 to "
         "use our program. If you're above 99 years old, please input '99' as "
         "it is the upper limit.", RED);
         return false;
      }

      // Create an SQL prepared statement.
      std::string sql = 
      "INSERT INTO ACCOUNTS "
      "(NAME,PASS,AGE,BALANCE) VALUES (?,?,?,?);";
      sqlite3_stmt* stmt;
      sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, 0);

      // Bind to the statement.
      sqlite3_bind_text(stmt, 1, acc.name.c_str(), -1, SQLITE_STATIC);
      sqlite3_bind_text(stmt, 2, acc.pass.c_str(), -1, SQLITE_STATIC);
      sqlite3_bind_int(stmt, 3, acc.age);
      sqlite3_bind_int(stmt, 4, acc.balance);

      // Execute, close and check for errors.
      int status = sqlite3_step(stmt);
      sqlite3_finalize(stmt);
      return handleInsertion(status, "Could not create an account: ");
   }

   // Delete the account if it exists.
   bool deleteAccount(int id) {
      // Account does not exist.
      if (selectById(id).size() < 1) {
         println("Account with id of '" + str(id) + "' does not exist, so it "
         "cannot be deleted.", RED);
         return false;
      }

      // Create an SQL prepared statement.
      std::string sql = "DELETE FROM ACCOUNTS WHERE ID = ?;";
      sqlite3_stmt* stmt;
      sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, 0);

      // Bind to statement.
      sqlite3_bind_int(stmt, 1, id);

      // Finish up and check for errors.
      int status = sqlite3_step(stmt);
      sqlite3_finalize(stmt);
      return handleInsertion(status, "Could not delete the account: ");
   }

   // Update user's name.
   bool updateName(std::string name, int id) {
      return update("NAME = '" + name + "'", id);
   }

   // Update user's balance.
   bool updateBalance(int balance, int id) {
      return update("BALANCE = " + str(balance), id);
   }

   // Update user's age.
   bool updateAge(int age, int id) {
      return update("AGE = " + str(age), id);
   }

   // Select all of the users.
   std::vector<account> selectAll() {
      return selectAccounts("", -1);
   }

   // Select a user with the given name.
   account selectByName(std::string name) {
      // Prepare the statement.
      std::string sql = "SELECT * FROM ACCOUNTS WHERE NAME = ?;";
      sqlite3_stmt* stmt;
      sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, 0);

      // Bind the name to the statement and return an account if there is one.
      sqlite3_bind_text(stmt, 1, name.c_str(), -1, SQLITE_STATIC);
      std::vector<account> accounts = retrieveInfo(stmt);
      return ((accounts.size() > 0) ? accounts.at(0) : account());
   }

   // Select users by id.
   std::vector<account> selectById(int id, std::string op = "=") {
      return selectAccounts("WHERE ID " + op, id);
   }

   // Select users by age.
   std::vector<account> selectByAge(int age, std::string op = "=") {
      return selectAccounts("WHERE AGE " + op, age);
   }

   // Select users by balance.
   std::vector<account> selectByBalance(int balance, std::string op = "=") {
      return selectAccounts("WHERE BALANCE " + op, balance);
   }

private:
   // Update given user's property like age, name and such.
   bool update(std::string type, int id) {
      // Create an SQL prepared statement.
      std::string sql = "UPDATE ACCOUNTS SET " + type + " WHERE ID = ?;";
      sqlite3_stmt* stmt;
      sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, 0);

      // Bind id to the statement.
      sqlite3_bind_int(stmt, 1, id);

      // Finish up and handle errors.
      int status = sqlite3_step(stmt);
      sqlite3_finalize(stmt);
      return handleInsertion(status, "Could not update account: ");
   }

   // Select all accounts or accounts by property.
   std::vector<account> selectAccounts(std::string type, int value) {
      // Prepare a statement.
      std::string sql = "SELECT * FROM ACCOUNTS " + type + " ?;";
      if (type.empty()) sql = "SELECT * FROM ACCOUNTS;";

      sqlite3_stmt* stmt;
      int status = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, 0);

      // Bind to the statement unless everything is selected.
      if (!type.empty()) sqlite3_bind_int(stmt, 1, value);
      return retrieveInfo(stmt);
   }

   // Retrieves info from the accounts table based on a statement.
   std::vector<account> retrieveInfo(sqlite3_stmt* stmt) {
      std::vector<account> accounts;

      // Get all of the users data.
      while (sqlite3_step(stmt) == SQLITE_ROW) {
         // ID, NAME, PASS, AGE, BALANCE
         int id = sqlite3_column_int(stmt, 0);
         std::string name = reinterpret_cast<const char*>(
            sqlite3_column_text(stmt, 1)
         );
         std::string pass = reinterpret_cast<const char*>(
            sqlite3_column_text(stmt, 2)
         );
         int age = sqlite3_column_int(stmt, 3);
         int balance = sqlite3_column_int(stmt, 4);

         accounts.push_back(account(id, name, pass, age, balance));
      }

      // Finalize and return account list.
      sqlite3_finalize(stmt);
      return accounts;
   }
};

// Transaction database for keeping track of transactions.
class Transactions : public Database {
public:
   // Create a new database with the given file name and table TRANSACTIONS if
   // there are none.
   Transactions(const Accounts& acc) : Database("database/database.db",
      "CREATE TABLE IF NOT EXISTS TRANSACTIONS("
      "ID INTEGER PRIMARY KEY AUTOINCREMENT, "
      "SENDER INTEGER, "
      "RECEIVER INTEGER NOT NULL, "
      "AMOUNT INTEGER NOT NULL, "
      "DATE DATETIME DEFAULT CURRENT_TIMESTAMP, "
      "FOREIGN KEY (SENDER) REFERENCES ACCOUNTS(ID), "
      "FOREIGN KEY (RECEIVER) REFERENCES ACCOUNTS(ID));"
   ), acc(acc) {}

   // Create a new transaction.
   bool createTransaction(transaction trans) {
      // One or both users don't exist.
      if (acc.selectById(trans.fromId).size() < 1 
      || acc.selectById(trans.toId).size() < 1) {
         println("One or both of the users does not exist.", RED);
         return false;
      }

      // Amount sent is not sufficient.
      if (trans.amount < 1) {
         println("Cannot send less than 1$.", RED);
         return false;
      }

      // Manage money accordingly.
      account sender = acc.selectById(trans.fromId).at(0);
      account receiver = acc.selectById(trans.toId).at(0);

      acc.updateBalance(sender.balance - trans.amount, sender.id);
      acc.updateBalance(receiver.balance + trans.amount, receiver.id);

      // Create SQL prepared statement.
      std::string sql = "INSERT INTO TRANSACTIONS (RECEIVER,SENDER,AMOUNT) "
      "VALUES(?,?,?);";
      sqlite3_stmt* stmt;
      sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, 0);

      // Bind to the statement.
      sqlite3_bind_int(stmt, 1, trans.toId);
      sqlite3_bind_int(stmt, 2, trans.fromId);
      sqlite3_bind_int(stmt, 3, trans.amount);

      // Finalize and check for errors.
      int status = sqlite3_step(stmt);
      sqlite3_finalize(stmt);
      return handleInsertion(status, "Could not create transaction.");
   }

   // Get the latest transaction where the given user has either received money
   // or sent money to someone else.
   transaction getLatestTransaction(int userId) {
      // User does not exist.
      if (acc.selectById(userId).size() < 1) {
         println("Account does not exist.", RED);
      }

      // Create SQL prepared statement. Orders by date descending and gets the
      // latest transaction.
      std::string sql = 
      "SELECT * FROM TRANSACTIONS WHERE RECEIVER = ? OR SENDER = ? "
      "ORDER BY DATE DESC LIMIT 1;";
      sqlite3_stmt* stmt;
      sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, 0);

      // Bind id to both values of the statement.
      sqlite3_bind_int(stmt, 1, userId);
      sqlite3_bind_int(stmt, 2, userId);

      // Finish up and return the first transaction or an invalid one.
      std::vector<transaction> transactions = retrieveInfo(stmt);
      return ((transactions.size() > 0) ? transactions.at(0) : transaction());
   }

   // Get all of the transactions by a specific user.
   std::vector<transaction> getTransactions(int userId) {
      // User does not exist.
      if (acc.selectById(userId).size() < 1) {
         println("Account does not exist.", RED);
      }

      // Create SQL prepared statement.
      std::string sql = 
      "SELECT * FROM TRANSACTIONS WHERE RECEIVER = ? OR SENDER = ? "
      "ORDER BY DATE ASC;";
      sqlite3_stmt* stmt;
      sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, 0);
      
      // Bind both of the values to the user id.
      sqlite3_bind_int(stmt, 1, userId);
      sqlite3_bind_int(stmt, 2, userId);

      // Return all of the transactions by user.
      return retrieveInfo(stmt);
   }

private:
   Accounts acc;

   // Retrieves info from transactions based on statement.
   std::vector<transaction> retrieveInfo(sqlite3_stmt* stmt) {
      std::vector<transaction> transactions;

      // Get all of the transaction data.
      while (sqlite3_step(stmt) == SQLITE_ROW) {
         // ID, FROM, TO, AMOUNT, DATE
         int id = sqlite3_column_int(stmt, 0);
         int from = sqlite3_column_int(stmt, 1);
         int to = sqlite3_column_int(stmt, 2);
         int amount = sqlite3_column_int(stmt, 3);
         std::string date = reinterpret_cast<const char*> (
            sqlite3_column_text(stmt, 4)
         );

         account sender = acc.selectById(from).at(0);
         account receiver = acc.selectById(to).at(0);

         transactions.push_back(transaction(
            id, from, to, amount, date, sender.name, receiver.name
         ));
      }

      // Finalize and return transaction list.
      sqlite3_finalize(stmt);
      return transactions;
   }
};