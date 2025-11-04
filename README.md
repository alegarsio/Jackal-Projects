# Jackal 🍂

Jackal is a simple, interpreted programming language built in C. This project was created as a way to learn how a *lexer*, *parser*, and *evaluator* work together.

This project implements a full language pipeline, including a *lexer*, *recursive descent parser*, and a *tree-walking evaluator*. It is a comprehensive exploration of language design, covering *lexical scoping*, *closures*, and *class-based objects*.

## Key Features

* **Dynamic Typing:** Variables do not require type declarations.
* **Object-Oriented:** `class`, `this`, *methods*, *initializers* (`init`), and *fields*.
* **Data Structures:** Supports `Array` literals (`[1, 2, 3]`), index access (`arr[0]`), and assignment (`arr[0] = 5`).
* **First-Class Functions:** Functions are values; they can be stored in variables, passed as arguments, and support *closures* (capturing the scope where they are defined).
* **Full Control Flow:** `if`/`else if`/`else`, `while` loops, and C-style `for` loops.
* **Complete Operators:**
    * Arithmetic: `+`, `-`, `*`, `/`
    * Comparison: `==`, `!=`, `<`, `<=`, `>`, `>=`
    * Logical: `&&` (with *short-circuiting*)
    * Postfix: `i++`, `i--`

## Syntax At a Glance

The following code demonstrates most of the language's features:

```jackal
// Welcome to Jackal!

class Greeter {
    // Initializer (constructor)
    function init(prefix) {
        this.prefix = prefix;
    }

    // Method
    function greet(name) {
        print this.prefix + ", " + name + "!";
    }
}

// Create an instance
let myGreeter = Greeter("Hello");
let names = ["Jackal", "World", "Developer"];

// Use a for loop and array
for (let i = 0; i < 3; i++) {
    myGreeter.greet(names[i]);
}

Getting Started
This section covers how to build and run the interpreter.

Prerequisites
A C compiler (e.g., gcc or clang)

make

Build
The project is compiled using the provided Makefile.

Bash

# 1. Clean previous builds (optional)
make clean

# 2. Compile the 'jackal' executable
make jackal