# Contributing to Jackal

Thank you for your interest in contributing to Jackal 🚀

Jackal is a minimal, high-performance programming language built around an AST-based interpreter architecture.  
It focuses on explicit behavior, predictable execution, and clean system integration.

We welcome contributions in core interpreter logic, native bridge improvements, and standard library extensions.

Please read this guide before contributing.

---

## Table of Contents

- Philosophy
- Architecture Overview
- Development Setup
- Contribution Areas
- Coding Standards
- Pull Request Process
- Bug Report Guidelines
- Feature Proposal Guidelines

---

## Philosophy

Jackal prioritizes:

- Simplicity over abstraction
- Explicit behavior over hidden magic
- Performance awareness
- Minimal but powerful standard library

Contributions must align with these principles.

---

## Architecture Overview

Jackal currently uses an AST Interpreter architecture.

### 1. Parser
Responsible for:
- Tokenization
- AST generation
- Syntax validation

Directory example:
```
/src/parser
```

### 2. AST Interpreter (Evaluator)
Responsible for:
- Walking AST nodes
- Evaluating expressions
- Executing statements

Directory example:
```
/src/eval
```

### 3. Environment System (Env)
Responsible for:
- Scope management
- Variable bindings
- Function context handling

Directory example:
```
/src/env
```

### 4. Native Bridge (Ring 0 – C)
Low-level system integration layer.

Responsibilities:
- File system access
- Networking
- System-level performance operations
- VM-native bridging

Directory example:
```
/native
```

### 5. Standard Library (Open Contribution Area)
The standard library is actively evolving.

Examples:
```
/std
```

Contributors are encouraged to propose and improve stdlib modules.

---

## Development Setup

### Clone the repository

```
git clone https://github.com/your-username/jackal.git
cd jackal
```

### Build

```
make build
```

Or manually:

```
gcc main.c -o jackal
```

(Adjust according to the build system.)

### Run

```
./jackal example.jk
```

---

## Contribution Areas

We currently accept contributions in:

- Parser improvements
- AST evaluation optimizations
- Memory safety fixes
- Environment scoping improvements
- Native bridge enhancements
- Standard library modules
- Documentation improvements
- Test coverage improvements

---

## Standard Library Contributions

Stdlib contributions should:

- Be minimal and focused
- Avoid unnecessary abstraction
- Avoid heavy dependencies
- Maintain consistent API style

Before adding a new stdlib module, open an issue describing:

- The problem it solves
- Why it belongs in core stdlib
- Alternative approaches

---

## Coding Standards

### General

- Keep logic simple and readable
- Avoid implicit behavior
- No hidden global state
- Prefer explicit return values

### C (Native Bridge)

- Use `snake_case`
- Document memory ownership clearly
- Free all allocated memory
- Avoid unsafe pointer usage

### Jackal Core

- AST nodes must remain predictable
- No silent type coercion unless defined
- Scope resolution must be deterministic

---

## Commit Message Convention

Format:

```
[TYPE] Short description
```

Examples:

```
[FIX] Fix variable shadowing bug in nested scope
[PERF] Optimize AST evaluation loop
[FEAT] Add json std module
[DOC] Improve interpreter documentation
```

Types:

- FIX
- FEAT
- PERF
- DOC
- REFACTOR
- TEST

---

## Pull Request Process

1. Fork repository
2. Create a new branch:

```
feature/short-description
```

3. Follow commit message convention
4. Push and open Pull Request

Your PR must explain:

- What was changed
- Why it was changed
- Affected modules (parser / eval / env / native / std)
- Backward compatibility impact

PRs without explanation may be closed.

---

## Bug Report Guidelines

When reporting bugs, include:

- OS
- Jackal version
- Minimal reproducible example
- Expected behavior
- Actual behavior

Example:

```
let x = 10
{
    let x = 20
}
print(x)
```

Expected:
10

Actual:
20
```

---

## Feature Proposal Guidelines

Major syntax changes or interpreter behavior changes require prior discussion.

Feature proposal must include:

- Problem statement
- Proposed syntax (if applicable)
- Interpreter impact
- Performance impact
- Compatibility considerations

---

## Stability Policy

Jackal prioritizes interpreter stability.

Breaking changes require:

- Clear justification
- Migration explanation
- Version bump

---

Thank you for contributing to Jackal.
