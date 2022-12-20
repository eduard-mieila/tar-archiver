/*
MIEILA Eduard-Robert
313CA
Tema 3
Programarea Calculatoarelor(Seria CA)
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <time.h>
#include <math.h>


#define FN_LIMIT 100
#define CN_LIMIT 8


int commandChoice();
int DtoO();
int getPerm();
unsigned int UItoO();
unsigned int OtoUI();
unsigned long LUtoO();
FILE* readFileData();
void transformDate();
void transformTime();
void getUIDGID();
void readCommand();
void listArchive();
void extractFile();
void createArchive();


typedef union record {
    char charptr[512];
    struct header {
        char name[100];
        char mode[8];
        char uid[8];
        char gid[8];
        char size[12];
        char mtime[12];
        char chksum[8];
        char typeflag;
        char linkname[100];
        char magic[8];
        char uname[32];
        char gname[32];
        char devmajor[8];
        char devminor[8];
    } header;
} dataRecordType;


int DtoO(int decimal) {
    int octal = 0, temp = 1;

    while (decimal) {
        octal = octal + (decimal % 8) * temp;
        decimal /= 8;
        temp *= 10;
    }

    return octal;
}

unsigned long LUtoO(unsigned long LUdecimal) {
    unsigned long octal = 0, temp = 1, decimal;

    decimal = (int)LUdecimal;
    while (decimal) {
        octal = octal + (decimal % 8) * temp;
        decimal /= 8;
        temp *= 10;
    }

    return octal;
}

unsigned int UItoO(unsigned int UIdecimal) {
    unsigned int octal = 0, temp = 1, decimal;

    decimal = (int)UIdecimal;
    while (decimal) {
        octal = octal + (decimal % 8) * temp;
        decimal /= 8;
        temp *= 10;
    }

    return octal;
}

unsigned int OtoUI(unsigned int octal) {
    unsigned int decimal = 0, pos = 0;
    while (octal) {
        decimal += (octal % 10) * pow(8, pos);
        pos++;
        octal /= 10;
    }
    return decimal;
}

int getPerm(char permissions[]) {
    int user = 0, group = 0, others = 0;
    if (permissions[1] == 'r') {
        user += 4;
    }

    if (permissions[2] == 'w') {
        user += 2;
    }

    if (permissions[3] == 'x') {
        user += 1;
    }

    if (permissions[4] == 'r') {
        group += 4;
    }

    if (permissions[5] == 'w') {
        group += 2;
    }

    if (permissions[6] == 'x') {
        group += 1;
    }

    if (permissions[7] == 'r') {
        others += 4;
    }

    if (permissions[8] == 'w') {
        others += 2;
    }

    if (permissions[9] == 'x') {
        others += 1;
    }

    return user * 100 + group * 10 + others;
}

void transformDate(char date[], int *year, int *month, int *day) {
    char *p;
    int counter = 0;

    p = strtok(date, "-");
    while (p != NULL) {
        if (counter == 0) {
            *year = atoi(p) - 1900;
        }

        if (counter == 1) {
            *month = atoi(p) - 1;
        }

        if (counter == 2) {
            *day = atoi(p);
        }

        counter++;
        p = strtok(NULL, "-");
    }
}

void transformTime(char time[], int *hour, int *minutes, int *seconds) {
    char *p;
    int counter = 0;

    p = strtok(time, ":");
    while (p != NULL) {
        if (counter == 0) {
            *hour = atoi(p);
        }

        if (counter == 1) {
            *minutes = atoi(p);
        }

        if (counter == 2) {
            *seconds = atoi(p);
        }

        counter++;
        p = strtok(NULL, ":");
    }
}

void getUIDGID(dataRecordType *dataBlock, FILE *userMap) {
    size_t len = 0;
    ssize_t readLine;
    char *line = NULL;
    int uidProv, gidProv, unit;

    // UID & GID
        while ((readLine = getline(&line, &len, userMap)) != -1) {
            if (strstr(line, dataBlock->header.uname) != NULL) {
                char *p = strtok(line, ":");
                unit = 0;
                while (unit < 4) {
                    if (unit == 2) {
                        uidProv = atoi(p);
                    }

                    if (unit == 3) {
                        gidProv = atoi(p);
                    }

                    p = strtok(NULL, ":");
                    unit++;
                }
            }
        }
        free(line);

        uidProv = DtoO(uidProv);
        gidProv = DtoO(gidProv);
        strcpy(dataBlock->header.uid, "0000000");
        strcpy(dataBlock->header.gid, "0000000");

        unit = 6;
        while (uidProv) {
            dataBlock->header.uid[unit] = 48 + uidProv % 10;
            uidProv /= 10;
            unit--;
        }
        dataBlock->header.uid[7] = 0;

        unit = 6;
        while (gidProv) {
            dataBlock->header.gid[unit] = 48 + gidProv % 10;
            gidProv /= 10;
            unit--;
        }
        dataBlock->header.gid[7] = 0;
        // UID & GID END
}

FILE* readFileData(FILE *filesMap, FILE *userMap,  dataRecordType *dataBlock) {
    char permissions[9], date[19], timeM[18], *p;
    int trash, size, unit;

        // Mode
        strcpy(dataBlock->header.mode, "0000000");
        fscanf(filesMap, "%s", permissions);
        dataBlock->header.mode[7] = 0;
        dataBlock->header.mode[6] = 48 + getPerm(permissions) % 10;
        dataBlock->header.mode[5] = 48 + (getPerm(permissions) / 10) % 10;
        dataBlock->header.mode[4] = 48 + getPerm(permissions) / 100;
        // Mode end

        fscanf(filesMap, "%d", &trash);                     // useless info
        fscanf(filesMap, "%s", dataBlock->header.uname);    // Uname
        fscanf(filesMap, "%s", dataBlock->header.gname);    // Gname

        getUIDGID(dataBlock, userMap);    // UID&GID

        // Size
        fscanf(filesMap, "%d", &size);
        size = DtoO(size);
        strcpy(dataBlock->header.size, "00000000000");
        unit = 10;
        while (size) {
            dataBlock->header.size[unit] = 48 + size % 10;
            size /= 10;
            unit--;
        }
        dataBlock->header.size[11] = 0;
        // Size end

        // Mtime
        fscanf(filesMap, "%s", date);
        fscanf(filesMap, "%s", timeM);
        struct tm dateModified;
        dateModified.tm_isdst = 0;
        transformDate(date, &dateModified.tm_year,
            &dateModified.tm_mon, &dateModified.tm_mday);
        strtok(timeM, ".");            // eliminam ce este dupa .(punct)
        transformTime(timeM, &dateModified.tm_hour,
            &dateModified.tm_min, &dateModified.tm_sec);
        unsigned long mtime = LUtoO(mktime(&dateModified));
        unit = 10;
        while (mtime) {
            dataBlock->header.mtime[unit] = 48 + mtime % 10;
            mtime /= 10;
            unit--;
        }
        dataBlock->header.mtime[11] = 0;
        // Mtime end

        fscanf(filesMap, "%d", &trash);    // useless info
        fscanf(filesMap, "%s", dataBlock->header.name);    // Name
        strcpy(dataBlock->header.linkname, dataBlock->header.name); // Linkname
        strcpy(dataBlock->header.magic, "GNUtar ");    // Magic

        // chksum
        strcpy(dataBlock->header.chksum, "        ");
        unsigned int sum = 0;
        p = (char*)dataBlock;
        for (int i = 0; i < 512; i++) {
            sum += (int)*(p+i);
        }
        sum = UItoO(sum);
        unit = 6;
        while (sum) {
            dataBlock->header.chksum[unit] = 48 + sum % 10;
            sum /= 10;
            unit--;
        }
        while (unit != -1) {
            dataBlock->header.chksum[unit] = '0';
            unit--;
        }
        dataBlock->header.chksum[7] = ' ';
        // chksum END

    return filesMap;
}

void readCommand(char command[], char param1[], char param2[]) {
    char line[511], *p;
    int unit;
    fgets(line, 510, stdin);
    line[strlen(line) - 1] = 0;
    p = strtok(line, " ");
    unit = 0;
    while (unit < 3 && p != NULL) {
        if (unit == 0) {
            strcpy(command, p);
        }
        if (unit == 1) {
            strcpy(param1, p);
        }
        if (unit == 2) {
            strcpy(param2, p);
        }
        unit++;
        p = strtok(NULL, " ");
    }
}

int commandChoice(char command[]) {
    if (!(strcmp(command, "create"))) {
        return 1;
    }

    if (!(strcmp(command, "list"))) {
        return 2;
    }

    if (!(strcmp(command, "extract"))) {
        return 3;
    }

    if (!(strcmp(command, "exit "))) {
        return 0;
    }

    return -1;
}

void createArchive(char archiveName[], char folder[]) {
    FILE *filesMap, *outputFile, *userMap, *currentFile;
    dataRecordType *dataBlock;
    int length;
    char fileName[100], pathToFile[200];

    filesMap = fopen("files.txt", "rt");
    if (filesMap == NULL) {
        printf("> Failed!\n");
        return;
    }
    outputFile = fopen(archiveName, "wb");
    if (outputFile == NULL) {
        printf("> Failed!\n");
        return;
    }

    // Determinam lungimea fisierului files.txt
    fseek(filesMap, 0, SEEK_END);
    length = ftell(filesMap) - 1;
    fseek(filesMap, 0, SEEK_SET);

    while (ftell(filesMap) < length) {
        // Vom extrage datele necesare pentru header
        if (!feof(filesMap)) {
            userMap = fopen("usermap.txt", "rt");
            if (userMap == NULL) {
                printf("> Failed!\n");
                return;
            }
            dataBlock = calloc(1, sizeof(dataRecordType));
            filesMap = readFileData(filesMap, userMap, dataBlock);
            fclose(userMap);
        }

        // Scriem headerul in arhiva
        fwrite(dataBlock, sizeof(dataRecordType), 1, outputFile);
        // Pastram numele fisierului
        strcpy(fileName, dataBlock->header.name);
        free(dataBlock);

        // Vom determina calea relativa a fisierului ce urmeaza sa fie scris
        strcpy(pathToFile, folder);
        strcat(pathToFile, fileName);
        currentFile = fopen(pathToFile, "rb");
        if (currentFile == NULL) {
            printf("> Failed!\n");
            return;
        }

        // scriem datele fisierului in blockuri de cate 512 bytes
        fseek(currentFile, 0, SEEK_END);
        int lengthCurrentFile = ftell(currentFile) - 1;
        fseek(currentFile, 0, SEEK_SET);
        while (ftell(currentFile) < lengthCurrentFile) {
            dataBlock = calloc(1, sizeof(dataRecordType));
            for (int i = 0; i < 512; i++) {
                fread(&dataBlock->charptr[i], sizeof(char), 1, currentFile);
            }
            fwrite(dataBlock, sizeof(dataRecordType), 1, outputFile);
            free(dataBlock);
        }
        fclose(currentFile);
    }

    // La final, scriem un block plin cu 0.
    dataBlock = calloc(1, sizeof(dataRecordType));
    fwrite(dataBlock, sizeof(dataRecordType), 1, outputFile);
    free(dataBlock);
    fclose(outputFile);
    fclose(filesMap);
    printf("> Done!\n");
}

void listArchive(char archiveName[]) {
    FILE *archive = fopen(archiveName, "rb");
    dataRecordType *dataBlock;

    if (archive == NULL) {
        printf("> File not found!\n");
        return;
    }
    while (!(feof(archive))) {
        dataBlock = calloc(1, sizeof(dataRecordType));
        fread(&dataBlock->header, sizeof(dataRecordType), 1, archive);
        if (strcmp(dataBlock->header.magic, "GNUtar ") == 0) {
            printf("> %s\n", dataBlock->header.name);
        }
        free(dataBlock);
    }

    fclose(archive);
}

void extractFile(char fileName[], char archiveName[]) {
    dataRecordType *dataBlock;
    unsigned int found = 0, fileSize;
    char extractedFileName[FN_LIMIT], byte;
    strcpy(extractedFileName, "extracted_");

    FILE *archive = fopen(archiveName, "rb");
    if (archive == NULL) {
        printf("> File not found!\n");
        return;
    }

    while (!found) {
        while (!feof(archive)) {
            dataBlock = calloc(1, sizeof(dataRecordType));
            fread(&dataBlock->header, sizeof(dataRecordType), 1, archive);
            if (strcmp(dataBlock->header.name, fileName) == 0) {
                found = 1;
                fileSize = OtoUI(atoi(dataBlock->header.size));
                strcat(extractedFileName, fileName);
                FILE *extractedFile = fopen(extractedFileName, "wb");
                for (int i = 0; i < fileSize; i++) {
                    fread(&byte, 1, 1, archive);
                    fwrite(&byte, 1, 1, extractedFile);
                }
                fclose(extractedFile);
            }
            free(dataBlock);
        }
        fclose(archive);
        if (!found) {
            printf("> File not found!\n");
            return;
        }
    }
    printf("> File extracted!\n");
}

int main() {
    char command[CN_LIMIT], param1[FN_LIMIT], param2[FN_LIMIT];

    readCommand(command, param1, param2);
    while (strcmp(command, "exit")) {
        switch (commandChoice(command)) {
            case (1) :
                // Command = CREATE
                if (!(param1[0])) {
                    printf("> Wrong command!\n");
                } else if (!(param2[0])) {
                    printf("> Wrong commmand!\n");
                } else {
                    createArchive(param1, param2);
                }
            break;

            case (2) :
                // Command = LIST
                if (!param1[0]) {
                    printf("> Wrong command!\n");
                } else {
                    listArchive(param1);
                }
            break;

            case (3) :
                // Command = EXTRACT
                if (!(param1[0])) {
                    printf("> Wrong command!\n");
                } else if (!(param2[0])) {
                    printf("> Wrong commmand!\n");
                } else {
                    extractFile(param1, param2);
                }
            break;

            case (0) :
                // Command = EXIT
                return 0;
            break;

            default :
                // UNKNOWN Command
                printf("> Wrong command!\n");
        }

        // Dupa fiecare comanda vom sterge
        // continutul parametrilor si al comenzii.
        memset(command, 0, CN_LIMIT);
        memset(param1, 0, FN_LIMIT);
        memset(param2, 0, FN_LIMIT);
        readCommand(command, param1, param2);
    }
    return 0;
}
