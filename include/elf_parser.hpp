#pragma once
#ifndef H_ELF_PARSER
#define H_ELF_PARSER

#include <elf.hpp>  

#include <sys/stat.h> 
#include <sys/mman.h>
#include <fcntl.h>

#include <string>
#include <vector>

typedef struct {
    int32_t section_number = 0; 
    int32_t section_offset;
    int32_t section_address;
    int32_t section_size;
    int32_t section_esize;
    int32_t section_add_al;
    std::string section_name;
    std::string section_type; 
} section_t;

typedef struct symbol_t_struct {
    int32_t symbol_number = 0;
    int32_t symbol_value;
    int32_t symbol_size = 0;
    std::string symbol_type;
    std::string symbol_bind;
    std::string symbol_vis;
    std::string symbol_index;
    std::string symbol_name;

    symbol_t_struct(int32_t arg1, 
                    int32_t arg2,
                    int32_t arg3,
                    std::string arg4,
                    std::string arg5,
                    std::string arg6,
                    std::string arg7,
                    std::string arg8 ) {
    symbol_number = arg1;
    symbol_value = arg2;
    symbol_size = arg3;
    symbol_type = arg4;
    symbol_bind = arg5;
    symbol_vis = arg6;
    symbol_index = arg7;
    symbol_name = arg8;
    }
} symbol_t;

class Elf_parser {
public:
    Elf_parser (std::string file_name_): file_name(file_name_) {   
            load_memory_map();
    }
    std::vector<section_t> get_sections();
    std::vector<symbol_t> get_symbols();
    section_t get_text_section();
    uint8_t *iterator_;
private:
    void load_memory_map();
    
    std::string file_name; 

    std::string get_section_type(int number);
     
    std::string get_symbol_type(int sym_type_number);
    std::string get_symbol_bind(int sym_bind_numer);
    std::string get_symbol_visibility(int sym_vis_numer);
    std::string get_symbol_index(int sym_idx_numer);
};

#endif // H_ELF_PARSER
