#include <iostream>
#include <fstream>

int main()
{
    double Gamma_dot;

    for (unsigned j = 0; j <= 100; j++)
    {
        Gamma_dot = (1.0e-2)*j;
        std::string fileName = "test_flow" + std::to_string(j) + ".dat";
        std::ofstream demoFile(fileName);
        if (!demoFile) return 1;

        for (unsigned i = 0; i < 10; i++)
        {
           demoFile << 0 << "  " << i << "  ";
           demoFile << Gamma_dot*i << "  " << 0 << std::endl;
        }
    }

    
return 0;

}