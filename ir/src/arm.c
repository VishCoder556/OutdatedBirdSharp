

char *arm64_regs64[REGISTERS] = {
    "x0",  "x19", "x3",  "x2",  "x1",  "x0",  "x4",  "x5",
    "x8",  "x9",  "x10", "x11", "x12", "x13", "x14", "x15",
    "x18", "x20", "x21", "x22", "x23", "x24", "x25", "x26",
    "x27", "x28", "x19", "x20", "x29", "x22", "x23", "x24"
};

char *arm64_regs32[REGISTERS] = {
    "w0",  "w19", "w3",  "w2",  "w1",  "w0",  "w4",  "w5",
    "w8",  "w9",  "w10", "w11", "w12", "w13", "w14", "w15",
    "w17", "w20", "w21", "w22", "w23", "w24", "w25", "w26",
    "w27", "w28", "w19", "w20", "w29", "w22", "w23", "w24"
};
char *arm64_regs16[REGISTERS] = {
    "w0",  "w19", "w3",  "w2",  "w1",  "w0",  "w4",  "w5",
    "w8",  "w9",  "w10", "w11", "w12", "w13", "w14", "w15",
    "w17", "w20", "w21", "w22", "w23", "w24", "w25", "w26",
    "w27", "w28", "w19", "w20", "w29", "w22", "w23", "w24"
};
char *arm64_regs8[REGISTERS] = {
    "w0",  "w19", "w3",  "w2",  "w1",  "w0",  "w4",  "w5",
    "w8",  "w9",  "w10", "w11", "w12", "w13", "w14", "w15",
    "w17", "w20", "w21", "w22", "w23", "w24", "w25", "w26",
    "w27", "w28", "w19", "w20", "w29", "w22", "w23", "w24"
};

char *cur_function = "";
int arm64_is_real_reg(char *reg){
    for (int i=0; i<REGISTERS; i++){
        if (strcmp(reg, arm64_regs64[i]) == 0) {
            return 0;
        };
    }
    for (int i=0; i<REGISTERS; i++){
        if (strcmp(reg, arm64_regs32[i]) == 0) {
            return 0;
        };
    }
    for (int i=0; i<REGISTERS; i++){
        if (strcmp(reg, arm64_regs16[i]) == 0) {
            return 0;
        };
    }
    for (int i=0; i<REGISTERS; i++){
        if (strcmp(reg, arm64_regs8[i]) == 0) {
            return 0;
        };
    }
    return -1;
}
char *arm64_change_reg_size(int reg, int size){
    if (reg > REGISTERS || reg < 0){
        reg = 0;
    }
    switch (size){
        case 1: return arm64_regs8[reg];
        case 2: return arm64_regs16[reg];
        case 4: return arm64_regs32[reg];
        case 8: return arm64_regs64[reg];
    };
    return arm64_regs64[reg];
}
char *arm64_reg_to_name(AST_Reg reg){
    switch (reg.size){
        case 1: return arm64_regs8[reg.reg];
        case 2: return arm64_regs16[reg.reg];
        case 4: return arm64_regs32[reg.reg];
        case 8: return arm64_regs64[reg.reg];
        case 0: return arm64_regs64[reg.reg];
    };
    char string[100];
    snprintf(string, 100, "Register doesn't exist");
    error_generate("InvalidRegisterError", string);
    return "";
};
AST_Reg arm64_reg_real_to_num(char *reg){
    AST_Reg reg1 = (AST_Reg){0};
    reg1.reg = -1;
    for (int i=0; i<REGISTERS; i++){
        if (strcmp(arm64_regs8[i], reg) == 0){
            reg1.reg = i;
            reg1.size = 1;
        };
    };
    for (int i=0; i<REGISTERS; i++){
        if (strcmp(arm64_regs16[i], reg) == 0){
            reg1.reg = i;
            reg1.size = 2;
        };
    };
    for (int i=0; i<REGISTERS; i++){
        if (strcmp(arm64_regs32[i], reg) == 0){
            reg1.reg = i;
            reg1.size = 4;
        };
    };
    for (int i=0; i<REGISTERS; i++){
        if (strcmp(arm64_regs64[i], reg) == 0){
            reg1.reg = i;
            reg1.size = 8;
        };
    };
    return reg1;
}
Compiler *arm64_init(Reviser *reviser, char *file){
    Compiler *compiler = malloc(sizeof(Compiler));
    FILE *f;
    compiler->_asm = find_available_tmp_file("asm");
    f = fopen(compiler->_asm, "w"); 
    if (!f) {
        perror("fopen");
        exit(1);
    }
    compiler->f = f;
    compiler->data = malloc(5000);
    compiler->name = file;
    compiler->asts = reviser->asts;
    compiler->astlen = reviser->astlen;
    compiler->cur = 0;
    compiler->templabel = 0;
    compiler->global = reviser->global;
    fprintf(compiler->f, ".text\n.globl main\n");
    compiler_write_text_line(compiler, "_and:");
    compiler_write_text_line(compiler, "cmp     w0, 0");
    compiler_write_text_line(compiler, "ccmp    w1, 0, 4, ne");
    compiler_write_text_line(compiler, "cset    w0, ne");
    compiler_write_text_line(compiler, "ret");
    compiler_write_text_line(compiler, "_or:");
    compiler_write_text_line(compiler, "orr     w0, w0, w1");
    compiler_write_text_line(compiler, "cmp     w0, 0");
    compiler_write_text_line(compiler, "cset    w0, ne");
    compiler_write_text_line(compiler, "ret");
    return compiler;
}



