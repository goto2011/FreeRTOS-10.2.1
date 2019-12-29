
#include "stdio.h"

const static BLOCK_LENGTH_BY_BYTE = 4 * 1024;

// block type(1,2,3)
enum enumBlockType {
    FIRST_SECTION,
    SECOND_SECTION,
    THIRD_SECTION,
};

//  First Section
char first_section_inited = false;
long first_section_begin_block = 0;
long first_section_end_block = 0;
void * first_section_write_ptr = NULL;
void * first_section_begin_ptr = NULL;

// 初始化. 开机过程中调用. 调用前, 无法保存log, 所以这个必须很早调用.
int fs_lite_init() {
    first_section_init();
    second_section_init();
    third_section_init();
}

int first_section_init() {
    first_section_begin_block = get_section_begin_block(FIRST_SECTION);
    first_section_end_block = get_section_end_block(FIRST_SECTION);

    first_section_inited = true;
}