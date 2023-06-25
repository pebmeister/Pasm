#include "pass.h"
#include "parseargs.h"

/**

 * \brief main entry point
 * \param argc number of arguments
 * \param argv argument values
 * \return 0 on success
 */
int main(const int argc, char* argv[])
{

    pre_parse();
    
    // parse the arguments
    int result =  parse_arguments(argc, argv);
    if (result < 1)
        return result;

    result = assemble();
    return result;
}
