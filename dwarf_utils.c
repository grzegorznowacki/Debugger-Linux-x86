//
// Created by gnowacki on 21.03.18.
//

#include "dwarf_utils.h"

void show_function_address_and_line(Dwarf_Debug dgb, Dwarf_Die the_die)
{
    char* die_name = 0;
    const char* tag_name = 0;
    Dwarf_Error err;
    Dwarf_Half tag;
    Dwarf_Attribute* attrs;
    Dwarf_Addr lowpc;
    Dwarf_Signed attrcount;
    Dwarf_Signed i;
    Dwarf_Unsigned line;

    int rc = dwarf_diename(the_die, &die_name, &err);

    if(rc == DW_DLV_ERROR)
    {
        printf("%s", "Error in dwarf_diename\n");
        exit(EXIT_FAILURE);
    }
    else if(rc == DW_DLV_NO_ENTRY)
        return;

    if (dwarf_tag(the_die, &tag, &err) != DW_DLV_OK)
    {
        printf("%s", "Error in dwarf_tag\n");
        exit(EXIT_FAILURE);
    }

    /* Only interested in subprogram DIEs here */
    if (tag != DW_TAG_subprogram)
        return;

    if (dwarf_get_TAG_name(tag, &tag_name) != DW_DLV_OK)
    {
        printf("%s", "Error in dwarf_get_TAG_name\n");
        exit(EXIT_FAILURE);
    }

    /* Print function name */
    printf("<%s>    ", die_name);

    /* Grab the DIEs attributes for display */
    if (dwarf_attrlist(the_die, &attrs, &attrcount, &err) != DW_DLV_OK)
    {
        printf("%s", "Error in dwarf_attlist\n");
        exit(EXIT_FAILURE);
    }

    for (i = 0; i < attrcount; ++i)
    {
        Dwarf_Half attrcode;
        if (dwarf_whatattr(attrs[i], &attrcode, &err) != DW_DLV_OK) {
            printf("%s", "Error in dwarf_whatattr\n");
            exit(EXIT_FAILURE);
        }

        if (attrcode == DW_AT_low_pc)
        {
            dwarf_formaddr(attrs[i], &lowpc, 0);
        }

        if (attrcode == DW_AT_decl_line)
        {
            dwarf_formudata(attrs[i], &line, 0);
        }
    }

    printf("%d    ", line);
    printf("0x%08llx\n", lowpc);
}

void list_functions_with_address(Dwarf_Debug dbg)
{
    Dwarf_Unsigned cu_header_length;
    Dwarf_Unsigned abbrev_offset;
    Dwarf_Unsigned next_cu_header;
    Dwarf_Half version_stamp;
    Dwarf_Half address_size;
    Dwarf_Error err;
    Dwarf_Die no_die = 0;
    Dwarf_Die cu_die;
    Dwarf_Die child_die;

    /* Find next compilation unit header */
    if (dwarf_next_cu_header(dbg, &cu_header_length, &version_stamp, &abbrev_offset, &address_size, &next_cu_header, &err) == DW_DLV_ERROR)
    {
        printf("%s", "Error reading DWARF cu header\n");
        exit(EXIT_FAILURE);
    }

    /* Expect the CU to have a single sibling - a DIE */
    if (dwarf_siblingof(dbg, no_die, &cu_die, &err) == DW_DLV_ERROR)
    {
        printf("%s", "Error getting sibling of CU\n");
        exit(EXIT_FAILURE);
    }

    /* Expect the CU DIE to have children */
    if (dwarf_child(cu_die, &child_die, &err) == DW_DLV_ERROR)
    {
        printf("%s", "Error getting child of CU DIE\n");
        exit(EXIT_FAILURE);
    }

    /* Go over all children DIEs */
    while(1)
    {
        int ret;

        show_function_address_and_line(dbg, child_die);

        ret = dwarf_siblingof(dbg, child_die, &child_die, &err);

        if(ret == DW_DLV_ERROR)
        {
            printf("%s", "Error getting sibling of DIE\n");
            exit(EXIT_FAILURE);
        }
        else if(ret == DW_DLV_NO_ENTRY)
        {
            break;
        }
    }
}

