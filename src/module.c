#include "module.h"
#include "kheap.h"
#include "paging.h"
#include "vfs.h"
#include "screen.h"
#include "elf.h"

symtab_t *ksyms = 0;
module_t *modules = 0;

//extern page_directory_t *kernel_dir;
//extern page_directory_t *current_dir;

char *get_module_name(char *path, char *name)
{
    char *s1 = basename(path);
    char *s2 = strrchr(s1, '.');
    u32int i = 0;
    for (i=0; i<s2-s1; i++) {
        name[i] = s1[i];
    }
    name[s2-s1] = 0;

    /*printk("module name %s\n", ret);*/
    return name;
}

static u32int resolve_symbol(char *sym_name)
{
    u32int i;
    for (i=0; i<ksyms->len; i++) {
        if (strcmp(ksyms->symbol[i].name, sym_name) == 0)
            return ksyms->symbol[i].value;
    }

    module_t *m = modules;
    while (m) {
        for (i=0; i<m->symtab->len; i++) {
            if (strcmp(m->symtab->symbol[i].name, sym_name) == 0)
                return m->symtab->symbol[i].value;
        }
        m = m->next;
    }
    
    return 0;
}

static void relocate_section(u32int addr, Elf32_Shdr *shdr_rel, Elf32_Shdr *shdr_sym, u8int *buf)
{
    if (!shdr_rel || !shdr_sym || !buf)
        return;

    u32int i;
    if (shdr_rel) {
        Elf32_Rel *sec_rel = (Elf32_Rel*)(buf + shdr_rel->sh_offset);
        u32int n = shdr_rel->sh_size / shdr_rel->sh_entsize;
        u32int a, s, p;

        for (i=0; i<n; i++) {
            p = addr + sec_rel->r_offset;
            a = *(u32int*)(p);
            
            Elf32_Sym *elf_sym = (Elf32_Sym*)(buf + shdr_sym->sh_offset) + ELF32_R_SYM(sec_rel->r_info);
            s = elf_sym->st_value;

            switch (ELF32_R_TYPE(sec_rel->r_info)) {
                case R_386_32:
                    *(u32int*)(p) = s+a;
                    break;
                case R_386_PC32:
                    *(u32int*)(p) = s+a-p;
                    break;
                default:
                    ;
            }

            /*printk("Reloc s: %p, a: %p, p: %p, off: %p\n", s, a, p, *(u32int*)(p));*/

            sec_rel += 1;
            
        }
    }
}

u32int verify_module_header(Elf32_Ehdr *header) 
{
    if (!header)
        return 0;

    u32int ok = 1;

    ok &= (header->e_ident[EI_MAG0] == ELFMAG0);
    ok &= (header->e_ident[EI_MAG1] == ELFMAG1);
    ok &= (header->e_ident[EI_MAG2] == ELFMAG2);
    ok &= (header->e_ident[EI_MAG3] == ELFMAG3);
    ok &= (header->e_ident[EI_CLASS] == ELFCLASS32);
    ok &= (header->e_ident[EI_DATA] == ELFDATA2LSB);
    ok &= (header->e_ident[EI_VERSION] == EV_CURRENT);


    ok &= (header->e_type == ET_REL);
    ok &= (header->e_machine == EM_386);

    return ok;
}

