BCS2204 Library Returns Register – Clang verified source

Files
- main.cpp: application entry point and menu
- Doublylinkedlist.h: templated doubly linked list and file persistence
- Sorting.h: insertion sort, merge sort and binary search templates
- tests.cpp: 18 automated data-structure and algorithm tests
- sample_data.txt: 12 sample return records (Windows CRLF line endings)
- benchmark.cpp / benchmark_results.csv: supplied timing experiment
- screenshots/: coursework evidence images
- BCS2204_Library_Returns_Register_Clang_Verified_Report.docx: updated report in the supplied coursework format

Build and test in the MSYS2 CLANG64 terminal (from this folder):

  clang++ -std=c++17 -Wall -Wextra -Wpedantic -Wconversion -Wshadow -Werror main.cpp -o library_register.exe
  clang++ -std=c++17 -Wall -Wextra -Wpedantic -Wconversion -Wshadow -Werror tests.cpp -o coursework_tests.exe
  ./coursework_tests.exe
  ./library_register.exe

AddressSanitizer test build (if your CLANG64 installation has the ASan runtime):

  clang++ -std=c++17 -Wall -Wextra -Wpedantic -Wconversion -Wshadow -g -O1 -fsanitize=address -fno-omit-frame-pointer tests.cpp -o coursework_tests_asan.exe
  ASAN_OPTIONS=detect_leaks=1:halt_on_error=1 ./coursework_tests_asan.exe

Important fix
loadFromFile now removes a trailing carriage return from each line before parsing. This handles Windows CRLF text files, where std::getline removes '\n' but leaves '\r'. Without this correction the return-date field was one character too long and all 12 supplied records were rejected.

Verification actually performed for this package
- Clang 17 strict-warning build: successful, no warnings promoted to errors.
- Automated tests: 18 passed, 0 failed.
- Application startup: 12 valid sample records loaded.
- AddressSanitizer: test executable and application ran; no sanitizer report was produced.

These results were obtained in the available Clang 17 execution environment. Re-run the commands in the user's MSYS2 CLANG64 environment and capture local screenshots if required by the lecturer.
