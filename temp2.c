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
    int res = sscanf(line,"%s %s %s %s", token->address, token->label, token->opcode, token->operands);
    if(strcmp(token->label, "START") == 0)
    {
        strcpy(token->operands, token->opcode);
        strcpy(token->opcode, token->label);
        strcpy(token->label, token->address);
        strcpy(token->address,"");
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
    FILE *input_pass1, *output_pass2, *optab, *fp2, *object_file,*len;
    char line[1000];

    input_pass1 = fopen("intermediate.txt", "r");
    output_pass2 = fopen("output_pass2.txt", "w");
    fp2 = fopen("symtab.txt", "r");
    optab = fopen("optab.txt", "r");
    object_file = fopen("object.txt", "w");
    len=fopen("length.txt","r");

    while (fscanf(fp2, "%s %x", sym[Line].label, &sym[Line].address) != EOF)
        Line++;
    rewind(fp2);

    int input_len=1;
    while(fscanf(input_pass1, "%x %s %s %s",token.address,token.label,token.opcode,token.operands)!=EOF)
    input_len++;

    rewind(input_pass1);

    fgets(line, 1000, input_pass1);
    tokenise(line, &token1);
    fgets(line,10,len);
    fprintf(object_file, "H^%-6s^%06X^%s\n", token1.label,atoi(token1.operands),line); // Header Record
    fprintf(output_pass2,"\t%s\t%s\t%d\t\n",token1.label,token1.opcode,1000);

    int text_record_length = 0;

    while (fgets(line, 1000, input_pass1) != NULL)
    {
        if(text_record_length==0)
        fprintf(object_file,"T");
        tokenise(line, &token);
        if (strcmp(token.opcode, "WORD") == 0)
        {
            fprintf(output_pass2, "%s\t%s\t%s\t%s\t%06X\n", token.address, token.label, token.opcode, token.operands, atoi(token.operands));
            address += 3;
            text_record_length+=3;
            fprintf(object_file,"^%06X",atoi(token.operands));
        }
        else if (strcmp(token.opcode, "RSUB") == 0)
            {
                token1=search_opcode(optab,token);
                fprintf(output_pass2, "%s\t%s\t%s\t\t\t%s%04x\n", token.address, token.label, token.opcode,token1.operands,0);
                fprintf(object_file,"^%s%04x",token1.operands,0);
            }
        else if (strcmp(token.opcode, "RESB") == 0 || strcmp(token.opcode, "RESW") == 0) {}
        else if (strcmp(token.opcode, "ORG") == 0) {}
        else if (strcmp(token.opcode, "BYTE") == 0)
        {
            int len = strlen(token.operands);
            int j;
            fprintf(object_file,"^");
            if (token.operands[0] == 'C')
            {
                fprintf(output_pass2, "%s\t%s\t%s\t%s\t\t", token.address, token.label, token.opcode, token.operands);
                address += strlen(token.operands) - 3;
                for (j = 2; j < len - 1; j++)
                {
                    fprintf(output_pass2, "%X", token.operands[j]);
                    fprintf(object_file,"%X",token.operands[j]);
                }
                text_record_length+=strlen(token.operands) - 3;
            }
            else if (token.operands[0] == 'X')
            {
                fprintf(output_pass2, "%s\t%s\t%s\t%s\t\t", token.address, token.label, token.opcode, token.operands);
                address += (strlen(token.operands) - 3) / 2;
                for (j = 2; j < len - 1; j++)
                {
                    fprintf(output_pass2, "%c", token.operands[j]);
                    fprintf(object_file,"%c",token.operands[j]);
                }
                text_record_length+= (strlen(token.operands) - 3) / 2;
            }
            fprintf(output_pass2, "\n");
        }
        else if (strcmp(token.opcode,"END") == 0)
        {
            fprintf(object_file,"^%X\nE^%s",text_record_length,token1.label);
        }
        else
        {
            rewind(optab);
            token1 = search_opcode(optab, token);
            tar_address = 0;
            int i;
            for (i = 0; i < Line; i++)
            {
                if (strcmp(token.operands, sym[i].label) == 0)
                    tar_address = sym[i].address;
            }
            address += 3;
            if (!tar_address)
            {
                printf("Invalid Operand Found at line %d\t%s %s %s %s\n", n + 1, token.address, token.label, token.opcode, token.operands);
                printf("No forward reference for %s\n",token.operands);
                fprintf(output_pass2, "%s\t%s\t%s\t%s\t\t%s%04x\n", token.address, token.label, token.opcode, token.operands, token1.operands,0);
            }
            else{
            text_record_length+=3;
            fprintf(output_pass2, "%s\t%s\t%s\t%s\t\t%s%x\n", token.address, token.label, token.opcode, token.operands, token1.operands, tar_address);
            fprintf(object_file,"^%s%x",token1.operands,tar_address);
            }
        }
        if(text_record_length>30){
            fprintf(object_file,"^%X\n",text_record_length-3);
            text_record_length=0;
        }
        n++;
    }
    fclose(input_pass1);
    fclose(output_pass2);
    fclose(fp2);
    fclose(object_file);
    return 0;
}
