#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "cJSON/cJSON.h"

#define MAX_SERVERS 10

;

typedef struct
{
    char agent[50];
    char url[100];
} ServerInfo;

int main()
{
    const char *fileName = "serversTest.json";
    time_t timestamp = time(NULL);

    FILE *jsonFile = fopen(fileName, "r");
    if (NULL == jsonFile)
    {
        printf("File not Found\n");
        return 1;
    }
    printf("%s", ctime(&timestamp));
    printf("file %s opened [OK]\n", fileName);

    // Calcul de la taille du fichier
    fseek(jsonFile, 0, SEEK_END);    // le curseur parcours le fichier de 0 à la fin.
    long fileSize = ftell(jsonFile); // j'initialise la variable fileSize avec le nombre d'octets calculés avec fseek()
    printf("File size = %ld octets\n", fileSize);
    fseek(jsonFile, 0, SEEK_SET); // remets le curseur au début du fichier.

    // j'alloue la memoire pour stocker tous les octets du fichier fileSize + 1 pour '\0'
    char *jsonBuffer = (char *)malloc(fileSize + 1);
    if (NULL == jsonBuffer)
    {
        printf("Memory allocation failure");
        fclose(jsonFile);
        return 1;
    }

    fread(jsonBuffer, 1, fileSize, jsonFile); // Je copie le contenu de jsonFile dans jsonBuffer
    fclose(jsonFile);                         // Je ferme jsonFile
    jsonBuffer[fileSize] = '\0';              // j'ajoute un caractère de fin de string à jsonBuffer

    // Je charge le JSON en tant qu'objet cJSON
    cJSON *root = cJSON_Parse(jsonBuffer);
    free(jsonBuffer);
    if (NULL == root)
    {
        const char *error_ptr = cJSON_GetErrorPtr();
        if (NULL != error_ptr)
        {
            fprintf(stderr, "Error before %s\n", error_ptr);
        }
        return 1;
    }
    // Récupération de l'objet "servers" du JSON
    cJSON *servers = cJSON_GetObjectItem(root, "servers");
    if (NULL == servers || !cJSON_IsArray(servers))
    {
        cJSON_Delete(root);
        perror("Error opening text file");
        return 1;
    }

    FILE *txtFile = fopen("newServerTxt.json", "r");
    if (NULL == txtFile)
    {
        cJSON_Delete(root);
        printf("File not found\n");
        return 1;
    }

    ServerInfo newServers[MAX_SERVERS];
    int serverCount = 0;

    while (serverCount < MAX_SERVERS && fscanf(txtFile, "%49s %99s", newServers[serverCount].agent, newServers[serverCount].url) == 2)
    {
        serverCount++;
    }
    fclose(txtFile);

    // Mise à jour des valeurs dans l'objet "servers" du JSON
    cJSON *server;
    int i = 0;
    cJSON_ArrayForEach(server, servers)
    {
        if (i < serverCount)
        {
            cJSON *agent = cJSON_GetObjectItem(server, "agent");
            cJSON *url = cJSON_GetObjectItem(server, "url");
            if (agent && cJSON_IsString(agent))
            {
                cJSON_SetValuestring(agent, newServers[i].agent);
            }
            if (url && cJSON_IsString(url))
            {
                cJSON_SetValuestring(url, newServers[i].url);
            }
        }
        i++;
    }

    // Écriture du JSON modifié dans un nouveau fichier
    char *modifiedJsonStr = cJSON_Print(root);
    if (modifiedJsonStr == NULL)
    {
        cJSON_Delete(root);
        fprintf(stderr, "Error generating JSON string\n");
        return 1;
    }

    FILE *outputFile = fopen("output.json", "w");
    if (outputFile == NULL)
    {
        cJSON_Delete(root);
        perror("Error opening output file");
        free(modifiedJsonStr);
        return 1;
    }

    fputs(modifiedJsonStr, outputFile);
    fclose(outputFile);
    free(modifiedJsonStr);

    // Libération de la mémoire utilisée par cJSON
    cJSON_Delete(root);

    printf("JSON file has been updated and saved to 'output.json'\n");

    return 0;
}
