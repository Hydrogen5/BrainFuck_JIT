#include <iostream>
#include <cstdio>
#include <fstream>
#include <variant>
#include "instruments.h"
#include <sys/mman.h>
#include "compile.cpp"
#include "bf_machine.cpp"
#include "luajit-2.0/dynasm/dasm_proto.h"
#include "luajit-2.0/dynasm/dasm_x86.h"
using instrument_t = std::variant<plus_t, minus_t, incre_t, decre_t, loop_start_t, loop_end_t, input_t, output_t>;

void (*bf_jit(std::vector<instrument_t> instruments))(unsigned char *, int(void), int(int))
{
    /*
    rcx : aPtr
    r12 : getchar
    r13 : putchar
    mov rcx, rdi ; mov r12, rsi; mov r13, rdx; ==== save aPtr -> H\x89\xf9
    add byte ptr [rcx], n ==== plus -> \x80\x01n
    sub byte ptr [rcx], n ==== minus -> \x80)n
    add rcx, n ==== incre -> H\x83\xc1n
    sub rcx, n ==== decre -> H\x83\xe9n
    cmp byte ptr [rcx], 0 ; jz x; \x809\x00 ==== jmp to end  -> \x0f\x85____ 
    cmp byte ptr [rcx], 0 ; jnz x; \x809\x00 ==== jmp to start -> \x0f\x84____ 
    mov r15, rcx ; mov rax, r12 ; call rax ; mov rcx, r15 ; mov byte ptr [rcx], al ==== input
    mov r15, rcx ; mov dl, byte ptr [rcx] ; mov rax, r13 ; call rax ; mov rcx, r15 ==== output
    */
    std::stack<int> call_stack;
    int t, t2;
    int program = 0;
    int ip(0);
    unsigned char *code = new unsigned char[0x300000];
    memcpy(code, "H\x89\xf9I\x89\xf4I\x89\xd5", 9);
    program += 9;
    while (ip < instruments.size())
    {
        switch (instruments[ip].index())
        {
        case 0:
            t = std::get<0>(instruments[ip]).x;
            code[program++] = '\x80';
            code[program++] = '\x01';
            code[program++] = static_cast<unsigned char>(t);
            break;
        case 1:
            t = std::get<1>(instruments[ip]).x;
            code[program++] = '\x80';
            code[program++] = ')';
            code[program++] = static_cast<unsigned char>(t);
            break;
        case 2:
            t = std::get<2>(instruments[ip]).x;
            code[program++] = 'H';
            code[program++] = '\x83';
            code[program++] = '\xc1';
            code[program++] = static_cast<unsigned char>(t);
            break;
        case 3:
            t = std::get<3>(instruments[ip]).x;
            code[program++] = 'H';
            code[program++] = '\x83';
            code[program++] = '\xe9';
            code[program++] = static_cast<unsigned char>(t);
            break;
        case 4:
            memcpy(&code[program], "\x80\x39\x00\x0f\x84\xde\xad\xbe\xef", 9);
            program += 9;
            call_stack.push(program);
            break;
        case 5:
            t = call_stack.top();
            memcpy(&code[program], "\x80\x39\x00\x0f\x85", 5);
            program += 5;
            t -= (program + 4);
            memcpy(&code[program], reinterpret_cast<unsigned char *>(&t), 4);
            program += 4;
            t = call_stack.top();
            t2 = program - t;
            memcpy(&code[t - 4], reinterpret_cast<unsigned char *>(&t2), 4);
            call_stack.pop();
            break;
        case 6:
            memcpy(&code[program], "I\x89\xcfL\x89\xe0\xff\xd0L\x89\xf9\x88\x01", 13);
            program += 13;
            break;
        case 7:
            memcpy(&code[program], "I\x89\xcfH1\xd2\x8a\x11H\x89\xd7L\x89\xe8\xff\xd0L\x89\xf9", 19);
            program += 19;
            break;
        }
        ++ip;
    }
    code[program++] = '\xc3';
    auto buf = mmap((void *)0x200000, program + 0x500, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    memcpy(buf, code, program);
    mprotect(buf, program + 0x500, PROT_READ | PROT_EXEC);
    // delete[] code;
    return reinterpret_cast<void (*)(unsigned char *, int (*)(), int (*)(int))>(buf);
}

int main(int argc, char **argv)
{
    // std::string src = "+++++++++++++++++++++++++++++++++++++++++++++++++.>++++++++++.>,.";
    // auto &&ins = optimize(compile(src));
    // auto a = bf_jit(ins);
    // a(new unsigned char[0x3000], getchar, putchar);
    // return 0;
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
            auto a = bf_jit(ins);
            auto t = new unsigned char[0x30000];
            memset(t, 0, 0x30000);
            a(t, getchar, putchar);
            exit(0);
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