void arm64_close(Compiler *compiler){
    if (target == TARGET_ARM64_MAC){
        fprintf(compiler->f, ".section __DATA,__data\n%s\n", compiler->data);
    }else if (target == TARGET_ARM64_LINUX){
        fprintf(compiler->f, ".data\n%s\n", compiler->data);
    }
    fclose(compiler->f);
    free(compiler->data);
    char string[1024];
    compiler->_o = find_available_tmp_file("o");


    snprintf(string, 1024, "cat %s", compiler->_asm);
    system(string);

    // snprintf(string, 100, "bat %s", compiler->_asm);
    // system(string);
    // snprintf(string, 100, "cat %s | pbcopy", compiler->_asm);
    // system(string);
    snprintf(string, 1024, "clang %s -c %s -o %s", target == TARGET_ARM64_MAC ? "-arch arm64" : "", compiler->_asm, compiler->_o);
    system(string);
    remove(compiler->_asm);
    if (target == TARGET_ARM64_MAC){
       snprintf(string, 1024, 
        "clang -O0 -fno-stack-protector -fno-common -fno-exceptions "
        "-arch arm64 %s -o %s -e main -Wl,-w "
        "-Wl,-platform_version,macos,11.0,11.0 -lc", 
        compiler->_o, compiler->name); 
    }else if (target == TARGET_ARM64_LINUX){
// Hope the target system is linux
        snprintf(string, 1024, 
        "clang  -target aarch64-unknown-linux-gnu -O0 -fno-stack-protector -fno-common -fno-exceptions "
        "%s -o %s -e main -Wl,-w -lc", 
        compiler->_o, compiler->name);
    }
    system(string);
    remove(compiler->_o);
}


char *arm64_format(char *str, int type){
    if (type != AST_STRING){
        if (str[0] == '#')
            return strdup(str + 1);
        return strdup(str);
    }
    char *out = malloc(1000);
    int b = 0;
    out[b++] = '\"';
    for (int i=0; i<strlen(str); i++){
        if (str[i] == '\n'){
            out[b++] = '\\';
            out[b++] = 'n';
        }else {
            out[b++] = str[i];
        }
    }
    out[b++] = '\\';
    out[b++] = '0';
    out[b++] = '\"';
    out[b++] = '\0';
    return out;
}

char *arm64_eat_expr(Compiler *compiler, AST *ast, int size);

