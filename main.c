#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include <limits.h>
#include <float.h>

int isTextIdent(char *identifierName);

int isIntegerIdent(char *identifierName);

const char keywords[16][9] = {"asString", "asText", "from", "input", "insert", "int", "locate",
                              "new", "output", "override", "read", "size", "subs", "text",
                              "to", "write"};

//when reading char by char from a file, ignores whitespaces
char passSpacesInOperation(FILE *filePtr, char ch); 

char passSpacesOutOp(FILE *filePtr, char ch);

bool isSpace(char ch); // determine char is a space or not

int parseStrToken(FILE *filePtr, char first, char *str); //get str tokens from a file and parse them

char parseIntToken(FILE *filePtr, char first, int *number); //get int tokens from a file and parse them 

bool isKeyword(char *token); // determine if any token is keyword or not

bool isCommand(char *token); //determine if any token is command

char keywordToken(FILE *filePtr, char ch, char *token);  //do operations for keyword token

char checkComment(FILE *filePtr); // if detect comment

char space(int checkOperation, FILE *filePtr, char ch); // choose passSpacesInOperation or 
// ...OutsideOpeartion functions to execute

char StringFunction(FILE *filePtr, char ch, char *funcName, char *strValue); 

char IntFunction(FILE *filePtr, char ch, char *funcName, int *numberValue);

char runCommand(FILE *filePtr, char ch, char *token);

char parseString(FILE *filePtr, char ch, char *str);

char returnString(FILE *filePtr, char ch, char *strValue);

char returnInteger(FILE *filePtr, char ch, int *intValue);

bool stringFunction(char *funcName);

bool intFunction(char *funcName);

char *subtractStrings(char *str1, char *str2);

void override(char *myText, int location, char *overText);

int locate(char *mainText, char *subText, int start);

int closeProgram(int code, char *errorMessage);

void freeMemory();

FILE *readFilePtr;
FILE *writeFilePtr;

char **txtIdent;
char **intIdent;

char **txtValues;
int *intValues;

int intListLen = 0, txtListLen = 0;
int intValueListLen = 0, txtValueListLen = 0;

int line = 1;

char passSpacesInOperation(FILE *filePtr, char ch) { 
    do {
        if (ch == '\n') // alt satıra gecme islemi
            line++;
        ch = fgetc(filePtr);

    } while ((ch != EOF) && isSpace(ch));

    if (ch == EOF)
        closeProgram(-1, "End of file before operations complete!");

    return ch;
}

char passSpacesOutOp(FILE *filePtr, char ch) {
    do {
        if (ch == '\n')
            line++;
        ch = fgetc(filePtr);

    } while ((ch != EOF) && isSpace(ch));

    return ch;
}

bool isSpace(char ch) { // checks ch is space, new line or tab
    return (ch == ' ' || ch == '\n' || (ch == '\t'));
}

int parseStrToken(FILE *filePtr, char first, char *str) { // reads tokens
    char ch = fgetc(filePtr);
    int size = 1;
    str[0] = first;

    while (ch != EOF && (isalpha(ch) || isdigit(ch) || ch == '_')) {
        str[size++] = ch;
        if (size > 30) {
            return -1;
        }

        ch = fgetc(filePtr);
    }
    str[size] = '\0';
    return ch;
}

char parseIntToken(FILE *filePtr, char first, int *number) { // reads digits
    char digit = fgetc(filePtr);
    int size = 1;
    char digits[10000];
    digits[0] = first;

    while (digit != EOF && isdigit(digit)) {
        if (size > UINT_MAX) // c unsized int size limit
            closeProgram(-1, "Mac int size!");
        digits[size++] = digit;
        digit = fgetc(filePtr);
    }
    digit = space(1, filePtr, digit);

    digits[size] = '\0';
    char *endDigits;
    *number = strtol(digits, &endDigits, 10); // convert string to integer
    return digit;
}

