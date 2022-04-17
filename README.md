# cscript
Scripting language similar to C in syntax, but without functions, strings, dynamic memory, local scope.

## Syntax

### Functions:
 * You cannot create functions in a cscript script. The script itself is meant to be a simple function itself.
 * You cannot call yourself recursively.
 
### Built-in operators:
 * Arithmetic operators: +, -, *, /, %, ++(prefix), --(prefix)
 * Relational operators: ==, !=, >, <, >=, <=
 * Assignment operators: =, +=, -=, *=, /=
 
### Built-in functions:
 * sqrt: square root
 * sin: sine function
 * cos: cosine function
 * exp: exponential function with base e
 * log: logarithmic function with base e
 * log2: logarithmic function with base 2
 * fabs: absolute value
 * tan: tangent function
 * atan: arctangent function
 * atan2: arctangent function with two arguments, atan2(y,x) computes the arctangent of y/x.
 * pow: power function where pow(x,y) computes x^y

### Variables & arrays:
 * There is no local scope. All variables should have a unique name. Names are case sensitive.
 * Type declarations are int or float, where int represents a 64-bit integer, and float represents a double precisiong floating point number.
 * Arrays of type int or float can be declared. There is no bound checking when calling arrays, so be careful.

### Statements:
 * if (expr) { codetrue }
 * if (expr) { codetrue } else {codefalse}
 * while (expr) { code }
 * for (precode; expr; postcode) { code }
 * int a;
 * int a = expr;
 * int a[number];
 * float a;
 * float a = expr;
 * float a[number];
 * a = expr;
 * a[expr] = expr
 
### cscript function call parameters:
 * Each cscript represents a function.
 * The last value computed in the function is its return value.
 * Each cscript starts with its function parameters. Function parameters can be of type: int, int*, float, float*. Some examples:
     * ()                   no function parameters
     * (int a)              input integer parameter named a
     * (int a, float b)     input integer parameter named a and input floating point parameter named b 
     * (int* a, float* b)   input pointers to a list of integers named a, and a list of floats named b. These pointers can point to dynamic memory. This is the way to use dynamic memory inside a cscript.
 * After the function call parameters brackets {} should be omitted.
 
## Examples

### Fibonacci

    (int i) // cscript function call parameter
    int a = 0; 
    int b = 1; 
    for (int j = 0; j < i; ++j) 
    { 
      int c = a+b; 
      a = b; 
      b = c; 
    } 
    a; // return value

### Quicksort
 
    (int* a, int* stack, int size) // cscript function call parameters
    /*
    int* a: This list of integers represents the list what we want to sort
    int* stack: Recursion is not possible, therefore we need a stack to store temporary results.
                This list should have the same size as the input list a.
    int size: The length of the input lists.
    */
    
    int lo = 0;
    int hi = size-1;
    // initialize top of stack
    int top = -1;
    // push initial values of l and h to stack
    stack[++top] = lo;
    stack[++top] = hi;
    while (top >= 0)
    {
      hi = stack[top];
      --top;
      lo = stack[top];
      --top;
      // partitioning algorithm
      // Set pivot element at its correct position
      // in sorted array
      int x = a[hi];
      int i = (lo - 1);
      for (int j = lo; j <= hi - 1; ++j)
      {
        if (a[j] <= x)
        {
          ++i;
          int tmp = a[i];
          a[i] = a[j];
          a[j] = tmp;
        }
      }
      int tmp2 = a[i+1];
      a[i+1] = a[hi];
      a[hi] = tmp2;
      int p = i+1;
      // end partitioning algorithm

      // If there are elements on left side of pivot,
      // then push left side to stack
      if (p - 1 > lo)
      {
        stack[++top] = lo;
        stack[++top] = p-1;
      }

      // If there are elements on right side of pivot,
      // then push right side to stack
      if (p + 1 < hi)
      {
        stack[++top] = p+1;
        stack[++top] = hi;
      }
    }
  