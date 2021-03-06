#if 0
# test this by typing "bash template_to_c_test.cc"
set -e -x
mkdir -p build_native
cd build_native
g++ -DECHO_OUTPUT=1 ../template_to_c.cc -o template_to_c_test
./template_to_c_test ../template_to_c_test.cc template_test_out.cc template_test_out.h
ls -l template_test_out.cc template_test_out.h
g++ -I. -I.. ../template_to_c_test.cc ../template_patterns.cc template_test_out.cc -o template_test
./template_test
exit 0 ;

________ pattern_one
this is @@one@@@@two@@ in a pattern (should be threefour).
________ END

#endif

#include "template_test_out.h"
#include <iostream>

int
main()
{
    pattern_value_map  patterns;

    patterns["one"] = "three";
    patterns["two"] = "four";

    output_pattern_one(std::cout, patterns);

    return 0;
}
