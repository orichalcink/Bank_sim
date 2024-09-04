#pragma once
#include "../lib/database.hpp"

// Provide commands list to the user.
inline void help() {
   println(
      "> Q - quit\n> T - new transaction\n> R - last transaction\n"
      "> L - list all transactions\n> B - check balance\n"
      "> O - log out of your account\n> D - deposit money\n"
      "> W - withdraw money\n> E - edit account\n", BLUE
   );
}

// Deposit some money into the account.
inline void deposit(int id, Transactions& tr) {
   int amount = getNumber("Amount to deposit > ", BLUE);

   // Create a transaction from BANK to user.
   if (tr.createTransaction(transaction(amount, 1, id))) {
      println("Successfully deposited " + str(amount) + "$.", GREEN);
   }
}

// Withdraw money from the account.
inline void withdraw(account& acc, Transactions& tr) {
   int amount = getNumber("Amount to withdraw > ", BLUE);

   // Not enough money in bank account.
   if (acc.balance < amount) {
      println("You don't have enough money to withdraw " + str(amount) + "$. "
      "You only have " + str(acc.balance) + "$.", RED);
      return;
   }

   // Create a transaction from user to BANK.
   if (tr.createTransaction(transaction(amount, acc.id, 1))) {
      println("Successfully withdraw " + str(amount) + "$.", GREEN);
   }
}

// Update accounts age.
inline void editAge(int id, Accounts& db) {
   int age = getNumber("Input your new age > ", BLUE);
      
   if (db.updateAge(age, id)) {
      println("Successfully updated age.", GREEN);
   }
}

// Update accounts username.
inline void editUsername(int id, Accounts& db) {
   std::string username = getInput("Input your new username > ", BLUE);
      
   if (db.updateName(username, id)) {
      println("Successfully updated username.", GREEN);
   }
}

// Update users password if they know the previous one.
inline void editPassword(account& acc, Accounts& db) {
   std::string pass = getHiddenInput("Input your old password > ", BLUE);

   // Passwords don't match.
   if (acc.pass != hashString(pass)) {
      println("Incorrect password.", RED);
      return;
   }

   std::string newPass = getHiddenInput("Input your new password > ", BLUE);

   // New password is not the correct size.
   if (newPass.size() < MIN_PASS_SIZE || newPass.size() > MAX_PASS_SIZE) {
      println("Password too long or too short.", RED);
      return;
   }

   if (db.updatePass(hashString(newPass), acc.id)) {
      println("Successfully updated password.", GREEN);
   }
}

// Delete the account if the user gives consent.
inline void deleteAccount(account& acc, Accounts& db) {
   if (!getConsent("Are you sure that you want to delete your "
   "account? This cannot be undone. [y/n] > ", ORANGE)) {
      println("Cancelled", RED);
   }

   if (db.deleteAccount(acc)) {
      println("Successfully deleted account.", GREEN);
   }
}

// Edit account properties.
inline void editAccount(account& acc, Accounts& db) {
   switch (getKey("Choose (A - age, N - name, P - password, D - delete) > ")) {
   case 'a':
      editAge(acc.id, db);
      break;
   case 'n':
      editUsername(acc.id, db);
      break;
   case 'p':
      editPassword(acc, db);
      break;
   case 'd':
      deleteAccount(acc, db);
      break;
   default:
      println("Unknown option.", RED);
      break;
   }
}

// Send money to a different user.
inline void createTransaction(account& acc, Accounts& db, Transactions& tr) {
   // Get receivers account.
   std::string username = getInput("Username to send the money to > ", BLUE);
   account receiver = db.selectByName(username);

   // Receiver does not exist.
   if (receiver.id == INVALID_ID || receiver.name == "DELETED") {
      println("Could not find user '" + username + "'.", RED);
      return;
   }

   // Get amount to be sent.
   int amount = getNumber("Amount to send (you have " + str(acc.balance) + "$) "
   "> ", BLUE);

   // Ask user for consent.
   if (!getConsent("Are you sure you want to send " + str(amount) + "$ to '"
   + receiver.name + "'? [y/n] > ", ORANGE)) {
      println("Cancelled transaction.", RED);
      return;
   }

   // Create transaction and update account.
   if (tr.createTransaction(transaction(amount, acc.id, receiver.id))) {
      println("Successfully sent " + str(amount) + "$ to '" 
      + receiver.name + "'.", GREEN);
   }
}

// Print the latest transaction.
inline void lastTransaction(int id, Transactions& tr) {
   transaction trans = tr.getLatestTransaction(id);
   std::string color = (trans.fromId == id) ? RED : GREEN;
   println(trans.string(), color);
}

// Retrieve and print out all of the transactions.
inline void allTransactions(int id, Transactions& tr) {
   for (transaction trans : tr.getTransactions(id)) {
      std::string color = (trans.fromId == id) ? RED : GREEN;
      println(trans.string(), color);
   }
}

// Print users ballance.
inline void getBalance(int balance) {
   std::string color = (balance < 1) ? RED : GREEN;
   println("Balance: " + str(balance) + "$", color);
}

// Log out if the user consents.
inline bool logout(account& acc) {
   // Get consent from user.
   if (!getConsent("Are you sure you want to log out? [y/n] > ", ORANGE)) {
      println("Cancelled.", RED);
      return false;
   }

   // User logged out of their account.
   println("Logged out of '" + acc.string() + "'.\n", GREEN);
   return true;
}