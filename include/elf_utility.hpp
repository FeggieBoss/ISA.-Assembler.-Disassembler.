#ifndef H_ELF_UTILITY
#define H_ELF_UTILITY

#include "elf_parser.hpp"

bool check_instruction_type(uint8_t least_instruction_byte); 
void bytes_to_bits(uint8_t *bytes, bool *bits, size_t ct_bytes);
std::string get_instruction_substring(bool *instruction, size_t l, size_t r);
size_t bitstring_to_register(bool *instruction, size_t l, size_t r, size_t start_bit) ;
std::pair<bool, std::string> get_symbol_name(std::vector<symbol_t> &symtab, int address);

class disassembler_printer {
public:
    disassembler_printer(std::string fname): file(fopen(fname.c_str(), "w")) {};
    
    void print_text_head();
    void print_symtab_head();
    void print_symbols(std::vector<symbol_t> &symbols);
    void print_xsssss(int32_t address, std::string symb_name, std::string cmd, std::string rd, std::string rs1, std::string rs2);
    void print_xxssss(int32_t address, std::string cmd, std::string rd, std::string rs1, std::string rs2);
    void print_xsssd(int32_t address, std::string symb_name, std::string cmd, std::string rd, int32_t offset);
    void print_xxssd(int32_t address, std::string cmd, std::string rd, int32_t offset);
    void print_xssssd(int32_t address, std::string symb_name, std::string cmd, std::string rs1, std::string rs2, int32_t offset);
    void print_xxsssd(int32_t address, std::string cmd, std::string rs1, std::string rs2, int32_t offset);
    void print_xsssds(int32_t address, std::string symb_name, std::string cmd, std::string rd, int32_t offset, std::string rs1);
    void print_xxssds(int32_t address, std::string cmd, std::string rd, int32_t offset, std::string rs1);
    
    ~disassembler_printer() { fclose(file); }
private:
    FILE *file;
};

#endif // H_ELF_UTILITY