bool isKeyword(char *token) { // checks token is a keyword
    int numberOfKeywords = sizeof(keywords) / sizeof(keywords[0]);

    for (int i = 0; i < numberOfKeywords; i++) {
        if (strcmp(token, keywords[i]) == 0)
            return true;
    }

    return false;
}

bool isCommand(char *token) { // checks token is a command
    bool control = (strcmp(token, keywords[3]) == 0) ||
                 (strcmp(token, keywords[7]) == 0) ||
                 (strcmp(token, keywords[8]) == 0) ||
                 (strcmp(token, keywords[10]) == 0) ||
                 (strcmp(token, keywords[15]) == 0);
    return control;
}

char keywordToken(FILE *filePtr, char ch, char *token) { // starts keyword operation
    if (isalpha(ch) != 0 || ch == '"') {
        if (!isCommand(token))
            closeProgram(-1, "Invalid keyword usage!");

        ch = runCommand(filePtr, ch, token);
    } else {
        printf("Invalid character '%c' !\n", ch);
        closeProgram(-1, "Invalid character after keyword!");
    }

    return ch;
}

char checkComment(FILE *filePtr) { // if there is a /* -> comment
    char ch = fgetc(readFilePtr);

    if (ch == '*') {
        ch = fgetc(filePtr);

        while (ch != EOF) {
            if (ch == '\n')
                line++;
            else if (ch == '*') {
                ch = fgetc(filePtr);
                if (ch == '/') {
                    return fgetc(filePtr);
                } else if (ch == '\n')
                    line++;
            }
            ch = fgetc(filePtr);
        }
        closeProgram(-1, "End of file before comment close!");
    } else
        closeProgram(-1, "Invalid operator '/' !");
}

char space(int checkOperation, FILE *filePtr, char ch) { // processes and passes space, tab or new line
    while (isSpace(ch) || ch == '/') {
        if (isSpace(ch)) {
            if (checkOperation == 1) // this checks if it is inside operation or not
                ch = passSpacesInOperation(filePtr, ch);
            else
                ch = passSpacesOutOp(filePtr, ch);
        } else
            ch = checkComment(filePtr);
    }
    return ch;
}

int isTextIdent(char *identifierName) { // if token is a text
    for (int i = 0; i < txtListLen; i++) {
        if (strcmp(txtIdent[i], identifierName) == 0)
            return i;
    }
    return -1;
}

int isIntegerIdent(char *identifierName) { // if token is an integer
    for (int i = 0; i < intListLen; i++) {
        if (strcmp(intIdent[i], identifierName) == 0)
            return i;
    }
    return -1;
}

