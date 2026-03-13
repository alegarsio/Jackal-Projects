<div align="center">

# Jackal Mysql Library

</div>


## About
Jackal Mysql standard library allows jackal to communicate with mysql database, it provides feature such as QueryBuilder 

---

### Import Mysql Library

Jackal Mysql library are located in jackal std library folder.

```js
import std.Mysql
```

### Connect to Mysql

You can create connection with your database with Mysql class, Mysql class constructor provides three arguments of your host, username and password 

```js
let connection = Mysql("host","user","password").connect("yorDatabase")
```



