#include <iostream>

int main(int argc, char **argv)
{
    if (argc==1) {
        std::cout<<"Usage:"<<argv[0]<<" qucs_netlist.net\n";
        return 1;
    }

    return 0;
}

