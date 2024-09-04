#include "actions.hpp"

// Pre-declare functions.
account login(Accounts& db);
account signup(Accounts& db);

// Main loop.
int main() {
   Accounts db;
   Transactions tr(db);

   // Create BANK account as ID 1 if it does not exist yet. Set password to 'BANK'
   // as you cannot log into an account with an unhashed password.
   if (db.selectByName("BANK").id == INVALID_ID) {
      db.createAccount(account("BANK", "BANK", 0, 0));
   }

   // Log in or sign up.
   account acc =
   (getConsent("Would you like to log in [y] or sign up [n]? > ", BLUE))
   ? login(db)
   : signup(db);

   println("\nWelcome, " + acc.name + "!", BLUE);
   println("What would you like to do today?", BLUE);

   // Action loop.
   while (true) {
      switch (getKey("Command (h for help) > ")) {
      case 'h': 
         help(); 
         break;
      case 'q':
         println("Quitting.", BLUE);
         exit(0);
      case 'd':
         deposit(acc.id, tr);
         acc = db.selectById(acc.id).at(0);
         break;
      case 'w':
         withdraw(acc, tr);
         acc = db.selectById(acc.id).at(0);
         break;
      case 'e':
         editAccount(acc, db);

         // If user deleted account then reload the program.
         if (acc.name == "DELETED") return main();
         acc = db.selectById(acc.id).at(0);
         break;
      case 't':
         createTransaction(acc, db, tr);
         acc = db.selectById(acc.id).at(0);
         break;
      case 'r':
         lastTransaction(acc.id, tr);
         break;
      case 'l':
         allTransactions(acc.id, tr);
         break;
      case 'b':
         getBalance(acc.balance);
         break;
      case 'o':
         if (logout(acc)) return main();
         break;
      default:
         // Unknown command.
         println("Unknown command, try typing 'h' for commands!", RED);
         break;
      }
   }
   return 0;
}

// Log into an existing account.
account login(Accounts& db) {
   account acc;

   while (true) {
      // Get username and password from user.
      std::string username = getInput("Input your username (s to sign up "
      "instead) > ", BLUE);
      if (username == "s" || username == "S") return signup(db);

      std::string password = getHiddenInput("Input your password > ", BLUE);
      acc = db.selectByName(username);

      // Account does not exist.
      if (acc.id == INVALID_ID) {
         println("Account with the given username does not exist.", RED);
         continue;
      }

      // Check if passwords match.
      if (acc.pass == hashString(password)) {
         println("Logged into '" + acc.string() + "'.", GREEN);
         return acc;
      } else {
         println("Incorrect password.", RED);
      }
   }

   return account();
}

// Create a brand new account.
account signup(Accounts& db) {
   account acc;

   while (acc.id == INVALID_ID) {
      // Get a new username, password and age from the user.
      std::string username = getInput("Input a new username (l to "
      "login instead) > ", BLUE).c_str();
      if (username == "l" || username == "L") return login(db);

      std::string password = getHiddenInput("Input a password > ", BLUE);
      int age = getNumber("Input your age > ", BLUE);

      // Password is within the length bounds.
      if (password.size() < MIN_PASS_SIZE || password.size() > MAX_PASS_SIZE) {
         println("Password is either too short or too long.", RED);
         continue;
      }

      // Handle all of the other edge cases in the create account function, the
      // password length cannot be checked there because it has to be hashed and
      // hashed string length is fixed.
      if (db.createAccount(account(username, hashString(password), age, 0))) {
         acc = db.selectByName(username);
         println("Signed up as '" + acc.string() + "'.", GREEN);
         return acc;
      }
   }

   return account();
}