char *arm64_move(Compiler *compiler, char *reg, char *buf, int typeinfo) {
    bool reg_is_mem = (strchr(reg, '[') != NULL);
    bool buf_is_mem = (strchr(buf, '[') != NULL);
    bool buf_is_imm = (isdigit(buf[0]) || buf[0] == '#' || (buf[0] == '-' && isdigit(buf[1])));

    char *final_reg = reg;
    char *final_buf = buf;
    
    AST_Reg _reg = arm64_reg_real_to_num(reg);
    if (_reg.reg != -1) final_reg = arm64_change_reg_size(_reg.reg, typeinfo);
    
    AST_Reg _buf = arm64_reg_real_to_num(buf);
    if (_buf.reg != -1) final_buf = arm64_change_reg_size(_buf.reg, typeinfo);

    if (strcmp(final_reg, final_buf) == 0) return final_reg;

    char *scratch = (typeinfo == 8) ? "x16" : "w16";

    if (reg_is_mem && buf_is_mem) {
        char *ld_op = (typeinfo == 1) ? "ldrb" : (typeinfo == 2) ? "ldrh" : "ldr";
        char *st_op = (typeinfo == 1) ? "strb" : (typeinfo == 2) ? "strh" : "str";
        compiler_write_text_line(compiler, "%s %s, %s", ld_op, scratch, buf);
        compiler_write_text_line(compiler, "%s %s, %s", st_op, scratch, reg);
        return reg;
    }

    if (reg_is_mem && buf_is_imm) {
        compiler_write_text_line(compiler, "ldr %s, =%s", (typeinfo == 8 ? "x16" : "w16"), (buf[0] == '#') ? buf + 1 : buf);
        char *st_op = (typeinfo == 1) ? "strb" : (typeinfo == 2) ? "strh" : "str";
        compiler_write_text_line(compiler, "%s %s, %s", st_op, scratch, reg);
        return reg;
    }

    if (reg_is_mem) {
        char *op = (typeinfo == 1) ? "strb" : (typeinfo == 2) ? "strh" : "str";
        compiler_write_text_line(compiler, "%s %s, %s", op, final_buf, final_reg);
    } else if (buf_is_mem) {
        char *op = (typeinfo == 1) ? "ldrb" : (typeinfo == 2) ? "ldrh" : "ldr";
        compiler_write_text_line(compiler, "%s %s, %s", op, final_reg, final_buf);
    } else if (buf_is_imm) {
        compiler_write_text_line(compiler, "ldr %s, =%s", final_reg, (buf[0] == '#') ? buf + 1 : buf);
    } else {
        compiler_write_text_line(compiler, "mov %s, %s", final_reg, final_buf);
    }
    return final_reg;
}

void arm64_compare(Compiler *compiler, AST *ast) {
    char *left = arm64_eat_expr(compiler, ast->data.expr.left, -1);
    arm64_move(compiler, "x14", left, 8);

    char *right = arm64_eat_expr(compiler, ast->data.expr.right, -1);
    arm64_move(compiler, "x15", right, 8);

    int size = ast->data.expr.left->typeinfo;
    if (ast->data.expr.right->typeinfo > size) {
        size = ast->data.expr.right->typeinfo;
    }

    if (size <= 4) {
        compiler_write_text_line(compiler, "cmp w14, w15");
    } else {
        compiler_write_text_line(compiler, "cmp x14, x15");
    }
}


char *arm64_eat_call(Compiler *compiler, AST ast){
    if(ast.type == AST_CALL){
        char string[100];
        snprintf(string, 100, "_fun%s", ast.data.jmpif.label);
        compiler_write_text_line(compiler, "bl %s", strdup(string)); 
        return "x0";
    }else if(ast.type == AST_CALLIF){
        char string[100];
        snprintf(string, 100, ".L%d", compiler->templabel++);
        char *reg = arm64_eat_expr(compiler, ast.data.jmpif.reg, -1);
        
        char *test_reg = reg;
        if (strchr(reg, '[')){
            arm64_move(compiler, "x21", reg, ast.data.jmpif.reg->typeinfo);
            test_reg = arm64_change_reg_size(arm64_reg_real_to_num("x21").reg, ast.data.jmpif.reg->typeinfo);
        }
        compiler_write_text_line(compiler, "cbz %s, %s", test_reg, strdup(string));

        char string1[100];
        snprintf(string1, 100, "_fun%s", ast.data.jmpif.label);
        compiler_write_text_line(compiler, "bl %s", strdup(string1));
        compiler_write_text(compiler, "%s:\n", strdup(string));
        return "x0";
    }else if(ast.type == AST_CALLIFN){
        char string[100];
        snprintf(string, 100, ".L%d", compiler->templabel++);
        char *reg = arm64_eat_expr(compiler, ast.data.jmpif.reg, -1);
        
        char *test_reg = reg;
        if (strchr(reg, '[')){
            arm64_move(compiler, "x21", reg, ast.data.jmpif.reg->typeinfo);
            test_reg = arm64_change_reg_size(arm64_reg_real_to_num("x21").reg, ast.data.jmpif.reg->typeinfo);
        }
        compiler_write_text_line(compiler, "cbnz %s, %s", test_reg, strdup(string));

        char string1[100];
        snprintf(string1, 100, "_fun%s", ast.data.jmpif.label);
        compiler_write_text_line(compiler, "bl %s", strdup(string1));
        compiler_write_text(compiler, "%s:\n", strdup(string));
        return "x0";
    }
    return "";
}
void arm64_eat_syscall(Compiler *compiler, AST ast){
    if (target == TARGET_ARM64_MAC){
        compiler_write_text_line(compiler, "ldr x16, =%s", ast.data.text.value);
        compiler_write_text_line(compiler, "svc #0x80");
    }else if (target == TARGET_ARM64_LINUX){
        compiler_write_text_line(compiler, "ldr x8, =%s", ast.data.text.value);
        compiler_write_text_line(compiler, "svc 0");
    }
}

