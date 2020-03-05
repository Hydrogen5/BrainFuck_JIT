#include <iostream>
#include <cstdio>
#include <fstream>
#include "instruments.h"
#include "compile.cpp"
#include "bf_machine.cpp"
using instrument_t = std::variant<plus_t, minus_t, incre_t, decre_t, loop_start_t, loop_end_t, input_t, output_t>;

// void debug(std::vector<instrument_t> instruments)
// {
//     int ip(0);
//     while (ip < instruments.size())
//     {
//         switch (instruments[ip].index())
//         {
//         case 0:
//             printf("[%d, %d]\n", instruments[ip].index(), std::get<0>(instruments[ip]).x);
//             break;
//         case 1:
//             printf("[%d, %d]\n", instruments[ip].index(), std::get<1>(instruments[ip]).x);
//             break;
//         case 2:
//             printf("[%d, %d]\n", instruments[ip].index(), std::get<2>(instruments[ip]).x);
//             break;
//         case 3:
//             printf("[%d, %d]\n", instruments[ip].index(), std::get<3>(instruments[ip]).x);
//             break;
//         case 4:
//             printf("[%d, %d]\n", instruments[ip].index(), ip, std::get<4>(instruments[ip]).end);
//             break;
//         case 5:
//             printf("[%d, %d]\n", instruments[ip].index(), std::get<5>(instruments[ip]).start);
//             break;
//             // case 6:
//             //     input();
//             //     break;
//             // case 7:
//             //     output();
//             // break;
//         }
//         ++ip;
//     }
// }

int main(int argc, char **argv)
{
    if (argc == 2)
    {
        std::string src;
        std::ifstream in(argv[1]);
        if (!in.is_open())
        {
            fprintf(stderr, "Cannot open %s\n", argv[1]);
            return 1;
        }
        std::string tmp;
        while (in >> tmp)
            src += tmp;
        try
        {
            auto &&ins = optimize(compile(src));
            bf_machine bf;
            bf.run(ins);
            return 0;
        }
        catch (const char *&e)
        {
            std::cerr << e << '\n';
        }
    }
    else
    {
        fprintf(stderr, "Usage: %s INFILE.bf\n", argv[0]);
        return 1;
    }
}