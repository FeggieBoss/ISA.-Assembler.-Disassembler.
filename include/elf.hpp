#ifndef H_ELF
#define H_ELF

#define EI_NIDENT	                16
#define EI_CLASS                  4
#define ELFCLASS32                1      

#define SHN_UNDEF	                0
#define SHN_LORESERVE	            0xff00
#define SHN_LOPROC	              0xff00
#define SHN_HIPROC	              0xff1f
#define SHN_LIVEPATCH             0xff20
#define SHN_ABS		                0xfff1
#define SHN_COMMON	              0xfff2
#define SHN_HIRESERVE             0xffff
#define SHN_XINDEX                0xffff

#define ELF_ST_BIND(x)		        ((x) >> 4)
#define ELF_ST_TYPE(x)		        (((unsigned int) x) & 0xf)
#define ELF32_ST_BIND(x)	        ELF_ST_BIND(x)
#define ELF32_ST_TYPE(x)          ELF_ST_TYPE(x)
#define ELF64_ST_BIND(x)	        ELF_ST_BIND(x)
#define ELF64_ST_TYPE(x)	        ELF_ST_TYPE(x)

#define ELF32_ST_VISIBILITY(o)    ((o)&0x3)

#include <iostream>

typedef uint32_t	Elf32_Addr;
typedef uint16_t	Elf32_Half;
typedef uint32_t	Elf32_Off;
typedef int32_t	  Elf32_Sword;
typedef uint32_t	Elf32_Word;

typedef struct elf32_hdr{
  unsigned char	e_ident[EI_NIDENT];
  Elf32_Half	e_type;
  Elf32_Half	e_machine;
  Elf32_Word	e_version;
  Elf32_Addr	e_entry;
  Elf32_Off	  e_phoff;
  Elf32_Off	  e_shoff;
  Elf32_Word	e_flags;
  Elf32_Half	e_ehsize;
  Elf32_Half	e_phentsize;
  Elf32_Half	e_phnum;
  Elf32_Half	e_shentsize;
  Elf32_Half	e_shnum;
  Elf32_Half	e_shstrndx;
} Elf32_Ehdr;

typedef struct elf32_shdr {
  Elf32_Word	sh_name;
  Elf32_Word	sh_type;
  Elf32_Word	sh_flags;
  Elf32_Addr	sh_addr;
  Elf32_Off	  sh_offset;
  Elf32_Word	sh_size;
  Elf32_Word	sh_link;
  Elf32_Word	sh_info;
  Elf32_Word	sh_addralign;
  Elf32_Word	sh_entsize;
} Elf32_Shdr;

typedef struct elf32_sym{
  Elf32_Word	st_name;
  Elf32_Addr	st_value;
  Elf32_Word	st_size;
  unsigned char	st_info;
  unsigned char	st_other;
  Elf32_Half	st_shndx;
} Elf32_Sym;

#endif // H_ELF