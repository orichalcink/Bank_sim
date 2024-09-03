#include "../lib/database.hpp"

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

   while (true) {
      // Garbage variables that might be used later.
      int amount;
      std::string username;
      account receiver;
      std::vector<transaction> transactions;
      transaction trans;

      switch (getKey("Command (h for help) > ")) {
         case 'h':
            // Print command to the terminal.
            println(
               "> Q - quit\n> T - new transaction\n> R - last transaction\n"
               "> L - list all transactions\n> B - check balance\n"
               "> O - log out of your account\n> D - deposit money\n"
               "> W - withdraw money\n> E - edit account\n", BLUE
            );
            break;
         case 'q':
            // Exit the program.
            println("Quitting.", BLUE);
            exit(0);
         case 'd':
            amount = getNumber("Amount to deposit > ", BLUE);

            // Create a transaction from BANK to user.
            if (tr.createTransaction(transaction(amount, 1, acc.id))) {
               println("Successfully deposited " + str(amount) + "$.", GREEN);
            }

            // Reload user.
            acc = db.selectById(acc.id).at(0);
            break;
         case 'w':
            amount = getNumber("Amount to withdraw > ", BLUE);

            // Not enough money in bank account.
            if (acc.balance < amount) {
               println("You don't have enough money to withdraw " + str(amount)
               + "$. You only have " + str(acc.balance) + "$.", RED);
               break;
            }

            // Create a transaction from user to BANK.
            if (tr.createTransaction(transaction(amount, acc.id, 1))) {
               println("Successfully withdraw " + str(amount) + "$.", GREEN);
            }

            // Reload user data.
            acc = db.selectById(acc.id).at(0);
            break;
         case 'e':
            switch (getKey("Choose (A - age, N - name, P - password, D - delete) > ")) {
               case 'a':
                  // Update age.
                  amount = getNumber("Input your new age > ", BLUE);
                  
                  if (db.updateAge(amount, acc.id)) {
                     println("Successfully updated age.", GREEN);
                  }
                  acc = db.selectById(acc.id).at(0);
                  break;
               case 'n':
                  // Update username.
                  username = getInput("Input your new username > ", BLUE);
                  
                  if (db.updateName(username, acc.id)) {
                     println("Successfully updated username.", GREEN);
                  }
                  acc = db.selectById(acc.id).at(0);
                  break;
               case 'p':
                  // Update password if the user can provide his old one.
                  username = getHiddenInput("Input your old password > ", BLUE);

                  // Passwords don't match.
                  if (acc.pass != hashString(username)) {
                     println("Incorrect password.", RED);
                     break;
                  }

                  username = getHiddenInput("Input your new password > ", BLUE);

                  // New password is not the correct size.
                  if (username.size() < MIN_PASS_SIZE || username.size() > MAX_PASS_SIZE) {
                     println("Password too long or too short.", RED);
                     break;
                  }

                  if (db.updatePass(hashString(username), acc.id)) {
                     println("Successfully updated password.", GREEN);
                  }
                  acc = db.selectById(acc.id).at(0);
                  break;
               case 'd':
                  if (!getConsent("Are you sure that you want to delete your "
                  "account? This cannot be undone. [y/n] > ", ORANGE)) {
                     println("Cancelled", RED);
                  }

                  if (db.deleteAccount(acc)) {
                     println("Successfully deleted account.", GREEN);
                  }
                  return main();
               default:
                  println("Unknown option.", RED);
                  break;
            }
            break;
         case 't':
            // Get receivers account.
            username = getInput("Username to send the money to > ", BLUE);
            receiver = db.selectByName(username);

            // Receiver does not exist.
            if (receiver.id == INVALID_ID || receiver.name == "DELETED") {
               println("Could not find user '" + username + "'.", RED);
               break;
            }

            // Get amount to be sent.
            amount = getNumber("Amount to send (you have " + str(acc.balance)
             + "$) > ", BLUE);

            // Ask user for consent.
            if (!getConsent("Are you sure you want to send " + str(amount) 
            + "$ to '" + receiver.name + "'? [y/n] > ", ORANGE)) {
               println("Cancelled transaction.", RED);
               break;
            }
            
            // Create transaction and update account.
            if (tr.createTransaction(transaction(amount, acc.id, receiver.id))) {
               println("Successfully sent " + str(amount) + "$ to '" 
               + receiver.name + "'.", GREEN);
            }
            acc = db.selectById(acc.id).at(0);
            break;
         case 'r':
            // Get the latest transaction and print it out.
            trans = tr.getLatestTransaction(acc.id);
            username = (trans.fromId == acc.id) ? RED : GREEN;
            println(trans.string(), username);
            break;
         case 'l':
            // Get all of the transactions.
            transactions = tr.getTransactions(acc.id);

            // Print transactions.
            for (auto tran : transactions) {
               username = (tran.fromId == acc.id) ? RED : GREEN;
               println(tran.string(), username);
            }
            break;
         case 'b':
            // Print balance.
            username = (acc.balance < 1) ? RED : GREEN;
            println("Balance: " + str(acc.balance) + "$", username);
            break;
         case 'o':
            // Get consent from user.
            if (!getConsent("Are you sure you want to log out? [y/n] > ", ORANGE)) {
               println("Cancelled.", RED);
               break;
            }

            // User logged out of their account.
            println("Logged out of '" + acc.string() + "'.\n", GREEN);
            return main();
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