#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "tree.h"

int main(void)
{
    FSNode* root = FSNewRoot();
    if (!root) {
        fprintf(stderr, "Failed to create root\n");
        return 1;
    }

    FSNode* n = FSMkdir(root, "/dev/disk");
    if (!n) { fprintf(stderr, "FSMkdir failed\n"); FSFree(root); return 1; }
    n = FSMkdir(root, "/sys");
    if (!n) { fprintf(stderr, "FSMkdir failed\n"); FSFree(root); return 1; }
    n = FSMkdir(root, "/etc");
    if (!n) { fprintf(stderr, "FSMkdir failed\n"); FSFree(root); return 1; }
    n = FSMkdir(root, "/home/user/crab");
    if (!n) { fprintf(stderr, "FSMkdir failed\n"); FSFree(root); return 1; }
    FSNode* pass = FSCreateFile(root, "/etc/passwd", "root:x:0:0:root:/root:/bin/bash\nuser:x:1000:1000::/home/user:/bin/bash\n");
    if (!pass) { fprintf(stderr, "FSCreateFile failed\n"); FSFree(root); return 1; }

    printf("Initial tree:\n");
    FSPrintTree(root);
    printf("\n");

    char* choice = malloc(sizeof(char) * 8);
    char* path = malloc(sizeof(char) * 16);
    char* def = malloc(sizeof(char) * 16);
    def = "/";
    FSNode* cur = root;

    while (1) {
        printf("%s > ", def);
        memset(choice, 0, 6);
        memset(path, 0, 16);
        // scanf("%s %s", choice, path);
        scanf("%s", choice);

        if (strncmp(choice, "q", 1) == 0) {
            free(choice);
            free(path);
            free(def);
            FSFree(root);
            exit(0);
        } else {
            scanf(" %s", path);

            if (strncmp(choice, "cd", 2) == 0) {

                cur = FSFind(root, path);
                if (cur) def = path;
                else printf("cd: %s not a valid path\n", path);

            } else if (strncmp(choice, "ls", 2) == 0) {

                printf("[%s]\n", path);
                if (strcmp(path, ".") == 0) {
                    PrintLS(cur);
                } else {
                    FSNode* tols = FSFind(root, path);
                    if (tols) PrintLS(tols);
                    else printf("invalid path\n");
                }

            } else if (strncmp(choice, "mkdir", 5) == 0) {

                n = FSMkdir(root, path);

            } else if (strncmp(choice, "touch", 5) == 0) {

                n = FSCreateFile(root, path, "Placeholder text\n");

            } else if (strncmp(choice, "cat", 3) == 0) {

                const char* content = FSGetFileContent(root, path);
                if (content) printf("%s:\n%s\n", path, content);
                else fprintf(stderr, "failed to read %s\n", path);

            } else if (strncmp(choice, "rm", 2) == 0) {

                int ret = FSRemove(root, path);
                if (ret != 0) printf("error: could not remove %s\n", path);
                else printf("removed %s\n", path);

            } else {
                printf("invalid choice\n");
            }
        }
    }


    free(choice);
    free(path);
    free(def);
    FSFree(root);
    return 0;
}

// int main(void)
// {
//     FSNode* root = FSNewRoot();
//     if (!root) {
//         fprintf(stderr, "Failed to create root\n");
//         return 1;
//     }
//
//     FSNode* n = FSMkdir(root, "/dev/disk");
//     if (!n) { fprintf(stderr, "FSMkdir failed\n"); FSFree(root); return 1; }
//     n = FSMkdir(root, "/dev/usb");
//     if (!n) { fprintf(stderr, "FSMkdir failed\n"); FSFree(root); return 1; }
//     n = FSMkdir(root, "/etc");
//     if (!n) { fprintf(stderr, "FSMkdir failed\n"); FSFree(root); return 1; }
//     n = FSMkdir(root, "/sys/kernel");
//     if (!n) { fprintf(stderr, "FSMkdir failed\n"); FSFree(root); return 1; }
//     n = FSMkdir(root, "/home/user/crab");
//     if (!n) { fprintf(stderr, "FSMkdir failed\n"); FSFree(root); return 1; }
//     n = FSMkdir(root, "/home/user/guest");
//     if (!n) { fprintf(stderr, "FSMkdir failed\n"); FSFree(root); return 1; }
//     n = FSMkdir(root, "/home/user/felix");
//     if (!n) { fprintf(stderr, "FSMkdir failed\n"); FSFree(root); return 1; }
//     FSNode* pass = FSCreateFile(root, "/etc/passwd", "root:x:0:0:root:/root:/bin/bash\nuser:x:1000:1000::/home/user:/bin/bash\n");
//     if (!pass) { fprintf(stderr, "FSCreateFile failed\n"); FSFree(root); return 1; }
//     FSNode* notes = FSCreateFile(root, "/home/user/crab/notes.txt", "Remember: buy milk\n");
//     if (!notes) { fprintf(stderr, "FSCreateFile failed\n"); FSFree(root); return 1; }
//     FSNode* log = FSCreateFile(root, "/sys/kernel/log.txt", "Nov 12 17:00 example: started\n");
//     if (!log) { fprintf(stderr, "FSCreateFile failed\n"); FSFree(root); return 1; }
//     FSNode* usb = FSCreateFile(root, "/dev/usb/sandisk.txt", "List of things from a usb drive\n");
//     if (!usb) { fprintf(stderr, "FSCreateFile failed\n"); FSFree(root); return 1; }
//
//     printf("Initial tree:\n");
//     FSPrintTree(root);
//     printf("\n");
//
//     const char* passwd = FSGetFileContent(root, "/etc/passwd");
//     if (passwd) {
//         printf("/etc/passwd content:\n%s\n", passwd);
//     } else {
//         fprintf(stderr, "failed to read /etc/passwd\n");
//     }
//
//     if (FSMove(root, "/dev/usb/sandisk.txt", "/home/user/guest") == 0) {
//         printf("Moved /dev/usb/sandisk.txt to /home/user/guest\n");
//     } else {
//         printf("Failed to move sandisk.txt\n");
//     }
//
//     if (FSRemove(root, "/dev/usb") == 0) {
//         printf("Removed /dev/usb\n");
//     } else {
//         printf("Failed to remove /dev/usb\n");
//     }
//
//     printf("\nTree after move and remove:\n");
//     FSPrintTree(root);
//     printf("\n");
//
//     FSFree(root);
//     return 0;
// }
