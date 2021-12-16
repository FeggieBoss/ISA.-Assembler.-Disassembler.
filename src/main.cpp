#include "elf_utility.hpp"
#include <algorithm>
#include <vector>
#include <utility>

int main(int argc, char* argv[]) {
    if(argc < 3) {
        printf("Wrong format of arguments\n");
        exit(-1);
    }
    uint8_t *buffer = new uint8_t[4];
    bool *bits = new bool[32];
    std::string registers_ABI[32] = { "zero", "ra", "sp", "gp", 
                                      "tp", "t0", "t1", "t2", 
                                      "s0", "s1", 
                                      "a0",  "a1", "a2", "a3", "a4", "a5", "a6", "a7", 
                                      "s2", "s3", "s4", "s5", "s6", "s7", "s8", "s9", "s10", "s11", "t3", "t4", "t5", "t6" };


    Elf_parser elf_parser(argv[1]);
    section_t text = elf_parser.get_text_section();
    std::vector<symbol_t> symtab = elf_parser.get_symbols();

    disassembler_printer dis_printer = disassembler_printer(argv[2]);
    dis_printer.print_text_head();
    //std::cout<<".text\n";

    FILE *pfile = fopen(argv[1], "rb");
    fseek(pfile, text.section_offset, SEEK_SET);

    Elf32_Ehdr ehdr; 
    fseek(elf_parser.iterator_, 0, SEEK_SET);
    size_t result = fread(&ehdr, sizeof(Elf32_Ehdr), 1, elf_parser.iterator_);
    int32_t address = ehdr.e_entry;
    size_t already_read = 0;
    while (already_read < (size_t)text.section_size) {
        result = fread(buffer,1,2,pfile);
        if (result != 2) break;

        auto maybe_symb_name = get_symbol_name(symtab, address);
        bool fl = maybe_symb_name.first;
        std::string symb_name = maybe_symb_name.second;

        if (check_instruction_type(buffer[0]) ) { // RV32I and RV32M
            fseek(pfile, -2, SEEK_CUR);
            result = fread(buffer,1,4,pfile);
            if (result != 4) break;

            bytes_to_bits(buffer, bits, 4);

            std::string opcode = get_instruction_substring(bits, 0, 6); std::reverse(opcode.begin(),opcode.end());
            std::string rd = registers_ABI[bitstring_to_register(bits, 7, 11, 0)];
            std::string funct3 = get_instruction_substring(bits, 12, 14); std::reverse(funct3.begin(),funct3.end());
            std::string rs1 = registers_ABI[bitstring_to_register(bits, 15, 19, 0)];
            std::string rs2 = registers_ABI[bitstring_to_register(bits, 20, 24, 0)];
            std::string funct2 = get_instruction_substring(bits, 25, 26); std::reverse(funct2.begin(),funct2.end());
            std::string funct5 = get_instruction_substring(bits, 27, 31); std::reverse(funct5.begin(),funct5.end());

            if(opcode=="0110011") { // RV32M or Alg.Op. RV32I
                if(funct2=="01") { // RV32M
                    std::string cmd;
                    if(funct3=="000")
                        cmd="MUL";
                    else if(funct3=="001")
                        cmd="MULH";
                    else if(funct3=="010")
                        cmd="MULHSU";
                    else if(funct3=="011")
                        cmd="MULHU";
                    else if(funct3=="100")
                        cmd="DIV";
                    else if(funct3=="101")
                        cmd="DIVU";
                    else if(funct3=="110")
                        cmd="REM";
                    else if(funct3=="111")
                        cmd="REMU";

                    if(fl)
                        dis_printer.print_xxssss(address, cmd, rd, rs1, rs2);
                    else
                        dis_printer.print_xsssss(address, symb_name, cmd, rd, rs1, rs2);
                }
                else if(funct2 == "00") { // Alg.Op. RV32I
                    std::string cmd;
                    if(funct3 == "000"){
                        if(funct5=="00000") 
                            cmd="ADD";
                        else if(funct5=="01000")
                            cmd="SUB";
                    }
                    else if(funct3=="001")
                        cmd="SLL";
                    else if(funct3=="010")
                        cmd="SLT";
                    else if(funct3=="011")
                        cmd="SLTU";
                    else if(funct3=="100")
                        cmd="XOR";
                    else if(funct3=="101"){
                        if(funct5=="00000") 
                            cmd="SRL";
                        else if(funct5=="01000")
                            cmd="SRA";
                    }
                    else if(funct3=="110")
                        cmd="OR";
                    else if(funct3=="111")
                        cmd="AND";

                    if(fl)
                        dis_printer.print_xxssss(address, cmd, rd, rs1, rs2);
                    else
                        dis_printer.print_xsssss(address, symb_name, cmd, rd, rs1, rs2);
                }
                else {
                    std::cout<<"Unknown command: incorrect opcode\n";
                }
            }
            else { // RV32I
                if(opcode == "0110111") { // LUI
                    int imm = bitstring_to_register(bits, 12, 31, 12);
                    if(fl)
                        dis_printer.print_xxssd(address, "LUI", rd, imm);
                    else
                        dis_printer.print_xsssd(address, symb_name, "LUI", rd, imm);
                }
                else if(opcode == "0010111") { // AUIPC
                    int offset = bitstring_to_register(bits, 12, 31, 12);
                    if(fl)
                        dis_printer.print_xxssd(address, "AUIPC", rd, offset);
                    else
                        dis_printer.print_xsssd(address, symb_name, "AUIPC", rd, offset);
                }
                else if(opcode == "1101111") { // JAl
                    int offset = bitstring_to_register(bits, 12, 19, 12)
                               - bitstring_to_register(bits, 20, 20, 0) * ((int32_t)1<<11)
                               + bitstring_to_register(bits, 21, 30, 1)
                               + bitstring_to_register(bits, 31, 31, 20);

                    //printf("JAl %s, %d\n", rd.c_str(), offset);
                    if(fl)
                        dis_printer.print_xxssd(address, "JAl", rd, offset);
                    else
                        dis_printer.print_xsssd(address, symb_name, "JAl", rd, offset);
                }
                else if(opcode == "1100111") { // JALR
                    int offset = bitstring_to_register(bits, 20, 30, 0)
                               - bitstring_to_register(bits, 31, 31, 0) * ((int32_t)1<<11);
                    if(fl)
                        dis_printer.print_xxsssd(address, "JALR", rd, rs1, offset);
                    else
                        dis_printer.print_xssssd(address, symb_name, "JALR", rd, rs1, offset);
                    //printf("JAlR %s, %s, %d\n", rd.c_str(), rs1.c_str(), offset);
                }
                else if(opcode == "1100011") { // B*
                    int offset = - bitstring_to_register(bits, 7, 7, 0) * ((int32_t)1<<11)
                               + bitstring_to_register(bits, 8, 11, 1)
                               + bitstring_to_register(bits, 25, 30, 5)
                               + bitstring_to_register(bits, 31, 31, 12);

                    std::string cmd;
                    if(funct3 == "000")
                        cmd="BEQ";
                    else if(funct3=="001")
                        cmd="BNE";
                    else if(funct3=="100")
                        cmd="BLT";
                    else if(funct3=="101")
                        cmd="BGE";
                    else if(funct3=="110")
                        cmd="BLTU";
                    else if(funct3=="111")
                        cmd="BGEU";

                    if(fl)
                        dis_printer.print_xxsssd(address, cmd, rs1, rs2, offset);
                    else
                        dis_printer.print_xssssd(address, symb_name, cmd, rs1, rs2, offset);
                    //printf("%s %s, %s, %d\n", cmd.c_str(), rs1.c_str(), rs2.c_str(), offset);
                }
                else if(opcode == "0000011") { // L*
                    int offset = bitstring_to_register(bits, 20, 30, 0)
                               - bitstring_to_register(bits, 31, 31, 0) * ((int32_t)1<<11);

                    std::string cmd;
                    if(funct3 == "000")
                        cmd="LB";
                    else if(funct3=="001")
                        cmd="LH";
                    else if(funct3=="010")
                        cmd="LW";
                    else if(funct3=="100")
                        cmd="LBU";
                    else if(funct3=="101")
                        cmd="LHU";

                    if(fl)
                        dis_printer.print_xxssds(address, cmd, rd, offset, rs1);
                    else
                        dis_printer.print_xsssds(address, symb_name, cmd, rd, offset, rs1);
                    //printf("%s %s, %d(%s)\n", cmd.c_str(), rd.c_str(), offset, rs1.c_str());
                }
                else if(opcode == "0100011") { // S*
                    int offset = bitstring_to_register(bits, 7, 11, 0)
                               + bitstring_to_register(bits, 25, 30, 5) 
                               - bitstring_to_register(bits, 31, 31, 0) * ((int32_t)1<<11);

                    std::string cmd;
                    if(funct3 == "000")
                    cmd="SB";
                    else if(funct3=="001")
                        cmd="SH";
                    else if(funct3=="010")
                        cmd="SW";

                    if(fl)
                        dis_printer.print_xxssds(address, cmd, rs2, offset, rs1);
                    else
                        dis_printer.print_xsssds(address, symb_name, cmd, rs2, offset, rs1);
                    //printf("%s %s, %d(%s)\n", cmd.c_str(), rs2.c_str(), offset, rs1.c_str());
                }
                else if(opcode == "0010011") { // Logic
                    int imm = bitstring_to_register(bits, 20, 30, 0)
                            - bitstring_to_register(bits, 31, 31, 0) * ((int32_t)1<<11);

                    std::string cmd;
                    if(funct3 == "000")
                        cmd="ADDI";
                    else if(funct3=="010")
                        cmd="SLTI";
                    else if(funct3=="011")
                        cmd="SLTIU";
                    else if(funct3=="100")
                        cmd="XORI";
                    else if(funct3=="110")
                        cmd="ORI";
                    else if(funct3=="111")
                        cmd="ANDI";
                    else if(funct3=="001")
                        cmd="SLLI";
                    else if(funct3=="101") {
                        imm = bitstring_to_register(bits, 20, 24, 0);

                        if(funct5=="00000") 
                            cmd="SRLI";
                        else if(funct5=="01000")
                            cmd="SRAI";
                    }

                    if(fl)
                        dis_printer.print_xxsssd(address, cmd, rd, rs1, imm);
                    else
                        dis_printer.print_xssssd(address, symb_name, cmd, rd, rs1, imm);

                    //printf("%s %s, %s, %d\n", cmd.c_str(), rd.c_str(), rs1.c_str(), imm);
                }
                else if(opcode == "0001111") { // FENCE*
                    // не обрабатываем
                    std::cout<<"Error command: FENCE\n";
                }
                else {
                    std::cout<<"Unknown command: incorrect opcode\n";
                }
            }

            address += 4;
        }
        else { // RV32C
            bytes_to_bits(buffer, bits, 2);

            /*
                TO DO
            */

            address += 2;
        }

        already_read += result;
    }

    if (already_read != (size_t)text.section_size) {
        printf("Reading error: incorrect file format"); 
        exit (-1);
    }

    //std::cout<<"\n.symtab\n";
    dis_printer.print_symtab_head();
    dis_printer.print_symbols(symtab);

    delete[] bits;
    delete[] buffer;
    fclose(pfile);

    return 0;
}