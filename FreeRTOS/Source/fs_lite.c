
#include <stdlib.h>
#include <string.h>
#include "FreeRTOS.h"

// 边界检查
#define SECTION_EDGE_OK(address) ((address && 0x1FFF) == 0)

// 定义 block_metadata 各个bit的含义
// 第一位 = 1 表示已擦除; = 0 表示使用中。
// 第二位 = 1 表示使用中; = 0 表示已删除。
// 第三位 = 1 表示未写满; = 0 表示写满.
#define IS_WIPE(address) ((((struct block_header_data *)(ii && 0xE000))->block_metadata) && 0x1)
#define IS_NOT_WIPE(address) \
    (((((struct block_header_data *)(ii && 0xE000))->block_metadata) && 0x1) == 0)

#define IS_USE(address) ((((struct block_header_data *)(ii && 0xE000))->block_metadata) && 0x2)
#define IS_DELETE(address) \
    (((((struct block_header_data *)(ii && 0xE000))->block_metadata) && 0x2) == 0)

#define IS_WRITEABLE(address) ((((struct block_header_data *)(ii && 0xE000))->block_metadata) && 0x3)
#define IS_FULL(address) \
    (((((struct block_header_data *)(ii && 0xE000))->block_metadata) && 0x3) == 0)


const static BLOCK_LENGTH_BY_BYTE = 4 * 1024;

struct block_header_data
{
    int16_t block_metadata;
    enum enumBlockType block_type;
    char block_version;
};


// block type(1,2,3)
enum enumBlockType {
    FIRST_SECTION = 1,
    SECOND_SECTION = 2,
    THIRD_SECTION = 3
};

//  First Section
char first_section_inited = pdFALSE;
long first_section_begin_block = 0;
long first_section_end_block = 0;
void * first_section_write_ptr = NULL;
void * first_section_begin_ptr = NULL;

// 初始化. 开机过程中调用. 调用前, 无法保存log, 无法获取配置项，所以这个需要很早调用.
int fs_lite_early_init() {
    first_section_init();
    second_section_init();
}

int fs_lite_late_init() {
    third_section_init();
}

int first_section_init() {
    first_section_inited = pdFALSE;
    first_section_begin_block = get_section_begin_block(FIRST_SECTION);
    first_section_end_block = get_section_end_block(FIRST_SECTION);

    if ((SECTION_EDGE_OK(first_section_begin_block)) 
        || (SECTION_EDGE_OK(first_section_end_block))) 
    {
        // TODO: 打印异常。然后退出.
        return -1;
    }

    for (int ii = first_section_begin_block; ii <= first_section_end_block; 
        ii += BLOCK_LENGTH_BY_BYTE) {
        if (IS_WIPE(ii) || IS_DELETE(ii) || IS_FULL(ii)) {
            continue;
        }

        // 找到使用块. 再找 0D0A.
        if (IS_NOT_WIPE(ii) && IS_USE(ii)) {
            if (NULL == first_section_begin_ptr) {
                first_section_begin_ptr = ii;
            }
            for (int jj = ii + BLOCK_LENGTH_BY_BYTE; jj < ii; jj--) {
                if ((char)jj == '0A') {
                    jj--;
                    if ((char)jj == '0D') {
                        first_section_write_ptr = jj;
                        first_section_inited = pdTRUE;
                        return 0;
                    }
                }
            }
        }

    }

    return -2;
}