char *arm64_eat_lhs(Compiler *compiler, AST ast, int size);
char *arm64_eat_lhs(Compiler *compiler, AST ast, int size){
    AST *left = ast.data.opexpr.left;
    if (left->type == AST_VAR) {
        if (left->data.var.offset == -1){
            return left->data.text.value;
        }else {
            char string[100];
            compiler_write_text_line(compiler, "add x11, x29, #-%d", left->data.var.offset);
            snprintf(string, 100, "[x11]");
            return strdup(string);
        }
    }else if(left->type == AST_REG){
        AST_Reg reg = left->data.operation.reg;
        return arm64_change_reg_size(reg.reg, reg.size);
    } else if(left->type == AST_REF) {
        char *buf = arm64_eat_expr(compiler, left->data.expr.left, -1);
        
        if (strncmp(buf, "rel ", 4) == 0) {
            char *label = buf + 4;
            compiler_write_text_line(compiler, "adrp x21, %s@PAGE", label);
            compiler_write_text_line(compiler, "add x21, x21, %s@PAGEOFF", label);
        } else if (buf[0] == '[') {
            char clean_buf[100];
            strncpy(clean_buf, buf + 1, strlen(buf) - 2);
            clean_buf[strlen(buf) - 2] = '\0';
            compiler_write_text_line(compiler, "add x21, %s", clean_buf);
        } else {
            compiler_write_text_line(compiler, "mov x21, %s", buf);
        }
        return "x21";
    }else if(left->type == AST_DEREF){
        AST *leftleft = left->data.expr.left;
        char *buf = "";
        bool shortcut = 0;
        if (leftleft->type == AST_PLUS) {
            if (is_immediate_value(leftleft->data.expr.right)){
                shortcut = 1;
            }
        }
        if (!shortcut) {
            buf = arm64_eat_expr(compiler, leftleft, -1);
        };
        char string[100];
        if (shortcut){
            char *left1 = arm64_eat_expr(compiler, leftleft->data.expr.left, -1);
            char *right1 = arm64_eat_expr(compiler, leftleft->data.expr.right, -1);
            snprintf(string, 100, "[%s, #%s]", left1, right1);
        }else {
            if (strchr(buf, '[')){
                snprintf(string, 100, "%s", buf);
            }else {
                snprintf(string, 100, "[%s]", buf);
            }
        }

        return strdup(string);
    }else {
        ;
    }
    return "";
};


