#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct Token
{
    char address[100];
    char label[100];
    char opcode[100];
    char operands[100];
};

struct SymbolTable
{
    char label[100];
    int address;
};

struct SymbolTable sym[100];

void tokenise(char line[1000], struct Token *token)
{
    int res = sscanf(line, "%s %s %s %s", token->address, token->label, token->opcode, token->operands);
    if (strcmp(token->label, "START") == 0)
    {
        strcmp(token->operands, token->opcode);
        strcmp(token->opcode, token->label);
        strcmp(token->label, token->address);
    }
    if (res == 3)
        strcpy(token->operands, "");
    else if (res == 2)
    {
        strcpy(token->opcode, token->address);
        strcpy(token->operands, token->label);
    }
}

struct Token search_opcode(FILE *optab, struct Token token)
{
    struct Token token2;
    char line2[1000];
    while (fgets(line2, 1000, optab) != NULL)
    {
        tokenise(line2, &token2);
        if (strcmp(token2.opcode, token.opcode) == 0)
            return token2;
    }
}

int main()
{
    struct Token token;
    struct Token token1;
    int Line = 1, tar_address, address = 0, n = 1;
    FILE *input_pass1, *output_pass2, *optab, *fp2, *object_file;
    char line[1000];

    input_pass1 = fopen("output.txt", "r");
    output_pass2 = fopen("output_pass2.txt", "w");
    fp2 = fopen("symtab.txt", "r");
    optab = fopen("optab.txt", "r");
    object_file = fopen("object.txt", "w");

    while (fscanf(fp2, "%s %x", sym[Line].label, &sym[Line].address) != EOF)
        Line++;
    rewind(fp2);

    int input_len=1;
    while(fscanf(input_pass1, "%x %s %s %s",token.address,token.label,token.opcode,token.operands)!=EOF)
    input_len++;

    rewind(input_pass1);

    fgets(line, 1000, input_pass1);
    tokenise(line, &token1);
    fprintf(object_file, "H^%-6s^%06X^%06X\n", token1.label, atoi(token1.address), token.address-token1.address); // Header Record

    int text_record_start = 0;
    char text_record[1000] = "T^";
    int text_record_length = 0;

    while (fgets(line, 1000, input_pass1) != NULL)
    {
        tokenise(line, &token);
        if (strcmp(token.opcode, "WORD") == 0)
        {
            fprintf(output_pass2, "%s\t%s\t%s\t%s\t%06X\n", token.address, token.label, token.opcode, token.operands, atoi(token.operands));
            address += 3;

            if (text_record_start == 0)
            {
                text_record_start = atoi(token.address);
                text_record_length = 3;
                strcat(text_record, token.operands);
            }
            else
            {
                text_record_length += 3;
                strcat(text_record, token.operands);
            }
        }
        else if (strcmp(token.opcode, "RSUB") == 0) {}
        else if (strcmp(token.opcode, "RESB") == 0 || strcmp(token.opcode, "RESW") == 0) {}
        else if (strcmp(token.opcode, "BYTE") == 0)
        {
            int len = strlen(token.operands);
            if (token.operands[0] == 'C')
            {
                fprintf(output_pass2, "%s\t%s\t%s\t%s\t\t", token.address, token.label, token.opcode, token.operands);
                address += strlen(token.operands) - 3;
                for (int j = 2; j < len - 1; j++)
                {
                    fprintf(output_pass2, "%X", token.operands[j]);
                }
            }
            else if (token.operands[0] == 'X')
            {
                fprintf(output_pass2, "%s\t%s\t%s\t%s\t\t", token.address, token.label, token.opcode, token.operands);
                address += (strlen(token.operands) - 3) / 2;
                for (int j = 2; j < len - 1; j++)
                {
                    fprintf(output_pass2, "%c", token.operands[j]);
                }
            }
            fprintf(output_pass2, "\n");
        }
        else
        {
            rewind(optab);
            token1 = search_opcode(optab, token);
            tar_address = 0;
            for (int i = 0; i < Line; i++)
            {
                if (strcmp(token.operands, sym[i].label) == 0)
                    tar_address = sym[i].address;
            }
            address += 3;
            if (!tar_address)
                printf("Invalid Operand Found at line %d\t%s %s %s %s\n", n + 1, token.address, token.label, token.opcode, token.operands);
            fprintf(output_pass2, "%s\t%s\t%s\t%s\t\t%s%x\n", token.address, token.label, token.opcode, token.operands, token1.operands, tar_address);

            if (text_record_start == 0)
            {
                text_record_start = atoi(token.address);
                text_record_length = 6;
                strcat(text_record, token1.operands);
                sprintf(text_record + strlen(text_record), "%X", tar_address);
            }
            else
            {
                text_record_length += 6;
                sprintf(text_record + strlen(text_record), "%X", tar_address);
            }
        }
        n++;
    }

    fprintf(object_file, "%s^%02X^%s\n", text_record, text_record_length / 2, token.address);

    fclose(input_pass1);
    fclose(output_pass2);
    fclose(fp2);
    fclose(object_file);

    return 0;
}