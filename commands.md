For someone building a Redis clone, I'd focus on the **core commands** first. Here's a comprehensive table with what each command does, why it exists, and its typical time complexity.

| Command         | Category    | Description                                                                                     | Example               | Time Complexity |
| --------------- | ----------- | ----------------------------------------------------------------------------------------------- | --------------------- | --------------- |
| `SET key value` | String      | Stores a value for a key. Creates the key if it doesn't exist or overwrites the existing value. | `SET name Vaibu`      | O(1)            |
| `GET key`       | String      | Retrieves the value associated with a key. Returns null if the key doesn't exist.               | `GET name`            | O(1)            |
| `DEL key`       | String      | Deletes one or more keys and their associated values.                                           | `DEL name`            | O(1)            |
| `EXISTS key`    | String      | Checks whether a key exists in the database. Returns 1 or 0.                                    | `EXISTS name`         | O(1)            |
| `MSET`          | String      | Sets multiple key-value pairs in a single command.                                              | `MSET a 1 b 2`        | O(N)            |
| `MGET`          | String      | Retrieves values of multiple keys at once.                                                      | `MGET a b`            | O(N)            |
| `APPEND`        | String      | Appends text to the end of an existing string. Creates the key if it doesn't exist.             | `APPEND msg " world"` | O(1)            |
| `STRLEN`        | String      | Returns the length of the stored string.                                                        | `STRLEN msg`          | O(1)            |
| `GETSET`        | String      | Returns the old value and replaces it with a new one.                                           | `GETSET count 10`     | O(1)            |
| `SETNX`         | String      | Sets a value only if the key does not already exist. Useful for locks.                          | `SETNX lock 1`        | O(1)            |
| `SETEX`         | String      | Sets a value with an expiration time in seconds.                                                | `SETEX token 60 abc`  | O(1)            |
| `GETDEL`        | String      | Retrieves a value and immediately deletes the key.                                              | `GETDEL session`      | O(1)            |
| `INCR`          | Numeric     | Increments an integer value by 1.                                                               | `INCR counter`        | O(1)            |
| `INCRBY`        | Numeric     | Increments an integer by a specified amount.                                                    | `INCRBY counter 5`    | O(1)            |
| `DECR`          | Numeric     | Decrements an integer by 1.                                                                     | `DECR counter`        | O(1)            |
| `DECRBY`        | Numeric     | Decrements an integer by a specified amount.                                                    | `DECRBY counter 5`    | O(1)            |
| `EXPIRE`        | Expiration  | Assigns a TTL (Time To Live) to a key. After expiry, Redis removes it.                          | `EXPIRE token 60`     | O(1)            |
| `PEXPIRE`       | Expiration  | Same as EXPIRE but uses milliseconds.                                                           | `PEXPIRE token 5000`  | O(1)            |
| `TTL`           | Expiration  | Returns the remaining lifetime of a key in seconds.                                             | `TTL token`           | O(1)            |
| `PTTL`          | Expiration  | Returns remaining lifetime in milliseconds.                                                     | `PTTL token`          | O(1)            |
| `PERSIST`       | Expiration  | Removes the expiration from a key, making it permanent again.                                   | `PERSIST token`       | O(1)            |
| `KEYS`          | Key         | Returns all keys matching a pattern. Expensive on large datasets.                               | `KEYS user:*`         | O(N)            |
| `SCAN`          | Key         | Incrementally scans keys without blocking the server. Preferred over KEYS.                      | `SCAN 0`              | O(1) per call   |
| `TYPE`          | Key         | Returns the data type of the value stored at a key.                                             | `TYPE user`           | O(1)            |
| `RENAME`        | Key         | Renames an existing key.                                                                        | `RENAME a b`          | O(1)            |
| `RANDOMKEY`     | Key         | Returns a random key from the database.                                                         | `RANDOMKEY`           | O(1)            |
| `PING`          | Server      | Checks if the server is alive. Returns `PONG`.                                                  | `PING`                | O(1)            |
| `ECHO`          | Server      | Returns the message sent by the client. Mostly for testing.                                     | `ECHO hello`          | O(1)            |
| `INFO`          | Server      | Displays server statistics, memory usage, clients, persistence info, etc.                       | `INFO`                | O(1)            |
| `DBSIZE`        | Server      | Returns the number of keys currently stored.                                                    | `DBSIZE`              | O(1)            |
| `FLUSHDB`       | Server      | Deletes all keys from the current database.                                                     | `FLUSHDB`             | O(N)            |
| `FLUSHALL`      | Server      | Deletes all keys from every database.                                                           | `FLUSHALL`            | O(N)            |
| `SAVE`          | Persistence | Saves the entire dataset to disk synchronously. Blocks clients until complete.                  | `SAVE`                | O(N)            |
| `BGSAVE`        | Persistence | Saves the dataset in the background without blocking clients.                                   | `BGSAVE`              | O(N)            |
| `LASTSAVE`      | Persistence | Returns the timestamp of the last successful save.                                              | `LASTSAVE`            | O(1)            |

---

# Recommended implementation order for your project

| Version  | Features           | Commands                              |
| -------- | ------------------ | ------------------------------------- |
| **v0.1** | Basic KV Store     | `SET`, `GET`, `DEL`, `EXISTS`, `PING` |
| **v0.2** | Expiration         | `EXPIRE`, `TTL`, `PERSIST`            |
| **v0.3** | Batch Operations   | `MSET`, `MGET`, `APPEND`, `STRLEN`    |
| **v0.4** | Numeric Operations | `INCR`, `INCRBY`, `DECR`, `DECRBY`    |
| **v0.5** | Persistence        | `SAVE`, `BGSAVE`, `LASTSAVE`          |
| **v0.6** | Key Management     | `KEYS`, `SCAN`, `TYPE`, `RENAME`      |
| **v1.0** | Data Structures    | Hashes, Lists, Sets, Sorted Sets      |

---

## If your goal is to understand Redis internals

Don't just implement the command. For every command, keep a design note with these questions:

| Question                                    | Example (`EXPIRE`)                                                                                                           |
| ------------------------------------------- | ---------------------------------------------------------------------------------------------------------------------------- |
| **Why does Redis provide this command?**    | To automatically remove stale or temporary data such as sessions, caches, OTPs, and tokens.                                  |
| **How does Redis implement it internally?** | Stores expiration timestamps separately and removes keys lazily on access or actively through a background expiration cycle. |
| **How will I implement it?**                | Add an expiry timestamp to each value and run a cleanup thread plus lazy checks during reads.                                |
| **Performance impact?**                     | Slight memory overhead and periodic CPU usage for cleanup, but avoids serving expired data.                                  |

If you maintain this kind of documentation alongside the code, you'll end up learning *why* Redis is engineered the way it is, not just reproducing its API. That's exactly the kind of understanding that stands out in backend and systems interviews.
