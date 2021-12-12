#include "elf_utility.hpp"

bool check_instruction_type(uint8_t least_instruction_byte) {
    bool t=1;
    t = std::min(t, (least_instruction_byte&1) == 1);
    least_instruction_byte >>= 1;
    t = std::min(t, (least_instruction_byte&1) == 1);
    return t;
}

void bytes_to_bits(uint8_t *bytes, bool *bits, size_t ct_bytes) {
    for(size_t i = 0; i < ct_bytes; ++i) {
        for(size_t j=0;j<8;++j) {
            bits[i*8+j] = (bytes[i]&(1<<j)) != 0;
        }
    }
}

std::string get_instruction_substring(bool *instruction, size_t l, size_t r) {
    std::string subs = "";
    for(size_t i = l; i <= r; ++i) {
        subs += (char)('0'+instruction[i]);
    }
    return subs;
}

size_t bitstring_to_register(bool *instruction, size_t l, size_t r, size_t start_bit) {
    int rs = 0, cur = (1<<start_bit);
    for(size_t i=l;i<=r;++i) {
        rs += cur * instruction[i];
        cur <<= 1;
    }
    return rs;
}

std::pair<bool, std::string> get_symbol_name(std::vector<symbol_t> &symtab, int address) {
    std::string symbol_name = "";
    bool find_symb = false;
    for(auto &el : symtab) {
        if(el.symbol_value == address) {
            symbol_name = el.symbol_name;
            find_symb = true;
        }
    }

    return make_pair(find_symb && symbol_name.empty(), symbol_name);
}

void disassembler_printer::print_text_head() {
    fprintf(file, "%s", ".text\n");
}

void disassembler_printer::print_symtab_head() {
    fprintf(file, "%s", "\n.symtab\n");
}

void disassembler_printer::print_symbols(std::vector<symbol_t> &symbols) {
    fprintf(file, "%s %-15s %7s %-8s %-8s %-8s %6s %s\n", "Symbol", "Value", "Size", "Type", "Bind", "Vis", "Index", "Name");
    for (auto &symbol : symbols) {
        fprintf(file, "[%4i] 0x%-15X %5i %-8s %-8s %-8s %6s %s\n",
                symbol.symbol_number, 
                (unsigned int)symbol.symbol_value,
                symbol.symbol_size, 
                symbol.symbol_type.c_str(),
                symbol.symbol_bind.c_str(),
                symbol.symbol_vis.c_str(),
                symbol.symbol_index.c_str(),
                symbol.symbol_name.c_str()
        );     
    }
}

void disassembler_printer::print_xsssss(int32_t address, std::string symb_name, std::string cmd, std::string rd, std::string rs1, std::string rs2) {
    fprintf(file, "%08x %10s: %s %s, %s, %s\n", address, symb_name.c_str(), cmd.c_str(), rd.c_str(), rs1.c_str(), rs2.c_str());
}

void disassembler_printer::print_xxssss(int32_t address, std::string cmd, std::string rd, std::string rs1, std::string rs2) {
    fprintf(file, "%08x LOC_%05x: %s %s, %s, %s\n", address, address, cmd.c_str(), rd.c_str(), rs1.c_str(), rs2.c_str());
}

void disassembler_printer::print_xsssd(int32_t address, std::string symb_name, std::string cmd, std::string rd, int32_t offset) {
    fprintf(file, "%08x %10s: %s %s, %d\n", address, symb_name.c_str(), cmd.c_str(), rd.c_str(), offset);
}

void disassembler_printer::print_xxssd(int32_t address, std::string cmd, std::string rd, int32_t offset) {
    fprintf(file, "%08x LOC_%05x: %s %s, %d\n", address, address, cmd.c_str(), rd.c_str(), offset);
}

void disassembler_printer::print_xssssd(int32_t address, std::string symb_name, std::string cmd, std::string rs1, std::string rs2, int32_t offset) {
    fprintf(file, "%08x %10s: %s %s, %s, %d\n", address, symb_name.c_str(), cmd.c_str(), rs1.c_str(), rs2.c_str(), offset);
}

void disassembler_printer::print_xxsssd(int32_t address, std::string cmd, std::string rs1, std::string rs2, int32_t offset) {
    fprintf(file, "%08x LOC_%05x: %s %s, %s, %d\n", address, address, cmd.c_str(), rs1.c_str(), rs2.c_str(), offset);
}

void disassembler_printer::print_xsssds(int32_t address, std::string symb_name, std::string cmd, std::string rd, int32_t offset, std::string rs1) {
    fprintf(file, "%08x %10s: %s %s, %d(%s)\n", address, symb_name.c_str(), cmd.c_str(), rd.c_str(), offset, rs1.c_str());
}

void disassembler_printer::print_xxssds(int32_t address, std::string cmd, std::string rd, int32_t offset, std::string rs1)  {
    fprintf(file, "%08x LOC_%05x: %s %s, %d(%s)\n", address, address, cmd.c_str(), rd.c_str(), offset, rs1.c_str());
}