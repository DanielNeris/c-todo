#include <stdio.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define MAX_DESCRIPTION_LENGTH 256
#define INITIAL_CAPACITY 10
#define TODO_FILE "todos.txt"

typedef struct {
  int id;
  char description[MAX_DESCRIPTION_LENGTH];
  bool completed;
} TodoItem;

TodoItem* todo_list = NULL;
int todo_count = 0;
int todo_capacity = 0;
int next_id = 1;

void initialize_todo_list() {
  todo_capacity = INITIAL_CAPACITY;
  todo_list = (TodoItem*)malloc(sizeof(TodoItem) * todo_capacity);

  if (todo_list == NULL) {
    printf("Error: fail to allocate memory \n");
    exit(1);
  }

  todo_count = 0;
  next_id = 1;
}

void free_todo_list() {
  if (todo_list != NULL) {
    free(todo_list);
    todo_list = NULL;
  }
  todo_count = 0;
  todo_capacity = 0;
}

// Makes sure there is room for one more todo. Returns false if memory ran out.
bool ensure_capacity() {
  // List is full: double its capacity
  if (todo_count >= todo_capacity) {
    int new_capacity = todo_capacity * 2;
    TodoItem* new_list = (TodoItem*)realloc(todo_list, sizeof(TodoItem) * new_capacity);

    if (new_list == NULL) {
      printf("Error: fail to allocate memory \n");
      return false;
    }

    todo_list = new_list;
    todo_capacity = new_capacity;
  }
  return true;
}

void add_todo(const char* description) {
  if (strlen(description) == 0) {
    printf("Description can't be empty.\n");
    return;
  }

  if (!ensure_capacity()) {
    return;
  }

  TodoItem* item = &todo_list[todo_count];
  item->id = next_id;
  strncpy(item->description, description, MAX_DESCRIPTION_LENGTH - 1);
  item->description[MAX_DESCRIPTION_LENGTH - 1] = '\0';
  item->completed = false;

  todo_count++;
  next_id++;

  printf("Todo added with ID %d\n", item->id);
}

void list_todos() {
  if (todo_count == 0) {
    printf("No todos yet.\n");
    return;
  }

  printf("\n--- Current Todo Items (%d items) ---\n", todo_count);
  for (int i = 0; i < todo_count; i++) {
    printf("ID: %-4d | Status: %-9s | Description: %s\n",
           todo_list[i].id,
           todo_list[i].completed ? "COMPLETED" : "PENDING",
           todo_list[i].description);
  }
  printf("----------------------------------\n");
}

void mark_todo_complete(int id) {
  for (int i = 0; i < todo_count; i++) {
    if (todo_list[i].id == id) {
      if (todo_list[i].completed) {
        printf("Todo %d is already complete.\n", id);
      } else {
        todo_list[i].completed = true;
        printf("Todo %d marked as complete.\n", id);
      }
      return;
    }
  }

  printf("Todo with ID %d not found.\n", id);
}

void delete_todo(int id) {
  for (int i = 0; i < todo_count; i++) {
    if (todo_list[i].id == id) {
      // shift every item after i one position to the left
      for (int j = i; j < todo_count - 1; j++) {
        todo_list[j] = todo_list[j + 1];
      }
      todo_count--;
      printf("Todo %d deleted.\n", id);
      return;
    }
  }

  printf("Todo with ID %d not found.\n", id);
}

void save_todos(const char* filename) {
  FILE* file = fopen(filename, "w");

  if (file == NULL) {
    printf("Error: could not open %s for writing.\n", filename);
    return;
  }

  // one todo per line: id|completed|description
  for (int i = 0; i < todo_count; i++) {
    fprintf(file, "%d|%d|%s\n",
            todo_list[i].id,
            todo_list[i].completed,
            todo_list[i].description);
  }

  fclose(file);
  printf("Saved %d todo(s) to %s\n", todo_count, filename);
}

// Loads todos saved by save_todos. Returns how many were loaded.
int load_todos_from_file(const char* filename) {
  FILE* file = fopen(filename, "r");

  if (file == NULL) {
    // No saved file yet (first run): start with an empty list
    return 0;
  }

  char line[MAX_DESCRIPTION_LENGTH + 32];
  int loaded = 0;

  while (fgets(line, sizeof(line), file) != NULL) {
    int id;
    int completed;
    char description[MAX_DESCRIPTION_LENGTH];

    // line format: id|completed|description
    if (sscanf(line, "%d|%d|%255[^\n]", &id, &completed, description) != 3) {
      continue; // skip malformed lines
    }

    if (!ensure_capacity()) {
      break;
    }

    TodoItem* item = &todo_list[todo_count];
    item->id = id;
    item->completed = completed;
    strcpy(item->description, description);
    todo_count++;
    loaded++;

    // new todos must get an ID bigger than any loaded one
    if (id >= next_id) {
      next_id = id + 1;
    }
  }

  fclose(file);
  return loaded;
}

int main(void) {
  int choice = 0;
  char temp_description[MAX_DESCRIPTION_LENGTH];
  int temp_id;

  initialize_todo_list();

  int loaded = load_todos_from_file(TODO_FILE);
  if (loaded > 0) {
    printf("Loaded %d todo(s) from %s\n", loaded, TODO_FILE);
  }

  do {
    // Display the menu
    printf("\n--- Todo List Application ---\n");
    printf("1. Add Todo\n");
    printf("2. List Todos\n");
    printf("3. Mark Todo as Complete\n");
    printf("4. Delete Todo\n");
    printf("5. Save Todos\n");
    printf("6. Exit\n");
    printf("Enter your choice: ");

    if (scanf("%d", &choice) != 1) {
      // Input wasn't a number: discard the rest of the line
      while (getchar() != '\n');
      continue;
    }

    // consume left over newline character
    while (getchar() != '\n');

    switch (choice) {
      case 1: // add todo
        printf("Enter the new todo description: ");
        fgets(temp_description, sizeof(temp_description), stdin);
        // remove the trailing newline left by fgets
        temp_description[strcspn(temp_description, "\n")] = '\0';
        add_todo(temp_description);
        break;
      case 2: // list todos
        list_todos();
        break;
      case 3: // mark todo as complete
        printf("Enter the ID of the todo to complete: ");
        if (scanf("%d", &temp_id) != 1) {
          printf("Invalid ID.\n");
        } else {
          mark_todo_complete(temp_id);
        }
        // consume left over newline character
        while (getchar() != '\n');
        break;
      case 4: // delete todo
        printf("Enter the ID of the todo to delete: ");
        if (scanf("%d", &temp_id) != 1) {
          printf("Invalid ID.\n");
        } else {
          delete_todo(temp_id);
        }
        // consume left over newline character
        while (getchar() != '\n');
        break;
      case 5: // save todos
        save_todos(TODO_FILE);
        break;
      case 6: // exit
        printf("Goodbye!\n");
        break;
      default:
        printf("Invalid choice. Please try again.\n");
    }

  } while (choice != 6);

  free_todo_list();
  return 0;
}