char StringFunction(FILE *filePtr, char ch, char *funcName, char *strValue) {
    ch = space(1, filePtr, ch);
    if (ch != '(')
        closeProgram(-1, "Syntax error!");
    ch = fgetc(filePtr);
    ch = space(1, filePtr, ch);

    if (strcmp(funcName, keywords[0]) == 0) { // asString
        int numberValue;
        ch = returnInteger(filePtr, ch, &numberValue);

        snprintf(strValue, sizeof(strValue), "%d", numberValue); // integer to string
    } else if (strcmp(funcName, keywords[4]) == 0) { // insert
        char myTextIdentifier[31], myText[10000], insertText[10000];
        int identifierIndex = -1;
        if (ch == '"')
            ch = returnString(filePtr, ch, myText);
        else {
            ch = parseStrToken(filePtr, ch, myTextIdentifier);

            identifierIndex = isTextIdent(myTextIdentifier);
            if (identifierIndex < 0)
                closeProgram(-1, "Invalid identifier!");
            strcpy(myText, txtValues[identifierIndex]);
        }
        if (ch != ',')
            closeProgram(-1, "Invalid 'insert' function usage!");

        ch = fgetc(filePtr);
        ch = space(1, filePtr, ch);

        int location;
        ch = returnInteger(filePtr, ch, &location);
        if (location > strlen(myText))
            closeProgram(-1, "Invalid 'location' integer value!");

        if (ch != ',')
            closeProgram(-1, "Invalid 'insert' function usage!");

        ch = fgetc(filePtr);
        ch = space(1, filePtr, ch);

        ch = returnString(filePtr, ch, insertText);

        int textLen = strlen(myText), insertTextLen = strlen(insertText);
        char temp[textLen + insertTextLen + 1];

        strncpy(temp, myText, location);
        temp[location] = '\0';
        strcat(temp, insertText);
        strcat(temp, myText + location);
        strcpy(strValue, temp);

        if (identifierIndex != -1)
            strcpy(txtValues[identifierIndex], temp);
    } else if (strcmp(funcName, keywords[9]) == 0) { // override
        char myTextIdentifier[31], myText[10000], overText[10000];
        int identifierIndex = -1;
        if (ch == '"')
            ch = returnString(filePtr, ch, myText);
        else {
            ch = parseStrToken(filePtr, ch, myTextIdentifier);

            identifierIndex = isTextIdent(myTextIdentifier);
            if (identifierIndex < 0)
                closeProgram(-1, "Invalid identifier!");
            strcpy(myText, txtValues[identifierIndex]);
        }
        if (ch != ',')
            closeProgram(-1, "Invalid 'override' function usage!");

        ch = fgetc(filePtr);
        ch = space(1, filePtr, ch);

        int location;
        ch = returnInteger(filePtr, ch, &location);
        if (location >= strlen(myText))
            closeProgram(-1, "Invalid 'location' integer value!");

        if (ch != ',')
            closeProgram(-1, "Invalid 'override' function usage!");

        ch = fgetc(filePtr);
        ch = space(1, filePtr, ch);

        ch = returnString(filePtr, ch, overText);
        override(myText, location, overText);
        strcpy(strValue, myText);

        if (identifierIndex != -1)
            strcpy(txtValues[identifierIndex], strValue);
    } else if (strcmp(funcName, keywords[12]) == 0) { // subs
        char text[10000];
        ch = returnString(filePtr, ch, text);
        if (ch != ',')
            closeProgram(-1, "Invalid function usage!");

        ch = fgetc(filePtr);
        ch = space(1, filePtr, ch);

        int begin, end;
        ch = returnInteger(filePtr, ch, &begin);
        if (begin > strlen(text))
            closeProgram(-1, "Invalid integer value!");

        if (ch != ',')
            closeProgram(-1, "Invalid function usage!");

        ch = fgetc(filePtr);
        ch = space(1, filePtr, ch);

        ch = returnInteger(filePtr, ch, &end);
        if (end < begin || end > strlen(text))
            closeProgram(-1, "Invalid integer value!");

        int len = end - begin + 1;
        strncpy(strValue, text + begin, len - 1);
        strValue[len - 1] = '\0';
    }

    if (ch != ')')
        closeProgram(-1, "Syntax error!");
    ch = fgetc(filePtr);
    return ch;
}

char IntFunction(FILE *filePtr, char ch, char *funcName, int *numberValue) { // run integer-return function
    ch = space(1, filePtr, ch);
    if (ch != '(')
        closeProgram(-1, "Syntax error!");
    ch = fgetc(filePtr);
    ch = space(1, filePtr, ch);

    if (strcmp(funcName, keywords[1]) == 0) { // asText
        if (ch == '"') {
            ch = fgetc(filePtr);
            ch = returnInteger(filePtr, ch, numberValue);
            if (ch != '"')
                closeProgram(-1, "String not converted to integer!");
            ch = fgetc(filePtr);
        } else
            ch = returnInteger(filePtr, ch, numberValue);
    } else if (strcmp(funcName, keywords[6]) == 0) { // locate
        char mainText[10000], subText[10000];
        int start;

        ch = returnString(filePtr, ch, mainText);
        if (ch != ',')
            closeProgram(-1, "Invalid function usage!");

        ch = fgetc(filePtr);
        ch = space(1, filePtr, ch);

        ch = returnString(filePtr, ch, subText);
        if (ch != ',')
            closeProgram(-1, "Invalid function usage!");

        ch = fgetc(filePtr);
        ch = space(1, filePtr, ch);

        ch = returnInteger(filePtr, ch, &start);

        *numberValue = locate(mainText, subText, start);
    } else if (strcmp(funcName, keywords[11]) == 0) { // size
        char text[10000];
        ch = returnString(filePtr, ch, text);
        *numberValue = strlen(text);
    }

    ch = space(1, filePtr, ch);
    if (ch != ')')
        closeProgram(-1, "Syntax error!");
    ch = fgetc(filePtr);
    return ch;
}