Dwarf_Addr find_function_address(Dwarf_Debug dgb, Dwarf_Die the_die, char* function_name)
{
    char* die_name = 0;
    const char* tag_name = 0;
    Dwarf_Error err;
    Dwarf_Half tag;
    Dwarf_Attribute* attrs;
    Dwarf_Addr lowpc;
    Dwarf_Signed attrcount;
    Dwarf_Signed i;
    Dwarf_Unsigned line;

    int rc = dwarf_diename(the_die, &die_name, &err);

    if(rc == DW_DLV_ERROR)
    {
        printf("%s", "Error in dwarf_diename\n");
        exit(EXIT_FAILURE);
    }
    else if(rc == DW_DLV_NO_ENTRY)
        return 0;

    if(strcmp(die_name, function_name) != 0)
        return 0;

    if (dwarf_tag(the_die, &tag, &err) != DW_DLV_OK)
    {
        printf("%s", "Error in dwarf_tag\n");
        exit(EXIT_FAILURE);
    }

    /* Only interested in subprogram DIEs here */
    if (tag != DW_TAG_subprogram)
        return 0;

    if (dwarf_get_TAG_name(tag, &tag_name) != DW_DLV_OK)
    {
        printf("%s", "Error in dwarf_get_TAG_name\n");
        exit(EXIT_FAILURE);
    }

    /* Grab the DIEs attributes for display */
    if (dwarf_attrlist(the_die, &attrs, &attrcount, &err) != DW_DLV_OK)
    {
        printf("%s", "Error in dwarf_attlist\n");
        exit(EXIT_FAILURE);
    }

    for (i = 0; i < attrcount; ++i)
    {
        Dwarf_Half attrcode;
        if (dwarf_whatattr(attrs[i], &attrcode, &err) != DW_DLV_OK) {
            printf("%s", "Error in dwarf_whatattr\n");
            exit(EXIT_FAILURE);
        }

        if (attrcode == DW_AT_low_pc)
        {
            dwarf_formaddr(attrs[i], &lowpc, 0);
            return lowpc;
        }
    }
    return 0;
}

void break_at_function(Dwarf_Debug dbg, pid_t child_pid, const char* command_name, breakpoint_struct** breakpoint_array, int* insert_elem)
{
    char function_name[FUNCTION_NAME_LEN];
    strncpy(function_name, command_name + 15, FUNCTION_NAME_LEN);
    char* new_line_ptr = strchr(function_name, '\n');
    *new_line_ptr = '\0';

    Dwarf_Unsigned cu_header_length;
    Dwarf_Unsigned abbrev_offset;
    Dwarf_Unsigned next_cu_header;
    Dwarf_Half version_stamp;
    Dwarf_Half address_size;
    Dwarf_Error err;
    Dwarf_Die no_die = 0;
    Dwarf_Die cu_die;
    Dwarf_Die child_die;

    /* Find next compilation unit header */
    if (dwarf_next_cu_header(dbg, &cu_header_length, &version_stamp, &abbrev_offset, &address_size, &next_cu_header, &err) == DW_DLV_ERROR)
    {
        printf("%s", "Error reading DWARF cu header\n");
        exit(EXIT_FAILURE);
    }

    /* Expect the CU to have a single sibling - a DIE */
    if (dwarf_siblingof(dbg, no_die, &cu_die, &err) == DW_DLV_ERROR)
    {
        printf("%s", "Error getting sibling of CU\n");
        exit(EXIT_FAILURE);
    }

    /* Expect the CU DIE to have children */
    if (dwarf_child(cu_die, &child_die, &err) == DW_DLV_ERROR)
    {
        printf("%s", "Error getting child of CU DIE\n");
        exit(EXIT_FAILURE);
    }

    Dwarf_Addr function_address = 0;     //typedef unsigned long

    /* Go over all children DIEs */
    while(1)
    {
        int ret;

        function_address = find_function_address(dbg, child_die, function_name);

        if(function_address != 0)
            break;

        ret = dwarf_siblingof(dbg, child_die, &child_die, &err);

        if(ret == DW_DLV_ERROR)
        {
            printf("%s", "Error getting sibling of DIE\n");
            exit(EXIT_FAILURE);
        }
        else if(ret == DW_DLV_NO_ENTRY)
        {
            break;
        }
    }


    breakpoint_struct* breakpoint = create_breakpoint(child_pid, (void*)function_address);
    breakpoint_array[*insert_elem] = breakpoint;
    (*insert_elem)++;
}

