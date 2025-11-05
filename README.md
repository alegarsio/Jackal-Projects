# Jackal 

Jackal is a simple, interpreted programming language built in C. This project was created as a way to learn how a *lexer*, *parser*, and *evaluator* work together.

## Current Features
* `let` variable declaration
* Number and String data types
* Arithmetic operators (`+`, `-`, `*`, `/`)
* Comparison operators (`==`, `!=`, `>`, `<`, `>=`, `<=`)
* Logical operators (`&&`)
* `if`, `else if`, and `else` statements
* `print` statements to output to the console
* Line comments (`//`)

## How to Run

This project uses `make` to compile.

**1. Build the Project**
(Make sure you have `gcc` and `make` installed)
```bash
# Clean old build files (optional)
make clean

# Compile the 'jackal' executable
make jackal