char *arm64_eat_expr(Compiler *compiler, AST *ast, int size){
    if (ast->type == AST_INT){
        char string[100];
        snprintf(string, 100, "#%s", strdup(ast->data.text.value));
        return strdup(string);
    }else if (ast->type == AST_STRING){
        return strdup(ast->data.text.value);
    }else if (ast->type == AST_CHAR){
        return strdup(string_int_to_string(ast->data.text.value[0]));
    }else if(ast->type == AST_VAR) {
        char string[100];
        if (ast->data.var.offset == -1) {
            char *label = ast->data.text.value;
            compiler_write_text_line(compiler, "adrp x21, _LBC%s@PAGE", label);
            compiler_write_text_line(compiler, "add x21, x21, _LBC%s@PAGEOFF", label);
            snprintf(string, 100, "x21"); 
            return strdup(string);
        } else {
            compiler_write_text_line(compiler, "add x21, x29, #-%d", ast->data.var.offset);
            snprintf(string, 100, "[x21]");
            return strdup(string);
        }
    }else if(ast->type == AST_REG){
        AST_Reg reg = ast->data.operation.reg;
        return arm64_change_reg_size(reg.reg, reg.size);
    }else if(ast->type == AST_GT){
        arm64_compare(compiler, ast);
        compiler_write_text_line(compiler, "cset w8, gt");
        return "x8";
    }else if(ast->type == AST_GTE){
        arm64_compare(compiler, ast);
        compiler_write_text_line(compiler, "cset w8, ge");
        return "x8";
    }else if(ast->type == AST_LT){
        arm64_compare(compiler, ast);
        compiler_write_text_line(compiler, "cset w8, lt");
        return "x8";
    }else if(ast->type == AST_LTE){
        arm64_compare(compiler, ast);
        compiler_write_text_line(compiler, "cset w8, le");
        return "x8";
    }else if(ast->type == AST_LAND){
        char *left = arm64_eat_expr(compiler, ast->data.expr.left, -1);
        arm64_move(compiler, "x0", left, 8);

        char *right = arm64_eat_expr(compiler, ast->data.expr.right, -1);
        arm64_move(compiler, "x1", right, 8);
        compiler_write_text_line(compiler, "bl _and");
        return "x0";
    }else if(ast->type == AST_LOR){
        char *left = arm64_eat_expr(compiler, ast->data.expr.left, -1);
        arm64_move(compiler, "x0", left, 8);

        char *right = arm64_eat_expr(compiler, ast->data.expr.right, -1);
        arm64_move(compiler, "x1", right, 8);
        compiler_write_text_line(compiler, "bl _or");
        return "x0";
    }else if(ast->type == AST_ARRAY){
        char string[100];
        string[0] = '\0';
        AST *ast_current = ast->data.astarray.block;
        for (int i=0; i<ast->data.astarray.blocklen; i++){
            strcat(string, arm64_eat_expr(compiler, ast_current, -1));
            if (i != ast->data.astarray.blocklen-1)
                strcat(string, ", ");
            ast_current = ast_current->next;
        }
        strcat(string, "\0");
        return strdup(string);
    }else if(ast->type == AST_EQ){
        arm64_compare(compiler, ast);
        compiler_write_text_line(compiler, "cset w8, eq");
        return "x8";
    }else if(ast->type == AST_NEQ){
        arm64_compare(compiler, ast);
        compiler_write_text_line(compiler, "cset w8, ne");
        return "x8";
    } else if(ast->type == AST_REF) {
        char *buf = arm64_eat_expr(compiler, ast->data.expr.left, -1);

        if (strncmp(buf, "rel ", 4) == 0) {
            char *label = buf + 4;
            compiler_write_text_line(compiler, "adrp x11, %s@PAGE", label);
            compiler_write_text_line(compiler, "add x11, x11, %s@PAGEOFF", label);
        } 
        else if (buf[0] == '[') {
            char clean_buf[100];
            strncpy(clean_buf, buf + 1, strlen(buf) - 2);
            clean_buf[strlen(buf) - 2] = '\0';
            compiler_write_text_line(compiler, "mov x11, %s", clean_buf);
        }
        else {
            compiler_write_text_line(compiler, "mov x11, %s", buf);
        }
        return "x11";
    }else if(ast->type == AST_CALL || ast->type == AST_CALLIF || ast->type == AST_CALLIFN){
        return arm64_eat_call(compiler, *ast);
    }else if(ast->type == AST_DEREF){
        AST *left = ast->data.expr.left;
        char *buf = "";
        bool shortcut = 0;
        if (left->type == AST_PLUS) {
            if (is_immediate_value(left->data.expr.right)){
                shortcut = 1;
            }
        }
        if (!shortcut) {
            buf = arm64_eat_expr(compiler, left, -1);
        }

        char str[100];
        if (shortcut){
            char *left1 = arm64_eat_expr(compiler, left->data.expr.left, -1);
            char *right1 = arm64_eat_expr(compiler, left->data.expr.right, -1);
            snprintf(str, 100, "[%s, #%s]", left1, right1);
        }else {
            if (buf[0] == '['){
                arm64_move(compiler, "x11", buf, ast->data.expr.left->typeinfo);
                snprintf(str, 100, "x11");
            }else if(arm64_is_real_reg(buf) == 0){
                int reg = arm64_reg_real_to_num(buf).reg;
                snprintf(str, 100, "[%s]", arm64_change_reg_size(reg, 8));
            }else {
                snprintf(str, 100, "[%s]", buf);
            }
        }
        return strdup(str);
    }else if(ast->type == AST_PLUS){
        char *left = arm64_eat_expr(compiler, ast->data.expr.left, -1);
        arm64_move(compiler, "x11", left, ast->data.expr.left->typeinfo);
        compiler_write_text_line(compiler, "str x11, [sp, #-16]!");
        char *right = arm64_eat_expr(compiler, ast->data.expr.right, -1);
        arm64_move(compiler, "x9", right, ast->data.expr.right->typeinfo);
        compiler_write_text_line(compiler, "ldr x11, [sp], #16");
        compiler_write_text_line(compiler, "add x11, x11, x9");
        return "x11";
    };
    return "";
}



