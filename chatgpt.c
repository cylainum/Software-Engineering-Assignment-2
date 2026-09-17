#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_EMAILS 1000
#define SUBJECT_SIZE 500
#define CATEGORY_SIZE 30

typedef struct {
    char sender[CATEGORY_SIZE];
    char subject[SUBJECT_SIZE];
    char date[11];

    int priority;
    int dateValue;
} Email;

typedef struct {
    Email heap[MAX_EMAILS];
    int size;
} MaxHeap;

/* ---------------------------------------------------------
   Convert MM-DD-YYYY into an integer that can be compared.
   YYYYMMDD gives chronological ordering.
   --------------------------------------------------------- */
int convertDate(char *date)
{
    int month, day, year;

    sscanf(date, "%d-%d-%d", &month, &day, &year);

    return year * 10000 + month * 100 + day;
}

/* ---------------------------------------------------------
   Assign priority based on sender category.
   Higher number = higher priority.
   --------------------------------------------------------- */
int getPriority(char *sender)
{
    if (strcmp(sender, "Boss") == 0)
        return 5;
    else if (strcmp(sender, "Subordinate") == 0)
        return 4;
    else if (strcmp(sender, "Peer") == 0)
        return 3;
    else if (strcmp(sender, "ImportantPerson") == 0)
        return 2;
    else
        return 1;   /* OtherPerson */
}

/* ---------------------------------------------------------
   Determine whether email a has higher priority than email b.
   
   First compare sender priority.
   If equal, compare dates.
   Newer date has higher priority.
   --------------------------------------------------------- */
int higherPriority(Email a, Email b)
{
    if (a.priority > b.priority)
        return 1;

    if (a.priority < b.priority)
        return 0;

    if (a.dateValue > b.dateValue)
        return 1;

    return 0;
}

/* ---------------------------------------------------------
   Swap two emails.
   --------------------------------------------------------- */
void swap(Email *a, Email *b)
{
    Email temp = *a;
    *a = *b;
    *b = temp;
}

/* ---------------------------------------------------------
   Initialize an empty MaxHeap.
   --------------------------------------------------------- */
void initializeHeap(MaxHeap *heap)
{
    heap->size = 0;
}

/* ---------------------------------------------------------
   Move an element upward until the MaxHeap property is
   restored.
   --------------------------------------------------------- */
void heapifyUp(MaxHeap *heap, int index)
{
    int parent;

    while (index > 0)
    {
        parent = (index - 1) / 2;

        if (higherPriority(heap->heap[index],
                           heap->heap[parent]))
        {
            swap(&heap->heap[index], &heap->heap[parent]);
            index = parent;
        }
        else
        {
            break;
        }
    }
}

/* ---------------------------------------------------------
   Insert an email into the MaxHeap.
   --------------------------------------------------------- */
void insert(MaxHeap *heap, Email email)
{
    if (heap->size >= MAX_EMAILS)
        return;

    heap->heap[heap->size] = email;

    heapifyUp(heap, heap->size);

    heap->size++;
}

/* ---------------------------------------------------------
   Move an element downward until the MaxHeap property is
   restored.
   --------------------------------------------------------- */
void heapifyDown(MaxHeap *heap, int index)
{
    int left;
    int right;
    int largest;

    while (1)
    {
        left = 2 * index + 1;
        right = 2 * index + 2;
        largest = index;

        if (left < heap->size &&
            higherPriority(heap->heap[left],
                           heap->heap[largest]))
        {
            largest = left;
        }

        if (right < heap->size &&
            higherPriority(heap->heap[right],
                           heap->heap[largest]))
        {
            largest = right;
        }

        if (largest != index)
        {
            swap(&heap->heap[index], &heap->heap[largest]);
            index = largest;
        }
        else
        {
            break;
        }
    }
}

/* ---------------------------------------------------------
   Remove and return the highest-priority email.
   --------------------------------------------------------- */
Email removeMax(MaxHeap *heap)
{
    Email result;

    result.sender[0] = '\0';
    result.subject[0] = '\0';
    result.date[0] = '\0';
    result.priority = 0;
    result.dateValue = 0;

    if (heap->size == 0)
        return result;

    result = heap->heap[0];

    heap->size--;

    if (heap->size > 0)
    {
        heap->heap[0] = heap->heap[heap->size];
        heapifyDown(heap, 0);
    }

    return result;
}

/* ---------------------------------------------------------
   Return the highest-priority email without removing it.
   This is needed because NEXT does not remove the email.
   --------------------------------------------------------- */
Email peek(MaxHeap *heap)
{
    Email result;

    result.sender[0] = '\0';
    result.subject[0] = '\0';
    result.date[0] = '\0';
    result.priority = 0;
    result.dateValue = 0;

    if (heap->size == 0)
        return result;

    return heap->heap[0];
}

/* ---------------------------------------------------------
   Process the input file.
   --------------------------------------------------------- */
int main(int argc, char *argv[])
{
    MaxHeap heap;
    FILE *file;

    char line[1000];
    char command[20];

    initializeHeap(&heap);

    /*
       If a filename is supplied:
           ./program test.txt

       Otherwise, input can be redirected:
           ./program < test.txt
    */
    if (argc > 1)
    {
        file = fopen(argv[1], "r");

        if (file == NULL)
        {
            printf("Unable to open file.\n");
            return 1;
        }
    }
    else
    {
        file = stdin;
    }

    while (fgets(line, sizeof(line), file) != NULL)
    {
        /* Remove newline characters. */
        line[strcspn(line, "\r\n")] = '\0';

        /* -------------------------------------------------
           EMAIL command
           Format:
           EMAIL Category,Subject,MM-DD-YYYY
           ------------------------------------------------- */
        if (strncmp(line, "EMAIL ", 6) == 0)
        {
            Email email;
            char *token;

            token = strtok(line + 6, ",");

            if (token == NULL)
                continue;

            strcpy(email.sender, token);

            token = strtok(NULL, ",");

            if (token == NULL)
                continue;

            strcpy(email.subject, token);

            token = strtok(NULL, ",");

            if (token == NULL)
                continue;

            strcpy(email.date, token);

            email.priority = getPriority(email.sender);
            email.dateValue = convertDate(email.date);

            insert(&heap, email);
        }

        /* -------------------------------------------------
           NEXT command

           NEXT displays the highest-priority email but
           DOES NOT remove it from the heap.
           ------------------------------------------------- */
        else if (strcmp(line, "NEXT") == 0)
        {
            Email email = peek(&heap);

            if (heap.size > 0)
            {
                printf("Next email:\n");
                printf("Sender: %s\n", email.sender);
                printf("Subject: %s\n", email.subject);
                printf("Date: %s\n", email.date);
            }
        }

        /* -------------------------------------------------
           READ command

           READ removes the highest-priority email without
           displaying it.
           ------------------------------------------------- */
        else if (strcmp(line, "READ") == 0)
        {
            if (heap.size > 0)
            {
                removeMax(&heap);
            }
        }

        /* -------------------------------------------------
           COUNT command
           ------------------------------------------------- */
        else if (strcmp(line, "COUNT") == 0)
        {
            printf("There are %d emails to read.\n", heap.size);
        }
    }

    if (file != stdin)
        fclose(file);

    return 0;
}