module_t* load_module(char *path)
{
//    scr_puts("load_module\n");
    file_t *f = file_open(path,0);

    if (!f) {
        /*
        scr_puts("load_module: lookup \"");
        scr_puts(path);
        scr_puts("\" failed.");
        */
        return 0;
    }

    u8int *buf = (u8int*)kmalloc(0x10000);
    u32int len = 0;
    len = file_read(f, buf, 0x10000);

    Elf32_Ehdr *ehdr = (Elf32_Ehdr*)buf;

    if (verify_module_header(ehdr)) {
        u32int shnum = ehdr->e_shnum;
        Elf32_Shdr *shdr;
        Elf32_Shdr *shdr_text = 0;
        Elf32_Shdr *shdr_rel_text = 0;
        Elf32_Shdr *shdr_rel_rodata = 0;
        Elf32_Shdr *shdr_rel_data = 0;
        Elf32_Shdr *shdr_sym = 0;
        Elf32_Shdr *shdr_data = 0;
        Elf32_Shdr *shdr_rodata = 0;
        Elf32_Shdr *shdr_export = 0;
        Elf32_Shdr *shdr_bss = 0;
        Elf32_Shdr *shdr_str = (Elf32_Shdr*) (buf + ehdr->e_shoff + ehdr->e_shentsize * ehdr->e_shstrndx);
        char *sec_str = (char*)buf + shdr_str->sh_offset;

        u32int i, j, k;
        for (i=1; i<shnum; i++) {
            shdr = (Elf32_Shdr*) (buf + ehdr->e_shoff + ehdr->e_shentsize*i);
            char *sh_name = sec_str + shdr->sh_name;

            if (strcmp(sh_name, ".text") == 0)
                shdr_text = shdr;
            if (strcmp(sh_name, ".rel.text") == 0)
                shdr_rel_text = shdr;
            if (strcmp(sh_name, ".rel.rodata") == 0)
                shdr_rel_rodata = shdr;
            if (strcmp(sh_name, ".rel.data") == 0)
                shdr_rel_data = shdr;
            if (strcmp(sh_name, ".data") == 0)
                shdr_data = shdr;
            if (strcmp(sh_name, ".rodata") == 0)
                shdr_rodata = shdr;
            if (strcmp(sh_name, ".bss") == 0)
                shdr_bss = shdr;
            if (strcmp(sh_name, ".rel__msymtab") == 0)
                shdr_export = shdr;
            if (shdr->sh_type == SHT_SYMTAB) 
                shdr_sym = shdr;
        }

        u32int text_addr = 0;
        u32int rodata_addr = 0;
        u32int data_addr = 0;
        u32int bss_addr = 0;

        u32int p, align;
        p = text_addr + shdr_text->sh_size;

        if (shdr_rodata) {
            align = shdr_rodata->sh_addralign;

            if ( (align == 0) || (align == 1) || !(p & (align-1)) )
                rodata_addr = p;
            else
                rodata_addr = (p & ~(align-1)) + align;

            p = rodata_addr + shdr_rodata->sh_size;
        }

        if (shdr_data) {
            align = shdr_data->sh_addralign;

            if ( (align == 0) || (align == 1) || !(p & (align-1)) )
                data_addr = p;
            else
                data_addr = (p & ~(align-1)) + align;

            p = data_addr + shdr_data->sh_size;
        }

        if (shdr_bss) {
            align = shdr_bss->sh_addralign;

            if ( (align == 0) || (align == 1) || !(p & (align-1)) )
                bss_addr = p;
            else
                bss_addr = (p & ~(align-1)) + align;

            p = bss_addr + shdr_bss->sh_size;
        }

        u32int bss_end = p;

        /*printk("text_addr %p\n",text_addr);*/
        /*printk("rodata_addr %p\n",rodata_addr);*/
        /*printk("data_addr %p\n",data_addr);*/
        /*printk("bss_addr %p\n",bss_addr);*/
        /*printk("bss_end %p\n",bss_end);*/

        u32int m_addr_end = bss_end;

        // calc m_addr_end first;
        if (shdr_sym) {
            u32int nsymbols = shdr_sym->sh_size / shdr_sym->sh_entsize;
            for (j=1; j<nsymbols; j++) {
                Elf32_Sym *elf_sym = (Elf32_Sym*)(buf + shdr_sym->sh_offset) + j;

                if (elf_sym->st_shndx == SHN_COMMON) {
                    align = elf_sym->st_value;

                    if ( (align == 0) || (align == 1) || !(m_addr_end & (align-1)) ) {
                        // aligned
                    } else {
                        m_addr_end = (m_addr_end & ~(align-1)) + align;
                    }

                    m_addr_end += elf_sym->st_size;
                }
            }
        }

        text_addr = (u32int)kmalloc_a(m_addr_end); 
        rodata_addr += text_addr;
        data_addr += text_addr;
        bss_addr += text_addr;
        bss_end += text_addr;
        m_addr_end += text_addr;

        // resolve symbols and compute the start address of relocated .text, .rodata, .data, .bss sections
        if (shdr_sym) {
            Elf32_Shdr *shdr_symstr = (Elf32_Shdr*) (buf + ehdr->e_shoff + ehdr->e_shentsize*shdr_sym->sh_link);
            char *sec_symstr = (char*)buf + shdr_symstr->sh_offset;
            u32int nsymbols = shdr_sym->sh_size / shdr_sym->sh_entsize;
            symtab_t *msyms = (symtab_t*)kmalloc(sizeof(symtab_t));
            memset(msyms, 0, sizeof(symtab_t));
            u32int symbol_all_resolved = 1;

            /*printk("shdr_sym->sh_info %d\n", shdr_sym->sh_info);*/

            for (j=1; j<nsymbols; j++) {
                Elf32_Sym *elf_sym = (Elf32_Sym*)(buf + shdr_sym->sh_offset) + j;
                char *elf_sym_name = sec_symstr + elf_sym->st_name;

                if (elf_sym->st_shndx == SHN_ABS) 
                    continue;

                if (elf_sym->st_shndx == SHN_UNDEF) {
                    u32int resolved_addr = resolve_symbol(elf_sym_name);
                    if (resolved_addr) {
                        /*printk("Resolve %s ... ok : %p\n",elf_sym_name, resolved_addr);*/
                        elf_sym->st_value = resolved_addr;
                    } else {
                        printk("Resolve %s ... failed.\n",elf_sym_name);
                        symbol_all_resolved = 0;
                        break; 
                    }
                } else if (elf_sym->st_shndx == SHN_COMMON) {
                    align = elf_sym->st_value;

                    if ( (align == 0) || (align == 1) || !(bss_end & (align-1)) )
                        elf_sym->st_value = bss_end;
                    else {
                        bss_end = (bss_end & ~(align-1)) + align;
                        elf_sym->st_value = bss_end;
                    }

                    /*printk("COMM %s allocated : %p \n", elf_sym_name, elf_sym->st_value);*/
                    bss_end += elf_sym->st_size;
                } else if (ELF32_ST_TYPE(elf_sym->st_info) == STT_SECTION) {
                    shdr = (Elf32_Shdr*) (buf + ehdr->e_shoff + ehdr->e_shentsize*elf_sym->st_shndx);

                    if (shdr == shdr_text)
                        elf_sym->st_value = text_addr;
                    else if (shdr == shdr_rodata)
                        elf_sym->st_value = rodata_addr;
                    else if (shdr == shdr_data)
                        elf_sym->st_value = data_addr;
                    else if (shdr == shdr_bss)
                        elf_sym->st_value = bss_addr;
                    else {
                    }

                    /*
                     *if (elf_sym->st_value)
                     *    printk("SECTION %s assign ..%p\n", sec_str + shdr->sh_name, elf_sym->st_value);
                     */
                } else if (ELF32_ST_BIND(elf_sym->st_info) == STB_GLOBAL) {
                    shdr = (Elf32_Shdr*) (buf + ehdr->e_shoff + ehdr->e_shentsize*elf_sym->st_shndx);

                    if (shdr == shdr_text)
                        elf_sym->st_value = text_addr + elf_sym->st_value;
                    else if (shdr == shdr_rodata)
                        elf_sym->st_value = rodata_addr + elf_sym->st_value;
                    else if (shdr == shdr_data)
                        elf_sym->st_value = data_addr + elf_sym->st_value;
                    else if (shdr == shdr_bss)
                        elf_sym->st_value = bss_addr + elf_sym->st_value;
                    else {
                    }
                }

                Elf32_Rel *sec_export_rel;
                for (k=0; k<shdr_export->sh_size/shdr_export->sh_entsize; k++) {
                    sec_export_rel = (Elf32_Rel*)(buf + shdr_export->sh_offset + k*shdr_export->sh_entsize);
                    if (j == ELF32_R_SYM(sec_export_rel->r_info)) {
                        msyms->symbol[msyms->len].name = strdup(elf_sym_name);
                        msyms->symbol[msyms->len].value =  elf_sym->st_value;
                        msyms->len++;
                        /*
                         *printk("Export %s ... %p\n", msyms->symbol[msyms->len-1].name, elf_sym->st_value);
                         */
                    }
                }

            } // for loop of all symbols

            if (symbol_all_resolved) {
                module_t *m = (module_t*)kmalloc(sizeof(module_t));
                memset(m,0,sizeof(module_t));
                get_module_name(path, m->name);
                strcpy(m->path, path);
                m->addr = text_addr;
                m->size = m_addr_end - text_addr;
                m->symtab = msyms;

                printk("load module at %p to %p\n", text_addr, bss_end);

                char init_func_name[MAX_NAME_LEN];
                char cleanup_func_name[MAX_NAME_LEN];

                sprintf(init_func_name, "module_%s_init", m->name);
                sprintf(cleanup_func_name, "module_%s_cleanup", m->name);

                for (j=0; j<msyms->len; j++) {
                    if (strcmp(msyms->symbol[j].name, init_func_name) == 0)
                        m->init = (module_init_func)msyms->symbol[j].value;

                    if (strcmp(msyms->symbol[j].name, cleanup_func_name) == 0)
                        m->cleanup = (module_cleanup_func)msyms->symbol[j].value;
                }


                if (shdr_text)
                    memcpy((void*)text_addr, (void*)buf + shdr_text->sh_offset, shdr_text->sh_size);
                if (shdr_rodata)
                    memcpy((void*)rodata_addr, (void*)buf + shdr_rodata->sh_offset, shdr_rodata->sh_size);
                if (shdr_data)
                    memcpy((void*)data_addr, (void*)buf + shdr_data->sh_offset, shdr_data->sh_size);
                if (shdr_bss)
                    memcpy((void*)bss_addr, (void*)buf + shdr_bss->sh_offset, shdr_bss->sh_size);                

                memset((void*)bss_addr, 0, bss_end-bss_addr);

                // start relocation
                relocate_section(text_addr, shdr_rel_text, shdr_sym, buf);
                relocate_section(rodata_addr, shdr_rel_rodata, shdr_sym, buf);
                relocate_section(data_addr, shdr_rel_data, shdr_sym, buf);

                m->next = modules;
                modules = m;

                if (m->init)
                    m->init();

                return m;
            } else {
                printk("Failed to resolve some symbols.\n");
            }
        }

    } else {
        printk("Bad elf32 header.\n");
    }

    kfree(buf);

    return 0;
}

