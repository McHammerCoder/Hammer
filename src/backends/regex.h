// Internal defs
#ifndef HAMMER_BACKEND_REGEX__H
#define HAMMER_BACKEND_REGEX__H

// each insn is an 8-bit opcode and a 16-bit parameter
// [a] are actions; they add an instruction to the stackvm that is being output.
// [m] are match ops; they can either succeed or fail, depending on the current character
// [c] are control ops. They affect the pc non-linearly.
typedef enum HRVMOp_ {
  RVM_ACCEPT,  // [a]
  RVM_GOTO,    // [c] parameter is an offset into the instruction table
  RVM_FORK,    // [c] parameter is an offset into the instruction table
  RVM_PUSH,    // [a] No arguments, just pushes a mark onto the stack
  RVM_ACTION,  // [a] argument is an action ID
  RVM_CAPTURE, // [a] Capture the last string, and push it on the stack. No arg.
  RVM_EOF,     // [m] Succeeds only if at EOF.
  RVM_MATCH,   // [m] The high byte of the parameter is an upper bound
	       //     and the low byte is a lower bound, both
	       //     inclusive. An inverted match should be handled
	       //     as two ranges.
  RVM_STEP,    // [a] Step to the next byte of input
  RVM_OPCOUNT
} HRVMOp;

typedef struct HRVMInsn_{
  uint8_t op;
  uint16_t arg;
} HRVMInsn;

#define TT_MARK TT_RESERVED_1

typedef struct HSVMContext_ {
  HParsedToken **stack;
  size_t stack_count;
  size_t stack_capacity;
} HSVMContext;

// These actions all assume that the items on the stack are not
// aliased anywhere.
typedef struct HSVMAction_ {
  bool (*fn)(HArena *arena, HSVMContext *ctx, void* env);
  void* env;
} HSVMAction;

typedef struct HRVMProg_ {
  size_t length;
  size_t action_count;
  HRVMInsn *insns;
  HSVMAction *actions;
} HRVMProg;

// Returns true IFF the provided parser could be compiled.
bool h_compile_regex(HRVMProg *prog, const HParser* parser);

// These functions are used by the compile_to_rvm method of HParser
uint16_t h_rvm_create_action(HRVMProg *prog, HSVMAction *action);

// returns the address of the instruction just created
uint16_t h_rvm_insert_insn(HRVMProg *prog, HRVMOp op, uint16_t arg);

// returns the address of the next insn to be created.
uint16_t h_rvm_get_ip(HRVMProg *prog);

// Used to insert forward references; the idea is to generate a JUMP
// or FORK instruction with a target of 0, then update it once the
// correct target is known.
void h_rvm_patch_arg(HRVMProg *prog, uint16_t ip, uint16_t new_val);

#endif
