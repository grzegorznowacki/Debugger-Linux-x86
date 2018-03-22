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
    Dwarf_Line line;

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
            dwarf_formaddr(attrs[i], &lowpc, 0);
        else if (attrcode == DW_AT_decl_line)
        {
            //dwarf_form
        }
    }

    printf("0x%08llx\n", lowpc);
}

void list_functions_with_address(Dwarf_Debug dbg, pid_t child_pid, int *wait_status, const char *command_name, breakpoint_struct **breakpoint_array, int *insert_elem)
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

void break_at_function(Dwarf_Debug dbg, pid_t child_pid, int* wait_status, const char* command_name, breakpoint_struct** breakpoint_array, int* insert_elem)
{
    //TODO wczytywanie adresu

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


    //todo to chyba do find_fuction_address przeniesc
    unsigned int address;   //TODO - TO BEDZIE ADRES POZYSKANY Z DWARFA
    breakpoint_struct* breakpoint = create_breakpoint(child_pid, (void*)address);
    breakpoint_array[*insert_elem] = breakpoint;
    (*insert_elem)++;
}