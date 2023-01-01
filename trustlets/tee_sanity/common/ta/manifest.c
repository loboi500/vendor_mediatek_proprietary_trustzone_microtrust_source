/* TA Configuration file */
#include <manifest.h>

// clang-format off

TA_CONFIG_BEGIN

uuid :  { 0x05150000, 0x0000, 0x0000, { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 } },
log_tag : "ta_sanity",
//ipc_buf_size: 0x10000,
ipc_buf_size: 0x401000, // MAX: 1025 * 4096
rpmb_size : 1024,

TA_CONFIG_END

/* Add stack size from 32KB to 1MB (0x100000) for testing */
typedef unsigned int ta_umword_t;
enum
{
   TA_ELF_AUX_T_NONE = 0,
   TA_ELF_AUX_T_VMA,
   TA_ELF_AUX_T_STACK_SIZE,
   TA_ELF_AUX_T_STACK_ADDR,
   TA_ELF_AUX_T_KIP_ADDR,
};
typedef struct ta_elf_aux_mword_t
{
   ta_umword_t type;
   ta_umword_t length;
   ta_umword_t value;
 } ta_elf_aux_mword_t;
#define TA_ELF_AUX_ELEM_T(type, id, tag, val...) \
   static const __attribute__((used, section(".rol4re_elf_aux"))) \
   type id = {tag, sizeof(type), val}
TA_ELF_AUX_ELEM_T(ta_elf_aux_mword_t, decl_name, TA_ELF_AUX_T_STACK_SIZE, 0x100000, 0x0);
