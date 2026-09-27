# c-todo

A terminal todo list in C, standard library only. Written as a first C project, so
each piece was added to answer a concrete question: where does the data live, who
owns the memory, and what happens when the list outgrows it.

```bash
cmake -B build            # configure, once
cmake --build build       # compile
./build/todo              # run from the project root so todos.txt is found
```

---

## File format

Todos are saved to `todos.txt`, one per line, fields separated by `|`.

```
1|0|buy bread
2|1|study pointers
```

| Field       | Type  | Notes                                 |
| ----------- | ----- | ------------------------------------- |
| id          | `int` | never reused after a delete           |
| completed   | `int` | `0` pending, `1` complete             |
| description | text  | up to 255 chars, rest of the line     |

The description is the last field on purpose: it is read with `%255[^\n]`, which
takes everything up to the newline, so a description containing `|` still loads intact.

---

## How it evolved

### 1. A menu loop

`do { ... } while (choice != 6)` with `scanf("%d", &choice)`.

`scanf` leaves the Enter key (`\n`) in the input buffer. The next `fgets` reads that
leftover newline and returns an empty line immediately, without waiting for input.
Fixed by draining the line with `getchar()` after every `scanf`.

### 2. A dynamic array

A global `TodoItem* todo_list = NULL` plus `malloc(sizeof(TodoItem) * capacity)`.
A pointer rather than a fixed `TodoItem list[10]`, because a fixed array can never grow.

`todo_count` (items in use) and `todo_capacity` (items that fit) are tracked separately,
since the array itself does not know either.

### 3. Adding a todo

`fgets` rather than `scanf("%s")`: `%s` stops at the first space, so "buy bread" would
be stored as "buy", and it has no length limit. `fgets` keeps the newline, which is
cut with `strcspn`.

Empty descriptions are rejected inside `add_todo` with an early `return`, not in `main`,
so every caller gets the check.

### 4. Growing the list

When `todo_count >= todo_capacity`, capacity doubles via `realloc`.

The result goes into a temporary pointer first. `realloc` returns `NULL` on failure but
leaves the old block alive, so writing straight to `todo_list` would lose the only
pointer to it along with every todo.

Verified by adding 12 todos with a capacity of 10: all 12 kept their IDs.

### 5. IDs, not positions

Complete and delete search by `id` rather than using the number as an array index.
The two diverge as soon as something is deleted.

Delete shifts every later item one position left and decrements `todo_count`. The last
slot still holds a stale copy, but nothing reads past `todo_count`.

Verified by creating A, B, C, deleting 2, then adding D: D gets ID 4, not 2.

### 6. Saving

`fopen` in `"w"` mode, one `fprintf` per todo, `fclose`. Without `fclose`, buffered
output may never reach the disk.

### 7. Loading, and the bug it exposed

A missing `todos.txt` is not an error: it is the first run, and the list starts empty.

After the first load, the next new todo got ID 1, colliding with a loaded one. `next_id`
was still at its initial value. Fixed by setting it to the highest loaded ID plus one.

Loading also needs to grow the list, so the `realloc` logic moved out of `add_todo`
into `ensure_capacity()`, shared by both. Verified by saving and reloading 27 todos.

---

## Known limitations

| Limitation                              | Why it matters                                                                                                                             |
| --------------------------------------- | ------------------------------------------------------------------------------------------------------------------------------------------ |
| Ctrl+D hangs the program                | `while (getchar() != '\n')` never sees a newline at end of input, since `getchar` returns `EOF` forever. The loop needs to check for `EOF`. |
| No save on exit                         | Anything added since the last "Save" is lost when choosing Exit.                                                                           |
| Long descriptions are silently cut      | Input past 255 chars is dropped without telling the user.                                                                                  |
| `todos.txt` path is relative            | It is resolved from the current directory, so running from inside `build/` reads and writes a different file.                              |
| Linear search and shifting              | Complete and delete are O(n). Irrelevant at this size, noticeable at thousands of items.                                                   |
| Memory never shrinks                    | Capacity only grows. Deleting every todo keeps the largest allocation until exit.                                                          |
| Malformed lines are skipped silently    | A corrupted `todos.txt` loads partially with no warning.                                                                                   |

---

## Background

- C reference (standard library): https://en.cppreference.com/w/c
- CMake tutorial: https://cmake.org/cmake/help/latest/guide/tutorial/index.html