void arm64_end(Compiler *compiler) {
    compiler_write_text_line(compiler, "mov sp, x29");
    compiler_write_text_line(compiler, "ldp x29, x30, [sp], #16");
    compiler_write_text_line(compiler, "ret");
}

void arm64_eat_ast(Compiler *compiler, AST ast){
    if (ast.type == AST_MOV){
        AST *right = ast.data.opexpr.right;
        char *buf = arm64_eat_expr(compiler, right, ast.typeinfo);
        char *left = arm64_eat_lhs(compiler, ast, ast.typeinfo);
        int typeinfo = ast.typeinfo;
        arm64_move(compiler, left, buf, ast.typeinfo);
    }else if(ast.type == AST_SYSCALL){
        arm64_eat_syscall(compiler, ast);
    }else if(ast.type == AST_RET){
        arm64_end(compiler);
    }else if(ast.type == AST_PUSH){
        char *buf = arm64_eat_expr(compiler, ast.data.operation.right, -1);
        compiler_write_text_line(compiler, "str %s, [sp, #-16]!", buf);
    }else if(ast.type == AST_POP){
        char *reg = arm64_reg_to_name(ast.data.operation.reg);
        compiler_write_text_line(compiler, "ldr %s, [sp], #16", reg);
    }else if(ast.type == AST_CALL || ast.type == AST_CALLIF || ast.type == AST_CALLIFN){
        arm64_eat_call(compiler, ast);
    }else if(ast.type == AST_ADD){
        char *reg = arm64_reg_to_name(ast.data.operation.reg);
        AST_Reg _reg = arm64_reg_real_to_num(reg);
        char *buf = arm64_eat_expr(compiler, ast.data.operation.right, -1);
        if (strchr(buf, '[')){
            arm64_move(compiler, "x9", buf, _reg.size);
            compiler_write_text_line(compiler, "add %s, %s, %s", reg, reg, arm64_change_reg_size(arm64_reg_real_to_num("x9").reg, _reg.size));
        }else if (arm64_is_real_reg(buf) == 0){
            AST_Reg _buf = arm64_reg_real_to_num(buf);
            compiler_write_text_line(compiler, "add %s, %s, %s", reg, reg, arm64_change_reg_size(_buf.reg, _reg.size));
        }else {
            compiler_write_text_line(compiler, "add %s, %s, %s", reg, reg, buf);
        }
    }else if(ast.type == AST_SUB){

        char *reg = arm64_reg_to_name(ast.data.operation.reg);
        AST_Reg _reg = ast.data.operation.reg;
        char *buf = arm64_eat_expr(compiler, ast.data.operation.right, -1);
        if (strchr(buf, '[')){
            arm64_move(compiler, "x9", buf, _reg.size);
            compiler_write_text_line(compiler, "sub %s, %s, %s", reg, reg, arm64_change_reg_size(arm64_reg_real_to_num("x9").reg, _reg.size));
        }else if (arm64_is_real_reg(buf) == 0){
            AST_Reg _buf = arm64_reg_real_to_num(buf);
            compiler_write_text_line(compiler, "sub %s, %s, %s", reg, reg, arm64_change_reg_size(_buf.reg, _reg.size));
        }else {
            compiler_write_text_line(compiler, "sub %s, %s, %s", reg, reg, buf);
        }

    }else if(ast.type == AST_MUL){
        char *reg = arm64_reg_to_name(ast.data.operation.reg);
        AST_Reg _reg = ast.data.operation.reg;
        char *buf = arm64_eat_expr(compiler, ast.data.operation.right, -1);
        if (strchr(reg, '[')){
            arm64_move(compiler, "x9", reg, _reg.size);
            char *l = arm64_change_reg_size(arm64_reg_real_to_num("x9").reg, _reg.size);
            compiler_write_text_line(compiler, "mul %s, %s, %s", l, l, buf);
            arm64_move(compiler, reg, "x9", _reg.size);
        }else if (arm64_is_real_reg(buf) == 0){
            AST_Reg _buf = arm64_reg_real_to_num(buf);
            compiler_write_text_line(compiler, "mul %s, %s, %s", reg, reg, arm64_change_reg_size(_buf.reg, _reg.size));
        }else {
            arm64_move(compiler, "x14", buf, 8);
            compiler_write_text_line(compiler, "mul %s, %s, %s", reg, reg, arm64_change_reg_size(arm64_reg_real_to_num("x14").reg, _reg.size));
        }
    }else if (ast.type == AST_DIV) {
        char *buf = arm64_eat_expr(compiler, ast.data.operation.right, -1);
        char *reg = arm64_reg_to_name(ast.data.operation.reg);

        arm64_move(compiler, "x16", reg, 8);
        arm64_move(compiler, "x17", buf, 8);

        compiler_write_text_line(compiler, "sdiv x16, x16, x17");
        arm64_move(compiler, reg, "x16", 8);
    }else if(ast.type == AST_SHL){
        char *reg = arm64_reg_to_name(ast.data.operation.reg);
        AST_Reg _reg = ast.data.operation.reg;
        char *buf = arm64_eat_expr(compiler, ast.data.operation.right, -1);
        if (strchr(reg, '[')){
            arm64_move(compiler, "x9", reg, _reg.size);
            char *l = arm64_change_reg_size(arm64_reg_real_to_num("x9").reg, _reg.size);
            compiler_write_text_line(compiler, "lsl %s, %s, %s", l, l, buf);
            arm64_move(compiler, reg, "x9", _reg.size);
        }else if (arm64_is_real_reg(buf) == 0){
            AST_Reg _buf = arm64_reg_real_to_num(buf);
            compiler_write_text_line(compiler, "lsl %s, %s, %s", reg, reg, arm64_change_reg_size(_buf.reg, _reg.size));
        }else {
            arm64_move(compiler, "x14", buf, 8);
            compiler_write_text_line(compiler, "lsl %s, %s, %s", reg, reg, arm64_change_reg_size(arm64_reg_real_to_num("x14").reg, _reg.size));
        }
    }else if(ast.type == AST_SHR){
        char *reg = arm64_reg_to_name(ast.data.operation.reg);
        AST_Reg _reg = ast.data.operation.reg;
        char *buf = arm64_eat_expr(compiler, ast.data.operation.right, -1);
        if (strchr(reg, '[')){
            arm64_move(compiler, "x9", reg, _reg.size);
            char *l = arm64_change_reg_size(arm64_reg_real_to_num("x9").reg, _reg.size);
            compiler_write_text_line(compiler, "lsr %s, %s, %s", l, l, buf);
            arm64_move(compiler, reg, "x9", _reg.size);
        }else if (arm64_is_real_reg(buf) == 0){
            AST_Reg _buf = arm64_reg_real_to_num(buf);
            compiler_write_text_line(compiler, "lsr %s, %s, %s", reg, reg, arm64_change_reg_size(_buf.reg, _reg.size));
        }else {
            arm64_move(compiler, "x14", buf, 8);
            compiler_write_text_line(compiler, "lsr %s, %s, %s", reg, reg, arm64_change_reg_size(arm64_reg_real_to_num("x14").reg, _reg.size));
        }
    }else if (ast.type == AST_MOD) {
        char *buf = arm64_eat_expr(compiler, ast.data.operation.right, -1);
        char *reg = arm64_reg_to_name(ast.data.operation.reg);

        arm64_move(compiler, "x16", reg, 8);
        arm64_move(compiler, "x17", buf, 8);

        compiler_write_text_line(compiler, "sdiv x15, x16, x17");
        compiler_write_text_line(compiler, "msub x15, x15, x17, x16");

        arm64_move(compiler, reg, "x15", 8);
    }else if(ast.type == AST_BAND){
        char *reg = arm64_reg_to_name(ast.data.operation.reg);
        AST_Reg _reg = arm64_reg_real_to_num(reg);
        char *buf = arm64_eat_expr(compiler, ast.data.operation.right, -1);
        if (strchr(buf, '[')){
            arm64_move(compiler, "x9", buf, _reg.size);
            compiler_write_text_line(compiler, "and %s, %s, %s", reg, reg, arm64_change_reg_size(arm64_reg_real_to_num("x9").reg, _reg.size));
        }else if (arm64_is_real_reg(buf) == 0){
            AST_Reg _buf = arm64_reg_real_to_num(buf);
            compiler_write_text_line(compiler, "and %s, %s, %s", reg, reg, arm64_change_reg_size(_buf.reg, _reg.size));
        }else {
            compiler_write_text_line(compiler, "and %s, %s, %s", reg, reg, buf);
        }
    }else if(ast.type == AST_BOR){
        char *reg = arm64_reg_to_name(ast.data.operation.reg);
        AST_Reg _reg = arm64_reg_real_to_num(reg);
        char *buf = arm64_eat_expr(compiler, ast.data.operation.right, -1);
        if (strchr(buf, '[')){
            arm64_move(compiler, "x9", buf, _reg.size);
            compiler_write_text_line(compiler, "orr %s, %s, %s", reg, reg, arm64_change_reg_size(arm64_reg_real_to_num("x9").reg, _reg.size));
        }else if (arm64_is_real_reg(buf) == 0){
            AST_Reg _buf = arm64_reg_real_to_num(buf);
            compiler_write_text_line(compiler, "orr %s, %s, %s", reg, reg, arm64_change_reg_size(_buf.reg, _reg.size));
        }else {
            compiler_write_text_line(compiler, "orr %s, %s, %s", reg, reg, buf);
        }
    }else if(ast.type == AST_JMP){
        compiler_write_text_line(compiler, "b %s_%s", ast.data.jmpif.label, cur_function);
    }else if(ast.type == AST_JMPIF){
        char *reg = arm64_eat_expr(compiler, ast.data.jmpif.reg, -1);
        char *test = reg;
        if (strchr(reg, '[')){
            arm64_move(compiler, "x21", reg, 8);
            test = arm64_change_reg_size(arm64_reg_real_to_num("x21").reg, ast.data.jmpif.reg->typeinfo);
        };
        compiler_write_text_line(compiler, "cbz %s, %s_%s", test, ast.data.jmpif.label, cur_function);
    }else if(ast.type == AST_JMPIFN){
        char *reg = arm64_eat_expr(compiler, ast.data.jmpif.reg, -1);
        char *test = reg;
        if (strchr(reg, '[')){
            arm64_move(compiler, "x21", reg, 8);
            test = arm64_change_reg_size(arm64_reg_real_to_num("x21").reg, ast.data.jmpif.reg->typeinfo);
        };
        compiler_write_text_line(compiler, "cbnz %s, %s_%s", test, ast.data.jmpif.label, cur_function);
    }else if(ast.type == AST_LABEL){
        compiler_write_text(compiler, ".%s_%s:\n", ast.data.text.value, cur_function);
        AST *current = ast.data.funcdef.block;
        while (current != NULL) {
            arm64_eat_ast(compiler, *current);
            current = current->next;
        }
        compiler_write_text(compiler, ".%s_end_%s:\n", ast.data.text.value, cur_function);
    };
};

