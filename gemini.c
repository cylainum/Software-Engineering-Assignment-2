#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LINE 256

typedef struct {
    char sender[30];
    char subject[100];
    char date[15]; // Format: MM-DD-YYYY
} Email;

typedef struct {
    Email *data;
    int size;
    int capacity;
} MaxHeap;

// Helper function to map sender category string to integer priority score.
// Higher number = higher priority.
int get_category_priority(const char *sender) {
    if (strcmp(sender, "Boss") == 0) return 5;
    if (strcmp(sender, "Subordinate") == 0) return 4;
    if (strcmp(sender, "Peer") == 0) return 3;
    if (strcmp(sender, "ImportantPerson") == 0) return 2;
    return 1; // OtherPerson
}

// Converts MM-DD-YYYY string into YYYYMMDD integer for chronological comparison
int date_to_int(const char *date) {
    int m, d, y;
    sscanf(date, "%d-%d-%d", &m, &d, &y);
    return y * 10000 + m * 100 + d;
}

// Returns > 0 if email A has higher priority than email B, < 0 if B > A, 0 if equal
int compare_emails(const Email *a, const Email *b) {
    int prioA = get_category_priority(a->sender);
    int prioB = get_category_priority(b->sender);

    if (prioA != prioB) {
        return prioA - prioB;
    }
    // Same category: newer date wins
    return date_to_int(a->date) - date_to_int(b->date);
}

// Initialize the Heap
MaxHeap* create_heap(int initial_capacity) {
    MaxHeap *heap = (MaxHeap*)malloc(sizeof(MaxHeap));
    heap->capacity = initial_capacity;
    heap->size = 0;
    heap->data = (Email*)malloc(sizeof(Email) * heap->capacity);
    return heap;
}

void swap(Email *a, Email *b) {
    Email temp = *a;
    *a = *b;
    *b = temp;
}

// Heapify up when inserting
void heapify_up(MaxHeap *heap, int index) {
    while (index > 0) {
        int parent = (index - 1) / 2;
        if (compare_emails(&heap->data[index], &heap->data[parent]) > 0) {
            swap(&heap->data[index], &heap->data[parent]);
            index = parent;
        } else {
            break;
        }
    }
}

// Heapify down when extracting max
void heapify_down(MaxHeap *heap, int index) {
    int largest = index;
    int left = 2 * index + 1;
    int right = 2 * index + 2;

    if (left < heap->size && compare_emails(&heap->data[left], &heap->data[largest]) > 0) {
        largest = left;
    }
    if (right < heap->size && compare_emails(&heap->data[right], &heap->data[largest]) > 0) {
        largest = right;
    }
    if (largest != index) {
        swap(&heap->data[index], &heap->data[largest]);
        heapify_down(heap, largest);
    }
}

void insert_email(MaxHeap *heap, Email e) {
    if (heap->size == heap->capacity) {
        heap->capacity *= 2;
        heap->data = (Email*)realloc(heap->data, sizeof(Email) * heap->capacity);
    }
    heap->data[heap->size] = e;
    heapify_up(heap, heap->size);
    heap->size++;
}

void read_email(MaxHeap *heap) {
    if (heap->size == 0) return;
    
    heap->data[0] = heap->data[heap->size - 1];
    heap->size--;
    if (heap->size > 0) {
        heapify_down(heap, 0);
    }
}

void free_heap(MaxHeap *heap) {
    free(heap->data);
    free(heap);
}

int main(void) {
    MaxHeap *heap = create_heap(10);
    char line[MAX_LINE];

    while (fgets(line, sizeof(line), stdin) != NULL) {
        // Strip trailing newline character
        line[strcspn(line, "\r\n")] = 0;
        if (strlen(line) == 0) continue;

        if (strncmp(line, "EMAIL ", 6) == 0) {
            Email e;
            char *token_str = line + 6;
            
            // Parse comma-delimited fields
            sscanf(token_str, "%29[^,],%99[^,],%14s", e.sender, e.subject, e.date);
            
            // Trim leading/trailing whitespace if present
            char *s = e.sender;
            while(*s == ' ') s++;
            
            insert_email(heap, e);
        } 
        else if (strcmp(line, "COUNT") == 0) {
            printf("There are %d emails to read.\n", heap->size);
        } 
        else if (strcmp(line, "NEXT") == 0) {
            if (heap->size > 0) {
                printf("Next email:\n");
                printf("Sender: %s\n", heap->data[0].sender);
                printf("Subject: %s\n", heap->data[0].subject);
                printf("Date: %s\n", heap->data[0].date);
            }
        } 
        else if (strcmp(line, "READ") == 0) {
            if (heap->size > 0) {
                read_email(heap);
            }
        }
    }

    free_heap(heap);
    return 0;
}