void unload_module(module_t *m)
{
    if (!m)
        return;

    if (modules == m) {
        modules = modules->next;
    } else {
        module_t *p = modules;
        while (p->next && (p->next != m))
            p = p->next;
        if (p->next == m) {
            p->next = m->next;
        }
    }

    if (m->cleanup)
        m->cleanup();

    kfree(m->symtab);
    kfree(m);
}

void init_module_system(u32int num, u32int size, u32int addr, u32int strndx)
{
    if (num<=1)
        return;

    ksyms = (symtab_t*)kmalloc(sizeof(symtab_t));
    memset(ksyms, 0, sizeof(symtab_t));

    /*Elf32_Shdr *shdr_str = (Elf32_Shdr*) (addr + size * strndx);*/
    /*char *sec_str = (char*)shdr_str->sh_addr;*/

    u32int i;
    Elf32_Shdr *shdr;
    for (i=1; i<num; i++) {
        shdr = (Elf32_Shdr*) (addr + size*i);
        /*scr_puts(sec_str+shdr->sh_name);*/
        /*scr_puts("\n");*/
        if (shdr->sh_type == SHT_SYMTAB) {
            /*scr_puts("found .symtab section\n");*/
            break;
        }
    }

    u32int nsymbols = shdr->sh_size / shdr->sh_entsize;
    Elf32_Shdr *shdr_symstr = (Elf32_Shdr*) (addr + size*shdr->sh_link);
    char *sec_symstr = (char*)shdr_symstr->sh_addr;

    // shdr->sh_info is the first index of global symbol
    for (i=shdr->sh_info; i<nsymbols; i++) {
        Elf32_Sym *elf_sym = (Elf32_Sym*)shdr->sh_addr + i;

        ksyms->symbol[ksyms->len].name = sec_symstr + elf_sym->st_name;
        ksyms->symbol[ksyms->len].value = elf_sym->st_value;
        ksyms->len++;
    }

}