char runCommand(FILE *filePtr, char ch, char *token) { // run command-keywords
    if (strcmp(token, keywords[3]) == 0) { // input
        char identifier[31];
        ch = parseStrToken(filePtr, ch, identifier);

        if (isSpace(ch) || ch == '/')
            ch = space(1, filePtr, ch);

        int identifierIndex = isTextIdent(identifier);

        if (identifierIndex < 0)
            closeProgram(-1, "Invalid identifier!");

        char promptKeyword[31];
        ch = parseStrToken(filePtr, ch, promptKeyword);

        if (strcmp(promptKeyword, "prompt") == 0) {
            if (isSpace(ch) || ch == '/')
                ch = space(1, filePtr, ch);

            char promptText[101];
            char text[101];

            if (ch == ';') {
                gets(text);
                if (strlen(text) > 100)
                    closeProgram(-1, "Text longer than 100 chars!");
                strcpy(txtValues[identifierIndex], text);
            } else if (ch == '"') {
                ch = parseString(filePtr, ch, promptText);

                printf("%s", promptText);
                gets(text);
                if (strlen(text) > 100)
                    closeProgram(-1, "Text longer than 100 chars!");
                strcpy(txtValues[identifierIndex], text);
            } else if (isalpha(ch) != 0) {
                char identifier2[31];
                ch = parseStrToken(filePtr, ch, identifier2);

                int identifier2Index = isTextIdent(identifier2);

                if (identifier2Index != -1) {
                    printf("%s", txtValues[identifier2Index]);
                    gets(text);
                    if (strlen(text) > 100)
                        closeProgram(-1, "Text longer than 100 chars!");
                } else if (isIntegerIdent(identifier2) != -1)
                    closeProgram(-1, "Integer cannot assign to a text identifier!");

                else
                    closeProgram(-1, "Invalid identifier!");

                strcpy(txtValues[identifierIndex], text);
            } else
                closeProgram(-1, "Syntax error!");
        } else
            closeProgram(-1, "Invalid keyword usage!");
    } else if (strcmp(token, keywords[7]) == 0) { // new
        char dataType[31];
        ch = parseStrToken(filePtr, ch, dataType);

        if (isSpace(ch) || ch == '/')
            ch = space(1, filePtr, ch);

        if (isalpha(ch) != 0) {
            if (strcmp(dataType, "text") == 0) { //data type text
                char identifier[31];
                ch = parseStrToken(filePtr, ch, identifier);
                if (ch < 0)
                    closeProgram(-1, "Longer than 30 characters!");

                else if ((isTextIdent(identifier) != -1) || isIntegerIdent(identifier) != -1) {
                    printf("Already identifier: %s\n", identifier);
                    closeProgram(-1, "Identifier is already exist!");
                } else if ((strcmp(identifier, "text") == 0) || (strcmp(identifier, "int") == 0))
                    closeProgram(-1, "Identifier name cannot be same name with a data type!");

                txtIdent[txtListLen] = (char *) malloc(31 * sizeof(char));
                strcpy(txtIdent[txtListLen++], identifier);

                txtIdent = (char **) realloc(txtIdent, (txtListLen + 1) * sizeof(char *));

                txtValues[txtValueListLen] = (char *) malloc(10000 * sizeof(char));
                strcpy(txtValues[txtValueListLen++], "");

                txtValues = (char **) realloc(txtValues, (txtValueListLen + 1) * sizeof(char *));
            } else if (strcmp(dataType, "int") == 0) { //data type integer
                char identifier[31];
                ch = parseStrToken(filePtr, ch, identifier);

                if (ch < 0)
                    closeProgram(-1, "Identifier longer than 30 chars!");

                if ((isTextIdent(identifier) != -1) || isIntegerIdent(identifier) != -1) {
                    printf("Already exist: %s\n", identifier);
                    closeProgram(-1, "Identifier is already exist!");
                }

                intIdent[intListLen] = (char *) malloc(31 * sizeof(char));
                strcpy(intIdent[intListLen++], identifier);

                intIdent = (char **) realloc(intIdent, (intListLen + 1) * sizeof(char *));

                intValues[intValueListLen++] = -1;

                intValues = (int *) realloc(intValues, (intValueListLen + 1) * sizeof(int));
            } else
                closeProgram(-1, "Invalid data type!");
        } else
            closeProgram(-1, "Syntax error!");
    } else if (strcmp(token, keywords[8]) == 0) { // output
        if (ch == '"') {
            char text[10000]; // unlimited size
            ch = parseString(filePtr, ch, text);
            printf("%s\n", text);
        } else {
            char identifier[31];
            ch = parseStrToken(filePtr, ch, identifier);

            if (isTextIdent(identifier) != -1) {
                int identifierIndex = isTextIdent(identifier);
                printf("%s\n", txtValues[identifierIndex]);
            } else if (isIntegerIdent(identifier) != -1) {
                int identifierIndex = isIntegerIdent(identifier);
                printf("%d\n", intValues[identifierIndex]);
            } else
                closeProgram(-1, "Invalid identifier or text!");
        }
    } else if (strcmp(token, keywords[10]) == 0) { // read
        char identifier[31];
        ch = parseStrToken(filePtr, ch, identifier);

        if (isSpace(ch) || ch == '/')
            ch = space(1, filePtr, ch);

        int identifierIndex = isTextIdent(identifier);

        if (identifierIndex < 0)
            closeProgram(-1, "Invalid identifier!");

        char fromKeyword[31];
        ch = parseStrToken(filePtr, ch, fromKeyword);

        if (isSpace(ch) || ch == '/')
            ch = space(1, filePtr, ch);

        if (isalpha(ch) == 0)
            closeProgram(-1, "Invalid file name!");

        if (strcmp(fromKeyword, keywords[2]) == 0) { 
            char fileName[10000]; 
            ch = parseStrToken(filePtr, ch, fileName);

            strcat(fileName, ".txt");

            readFilePtr = fopen(fileName, "r");

            if (readFilePtr == NULL) {
                printf("File '%s' not found or file error!\n", fileName);
                closeProgram(-1, "Cannot open the file!");
            }

            char readCh, textFile[10000]; 
            int size = 0;
            readCh = fgetc(readFilePtr);

            while (readCh != EOF) {
                textFile[size++] = readCh;
                readCh = fgetc(readFilePtr);
            }
            textFile[size] = '\0';
            strcpy(txtValues[identifierIndex], textFile);
            fclose(readFilePtr);
        } else
            closeProgram(-1, "Invalid keyword usage!");
    } else if (strcmp(token, keywords[15]) == 0) { // write
        char printText[10000];

        if (ch == '"') {
            ch = parseString(filePtr, ch, printText);

            if (isSpace(ch) || ch == '/')
                ch = space(1, filePtr, ch);
        } else {
            char identifier[31];
            ch = parseStrToken(filePtr, ch, identifier);

            if (isSpace(ch) || ch == '/')
                ch = space(1, filePtr, ch);

            int identifierIndex = isTextIdent(identifier);

            if (identifierIndex < 0)
                closeProgram(-1, "Invalid identifier!");
            strcpy(printText, txtValues[identifierIndex]);
        }
        char toKeyword[31];
        ch = parseStrToken(filePtr, ch, toKeyword);

        if (isSpace(ch) || ch == '/')
            ch = space(1, filePtr, ch);

        if (isalpha(ch) == 0)
            closeProgram(-1, "Invalid file name!");

        if (strcmp(toKeyword, keywords[14]) == 0) { 
            char fileName[10000]; 
            ch = parseStrToken(filePtr, ch, fileName);
            strcat(fileName, ".txt");

            writeFilePtr = fopen(fileName, "w");

            if (writeFilePtr == NULL)
                closeProgram(-1, "File error!");

            fprintf(writeFilePtr, "%s", printText);
            fclose(writeFilePtr);
        } else
            closeProgram(-1, "Invalid keyword!");
    } else
        closeProgram(-1, "Invalid keyword!");

    if (isSpace(ch) || ch == '/')
        ch = space(1, filePtr, ch);

    return ch;
}

