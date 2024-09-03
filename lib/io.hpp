#pragma once
#include <openssl/sha.h>
#include <termios.h>
#include <unistd.h>
#include <limits>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>

// Convert integer to string.
inline std::string str(int number) {
   return std::to_string(number);
}

// Convert char[] to string.
inline std::string str(std::string prompt) {
   return prompt;
}

// Used for terminal ANSI escape codes.
struct rgb {
   int r, g, b;

   rgb(): r(0), g(0), b(0) {}
   rgb(int size): r(size), g(size), b(size) {}
   rgb(int r, int g, int b): r(r), g(g), b(b) {}

   // Convert to string.
   operator std::string() const {
      return "\033[38;2;" + str(r) + ";" + str(g) + ";" + str(b) + "m";
   }
};

// Predefined colors.
const rgb RED(222, 63, 42);
const rgb ORANGE(240, 157, 34);
const rgb GREEN(126, 232, 51);
const rgb BLUE(58, 122, 224);
const std::string RES = "\033[0m";

// Print to terminal.
inline void print() {
   std::cout << "";
}

inline void print(std::string prompt) {
   std::cout << prompt;
}

inline void print(std::string prompt, std::string color) {
   std::cout << color << prompt << RES;
}

inline void print(char prompt) {
   std::cout << prompt;
}

inline void print(char prompt, std::string color) {
   std::cout << color << prompt << RES;
}

// Print to terminal with a newline character at the end.
inline void println() {
   std::cout << "\n";
}

inline void println(std::string prompt) {
   std::cout << prompt << "\n";
}

inline void println(std::string prompt, std::string color) {
   std::cout << color << prompt << "\n" << RES;
}

inline void println(char prompt) {
   std::cout << prompt << "\n";
}

inline void println(char prompt, std::string color) {
   std::cout << color << prompt << "\n" << RES;
}

// Get string input from the user.
inline std::string getInput(std::string prompt) {
   print(prompt);
   std::string input;
   std::getline(std::cin, input);
   return input;
}

inline std::string getInput(std::string prompt, std::string color) {
   print(prompt, color);
   std::string input;
   std::getline(std::cin, input);
   return input;
}

inline int getNumber(std::string prompt) {
   print(prompt);
   int number;

   while (!(std::cin >> number)) {
      std::cin.clear();
      std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
      print("Invalid input. Please try again > ", RED);
   }
   getInput("");
   return number;
}

inline int getNumber(std::string prompt, std::string color) {
   print(prompt, color);
   int number;

   while (!(std::cin >> number)) {
      std::cin.clear();
      std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
      print("Invalid input. Please try again > ", RED);
   }
   getInput("");
   return number;
}

// Save original terminal settings.
struct termios original;

// Set raw mode so enter is not necessary to get a character input.
inline void setRawMode() {
   tcgetattr(STDIN_FILENO, &original);

   struct termios tty;
   tcgetattr(STDIN_FILENO, &tty);
   tty.c_lflag &= ~(ECHO | ICANON);

   tcsetattr(STDIN_FILENO, TCSANOW, &tty);
}

// Reset terminal to original state.
inline void resetTerminal() {
   tcsetattr(STDIN_FILENO, TCSANOW, &original);
}

// Get a string while hiding what's typed by the user.
inline std::string getHiddenInput(std::string prompt) {
   print(prompt);

   setRawMode();
   std::string input;
   std::getline(std::cin, input);
   resetTerminal();

   println();
   return input;
}

inline std::string getHiddenInput(std::string prompt, std::string color) {
   print(prompt, color);

   setRawMode();
   std::string input;
   std::getline(std::cin, input);
   resetTerminal();

   println();
   return input;
}

// Get a character input from the user.
inline char getKey(std::string prompt) {
   print(prompt);
   setRawMode();

   char key = std::tolower(getchar());
   resetTerminal();
   println(key);
   return key;
}

inline char getKey(std::string prompt, std::string color) {
   print(prompt, color);
   setRawMode();

   char key = std::tolower(getchar());
   resetTerminal();
   println(key);
   return key;
}

// Get consent from the user, returns true if the character is 'y', else false.
inline bool getConsent(std::string prompt) {
   return getKey(prompt) == 'y';
}

inline bool getConsent(std::string prompt, std::string color) {
   return getKey(prompt, color) == 'y';
}

// Hash a string for security reasons, for example, a password. This code is not
// by me and i don't understand how it works.
inline std::string hashString(std::string prompt) {
   unsigned char hash[SHA256_DIGEST_LENGTH];
   SHA256((unsigned char*)prompt.c_str(), prompt.size(), hash);
   
   std::stringstream ss;
   for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
      ss << std::hex << std::setw(2) << std::setfill('0') << (int)hash[i];
   }

   return ss.str();
}