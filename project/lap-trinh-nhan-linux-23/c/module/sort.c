#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/time.h>

#define MODULE_NAME "sort"
#define MAX_LENGTH 256

int select, values[MAX_LENGTH], length;

module_param(select, int, S_IRUGO);
module_param_array(values, int, &length, S_IRUGO);

void bubble_sort(void) {
    printk(KERN_INFO "Bubble sort:\n");
    ktime_t start_time, end_time;
    start_time = ktime_get();

    //code
    for (int i = 0; i < length-1; i++) {     
        for (int j = 0; j < length-i-1; j++) { 
            if (values[j] > values[j+1]) {
                // Swap values[j] and values[j+1]
                int temp = values[j];
                values[j] = values[j+1];
                values[j+1] = temp;
            }
        }
    }

    // Print sorted array
    printk(KERN_INFO "Sorted array:\n");
    for (int i = 0; i < length; i++) {
        printk(KERN_INFO "%d", values[i]);
    }

    end_time = ktime_get();
    ktime_t elapsed_time = ktime_sub(end_time, start_time);
    printk(KERN_INFO "Elapsed time: %llu ns", ktime_to_ns(elapsed_time));
}

void selection_sort(void) {
    printk(KERN_INFO "Selection sort:\n");
    ktime_t start_time, end_time;
    start_time = ktime_get();

    //code
    int i, j, min_idx, temp;
    for (i = 0; i < length - 1; i++) {
        min_idx = i;
        for (j = i + 1; j < length; j++) {
            if (values[j] < values[min_idx]) {
                min_idx = j;
            }
        }
        if (min_idx != i) {
            temp = values[i];
            values[i] = values[min_idx];
            values[min_idx] = temp;
        }
    }

    // Print sorted array
    printk(KERN_INFO "Sorted array:\n");
    for (int i = 0; i < length; i++) {
        printk(KERN_INFO "%d", values[i]);
    }

    end_time = ktime_get();
    ktime_t elapsed_time = ktime_sub(end_time, start_time);
    printk(KERN_INFO "Elapsed time: %llu ns", ktime_to_ns(elapsed_time));
}

void insertion_sort(void) {
    printk(KERN_INFO "Insertion sort:\n");
    ktime_t start_time, end_time;
    start_time = ktime_get();

    //code
    int i, key, j;
    for (i = 1; i < length; i++) {
        key = values[i];
        j = i - 1;

        /* Move elements of values[0..i-1], that are
        greater than key, to one position ahead
        of their current position */
        while (j >= 0 && values[j] > key) {
            values[j + 1] = values[j];
            j = j - 1;
        }
        values[j + 1] = key;
    }

    // Print sorted array
    printk(KERN_INFO "Sorted array:\n");
    for (int i = 0; i < length; i++) {
        printk(KERN_INFO "%d", values[i]);
    }

    end_time = ktime_get();
    ktime_t elapsed_time = ktime_sub(end_time, start_time);
    printk(KERN_INFO "Elapsed time: %llu ns", ktime_to_ns(elapsed_time));
}

// Merges two subarrays of arr[]. 
// First subarray is arr[l..m] 
// Second subarray is arr[m+1..r] 
void merge(int arr[], int l, int m, int r) 
{ 
    int i, j, k; 
    int n1 = m - l + 1; 
    int n2 = r - m; 
  
    // Create temp arrays 
    int L[n1], R[n2]; 
  
    // Copy data to temp arrays 
    // L[] and R[] 
    for (i = 0; i < n1; i++) 
        L[i] = arr[l + i]; 
    for (j = 0; j < n2; j++) 
        R[j] = arr[m + 1 + j]; 
  
    // Merge the temp arrays back 
    // into arr[l..r] 
    // Initial index of first subarray 
    i = 0; 
  
    // Initial index of second subarray 
    j = 0; 
  
    // Initial index of merged subarray 
    k = l; 
    while (i < n1 && j < n2) { 
        if (L[i] <= R[j]) { 
            arr[k] = L[i]; 
            i++; 
        } 
        else { 
            arr[k] = R[j]; 
            j++; 
        } 
        k++; 
    } 
  
    // Copy the remaining elements 
    // of L[], if there are any 
    while (i < n1) { 
        arr[k] = L[i]; 
        i++; 
        k++; 
    } 
  
    // Copy the remaining elements of 
    // R[], if there are any 
    while (j < n2) { 
        arr[k] = R[j]; 
        j++; 
        k++; 
    } 
} 
  