char parseString(FILE *filePtr, char ch, char *str) { // string read
    int len = 0, i = 0;
    ch = fgetc(filePtr);

    while (str[i] != '\0') // clean old string value
        str[i++] = '\0';

    while (ch != EOF && ch != '"') {
        if (ch == '\n')
            line++;

        str[len++] = ch;
        ch = fgetc(filePtr);
    }
    if (ch == EOF)
        closeProgram(-1, "End of file before string ends!");

    str[len] = '\0';
    ch = fgetc(filePtr);
    return ch;
}

char returnString(FILE *filePtr, char ch, char *strValue) { 
    if (isalpha(ch) != 0) {
        char token[31];
        ch = parseStrToken(filePtr, ch, token);

        int identifierIndex = isTextIdent(token);

        if (identifierIndex != -1)
            strcpy(strValue, txtValues[identifierIndex]);
        else if (stringFunction(token))
            ch = StringFunction(filePtr, ch, token, strValue);
        else
            closeProgram(-1, "Invalid assignment!");
    } else if (ch == '"')
        ch = parseString(filePtr, ch, strValue);
    else
        closeProgram(-1, "Invalid assignment!");

    ch = space(1, readFilePtr, ch);
    return ch;
}

char returnInteger(FILE *filePtr, char ch, int *intValue) { 
    if (isalnum(ch)) {
        if (isalpha(ch) != 0) {
            char token[31];
            ch = parseStrToken(filePtr, ch, token);

            int identifierIndex = isIntegerIdent(token);

            if (identifierIndex != -1)
                *intValue = intValues[identifierIndex];
            else if (intFunction(token))
                ch = IntFunction(filePtr, ch, token, intValue);
            else
                closeProgram(-1, "Invalid assignment!");
        } else
            ch = parseIntToken(filePtr, ch, intValue);
    } else
        closeProgram(-1, "Invalid assignment!");

    ch = space(1, readFilePtr, ch);
    return ch;
}

