# Jackal

<div align="center">

**A lightweight, elegant programming language for the open source community**

[![Website](https://img.shields.io/badge/website-jackal--lang-blue)](https://jackal-intro-page-qhqv.vercel.app)
[![License](https://img.shields.io/badge/license-MIT-green.svg)](LICENSE)
[![Built with C](https://img.shields.io/badge/built%20with-C-orange.svg)](https://en.wikipedia.org/wiki/C_(programming_language))

[Website](https://jackal-intro-page-qhqv.vercel.app) • [Documentation](#documentation) • [Contributing](#contributing) • [Examples](#examples)

</div>

---

## About

Jackal is a dynamic, object-oriented interpreted programming language built from the ground up in C. Designed with elegance and simplicity at its core, Jackal provides a modern programming experience without sacrificing performance or clarity.

The language features a hand-crafted lexer, recursive descent parser, and tree-walking evaluator—making it an excellent study in language implementation and compiler theory.

## Project Vision

Jackal is more than just a programming language—it's a collaborative journey. Our mission is to create a **lightweight and elegant** programming language that:

- Removes unnecessary complexity while maintaining expressive power
- Provides intuitive syntax that reads like natural logic
- Offers modern features without bloating the runtime
- Empowers developers to write clean, maintainable code
- Grows organically through community collaboration

We believe the best languages are built **together**—by scientists, developers, students, and enthusiasts who share a passion for elegant design and open source values.

## Why Jackal?

- **🪶 Lightweight**: Minimal runtime overhead with fast startup times
- **✨ Elegant Syntax**: Clean, readable code that expresses intent clearly
- **🎯 Modern Features**: Advanced capabilities without complexity
- **🔓 Open Source**: Built by the community, for the community
- **📚 Educational**: Learn language design through hands-on contribution

## Key Features

### Core Language Features

- **Dynamic Typing** — Write code without ceremony or boilerplate
- **Object-Oriented Programming** — Full support for classes, inheritance, and interfaces
- **First-Class Functions** — Functions as values with closure support
- **Pattern Matching** — Elegant conditional logic with `match` expressions
- **Reactive Programming** — Built-in support for time-based reactive patterns
- **Concurrent Execution** — Parallel and async decorators for performance

### Data Structures

- **Arrays** — `[1, 2, 3]` with intuitive indexing
- **Maps** — `{"key": "value"}` for key-value storage
- **Destructuring** — `let [a, b] = [1, 2]` for elegant assignment
- **Structs** — Lightweight data containers

### Control Flow

- **Conditional Logic** — `if`/`else if`/`else` chains
- **Loops** — Classic `for`, range-based `for`, and `while`
- **Pattern Matching** — `match` expressions for multi-way branching

### Advanced Features

- **Generics** *(Beta)* — Type-safe generic programming
- **Type Annotations** *(In Development)* — Optional static typing
- **Decorators** — Metadata-driven function enhancement
- **Pipe Operator** *(Beta)* — Functional composition with `|>`
- **Singletons** — Object-level programming pattern

---

## Quick Start

### Prerequisites

- Standard C compiler (`gcc`, `clang`, or `msvc`)
- `make` build system

### Installation

Clone the repository and build the interpreter:

```bash
git clone https://github.com/alegarsio/Jackal-Projects
cd jackal
make jackal
```

### Your First Program

Create a file named `hello.jackal`:

```js
@main
func main() {
    println("Hello, Jackal!")
}
```

Run it:

```bash
./jackal hello.jackal
```

---

## Language Guide

### Variables and Data Types

```js
// Dynamic typing
let name = "Jackal"
let version = 1.0
let isOpen = true

// Type annotations (in development)
let count: Int = 42
let price: Float = 99.99
let message: String = "Hello"
```

### Functions

```js
// Basic function
func greet(name) {
    return "Hello, " + name
}

// First-class functions
let sayHello = greet
sayHello("World")

// Type-safe functions (in development)
func add(a: Int, b: Int) -> Int {
    return a + b
}
```

### Classes and Objects

```js
class Person {
    init(name, age) {
        this.name = name
        this.age = age
    }
    
    func introduce() {
        println("I'm " + this.name + ", " + this.age + " years old")
    }
}

let person = Person("Alice", 30)
person.introduce()
```

### Inheritance

```js
class Student : Person {
    init(name, age, major) {
        this.name = name
        this.age = age
        this.major = major
    }
    
    @override
    func introduce() {
        println("I'm " + this.name + ", studying " + this.major)
    }
}

let student = Student("Bob", 20, "Computer Science")
student.introduce()
```

### Interfaces

```js
interface Drawable {
    func draw();
}

class Circle implements Drawable {
    @override
    func draw() {
        println("Drawing a circle")
    }
}

let shape = Circle()
shape.draw()
```

### Structs

```js
struct Point(x, y, z)

let position = Point(10, 20, 30)
println(position.x)  // 10
```

### Singleton Pattern

```js
object Logger(level) {
    func log(message) {
        println("[" + this.level + "] " + message)
    }
}

Logger.level = "INFO"
Logger.log("Application started")
```

### Arrays

```js
let numbers = [1, 2, 3, 4, 5]

// Classic for loop
for (let i = 0; i < 5; i++) {
    println(numbers[i])
}

// Range-based for loop
for (num in numbers) {
    println(num)
}

// Nested arrays
let matrix = [
    [1, 2, 3],
    [4, 5, 6],
    [7, 8, 9]
]
```

### Maps

```js
let user = {
    "id": 1,
    "name": "Alice",
    "email": "alice@example.com"
}

println(user["name"])  // Alice
```

### Destructuring

```js
let [x, y, z] = [10, 20, 30]
println(x)  // 10
println(y)  // 20
println(z)  // 30
```

### Control Flow

```js
// If-else
let score = 85

if (score >= 90) {
    println("Excellent")
} else if (score >= 75) {
    println("Good")
} else {
    println("Keep trying")
}

// While loop
let count = 0
while (count < 5) {
    println(count)
    count++
}

// Range-based for loop
for (i in 1 to 10) {
    println(i)
}

// Range with step
for (i in 0 to 100 step 5) {
    println(i)
}
```

### Pattern Matching

```js
let status = 200

match (status) {
    200 => println("OK")
    404 => println("Not Found")
    500 => println("Server Error")
    default => println("Unknown Status")
}
```

### Pipe Operator *(Beta)*

```js
@main
func main() {
    "Hello, Jackal!" |> println()
    
    // Chain operations
    let result = getValue()
        |> transform()
        |> validate()
        |> save()
}
```

### Generics *(Beta)*

```js
class Container<T> {
    init(value: T) {
        this.value = value
    }
    
    func get() {
        return this.value
    }
}

let stringContainer = Container<String>("Hello")
let intContainer = Container<Int>(42)
```

### Reactive Programming *(Beta)*

```js
let counter = 0

every(1000) {
    counter++
    println("Counter: " + counter)
}

until(counter >= 5) {
    println("Counter stopped at 5")
}
```

---

## Decorators

Jackal provides powerful decorator metadata for function enhancement.

### `@main` — Entry Point

Specifies the program's entry point in complex applications:

```js
@main
func main() {
    println("Application started")
    initialize()
    run()
}
```

### `@memoize` — Result Caching

Automatically cache function results for improved performance:

```js
@memoize
func fibonacci(n) {
    if (n <= 1) return n
    return fibonacci(n - 1) + fibonacci(n - 2)
}

// First call computes, subsequent calls use cache
println(fibonacci(40))  // Computed
println(fibonacci(40))  // Cached
```

### `@parallel` — Multi-threaded Execution

Execute functions on separate CPU cores using POSIX threads:

```js
@parallel
func processLargeDataset(data) {
    let result = 0
    for (item in data) {
        result = result + compute(item)
    }
    return result
}
```

### `@async` — Asynchronous Execution

Run functions in the background without blocking:

```js
@async
func fetchData(url) {
    let response = httpGet(url)
    return parseJSON(response)
}

// Main thread continues while fetchData runs
let dataPromise = fetchData("https://api.example.com/data")
```

### `@override` — Explicit Method Override

Mark methods that intentionally override parent implementations:

```js
class Animal {
    func speak() {
        println("Some sound")
    }
}

class Dog : Animal {
    @override
    func speak() {
        println("Woof!")
    }
}
```

---

## Examples

### User Input

```js
@main
func main() {
    print("Enter your name: ")
    let name = read()
    println("Hello, " + name + "!")
}
```

### Temperature Converter

```js
func celsiusToFahrenheit(celsius) {
    return (celsius * 9/5) + 32
}

@main
func main() {
    print("Enter temperature in Celsius: ")
    let celsius = read()
    let fahrenheit = celsiusToFahrenheit(celsius)
    println(celsius + "°C = " + fahrenheit + "°F")
}
```

### Simple To-Do List

```js
let todos = []

func addTodo(task) {
    todos = todos + [task]
    println("Added: " + task)
}

func listTodos() {
    println("=== To-Do List ===")
    for (i in 0 to todos.length) {
        println((i + 1) + ". " + todos[i])
    }
}

@main
func main() {
    addTodo("Learn Jackal")
    addTodo("Build a project")
    addTodo("Contribute to open source")
    listTodos()
}
```

---

## Contributing

We welcome contributions from developers of all skill levels! Whether you're fixing bugs, adding features, improving documentation, or sharing ideas—your contribution matters.

### How to Contribute

1. **Fork the repository**
2. **Create a feature branch**: `git checkout -b feature/amazing-feature`
3. **Make your changes**
4. **Test thoroughly**
5. **Commit with clear messages**: `git commit -m "Add amazing feature"`
6. **Push to your branch**: `git push origin feature/amazing-feature`
7. **Open a Pull Request**

### Development Guidelines

- Follow existing code style and conventions
- Write clear, descriptive commit messages
- Add tests for new features
- Update documentation as needed
- Keep PRs focused on a single feature or fix

### Areas for Contribution

- 🐛 Bug fixes and stability improvements
- ✨ New language features
- 📚 Documentation and examples
- 🧪 Test coverage expansion
- ⚡ Performance optimizations
- 🌍 Internationalization

---

## Documentation

Comprehensive documentation is available on our [website](https://jackal-intro-page-qhqv.vercel.app).

### Topics Covered

- Language specification
- Standard library reference
- Advanced tutorials
- Contribution guidelines
- Architecture deep-dive

---

## Roadmap

### Current Focus (v1.0)

- ✅ Core language features
- ✅ Object-oriented programming
- ✅ Basic standard library
- 🚧 Type annotations system
- 🚧 Generic programming
- 🚧 Package manager

### Future Plans (v2.0+)

- Module system
- Foreign Function Interface (FFI)
- JIT compilation
- Standard library expansion
- IDE tooling support
- REPL environment

---

## Community

Join our growing community of Jackal developers:

- **Website**: [jackal-intro-page-qhqv.vercel.app](https://jackal-intro-page-qhqv.vercel.app)
- **GitHub**: [github.com/alegarsio/Jackal-Projects](https://github.com/alegarsio/Jackal-Projects)
- **Issues**: Report bugs and request features

---

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

---

## Acknowledgments

Built with passion by the open source community. Special thanks to all contributors who help make Jackal better every day.

---

<div align="center">

**Made with ❤️ by the Jackal Community**

⭐ Star this repo if you find it useful!

</div>