// l is for left index and r is 
// right index of the sub-array 
// of arr to be sorted 
void mergeSort(int arr[], int l, int r) 
{ 
    if (l < r) { 
        // Same as (l+r)/2, but avoids 
        // overflow for large l and r 
        int m = l + (r - l) / 2; 
  
        // Sort first and second halves 
        mergeSort(arr, l, m); 
        mergeSort(arr, m + 1, r); 
  
        merge(arr, l, m, r); 
    } 
} 

void merge_sort(void) {
    printk(KERN_INFO "Merge sort:\n");
    ktime_t start_time, end_time;
    start_time = ktime_get();

    //code
    mergeSort(values, 0, length - 1);

    // Print sorted array
    printk(KERN_INFO "Sorted array:\n");
    for (int i = 0; i < length; i++) {
        printk(KERN_INFO "%d", values[i]);
    }

    end_time = ktime_get();
    ktime_t elapsed_time = ktime_sub(end_time, start_time);
    printk(KERN_INFO "Elapsed time: %llu ns", ktime_to_ns(elapsed_time)); //nanoseconds
}

// Function to swap two elements 
void swap_elements(int* a, int* b) 
{ 
    int temp = *a; 
    *a = *b; 
    *b = temp; 
} 
  
// Partition function 
int partition(int arr[], int low, int high) 
{ 
    // initialize pivot to be the first element 
    int pivot = arr[low]; 
    int i = low; 
    int j = high; 
  
    while (i < j) { 
        // condition 1: find the first element greater than 
        // the pivot (from starting) 
        while (arr[i] <= pivot && i <= high - 1) { 
            i++; 
        } 
  
        // condition 2: find the first element smaller than 
        // the pivot (from last) 
        while (arr[j] > pivot && j >= low + 1) { 
            j--; 
        } 
        if (i < j) { 
            swap_elements(&arr[i], &arr[j]); 
        } 
    } 
    swap_elements(&arr[low], &arr[j]); 
    return j; 
} 
  
// QuickSort function 
void quickSort(int arr[], int low, int high) 
{ 
    if (low < high) { 
        // call Partition function to find Partition Index 
        int partitionIndex = partition(arr, low, high); 
  
        // Recursively call quickSort() for left and right 
        // half based on partition Index 
        quickSort(arr, low, partitionIndex - 1); 
        quickSort(arr, partitionIndex + 1, high); 
    } 
}

void quick_sort(void) {
    printk(KERN_INFO "Quick sort:\n");
    ktime_t start_time, end_time;
    start_time = ktime_get();

    //code
    // calling quickSort() to sort the given array 
    quickSort(values, 0, length - 1); 

    // Print sorted array
    printk(KERN_INFO "Sorted array:\n");
    for (int i = 0; i < length; i++) {
        printk(KERN_INFO "%d", values[i]);
    }

    end_time = ktime_get();
    ktime_t elapsed_time = ktime_sub(end_time, start_time);
    printk(KERN_INFO "Elapsed time: %llu ns", ktime_to_ns(elapsed_time));
}


static int __init sort_init(void) {
    printk(KERN_INFO "length = %d", length);
    
    switch (select) {
        case 1:
            bubble_sort();
            break;
        case 2:
            selection_sort();
            break;
        case 3:
            insertion_sort();
            break;
        case 4:
            merge_sort();
            break;
        case 5:
            quick_sort();
            break;
        default:
            bubble_sort();
            break;
    }
    printk(KERN_INFO "Module %s loaded successfully.\n", MODULE_NAME);
    return 0;
}

static void __exit sort_exit(void) {
    printk(KERN_INFO "Module %s unloaded successfully.\n", MODULE_NAME);
}

module_init(sort_init);
module_exit(sort_exit);

MODULE_LICENSE("GPL");