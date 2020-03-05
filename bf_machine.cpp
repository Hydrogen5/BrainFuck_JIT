#include <vector>
#include <variant>
#include "instruments.h"

#ifndef _BF_MACHINE_
#define _BF_MACHINE_
class bf_machine
{
    using cube = unsigned short;
    using instrument_t = std::variant<plus_t, minus_t, incre_t, decre_t, loop_start_t, loop_end_t, input_t, output_t>;

public:
    bf_machine() : it(0),
                   cubes(0x3000, 0)
    {
    }
    ~bf_machine() {}

    void run(const std::vector<instrument_t> &instruments)
    {
        int ip(0);
        while (ip < instruments.size())
        {
            switch (instruments[ip].index())
            {
            case 0:
                plus(std::get<0>(instruments[ip]).x);
                break;
            case 1:
                minus(std::get<1>(instruments[ip]).x);
                break;
            case 2:
                incre(std::get<2>(instruments[ip]).x);
                break;
            case 3:
                decre(std::get<3>(instruments[ip]).x);
                break;
            case 4:
                loop_start(ip, std::get<4>(instruments[ip]).end);
                break;
            case 5:
                loop_end(ip, std::get<5>(instruments[ip]).start);
                break;
            case 6:
                input();
                break;
            case 7:
                output();
                break;
            }
            ++ip;
        }
    }

private:
    void plus(int x)
    {
        cubes[it] += x;
    }
    void minus(int x)
    {
        cubes[it] -= x;
    }
    void incre(int x)
    {
        it += x;
    }
    void decre(int x)
    {
        it -= x;
    }
    void loop_start(int &ip, int jmp)
    {
        if (cubes[it] == 0)
            ip = jmp;
    }
    void loop_end(int &ip, int jmp)
    {
        if (cubes[it] != 0)
            ip = jmp;
    }
    void output()
    {
        putchar(static_cast<char>(cubes[it]));
    }
    void input()
    {
        cubes[it] = static_cast<cube>(getchar());
    }

    std::vector<cube> cubes;
    unsigned int it;
};

#endif