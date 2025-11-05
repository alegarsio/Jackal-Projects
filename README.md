# Jackal

Jackal is a dynamic, object-oriented interpreted programming language built from scratch in C. This project is designed as an in-depth exploration of language implementation, covering a lexer, a recursive descent parser, and a tree-walking evaluator.

The language is named for its agility and cleverness, represented by our mascot, **Jack** the Fox!

## Key Features

* **Dynamic Typing:** No complex type declarations required.
* **Fully Object-Oriented:** Support for `class`, `this`, methods, constructors (`init`), and object properties.
* **Modern Data Structures:** Array literals (`[1, 2, 3]`) with easy index access and manipulation.
* **First-Class Functions:** Functions can be stored in variables, passed as arguments, and support closures.
* **Complete Control Flow:** `if`/`else if`/`else`, `while` loops, and C-style `for` loops.
* **Comprehensive Operators:** Arithmetic, comparison, logical (`&&` with short-circuit), and postfix (`i++`, `i--`).

## Quick Start

### Prerequisites

* Standard C compiler (like `gcc` or `clang`)
* `make`

### Installation & Build

1.  **Clone this repository:**
    ```bash
    git clone [https://github.com/alegarsio/Jackal-Projects](https://github.com/alegarsio/Jackal-Projects)
    cd jackal
    ```
2.  **Compile the interpreter:**
    ```bash
    make jackal
    ```

### Running a Program

Create a file with a `.jackal` extension and run it using the newly built interpreter:

```bash
./jackal your_script.jackal