bool stringFunction(char *funcName) { 
    bool control = (strcmp(funcName, keywords[0]) == 0) ||
                 (strcmp(funcName, keywords[4]) == 0) ||
                 (strcmp(funcName, keywords[9]) == 0) ||
                 (strcmp(funcName, keywords[12]) == 0);
    return control;
}

bool intFunction(char *funcName) { 
    bool control = (strcmp(funcName, keywords[1]) == 0) ||
                 (strcmp(funcName, keywords[6]) == 0) ||
                 (strcmp(funcName, keywords[11]) == 0);
    return control;
}

char *subtractStrings(char *str1, char *str2) { // str subtraction
    int len1 = strlen(str1);
    int len2 = strlen(str2);
    int resultLen = len1 - len2;

    char *result = (char *) malloc((resultLen + 1) * sizeof(char));

    int i = 0;
    int j = 0;
    int k = 0;

    while (str1[i] != '\0') {
        if (str1[i] == str2[j]) {
            i++;
            j++;
        } else {
            result[k] = str1[i];
            i++;
            k++;
        }
    }
    result[k] = '\0';
    return result;
}

void override(char *myText, int location, char *overText) { 
    int textLen = strlen(myText);
    int ovrTextLen = strlen(overText);

    if (location >= textLen)
        return;
    for (int i = 0; i < ovrTextLen; i++) {
        if (location + i >= textLen)
            break;
        myText[location + i] = overText[i];
    } 
}

