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

## Pipe Operator ( Beta )

Here's a simple example demonstrating pipeline operator  in jackal

```js
@main
func main(){
    "Hello world " |> println()
}
```


## Type Safe ( In Development )

Here's a simple example demonstrating type safe feature in jackal

```js
let a : Int = 10
let b : Float = 20
let c : String = "hello"
let d : Number = 20

func add(a : Int, b : Int){
    return a + b
}

func add(a,b) -> Int{
    return a + b
}
```

## For loop

Here's a simple example demonstrating for loop in jackal

```js
for (let i = 0 ; i < 10 ; i ++){
    println(i)
}

for(i in 1 to 100){
    println(i)
}

for(i in 1 to 100 step 2){
    println(i)
}
```

## Class

Here's a simple example demonstrating object-oriented programming in Jackal:

```js
class Jack{
    
    init(name){
        this.name = name;
    }

    func greet(){
        println("Hello " + this.name)
    }
}

let jack = Jack("Jack");
jack.greet();
```

This code creates a `Jack` class with a constructor and a method, then instantiates an object and calls its method.


## Singleton

Here's a simple example demonstrating singleton programming pattern in Jackal:

```js
object Person(name){
    func getName(){
        return this.name
    }
}

Person.name = "jack"
println(Person.getName())
```


## Inheritance

Jackal also support inheritance 

```js
class Person{
    init(name){
        this.name = name;
    }
}
class Jack extends Person{
    func greet(){
        println("Hello " + this.name)
    }
}

let jack = Jack("Jack");
jack.greet();
```

## interface

Jackal also support interface 

```js
interface Greetable{
    func greet() ;
}

class Jack implements Greetable{
    
    @override
    func greet() {
        println("hello jackal")
    }
}

let jack = Jack();
jack.greet();
```


### Array Example

Working with arrays and loops:

```js
let data = ["jack","john","davin","ale"];

for (let i = 0 ; i < 4 ; i ++ ){
    print(data[i]);
}

for (i in data){
    print(data[i])
}
```

```js
let data = [
    [1,2],
    [3,4],
    [5,6]
]

for (i in data){
    println(i)
}
```

This demonstrates array literals, C-style for loops, and array index access.


### Map Example

Working with maps

```js
let data {
    "id" : 1,
    "name" : "ale"
}

println(data["name"])
```


### User Input Example

Reading input from the user:

```js
print("Hello ");
let name = read();
print("Hello, " + name + "!");
```

This shows how to use the `read()` function to get user input and interact with your programs.

### Pattern Matching Example

Using match expressions for conditional logic:

```js
let score = 90;
match (score) {
    90 => println("Passed");
    75 => println("Ok");
    default => println("Not in the list");
}
```

This demonstrates pattern matching syntax for elegant conditional branching.

### Generic (Beta)

Jackal also support generic programming pattern
```js
class Person<T>{
    init (name : T){
        this.name = name
    }

    func getName() {
        return this.name
    }
}

let person = Person<String>("jack")
```

### Reactive (Beta)

Jackal also support reactive programming pattern
```js
let counter = 0
every(1000){
    counter ++
    println(counter)
} until (counter >= 5 ){
    println("loop stop")
}
```

## Decorator

Here's a simple example demonstrating Decorator Metadata in Jackal:

## Main Decorator 

In complex programming, a script might contain hundreds of functions. The @main decorator tells the Jackal interpreter exactly where to start execution. Without it, the interpreter wouldn't know which function to run first after loading all definitions.

```js
@main
func main(){
    println("hello world")
}
```

## Memoize Decorator

Memoization is a specific form of caching. When a function is marked with @memoize, the interpreter saves the result of every function call in a hidden table

```js
@memoize
func counter(n) {
    let count = 1
    for (let i = 1; i < n; i++) {
        count = count * 2
    }
    return count
}
```

## Parallel Decorator

In standard programming, functions run sequentially (one after another). If Function A takes 5 seconds, Function B must wait 5 seconds to start. With @parallel, Jackal uses the POSIX Threads (pthreads) library from C to run the function on a different core of your processor.

```js
@parallel
func counter(n) {
    let count = 1
    for (let i = 1; i < n; i++) {
        count = count + 1
    }
    return count
}

```

## Override Decorator

When you create a specialized object based on a generic one, you might want to change how a specific behavior works. By marking a function as @override, you are explicitly telling the interpreter: "I know this function exists in the parent, and I am intentionally replacing it with this new version.

```js

interface Greetable{
    func greet() ;
}

class Jack implements Greetable{
    
    @override
    func greet() {
        println("hello jackal")
    }
}

let jack = Jack();
jack.greet();

```
