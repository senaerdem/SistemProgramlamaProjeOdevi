#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "fields.h"

#define MAX_COMMAND_LEN 10
#define MAX_OPERANDS 10
#define MAX_OPERAND_LEN 10
#define MAX_OUTPUT_LEN 1000

typedef struct {
    char command[MAX_COMMAND_LEN]; // komutu tutan dizi
    char **operands; // operand tutan diziyi dinamik hale getir
    int num_operands; // komutun kaç operand aldığını tutar
} Instruction;

void parse_instruction(char *line, Instruction *instr) {
    char *token;
    instr->num_operands = 0;
    token = strtok(line, " :");  
    strcpy(instr->command, token);
    strcat(instr->command, ":");

    token = strtok(NULL, " :");
    instr->operands = malloc(MAX_OPERANDS * sizeof(char*)); // operands için dinamik bellek ayır
    while (token != NULL) {
        instr->operands[instr->num_operands] = malloc((strlen(token) + 1) * sizeof(char)); // her operand için ayrı ayrı bellek ayır
        if (token[0] == '\\') {
            if (token[1] == 'b') {
                strcpy(instr->operands[instr->num_operands], " ");
            } else if (token[1] == 'n') {
                strcpy(instr->operands[instr->num_operands], "\n");
            }
        } else {
            strcpy(instr->operands[instr->num_operands], token);
        }
        instr->num_operands++;
        token = strtok(NULL, " :");
    }
}

void free_instruction(Instruction *instr) {
    for (int i = 0; i < instr->num_operands; i++) {
        free(instr->operands[i]); // her operand için ayrılan belleği serbest bırak
    }
    free(instr->operands); // operands dizisini serbest bırak
}

void execute_instruction(Instruction *instr, char *output, int *cursor_position, int *output_length) {
    if (strcmp(instr->command, "yaz:") == 0 || strcmp(instr->command, "sil:") == 0) {
        int valid_number = 0; // Geçerli bir sayı var mı kontrolü
        char valid_character = 0; // Geçerli bir karakter var mı kontrolü

        for (int i = 0; i < instr->num_operands; i++) {
            if (isdigit(instr->operands[i][0])) {
                valid_number = atoi(instr->operands[i]);
                valid_character = 0; // Sayı bulunduğunda karakteri sıfırla
            } else if (isalpha(instr->operands[i][0]) || instr->operands[i][0] == ' ' || instr->operands[i][0] == '\n') {
                valid_character = instr->operands[i][0];
            } else {
                continue; // Geçersiz karakter, döngüyü devam ettir
            }

            if (valid_number > 0 && valid_character != 0) {
                // Geçerli sayı ve karakter varsa işlem yap
                if (strcmp(instr->command, "yaz:") == 0) {
                    for (int j = 0; j < valid_number; j++) {
                        memmove(&output[*cursor_position + 1], &output[*cursor_position], *output_length - *cursor_position);
                        output[(*cursor_position)++] = valid_character;
                        (*output_length)++;
                    }
                } else if (strcmp(instr->command, "sil:") == 0) {
                    int delete_count = 0;
                    *cursor_position = *output_length; // İmleci her zaman sona ayarla
                    while (delete_count < valid_number && (*cursor_position) > 0) {
                        (*cursor_position)--;
                        if (output[*cursor_position] == valid_character) {
                            delete_count++;
                            memmove(&output[*cursor_position], &output[*cursor_position + 1], *output_length - *cursor_position);
                            (*output_length)--;
                        }
                    }
                    if (delete_count < valid_number) { // Gerekli silme sayısına ulaşamazsa
                        *cursor_position = 0; // İmleci başa al
                    }
                }
                // İşlem yapıldıktan sonra sayı ve karakteri sıfırla
                valid_number = 0;
                valid_character = 0;
            }
        }
    } else if (strcmp(instr->command, "sonagit:") == 0) {
        *cursor_position = *output_length; // İmleci çıktının sonuna taşı
    } else {
        return;
    }
}




int main(int argc, char **argv) {
    IS is;
    Instruction instr;
    char *output = calloc(MAX_OUTPUT_LEN, sizeof(char)); // output için dinamik bellek ayır
    int cursor_position = 0;
    int output_length = 0;
    int terminate_program = 0;

    if (argc != 3) { 
        fprintf(stderr, "kullanim: bin/program giris_dosya_ismi cikis_dosya_ismi\n"); 
        exit(1); 
    }

    // Dosya adı uzantılarını kontrol et
    char *input_dot = strrchr(argv[1], '.');
    char *output_dot = strrchr(argv[2], '.');

    if (!input_dot || strcmp(input_dot, ".dat") != 0 || !output_dot || strcmp(output_dot, ".dat") != 0) {
        fprintf(stderr, "Giris ve cikis dosyalari .dat uzantili olmalidir!\n");
        exit(1);
    }
    

    is = new_inputstruct(argv[1]);
    if (is == NULL) {
        perror(argv[1]);
        exit(1);
    }

    FILE *output_file = fopen(argv[2], "w");
    if (output_file == NULL) {
        perror(argv[2]);
        exit(1);
    }

    while (get_line(is) >= 0 && !terminate_program) {
        parse_instruction(is->text1, &instr);
        execute_instruction(&instr, output, &cursor_position, &output_length);
        if (strcmp(instr.command, "dur:") == 0) {
            terminate_program = 1;
            fprintf(output_file, "%s", output);
        }
        free_instruction(&instr); // parse_instruction'dan sonra kullanılan belleği serbest bırak
    }

    fclose(output_file);
    jettison_inputstruct(is);
    free(output); // output için ayrılan belleği serbest bırak
    return 0;
}