int locate(char *mainText, char *subText, int start) { 
    int bigTextLen = strlen(mainText);
    int smallTextLen = strlen(subText);

    if (start >= bigTextLen || smallTextLen == 0)
        return 0;

    for (int i = start; i <= bigTextLen - smallTextLen; i++) {
        int j;
        for (j = 0; j < smallTextLen; j++) {
            if (mainText[i + j] != subText[j])
                break;
        }
        if (j == smallTextLen)
            return i;
    }
    return 0;
}



int closeProgram(int code, char *errorMessage) { // prints error and exit
    printf("ERROR: %s (at line %d of your TextJedi code)", errorMessage, line);
    freeMemory();
    fclose(readFilePtr);
    exit(code);
}

void freeMemory() {
    for (int i = 0; i < txtListLen; i++)
        free(txtIdent[i]);

    for (int i = 0; i < txtValueListLen; i++)
        free(txtValues[i]);

    for (int i = 0; i < intListLen; i++)
        free(intIdent[i]);

    free(txtIdent);
    free(txtValues);
    free(intIdent);
    free(intValues);
}




int main(int argc, char *argv[]) {
    char ch;

    txtIdent = (char **) malloc(sizeof(char *) * 1); // text identifier names list for control
    intIdent = (char **) malloc(sizeof(char *) * 1); // integer identifier names list for control

    txtValues = (char **) malloc(sizeof(char *) * 1); // text identifiers' values list
    intValues = (int *) malloc(sizeof(int) * 1); // integer identifiers' values list

    if ((txtIdent == NULL || intIdent == NULL) || (txtValues == NULL || intValues == NULL)) {
        printf("Memory allocation failed!");
        exit(1);
    }

    if (argc < 2) { // getting file name from cmd
        printf("You should enter file name from command prompt!\nExample: TextJedi myProg.tj\n");
        return -1;
    }

    char *fileName = argv[1];

    readFilePtr = fopen(fileName, "r");

    if (readFilePtr == NULL) {
        printf("File '%s' not found!\n", fileName);
        closeProgram(-1, "Could not open the file!");
    }

    do {
        ch = fgetc(readFilePtr);
        ch = space(0, readFilePtr, ch);

        if (isalpha(ch) != 0) { // ch is an alph character
            char token[31];

            ch = parseStrToken(readFilePtr, ch, token); //parsing

            if (ch < 0)
                closeProgram(-1, "Identifier or keyword cannot longer than 30 characters!");

            if (isKeyword(token)) { // keyword or not
                if (stringFunction(token) || intFunction(token))
                    closeProgram(-1, "Non-void function cannot be used without expression!");

                if (isSpace(ch) || ch == '/') {
                    ch = space(1, readFilePtr, ch);
                    
                    ch = keywordToken(readFilePtr, ch, token);
                } else
                    closeProgram(-1, "Syntax error!");

                ch = space(1, readFilePtr, ch);

                if (ch == ';')
                    continue;
                closeProgram(-1, "Missing ';' !");
            } else if (isTextIdent(token) != -1) { // text identifier
                int prevIdentIndex = isTextIdent(token);
                ch = space(1, readFilePtr, ch);

                if (ch == ':') {
                    ch = fgetc(readFilePtr);

                    if (ch == '=') {
                        ch = fgetc(readFilePtr);
                        ch = space(1, readFilePtr, ch);

                        char text[10000]; 
                        ch = returnString(readFilePtr, ch, text);

                        if (ch == '+' || ch == '-') {
                            char text2[10000]; 

                            if (ch == '+') { // Addition
                                ch = fgetc(readFilePtr);
                                ch = space(1, readFilePtr, ch);

                                ch = returnString(readFilePtr, ch, text2);
                                strcpy(txtValues[prevIdentIndex], text);
                                strcat(txtValues[prevIdentIndex], text2);
                            } else { // Subtraction
                                ch = fgetc(readFilePtr);
                                ch = space(1, readFilePtr, ch);

                                ch = returnString(readFilePtr, ch, text2);
                                if (strlen(text) < strlen(text2))
                                    closeProgram(-1,
                                                "Second string cannot bigger than the first string in subtraction operation!");

                                char *text3 = subtractStrings(text, text2); // text - text2
                                strcpy(txtValues[prevIdentIndex], text3);
                                free(text3);
                            }
                        } else if (ch == ';')
                            strcpy(txtValues[prevIdentIndex], text);
                        else
                            closeProgram(-1, "Invalid assignment!");
                    } else
                        closeProgram(-1, "Invalid operator ':' !");

                    if (ch == ';')
                        continue;
                    closeProgram(-1, "Missing ';' !");
                } else
                    closeProgram(-1, "Operator ':=' missing!");
            } else if (isIntegerIdent(token) != -1) { // if token is a integer identifier
                int prevIdentIndex = isIntegerIdent(token);
                ch = space(1, readFilePtr, ch);

                if (ch == ':') {
                    ch = fgetc(readFilePtr);

                    if (ch == '=') {
                        ch = fgetc(readFilePtr);
                        ch = space(1, readFilePtr, ch);

                        int number;
                        ch = returnInteger(readFilePtr, ch, &number);

                        if (ch == '+' || ch == '-') {
                            int number2, number3;

                            if (ch == '+') { // Addition
                                ch = fgetc(readFilePtr);
                                ch = space(1, readFilePtr, ch);

                                ch = returnInteger(readFilePtr, ch, &number2);
                                number3 = number + number2;
                            } else { // Subtraction
                                ch = fgetc(readFilePtr);
                                ch = space(1, readFilePtr, ch);

                                ch = returnInteger(readFilePtr, ch, &number2);
                                number3 = number - number2;
                                if (number3 < 0)
                                    closeProgram(-1, "Negative values are not allowed!");
                            }
                            intValues[prevIdentIndex] = number3;
                        } else if (ch == ';')
                            intValues[prevIdentIndex] = number;

                        else if (isSpace(ch) || ch == '/')
                            ch = space(1, readFilePtr, ch);

                        else
                            closeProgram(-1, "Invalid assignment!");
                    } else
                        closeProgram(-1, "Invalid operator ':' !");

                    if (ch == ';')
                        continue;
                    closeProgram(-1, "Missing ';'!");
                } else
                    closeProgram(-1, "':=' operator missing!");
            } else {
                char errorMessage[100] = "Unknown token !";
                strcat(errorMessage, token);
                strcat(errorMessage, "' !");
                closeProgram(-1, errorMessage);
            }
        } else if (isdigit(ch)) { // if ch is a number
            closeProgram(-1, "Invalid usage of number!");
        } else if (ch == ';')
            continue;

        else if (isSpace(ch) || ch == '/')
            ch = space(0, readFilePtr, ch);

        else if (ch == '+')
            closeProgram(-1, "Choose any operation before using '+' operator!");

        else if (ch == '-')
            closeProgram(-1, "Choose any operation before using '-' operator!");

        else if (ch == ':') {
            ch = fgetc(readFilePtr);

            if (ch == '=')
                closeProgram(-1, "Choose operation before using ':=' operator!");
            else
                closeProgram(-1, "Invalid operator ':' !");
        } else if (ch == EOF)
            break;

        else {
            printf("Unknown character found '%c' !\n", ch);
            closeProgram(-1, "Invalid character!");
        }

    } while (ch != EOF);

    printf("Operations completed successfully!");
    freeMemory();
    fclose(readFilePtr);
    return 0;
}