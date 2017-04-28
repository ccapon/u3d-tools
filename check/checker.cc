#include "u3d_internal.hh"

int main(int argc, char *argv[])
{
    std::cout << "Universal 3D checker" << std::endl;

    if(argc < 2) {
        std::cerr << "Please specify an input file." << std::endl;
        return 1;
    }

    U3D::FileStructure model(argv[1]);

    std::cerr << argv[1] << " successfully parsed." << std::endl;

    return 0;
}
