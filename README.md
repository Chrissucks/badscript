# badscript
Interpreted language written in No-CRT C90 for a Windows target. This language is purposefully slow to prove a point.

## building
   ```bash
   mkdir build
   cd build
   cmake ..
   cmake --build . --config MinSizeRel
   ```
files will be found in build/bin

## usage
```bash
badscript_interpreter.exe <file.bs>
```

## badscript features

  ```plaintext
  x = 5 + 3;
  y = x * 2;
  if (y >= 16) { println("large"); }
  ```

```plaintext
counter = 10;
```

  ```plaintext
  println("Hello");
  print(123);
  ```

## libraries
  
  ```plaintext
  library_name();
  ```
  
  **Example:**
  
  ```plaintext
  import "count100";
  count100();
  ```

  if you want to make your own library reference the example counter for how to setup the basic structure.
