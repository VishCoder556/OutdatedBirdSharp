#include <libgen.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct {
    Token *in;
    int inlen;
    Token *out;
    int outlen;
} Preprocessor_Define;

typedef struct {
    Preprocessor_Define *defines;
    int definecap;
    int definelen;
} Preprocessor_SymbolTable;

Preprocessor_SymbolTable preprocess_symtab;

char token_eq(Token token1, Token token2) {
    if (token1.type == token2.type) {
        return strcmp(token1.value, token2.value) == 0;
    }
    return 0;
}

void preprocess_substitute(Tokenizer *tokenizer, int *i) {
    for (int d = 0; d < preprocess_symtab.definelen; d++) {
        Preprocessor_Define *def = &preprocess_symtab.defines[d];
        if (def->inlen == 0) continue;
        if (*i + def->inlen > tokenizer->tokenlen) continue;

        char match = 1;
        for (int k = 0; k < def->inlen; k++) {
            if (!token_eq(tokenizer->tokens[*i + k], def->in[k])) {
                match = 0;
                break;
            }
        }

        if (match) {
            tokenizer_insert_at(tokenizer, *i, def->out, def->outlen, def->inlen);
            if (def->outlen > 0) {
                *i += (def->outlen - 1);
            } else {
                *i -= 1; 
            }
            return;
        }
    }
}

#define MAX_DEFINES 10
#define INITIAL_CAPACITY 5

void preprocess_define(Tokenizer *tokenizer, int *i) {
    if (preprocess_symtab.definelen >= MAX_DEFINES) {
        tokenizer_remove_at(tokenizer, *i, 1);
        return;
    }

    tokenizer_remove_at(tokenizer, *i, 3);

    if (preprocess_symtab.definelen >= preprocess_symtab.definecap) {
        int new_cap = preprocess_symtab.definecap + INITIAL_CAPACITY;
        Preprocessor_Define *new_defines = realloc(
            preprocess_symtab.defines, 
            sizeof(Preprocessor_Define) * new_cap
        );
        if (!new_defines) return;
        
        preprocess_symtab.defines = new_defines;
        preprocess_symtab.definecap = new_cap;
    }

    Preprocessor_Define *def = &preprocess_symtab.defines[preprocess_symtab.definelen];

    int incap = INITIAL_CAPACITY;
    int outcap = INITIAL_CAPACITY;

    def->in = malloc(sizeof(Token) * incap);
    def->out = malloc(sizeof(Token) * outcap);
    if (!def->in || !def->out) return;

    def->inlen = 0;
    def->outlen = 0;

    while (*i < tokenizer->tokenlen && tokenizer->tokens[*i].type != TOKEN_EQ) {
        if (def->inlen >= incap) {
            incap += INITIAL_CAPACITY;
            Token *new_in = realloc(def->in, sizeof(Token) * incap);
            if (!new_in) break;
            def->in = new_in;
        }
        def->in[def->inlen++] = tokenizer->tokens[*i];
        tokenizer_remove_at(tokenizer, *i, 1);
    }

    if (*i < tokenizer->tokenlen && tokenizer->tokens[*i].type == TOKEN_EQ) {
        tokenizer_remove_at(tokenizer, *i, 1);
    }

    while (*i < tokenizer->tokenlen) {
        int start_val = *i;
        preprocess_substitute(tokenizer, i);

        if (start_val == *i) {
            if (def->outlen >= outcap) {
                outcap += INITIAL_CAPACITY;
                Token *new_out = realloc(def->out, sizeof(Token) * outcap);
                if (!new_out) break;
                def->out = new_out;
            }

            def->out[def->outlen++] = tokenizer->tokens[*i];

            if (def->outlen >= 3) {
                Token *t1 = &def->out[def->outlen - 3];
                Token *t2 = &def->out[def->outlen - 2];
                Token *t3 = &def->out[def->outlen - 1];

                if (t1->type == TOKEN_HASH && 
                    t2->type == TOKEN_EXC && 
                    t3->value[0] != '\0' && strcmp(t3->value, "end") == 0) {
                    
                    tokenizer_remove_at(tokenizer, *i, 1);
                    def->outlen -= 3;
                    break;
                }
            }
            tokenizer_remove_at(tokenizer, *i, 1);
        }
    }

    preprocess_symtab.definelen++;
    if (*i > 0) {
        *i -= 1;
    }

}

void preprocess(char *main_file, Tokenizer *tokenizer);

char preprocess_include(Tokenizer *tokenizer, char *main_file, int i) {
    char path[512];
    strncpy(path, main_file, sizeof(path) - 1);
    path[sizeof(path) - 1] = '\0';

    char *dir = dirname(path);
    char input_file[512];
    snprintf(input_file, sizeof(input_file), "%s/%s", dir, tokenizer->tokens[i + 3].value);

    Tokenizer *tokenizer2 = tokenizer_init(input_file, 0);
    if (tokenizer2 == NULL) return 0;

    while (tokenizer_token(tokenizer2) != -1);

    if (tokenizer2->tokenlen > 0 && tokenizer2->tokens[tokenizer2->tokenlen - 1].type == TOKEN_EOF) {
        tokenizer2->tokenlen--;
    }

    preprocess(input_file, tokenizer2);

    if (tokenizer2->tokenlen > 0) {
        tokenizer_insert_at(tokenizer, i, tokenizer2->tokens, tokenizer2->tokenlen, 4);
    } else {
        tokenizer_remove_at(tokenizer, i, 4);
    }
    
    return 1;
}

void preprocess(char *main_file, Tokenizer *tokenizer) {

    preprocess_symtab.definecap = 10;
    preprocess_symtab.defines = malloc(sizeof(Preprocessor_Define) * preprocess_symtab.definecap);
    if (!tokenizer || !tokenizer->tokens) return;

    int i = 0;
    while (i < tokenizer->tokenlen) {
        if (i + 2 < tokenizer->tokenlen &&
            tokenizer->tokens[i].type == TOKEN_HASH &&
            tokenizer->tokens[i + 1].type == TOKEN_EXC &&
            tokenizer->tokens[i + 2].type == TOKEN_ID) {

            if (strcmp(tokenizer->tokens[i + 2].value, "include") == 0) {
                if (i + 3 < tokenizer->tokenlen && tokenizer->tokens[i + 3].type == TOKEN_STRING) {
                    preprocess_include(tokenizer, main_file, i);
                } else {
                    tokenizer_remove_at(tokenizer, i, 3);
                }
                continue;
            } else if (strcmp(tokenizer->tokens[i + 2].value, "define") == 0) {

                preprocess_define(tokenizer, &i);
                continue;
            }
        }

        int start_i = i;
        preprocess_substitute(tokenizer, &i);
        
        if (start_i == i) {
            i++;
        } else if (i < 0) {
            i = 0;
        }
    }
}
