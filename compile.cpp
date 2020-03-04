#include <vector>
#include <variant>
#include <stack>
#include "instruments.h"
using instrument_t = std::variant<plus_t, minus_t, incre_t, decre_t, loop_start_t, loop_end_t, input_t, output_t>;

std::vector<instrument_t> compile(const std::string &src)
{
    std::vector<instrument_t> ins;
    std::stack<int> call_stack;
    ins.reserve(src.size() + 0x30);
    int t;
    for (int i = 0; i < src.length(); i++)
    {
        switch (src[i])
        {
        case '+':
            ins.push_back(plus_t{1});
            break;
        case '-':
            ins.push_back(minus_t{1});
            break;
        case '>':
            ins.push_back(incre_t{1});
            break;
        case '<':
            ins.push_back(decre_t{1});
            break;
        case '[':
            ins.push_back(loop_start_t{19260817});
            call_stack.push(i);
            break;
        case ']':
            t = call_stack.top();
            call_stack.pop();
            ins.push_back(loop_end_t{t});
            ins[t] = loop_start_t{i};
            break;
        case '.':
            ins.push_back(output_t{});
            break;
        case ',':
            ins.push_back(input_t{});
            break;
        default:
            break;
        }
    }
    return ins;
}