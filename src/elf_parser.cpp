#include "elf_parser.hpp"

section_t Elf_parser::get_text_section() {
    std::vector<section_t> secs = get_sections();
    section_t text;
    for(auto el : secs) {
        if(el.section_name == ".text")
            text = el;
    }
    return text;
}

void Elf_parser::load_memory_map() {
    iterator_ = fopen(file_name.c_str(), "rb");
}

std::string Elf_parser::get_section_type(int number) {
    if(number == 2)
        return ".symtab";
    if(number == 3)
        return ".strtab";
    if(number == 11)
        return ".dynsym";
    return "";
}

char buffer[1000];
std::vector<section_t> Elf_parser::get_sections() {
    Elf32_Ehdr ehdr; 
    fseek(iterator_, 0, SEEK_SET);
    auto r = fread(&ehdr, sizeof(Elf32_Ehdr), 1, iterator_);

    Elf32_Shdr sh_strtab;
    fseek(iterator_, ehdr.e_shoff + (ehdr.e_shstrndx)*sizeof(Elf32_Shdr), SEEK_SET);
    r = fread(&sh_strtab, sizeof(Elf32_Shdr), 1, iterator_);

    std::vector<section_t> sections;
    for (size_t i = 0; i < ehdr.e_shnum; ++i) {
        Elf32_Shdr shdr; 
        fseek(iterator_, ehdr.e_shoff + i*sizeof(Elf32_Shdr), SEEK_SET);
        r = fread(&shdr, sizeof(Elf32_Shdr), 1, iterator_);
        
        fseek(iterator_, sh_strtab.sh_offset + shdr.sh_name, SEEK_SET);
        r = fscanf(iterator_, "%s", buffer);

        section_t section = section_t(
            i, 
            shdr.sh_offset,
            shdr.sh_addr,
            shdr.sh_size,
            shdr.sh_entsize,
            shdr.sh_addralign,
            std::string(buffer),
            get_section_type(shdr.sh_type)            
        );
        sections.push_back(section);
    }
    if(r){};

    return sections;
}

std::string Elf_parser::get_symbol_bind(int sym_bind_number) {
    if(ELF32_ST_BIND(sym_bind_number) == 0)
        return "LOCAL";
    if(ELF32_ST_BIND(sym_bind_number) == 1)
        return "GLOBAL";
    if(ELF32_ST_BIND(sym_bind_number) == 2)
        return "WEAK";
    if(ELF32_ST_BIND(sym_bind_number) == 3)
        return "NUM";
    if(ELF32_ST_BIND(sym_bind_number) == 10)
        return "UNIQUE";
    if(ELF32_ST_BIND(sym_bind_number) == 12)
        return "HIOS";
    if(ELF32_ST_BIND(sym_bind_number) == 13)
        return "LOPROC";
    return "UNKNOWN";
}

std::string Elf_parser::get_symbol_visibility(int sym_vis_numer) {
    if(ELF32_ST_VISIBILITY(sym_vis_numer)==0)
        return "DEFAULT";
    if(ELF32_ST_VISIBILITY(sym_vis_numer)==1)
        return "INTERNAL";
    if(ELF32_ST_VISIBILITY(sym_vis_numer)==2)
        return "HIDDEN";
    if(ELF32_ST_VISIBILITY(sym_vis_numer)==3)
        return "PROTECTED";
    return "UNKNOWN";
}

std::string Elf_parser::get_symbol_index(int sym_idx_numer) {
    if(sym_idx_numer == SHN_ABS)
        return "ABS";
    if(sym_idx_numer == SHN_COMMON || sym_idx_numer == SHN_XINDEX)
        return "COM";
    if(sym_idx_numer == SHN_UNDEF)
        return "UNDEF";
    return std::to_string(sym_idx_numer);
}

std::string Elf_parser::get_symbol_type(int sym_type_number) {
    if(ELF32_ST_TYPE(sym_type_number) == 0)
        return "NOTYPE";
    if(ELF32_ST_TYPE(sym_type_number) == 1)
        return "OBJECT";
    if(ELF32_ST_TYPE(sym_type_number) == 2)
        return "FUNC";
    if(ELF32_ST_TYPE(sym_type_number) == 3)
        return "SECTION";
    if(ELF32_ST_TYPE(sym_type_number) == 4)
        return "FILE";
    if(ELF32_ST_TYPE(sym_type_number) == 6)
        return "TLS";
    if(ELF32_ST_TYPE(sym_type_number) == 7)
        return "NUM";
    if(ELF32_ST_TYPE(sym_type_number) == 10)
        return "LOOS";
    if(ELF32_ST_TYPE(sym_type_number) == 12)
        return "HIOS";
    return "UNKNOWN";
}

std::vector<symbol_t> Elf_parser::get_symbols() {
    std::vector<section_t> all_sections = get_sections();
    size_t strtab_offset=-1;
    size_t dynstr_offset=-1; 

    for(auto &el: all_sections) {
        if(el.section_type == ".strtab" && 
           el.section_name == ".dynstr") {
            dynstr_offset = el.section_offset;
            break;
        }
    }
    for(auto &el: all_sections) {
        if(el.section_type == ".strtab" && 
           el.section_name == ".strtab") {
            strtab_offset = el.section_offset;
            break;
        }
    }

    std::vector<symbol_t> symbols;
    for(auto &el : all_sections) {
        if(el.section_type != ".symtab" && 
           el.section_type != ".dynsym")
            continue;
        size_t r;
        for (size_t i = 0; i < el.section_size / sizeof(Elf32_Sym); ++i) {
            Elf32_Sym info;
            fseek(iterator_, el.section_offset + i*sizeof(Elf32_Sym), SEEK_SET);
            r = fread(&info, sizeof(Elf32_Sym), 1, iterator_);

            symbol_t symbol = symbol_t(
                i, 
                info.st_value, 
                info.st_size,
                get_symbol_type(info.st_info),
                get_symbol_bind(info.st_info),
                get_symbol_visibility(info.st_other),
                get_symbol_index(info.st_shndx),
                ""
            );
            if(el.section_type == ".symtab") {
                fseek(iterator_, strtab_offset + info.st_name, SEEK_SET);
                r = fscanf(iterator_, "%s", buffer);
                symbol.symbol_name = std::string(buffer);
            }
            if(el.section_type == ".dynsym") {
                fseek(iterator_, dynstr_offset + info.st_name, SEEK_SET);
                r = fscanf(iterator_, "%s", buffer);
                symbol.symbol_name = std::string(buffer);
            }       
            if(r){}     
            symbols.push_back(symbol);
        }
    }
    return symbols;
}