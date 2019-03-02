#if 0
# test this by typing "bash template_2.cc"
set -e -x
cd build_native
g++ -DECHO_OUTPUT=1 ../template_to_c.cc -o template_to_c
./template_to_c ../template_2.cc template_2_out.cc template_2_out.h
ls -l template_2_out.cc template_2_out.h
g++ -I. -I.. ../template_2.cc ../template_patterns.cc template_2_out.cc -o template_2
./template_2 
exit 0 ;

________ pattern_one
this is @@one@@@@two@@ in a pattern (should be threefour).
________ END

#endif

#include "template_2_out.h"
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
