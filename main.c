#include <stdio.h>
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

    enum { CHOICE_SIZE = 16, PATH_SIZE = 256 };
    char choice[CHOICE_SIZE];
    char path[PATH_SIZE];
    char def[PATH_SIZE];
    strcpy(def, "/");
    FSNode* cur = root;
    while (1) {
        printf("%s > ", def);
        if (scanf("%15s", choice) != 1) {
            break;
        }
        if (strncmp(choice, "q", 1) == 0) {
            break;
        } else {
            if (scanf(" %255s", path) != 1) {
                path[0] = '\0';
            }
            if (strncmp(choice, "cd", 2) == 0) {
                FSNode* target = FSFind(root, path);
                if (target) {
                    cur = target;
                    strncpy(def, path, PATH_SIZE - 1);
                    def[PATH_SIZE - 1] = '\0';
                } else {
                    printf("cd: %s isnt a valid path\n", path);
                }
            } else if (strncmp(choice, "ls", 2) == 0) {
                if (path[0] == '\0' || strcmp(path, ".") == 0) {
                    PrintLS(cur);
                } else {
                    FSNode* tols = FSFind(root, path);
                    if (tols) PrintLS(tols);
                    else printf("invalid path\n");
                }

            } else if (strncmp(choice, "mkdir", 5) == 0) {

                n = FSMkdir(root, path);
                if (!n) fprintf(stderr, "FSMkdir failed for %s\n", path);

            } else if (strncmp(choice, "touch", 5) == 0) {

                n = FSCreateFile(root, path, "Placeholder text\n");
                if (!n) fprintf(stderr, "FSCreateFile failed for %s\n", path);

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

    FSFree(root);
    return 0;
}
