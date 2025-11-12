#ifndef TREE_H
#define TREE_H

#include <stddef.h>

typedef enum { FS_DIR, FS_FILE } FSNodeType;
typedef struct FSNode {
    char* name;
    FSNodeType type;
    char* content;
    struct FSNode* parent;
    struct FSNode* child;
    struct FSNode* next;
} FSNode;

// PUBLIC
void FSFree(FSNode *root);
FSNode *FSNewRoot(void);
FSNode* FSFind(FSNode* root, const char* path);
FSNode* FSMkdir(FSNode* root, const char* path);
FSNode *FSCreateFile(FSNode *root, const char *path, const char *content);
int FSRemove(FSNode *root, const char *path);
int FSMove(FSNode *root, const char *src_path, const char *dest_dir_path);
const char *FSGetFileContent(FSNode *root, const char *path);
void FSPrintTree(const FSNode *node);
void PrintLS(FSNode* node);

// PRIVATE
static char* SafeStrdup(const char* str);
static FSNode* NewNode(const char* name, FSNodeType type);
static int UnlinkFromParent(FSNode* node);
static char** SplitPath(const char* path, int* count);
static void FreeParts(char** parts, int count);
static FSNode* FindChild(const FSNode* parent, const char* name);
static int AddChild(FSNode* parent, FSNode* child);
static FSNode* Traverse(FSNode* root, const char* path, int create_dirs);
static void PrintIndent(int depth);
static void PrintTreeRec(const FSNode *node, int depth);


#endif // TREE_H
