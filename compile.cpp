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
            if (call_stack.empty())
                throw "unexpected right bracket";
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
            ins.push_back(plus_t{0});
            break;
        }
    }
    if (!call_stack.empty())
        throw "unclosed left bracket";
    return ins;
}
template <typename T, size_t Ty>
instrument_t fold(std::vector<instrument_t>::const_iterator &it)
{
    int now = std::get<Ty>(*it).x;
    ++it;
    while (it->index() == Ty)
    {
        now += std::get<Ty>(*it).x;
        ++it;
    }
    --it;
    return T{now};
}

std::vector<instrument_t> optimize(const std::vector<instrument_t> &code)
{
    std::vector<instrument_t> ins;
    std::stack<int> call_stack;
    int t;
    int program(0);
    ins.reserve(code.size());
    auto it = code.begin();
    while (it != code.end())
    {
        switch (it->index())
        {
        case 0:
            ins.push_back(fold<plus_t, 0>(it));
            break;
        case 1:
            ins.push_back(fold<minus_t, 1>(it));
            break;
        case 2:
            ins.push_back(fold<incre_t, 2>(it));
            break;
        case 3:
            ins.push_back(fold<decre_t, 3>(it));
            break;
        case 4:
            ins.push_back(loop_start_t{19260817});
            call_stack.push(program);
            break;
        case 5:
            t = call_stack.top();
            call_stack.pop();
            ins.push_back(loop_end_t{t});
            ins[t] = loop_start_t{program};
            break;
        default:
            ins.push_back(*it);
        }
        ++program;
        ++it;
    }
    return ins;
}