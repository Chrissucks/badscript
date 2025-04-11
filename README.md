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
  if (y >= 16) { println("y is large"); }
  ```

```plaintext
counter = 10;
```

  ```plaintext
  println("Hello, BadScript!");
  print(123);
  ```

## libraries

- **import "library_name":** Dynamically loads a DLL from the `bin\libraries\` folder. The DLL file must be named `library_name.dll`.

- After importing, you can call the library's exported function by writing:
  
  ```plaintext
  library_name();
  ```
  
  **Example:**
  
  ```plaintext
  import "count100";
  count100();
  ```

  if you want to make your own library reference the example counter for how to setup the basic structure.
