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
    int open_fl = open(file_name.c_str(), O_RDONLY);
    struct stat stata_;
    fstat(open_fl, &stata_);
    iterator_ = static_cast<uint8_t*>(mmap(NULL, stata_.st_size, PROT_READ, MAP_PRIVATE, open_fl, 0));
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

std::vector<section_t> Elf_parser::get_sections() {
    Elf32_Ehdr *ehdr = (Elf32_Ehdr*)iterator_;
    Elf32_Shdr *shdr = (Elf32_Shdr*)(iterator_ + ehdr->e_shoff);
    Elf32_Shdr *sh_strtab = &shdr[ehdr->e_shstrndx];

    std::vector<section_t> sections;
    for (size_t i = 0; i < ehdr->e_shnum; ++i) {
        section_t section;
        section.section_number = i;
        section.section_name = std::string(((char*)iterator_ + sh_strtab->sh_offset) + shdr[i].sh_name);
        section.section_type = get_section_type(shdr[i].sh_type);
        section.section_address = shdr[i].sh_addr;
        section.section_offset = shdr[i].sh_offset;
        section.section_size = shdr[i].sh_size;
        section.section_esize = shdr[i].sh_entsize;
        section.section_add_al = shdr[i].sh_addralign; 
        sections.push_back(section);
    }
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
    char *strtab_pointer = nullptr;
    char *dynstr_pointer = nullptr;

    for(auto &el: all_sections) {
        if(el.section_type == ".strtab" && 
           el.section_name == ".dynstr") {
            dynstr_pointer = (char*)iterator_ + el.section_offset;
            break;
        }
    }
    for(auto &el: all_sections) {
        if(el.section_type == ".strtab" && 
           el.section_name == ".strtab") {
            strtab_pointer = (char*)iterator_ + el.section_offset;
            break;
        }
    }

    std::vector<symbol_t> symbols;
    for(auto &el : all_sections) {
        if(el.section_type != ".symtab" && 
           el.section_type != ".dynsym")
            continue;

        auto info = (Elf32_Sym*)(iterator_ + el.section_offset);
        for (size_t i = 0; i < el.section_size / sizeof(Elf32_Sym); ++i) {
            symbol_t symbol = symbol_t(
                i, info[i].st_value, info[i].st_size,
                get_symbol_type(info[i].st_info),
                get_symbol_bind(info[i].st_info),
                get_symbol_visibility(info[i].st_other),
                get_symbol_index(info[i].st_shndx),
                ""
            );
            if(el.section_type == ".symtab") symbol.symbol_name = std::string(strtab_pointer + info[i].st_name);
            if(el.section_type == ".dynsym") symbol.symbol_name = std::string(dynstr_pointer + info[i].st_name);
            
            symbols.push_back(symbol);
        }
    }
    return symbols;
}