char *arm64_typeinfo_to_specifier(AST_TypeInfo type){
    if (type == 1){
        return ".ascii";
    }else if (type == 2){
        return ".short";
    }else if (type == 8){
        return ".quad";
    }else if (type == 4){
        return ".long";
    }

    return "";
}

int arm64_function_num = 0;


void arm64_eat_body(Compiler *compiler){
    AST ast = *(load_ast_compiler(compiler));
    if (ast.type == AST_FUNCDEF){
        cur_function = ast.data.funcdef.name;
        char string[100];
        snprintf(string, 100, "_fun%s", ast.data.funcdef.name);
        char *name = strdup(string);
        if (string_compare(name, "_funmain", strlen(name)) == 0) {
            name = "main";
        }
        compiler->templabel = 0;
        compiler_write_text(compiler, "%s:\n", name);
        compiler_write_text_line(compiler, "stp x29, x30, [sp, #-16]!"); 
        compiler_write_text_line(compiler, "mov x29, sp");
        int size = 0;
        Reviser_Function *function = reviser_get_function(compiler->global->functions, arm64_function_num);
        for (int i=0; i<function->scope.variablelen; i++){
            size += reviser_get_variable(function->scope.variables, i)->size;
        };
        if (size > 0) {
            compiler_write_text_line(compiler, "sub sp, sp, %d", (size + 15) & ~15);
        }
        AST *current = ast.data.funcdef.block;
        while (current != NULL) {
            arm64_eat_ast(compiler, *current);
            current = current->next;
        }

        arm64_function_num++;
        arm64_end(compiler);
    }else if(ast.type == AST_LABEL){
        compiler_write_text(compiler, "%s:\n", ast.data.text.value);
        AST *current = ast.data.funcdef.block;
        while (current != NULL) {
            arm64_eat_ast(compiler, *current);
            current = current->next;
        }
    }else if (ast.type == AST_VAR){
        AST *data = ast.data.var.value;
        char *r = arm64_format(arm64_eat_expr(compiler, data, -1), data->type);
        compiler_write(compiler->data, "\t.balign 8\n\t_LBC%s: %s %s\n", ast.data.var.name, arm64_typeinfo_to_specifier(ast.typeinfo), r);
        free(r);
    }
    compiler_peek(compiler);
}

char arm64_eat(Compiler *compiler){
    if(compiler->cur >= compiler->astlen)
        return -1;
    arm64_eat_body(compiler);
    return 0;
}
