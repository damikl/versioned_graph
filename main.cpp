#include "tests.h"

using namespace std;
using namespace boost;

int main()
{
    FILELog::ReportingLevel() = logDEBUG4;
    FILE* log_fd = fopen( "mylogfile.txt", "w" );
    Output2FILE::Stream() = log_fd;
    //create an -undirected- graph type, using vectors as the underlying containers
    //and an adjacency_list as the basic representation





    return 0;
}

