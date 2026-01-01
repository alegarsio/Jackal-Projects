# Jackal Compiler

**The open source programming language for all open source scientists**

🌐 **Official Website:** [jackal-intro-page-qhqv.vercel.app](https://jackal-intro-page-qhqv.vercel.app)

Jackal is a dynamic, object-oriented interpreted programming language built from scratch in C. This project is designed as an in-depth exploration of language implementation, covering a lexer, a recursive descent parser, and a tree-walking evaluator.

The language is named for its agility and cleverness, represented by our mascot, **Jack** the Fox!

## Project Vision

The goal of this project is to create a programming language **together** as a community. Jackal is built by and for open source scientists and developers who want to learn, contribute, and shape the future of a language collaboratively.

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
    git clone https://github.com/alegarsio/Jackal-Projects
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
```

## Example Code

Here's a simple example demonstrating object-oriented programming in Jackal:

```jackal
class Jack{
    
    init(name){
        this.name = name;
    }

    function greet(){
        print "Hello " + this.name;
    }
}

let jack = Jack("Jack");
jack.greet();
```

This code creates a `Jack` class with a constructor and a method, then instantiates an object and calls its method.

### Array Example

Working with arrays and loops:

```jackal
let data = ["jack","john","davin","ale"];

for (let i = 0 ; i < 4 ; i ++ ){
    print(data[i]);
}

for (i in data){
    print(data[i])
}
```

This demonstrates array literals, C-style for loops, and array index access.

### User Input Example

Reading input from the user:

```jackal
print "Hello ";
let name = read();
print "Hello, " + name + "!";
```

This shows how to use the `read()` function to get user input and interact with your programs.

### Pattern Matching Example

Using match expressions for conditional logic:

```jackal
let score = 90;
match (score) {
    90 => print "Passed";
    75 => print "Ok";
    default => print "Not in the list";
}
```

This demonstrates pattern matching syntax for elegant conditional branching.