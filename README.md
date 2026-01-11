<div align="center">

# The Jackal Scripting Language

**A lightweight, elegant programming language for the open source community**


[![New Website](https://img.shields.io/badge/website-jackal--lang-blue)](https://jackal-new-documentation.vercel.app/)
[![License](https://img.shields.io/badge/license-Apache%202.0-green.svg)](LICENSE)
[![Built with C](https://img.shields.io/badge/built%20with-C-orange.svg)](https://en.wikipedia.org/wiki/C_(programming_language))

[Website](https://jackal-intro-page-qhqv.vercel.app) • [Documentation](#language-guide) • [Contributing](#contributing) • [Examples](#language-guide)

</div>

---

## About

Jackal is a dynamic, object-oriented interpreted programming language built from scratch in C. This project is designed as an in-depth exploration of language implementation, covering a lexer, a recursive descent parser, and a tree-walking evaluator.

## Project Vision

The goal of this project is to create a **lightweight and elegant** programming language **together** as a community. Jackal is built by and for open source scientists and developers who want to learn, contribute, and shape the future of a language collaboratively.

## Key Features

- **Dynamic Typing** — No complex type declarations required
- **Fully Object-Oriented** — Support for `class`, `this`, methods, constructors (`init`), and object properties
- **Modern Data Structures** — Array literals and maps with easy manipulation
- **First-Class Functions** — Functions can be stored in variables, passed as arguments, and support closures
- **Complete Control Flow** — `if`/`else if`/`else`, `while` loops, and C-style `for` loops
- **Comprehensive Operators** — Arithmetic, comparison, logical (`&&` with short-circuit), and postfix (`i++`, `i--`)

---

## Quick Start

### Prerequisites

- Standard C compiler (like `gcc` or `clang`)
- `make`

### Installation

```bash
git clone https://github.com/alegarsio/Jackal-Projects
cd jackal
make jackal
```

### Running a Program

Create a file with a `.jackal` extension and run it:

```bash
./jackal your_script.jackal
```

---

## Language Guide

### Print 

### Old Print

```js
let a = "jack"
println("name" + a)
```

```js
let a = "jack"

let age = 19

println("name" + a + 19.toString())
```

### New Print 
In the new jackal update we bring new feature to the print function
```js
let a = "jack"

println("name", a)
```

```js
let a = "jack"

let age = 19

println("name", a,age)
```
### Type Safe (In Development)

```js
let a: Int = 10
```
```js
let b: Float = 2.0
```
```js
let c: String = "hello"
```
```js
let d: Number = 20
```
### User Input

```js
print("Hello ");

let name = read();

print("Hello, ",name);
```

### Function

```js
func add(a,b){
    return a + b
}
```

```js
func add(a : Int , b : Int){
    return a + b
}
```

```js
func add(a : Int , b : Int) -> Int{
    return a + b
}
```

```js
func square(n : Int) -> Int =  n * n
```

```js
func square(n) =  n * n
```


### Destructor

```js

let [a, b] = [1, 2]

println(a)

println(b)
```


### Pipe Operator (Beta)

```js
"hello world" |> println()
```

```js
func square(n : Int) -> Int =  n * n

square(12) |> println()

```

```js
func square(n : Int) -> Int =  n * n

func mul(a,b) -> Int = a * b

let result = square(12)

square(12) * mul(1,2) |> println()

```



### For Loop

Classic for loop:

```js
for (let i = 0; i < 10; i++) {
    println(i)
}
```

Range based loop:

```js
for (i in 1 to 100) {
    println(i)
}
```

```js
for(i in 1 .. 10){
    println(i)
}
```

Range based loop with step:

```js
for (i in 1 to 100 step 2) {
    println(i)
}

```
### Struct

```js
struct Point(x, y, z)

let p = Point(1, 2, 3)

println(p.x)
```

### Class

```js
class Jack {
    init(name) {
        this.name = name;
    }
    
    func greet() {
        println("Hello " + this.name)
    }
}

let jack = Jack("Jack");
jack.greet();
```


### Singleton

```js
object Person(name) {
    func getName() {
        return this.name
    }
}

Person.name = "jack"
println(Person.getName())
```

### Inheritance

```js
class Person {
    init(name) {
        this.name = name;
    }
}

class Jack: Person {
    func greet() {
        println("Hello " + this.name)
    }
}

let jack = Jack("Jack");
jack.greet();
```

### Interface

```js
interface Greetable {
    func greet();
}

class Jack implements Greetable {
    @override
    func greet() {
        println("hello jackal")
    }
}

let jack = Jack();
jack.greet();
```

### Array

```js
let data = ["jack", "john", "davin", "ale"];

for (let i = 0; i < 4; i++) {
    print(data[i]);
}

for (i in data) {
    print(data[i])
}
```

Nested arrays:

```js
let data = [
    [1, 2],
    [3, 4],
    [5, 6]
]

for (i in data) {
    println(i)
}
```

### Map

```js
let data = {
    "id": 1,
    "name": "ale"
}

println(data["name"])
```


### Pattern Matching

```js
let score = 90;

match (score) {
    90 => println("Passed");
    75 => println("Ok");
    any => println("Not in the list");
}
```
```js
let pattern = [1,2,3,4]

match(pattern){
    [1,2,3,4] => println("Ok")
    any => println("Not Ok")
}
```

```js
let data = {"items": [1,2,3], "name": "Jack"}

match(data) {
    {"items": [1,2,3], "name": "Jack"} => println("Perfect match")
    any => println("not ok")
}

```
### With 

```js
let user = {"name": "Jackal", "version": 1};

with (user) {
    if (version == 1){
        println("ok")
    }
}
```
```js

let user = {"name": "Jackal", "version": 1};

with (user) {
    println(name)
    println(version)
}
```


### Generic (Beta)

```js
class Person<T> {
    init(name: T) {
        this.name = name
    }
    
    func getName() {
        return this.name
    }
}

let person = Person("jack")
```

### Reactive (Beta)

```js
let counter = 0

every(1000) {
    counter++
    println(counter)
}

until (counter >= 5) {
    println("loop stop")
}
```

---

## Decorators

### @main Decorator

Tells the Jackal interpreter where to start execution:

```js
@main
func main() {
    println("hello world")
}
```

### @memoize Decorator

Saves the result of every function call for caching:

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

### @parallel Decorator

Runs the function on a different core using POSIX Threads:

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

### @async Decorator

Executes tasks in the background without blocking:

```js
@async
func count(n) {
    let count = 0
    for (let i = 0; i < n; i++) {
        count++
    }
    return count
}
```

### @override Decorator

Explicitly marks method overrides:

```js
interface Greetable {
    func greet();
}

class Jack implements Greetable {
    @override
    func greet() {
        println("hello jackal")
    }
}

let jack = Jack();
jack.greet();
```

---

## Contributing

We welcome contributions from developers of all skill levels! 

### How to Contribute

1. Fork the repository
2. Create a feature branch: `git checkout -b feature/amazing-feature`
3. Make your changes
4. Commit your changes: `git commit -m "Add amazing feature"`
5. Push to your branch: `git push origin feature/amazing-feature`
6. Open a Pull Request

---

## License

This project is licensed under the Apache License 2.0 - see the [LICENSE](LICENSE) file for details.

---

**Made with ❤️ by the Jackal Community**