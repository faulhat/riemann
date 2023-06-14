#include <sstream>
#include <cstring>

int run_tests();

void repl();
   
int main(int argc, char **argv) {
   if (argc > 1 && (strncmp(argv[1], "-t", 2) == 0 ||
                    strcmp(argv[1], "--test") == 0)) {
      return run_tests();
   }
   else {
      repl();
      return 0;
   }
}