void line_address_mapping(Dwarf_Debug dbg)
{
    Dwarf_Unsigned cu_header_length;
    Dwarf_Unsigned abbrev_offset;
    Dwarf_Unsigned next_cu_header;
    Dwarf_Half version_stamp;
    Dwarf_Half address_size;
    Dwarf_Error err;
    Dwarf_Die no_die = 0;
    Dwarf_Die cu_die;
    Dwarf_Die child_die;

    int n;
    Dwarf_Line *lines;
    Dwarf_Signed nlines;
    char *filename;
    Dwarf_Addr lineaddr;
    Dwarf_Unsigned lineno;

    /* Find next compilation unit header */
    if (dwarf_next_cu_header(dbg, &cu_header_length, &version_stamp, &abbrev_offset, &address_size, &next_cu_header, &err) == DW_DLV_ERROR)
    {
        printf("%s", "Error reading DWARF cu header\n");
        exit(EXIT_FAILURE);
    }

    /* Expect the CU to have a single sibling - a DIE */
    if (dwarf_siblingof(dbg, no_die, &cu_die, &err) == DW_DLV_ERROR)
    {
        printf("%s", "Error getting sibling of CU\n");
        exit(EXIT_FAILURE);
    }

    if (dwarf_srclines(cu_die, &lines, &nlines, &err) != DW_DLV_OK)
    {
        printf("%s", "Error dwarf_srclines");
        exit(EXIT_FAILURE);
    }

    for (n = 0; n < nlines; n++) {
        /* Retrieve the file name for this descriptor. */
        if (dwarf_linesrc(lines[n], &filename, &err))
        {
            printf("%s", "Error dwarf_linesrc");
            exit(EXIT_FAILURE);
        }

        /* Retrieve the line number in the source file. */
        if (dwarf_lineno(lines[n], &lineno, &err))
        {
            printf("%s", "Error dwarf_lineno");
            exit(EXIT_FAILURE);
        }

        /* Retrieve the virtual address for this line. */
        if (dwarf_lineaddr(lines[n], &lineaddr, &err))
        {
            printf("%s", "Error dwarf_lineaddr");
            exit(EXIT_FAILURE);
        }

        printf("%s    %d    0x%08llx\n", filename, lineno, lineaddr);
    }
}

void break_at_line(Dwarf_Debug dbg, pid_t child_pid, const char* command_name, breakpoint_struct** breakpoint_array, int* insert_elem)
{
    char function_line_array[FUNCTION_LINE_LEN];
    strncpy(function_line_array, command_name + 11, FUNCTION_LINE_LEN);
    char* new_line_ptr = strchr(function_line_array, '\n');
    *new_line_ptr = '\0';
    unsigned long line_num = (unsigned long)strtol(function_line_array, NULL, 10);

    Dwarf_Unsigned cu_header_length;
    Dwarf_Unsigned abbrev_offset;
    Dwarf_Unsigned next_cu_header;
    Dwarf_Half version_stamp;
    Dwarf_Half address_size;
    Dwarf_Error err;
    Dwarf_Die no_die = 0;
    Dwarf_Die cu_die;
    Dwarf_Die child_die;

    int n;
    Dwarf_Line *lines;
    Dwarf_Signed nlines;
    char *filename;
    Dwarf_Addr lineaddr;
    Dwarf_Unsigned lineno;

    /* Find next compilation unit header */
    if (dwarf_next_cu_header(dbg, &cu_header_length, &version_stamp, &abbrev_offset, &address_size, &next_cu_header, &err) == DW_DLV_ERROR)
    {
        printf("%s", "Error reading DWARF cu header\n");
        exit(EXIT_FAILURE);
    }

    /* Expect the CU to have a single sibling - a DIE */
    if (dwarf_siblingof(dbg, no_die, &cu_die, &err) == DW_DLV_ERROR)
    {
        printf("%s", "Error getting sibling of CU\n");
        exit(EXIT_FAILURE);
    }

    if (dwarf_srclines(cu_die, &lines, &nlines, &err) != DW_DLV_OK)
    {
        printf("%s", "Error dwarf_srclines");
        exit(EXIT_FAILURE);
    }

    for (n = 0; n < nlines; n++) {
        /* Retrieve the file name for this descriptor. */
        if (dwarf_linesrc(lines[n], &filename, &err))
        {
            printf("%s", "Error dwarf_linesrc");
            exit(EXIT_FAILURE);
        }

        /* Retrieve the line number in the source file. */
        if (dwarf_lineno(lines[n], &lineno, &err))
        {
            printf("%s", "Error dwarf_lineno");
            exit(EXIT_FAILURE);
        }

        /* Retrieve the virtual address for this line. */
        if (dwarf_lineaddr(lines[n], &lineaddr, &err))
        {
            printf("%s", "Error dwarf_lineaddr");
            exit(EXIT_FAILURE);
        }

        if(lineno == line_num)
            break;
    }

    break_at_address_dwarf(child_pid, lineaddr, breakpoint_array, insert_elem);
}
