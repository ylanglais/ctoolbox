#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <libelf.h>
#include <fcntl.h>

typedef struct {
    int     	 fd;
    Elf    		*elf;
    Elf32_Ehdr  *ehdr;
    Elf_Scn 	*scn0;
    Elf_Data 	*edat;
    char   		*strings;
    int     	 nsheaders;
    Elf32_Shdr **allsheaders;
} elf_t, *pelf_t;

void
elf_error() {
    fprintf(stderr, "%s\n", elf_errmsg(elf_errno()));
}

pelf_t
elf_destroy(pelf_t e) {
    if (!e)
        return NULL;
    if (e->fd > 0)
        close(e->fd);
    if (e->allsheaders)
        free(e->allsheaders);
    free(e);
    return NULL;
}

pelf_t
elf_new(char *filename) {
    int     i;
    pelf_t  e;
    Elf32_Shdr *p;

    elf_version(EV_CURRENT);

    if (!filename) {
        fprintf(stderr, "bad filename\n");
        return NULL;
    }

    if (!(e = (pelf_t) malloc(sizeof(elf_t))))
        return NULL;

    if ((e->fd = open(filename, O_RDONLY)) < 0) {
        fprintf(stderr, "cannot open %s\n", filename);
        return elf_destroy(e);
    }

    /* interpret file with elf format */
    if (!(e->elf = elf_begin(e->fd, ELF_C_READ, NULL))) {
        elf_error();
        return elf_destroy(e);
    }

    /* read ELF header: */
    if (!(e->ehdr = elf32_getehdr(e->elf))) {
        elf_error();
        return elf_destroy(e);
    }

    /* read ELF 1st section: */
    if (!(e->scn0 = elf_getscn(e->elf, e->ehdr->e_shstrndx))) {
        elf_error();
        return elf_destroy(e);
    }

    /* read ELF data: */
    if (!(e->edat = elf_getdata(e->scn0, NULL))) {
        elf_error();
        return elf_destroy(e);
    }

    if (!(e->allsheaders = (Elf32_Shdr **) malloc(sizeof(Elf32_Shdr *) * (e->ehdr->e_shnum + 1)))) {
        fprintf(stderr, "cannot allocate memory for section headers pointers\n");
        return elf_destroy(e);
    }

    p = (Elf32_Shdr *) ((char *) e->edat->d_buf + e->ehdr->e_shoff);
    for (i = 0; i < e->ehdr->e_shnum; i++) {
        e->allsheaders[i] = p;
        p += e->ehdr->e_shentsize;
    }
    e->nsheaders = e->ehdr->e_shnum;

    return e;
}


/* Given Elf header, Elf_Scn, and Elf32_Shdr 
 * print out the symbol table 
 */
void
print_symbols(Elf * elf, Elf_Scn * scn, Elf32_Shdr * shdr) {
    Elf32_Sym *esym, *lastsym;

    Elf_Data *data = NULL;
    char   *name;
    int     number = 0;
    char   *tt[] = {
        "NOTYPE",
        "OBJECT",
        "FUNC",
        "SECTION",
        "FILE",
        "COMMON",
        "TLS",
        "NUM"
    };
    if ((data = elf_getdata(scn, data)) == 0 || data->d_size == 0) {
        /* error or no data */
        fprintf(stderr, "Section had no data!\n");
        return;
    }
    /*now print the symbols */
    esym = (Elf32_Sym *) data->d_buf;
    lastsym = (Elf32_Sym *) ((char *) data->d_buf + data->d_size);
    /* now loop through the symbol table and print it */
    for (; esym < lastsym; esym++) {
        if ((esym->st_value == 0) ||
            (ELF32_ST_BIND(esym->st_info) == STB_WEAK) ||
            (ELF32_ST_BIND(esym->st_info) == STB_NUM) ||
            (ELF32_ST_TYPE(esym->st_info) != STT_FUNC))
            continue;
        name = elf_strptr(elf, shdr->sh_link, (size_t) esym->st_name);
        if (!name) {
            fprintf(stderr, "%s\n", elf_errmsg(elf_errno()));
            exit(-1);
        }
		/* printf("\t%d: %s\n", number++, name); */
		printf("\t%4d: type = %-8s, size = %4d, name = %s\n",
					   number++,
                       tt[ELF32_ST_TYPE(esym->st_info)], 
						esym->st_size, name);
    }
}

int
main(int nargs, char **args) {
    int         secnum;
    elf_t      *e;
    Elf_Scn    *section;
    Elf32_Shdr *sheader;


    if (nargs < 2) {
        fprintf(stderr, "usage:\n%s file\n", args[0]);
        return 1;
    }

    if (!(e = elf_new(args[1])))
        return 2;

    section = NULL;
    secnum = 0;
    while ((section = elf_nextscn(e->elf, section))) {
        if (!(sheader = elf32_getshdr(section)))
            continue;
        printf("section %4d: %s\n", secnum, (char *) e->edat->d_buf + sheader->sh_name);


        if (sheader->sh_type == SHT_SYMTAB || sheader->sh_type == SHT_DYNSYM) {

            print_symbols(e->elf, section, sheader);

#if 0
            /* get string section: */
            sym = (Elf32_Sym *) ((char *) e->edat->d_buf) + sheader->sh_offset;
            endsym =
                (Elf32_Sym *) ((char *) e->edat->d_buf) + sheader->sh_offset +
                sheader->sh_size;

            for (; sym < endsym; sym++) {
                printf("type = %-8si, size = %d, name = %s\n",
                       tt[ELF32_ST_TYPE(sym->st_info)], sym->st_size,
                       strings + sym->st_name);
            }
#endif
        }
		secnum++;
    }
    e = elf_destroy(e);
    return 0;
}




    /* loop on sections: */
